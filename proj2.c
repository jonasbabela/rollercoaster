#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/wait.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define OUTPUT_FILE "proj2.out"

typedef struct {
    int value;
    sem_t* mutex;
} Counter;

typedef struct {
    char type;
    int id;
} Process;

/*Prints error message on stderr and returns 1
@param message Error message*/
int error_raise(char *message) {
    fprintf(stderr, "Error: %s\n", message);
    return 1;
}

/*Creates and initializes semaphore in shared memory
@param value Initial value of semaphore*/
sem_t* create_shrd_sem(int value) {

    sem_t* sem;

    int shmid = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | 0666);
    sem = (sem_t *) shmat(shmid, NULL, 0);

    sem_init(sem, 1, value);

    return sem;
}

void destroy_shrd_sem(sem_t* sem) {
    sem_destroy(sem);

    shmdt(sem);
}

/*Creates shared memory for integer and sets it value
@param value Value of integer*/
int* create_shrd_int(int value) {

    int* num;

    int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    num = (int *) shmat(shmid, NULL, 0);

    *num = value;

    return num;
}

/*Creates and initializes counter in shared memory
@param init Initial value of counter*/
Counter* init_counter(int init)  {

    Counter* counter;

    int shmid = shmget(IPC_PRIVATE, sizeof(Counter), IPC_CREAT | 0666);
    counter = (Counter *) shmat(shmid, NULL, 0);

    counter->value = init;
    counter->mutex = create_shrd_sem(1);

    return counter;
}

void destroy_counter(Counter* counter) {
    sem_destroy(counter->mutex);
    shmdt(counter);
}

/*Increments counter protected by mutex
@param counter Pointer to counter*/
void inc_counter(Counter* counter)  {
    sem_wait(counter->mutex);
    counter->value++;
    sem_post(counter->mutex);
}

/*Decrements counter protected by mutex
@param counter Pointer to counter*/
void dec_counter(Counter* counter)  {
    sem_wait(counter->mutex);
    counter->value--;
    sem_post(counter->mutex);
}

/*Prints current action message to the file
@param counter Pointer to counter of all actions
@param Process Pointer to process which sends the message
@param action_str Type of action
@param file Output file*/
void print_action(Counter* counter, Process* process, char* action_str, FILE* file) {

    sem_wait(counter->mutex);

    if(process->type == 'D') fprintf(file, "%d: %c: %s\n", counter->value, process->type, action_str);
    else fprintf(file, "%d: %c %d: %s\n", counter->value, process->type, process->id, action_str);
    counter->value++;
    fflush(file);

    sem_post(counter->mutex);
}

/*Calculates how many visitors should enter the cart
@param capacity Maximal capacity allowed
@param dispatched Number of visitors already boarded
@param total Number of visitors at start*/
int get_current_capacity (int capacity, int dispatched, int total) {

    if (total-dispatched >= capacity) return capacity; //If still enough visitors, use maximal capacity
    else return total-dispatched; //Else wait for the remaining visitors

}


int main(int argc, char **argv) {

    if (argc != 7) return error_raise("Error: Invalid number of arguments");

    char *endptr;
    long int nof_carts;
    long int nof_visitors;
    long int cart_capacity;
    long int cart_travel_time;
    long int visitor_queue;
    long int cart_min_distance;

    /*Parse of arguments*/
    nof_carts = strtol(argv[1], &endptr, 10);
    if (*endptr != '\0') return error_raise("Invalid argument format");
    if (nof_carts <= 0 || nof_carts >=10) return error_raise("Invalid number of carts");

    nof_visitors = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0') return error_raise("Invalid argument format");
    if (nof_visitors <= 0 || nof_visitors >= 10000) return error_raise("Invalid number of visitors");

    cart_capacity = strtol(argv[3], &endptr, 10);
    if (*endptr != '\0') return error_raise("Invalid argument format");
    if (cart_capacity < 4 || cart_capacity > 40) return error_raise("Invalid cart capacity");

    cart_travel_time = strtol(argv[4], &endptr, 10);
    if (*endptr != '\0') return error_raise("Invalid argument format");
    if (cart_travel_time < 0 || cart_travel_time > 1000) return error_raise("Invalid cart travel time");

    visitor_queue = strtol(argv[5], &endptr, 10);
    if (*endptr != '\0') return error_raise("Invalid argument format");
    if (visitor_queue < 0 || visitor_queue > 1000) return error_raise("Invalid visitor queue time");

    cart_min_distance = strtol(argv[6], &endptr, 10);
    if (*endptr != '\0') return error_raise("Invalid argument format");
    if (cart_min_distance <= 0 || cart_min_distance > 100) return error_raise("Invalid minimal distance for cart");

    /*Counters*/
    Counter* counter = init_counter(1); //Counter of actions
    Counter* dispatched = init_counter(0); //Number of visitors dispatched

    /*Semaphores*/
    sem_t* queue_waiting = create_shrd_sem(0); //Visitor waiting in queue
    sem_t* leaving = create_shrd_sem(0); //Visitor leaving cart

    sem_t* boarding_entry = create_shrd_sem(0); //Cart entering initial station
    sem_t* on_board_queue = create_shrd_sem(0); //Passangers inside cart

    sem_t* leaving_entry = create_shrd_sem(1); //Cart entering final station

    sem_t* confirmed_onboard = create_shrd_sem(0); //Passanger confirmation of entering cart
    sem_t* confirmed_offboard = create_shrd_sem(0); //Passanger confirmation of leaving cart

    sem_t* next_cart = create_shrd_sem(1); //Dispatcher signal to let next cart on track


    int* is_closed = create_shrd_int(0); //Determines if roller coaster is open
    

    FILE* file = fopen(OUTPUT_FILE, "w");
    if (file == NULL) return error_raise("File open failed");
    
    /*DISPATCHER PROCESS*/
    pid_t id = fork();

    if (id < 0) return error_raise("Fork failed");
    if (id == 0)    {

        Process proc = {'D', 1};
        print_action(counter, &proc, "started", file);

        while (1) {
            
            sem_wait(next_cart); //Wait for signal from cart leaving initial station

            //If all visitors serviced
            if (nof_visitors == dispatched->value) {
                print_action(counter, &proc, "closing", file);
                *is_closed = 1;
                
                for (int i = 0; i < nof_carts; i ++) {
                    sem_post(boarding_entry);
                }

                break;
            }

            print_action(counter, &proc, "next cart", file);
            usleep(cart_min_distance); //Wait safe distance
            sem_post(boarding_entry); //Let next cart begin to board
        }

        exit(0);
    }


    /*CART PROCESS*/
    for(int i = 0; i < nof_carts; i ++) {
        pid_t id = fork();

        if (id < 0) return error_raise("Fork failed");
        if (id != 0) continue;

        Process proc = {'V', i+1};

        print_action(counter, &proc, "started", file); //Announce cart initialization

        while(1) {
            sem_wait(boarding_entry); //Wait for dispatcher to allow entering starting station

            //Check if attraction is closed
            sem_wait(dispatched->mutex);
            if (*is_closed) {
                sem_post(dispatched->mutex);
                break;
            }

            //Calculate amount of visitors needed to board
            int current_capacity = get_current_capacity(cart_capacity, dispatched->value, nof_visitors);
            dispatched->value += current_capacity;
            sem_post(dispatched->mutex);

            print_action(counter, &proc, "boarding started", file); //Anounce start of boarding

            //Board visitors by one until capacity is reached
            for(int j = 0; j < current_capacity; j++)  {
                sem_post(queue_waiting);
                sem_wait(confirmed_onboard);
            }

            print_action(counter, &proc, "boarding complete", file); //Announce all passangers aboard
            sem_post(next_cart); //Call dispatcher to let next cart go

            usleep(cart_travel_time); //Cart travel time - user argument

            sem_wait(leaving_entry); //Entering final station


            print_action(counter, &proc, "leaving started", file); //Leaving of passangers started

            for (int j = 0; j < current_capacity; j++) {
                sem_post(on_board_queue); //Let only passangers from on board leave
            }

            //Kick out passangers one by one
            for (int j = 0; j < current_capacity; j++) {
                sem_post(leaving); 
                sem_wait(confirmed_offboard);
            }

            print_action(counter, &proc, "leaving complete", file); //Announce empty cart

            sem_post(leaving_entry); //Move cart back in line and declare final station free
        }

        print_action(counter, &proc, "closed", file);

        exit(0);
    }

    /*VISITOR PROCESS*/
    for(int i = 0; i < nof_visitors; i ++) {
        pid_t id = fork();
        
        if (id < 0) return error_raise("Fork failed");
        if (id != 0) continue;

        Process proc = {'N', i+1};

        print_action(counter, &proc, "started", file); //Announce visitor initialization

        if (visitor_queue != 0) usleep(rand() % visitor_queue); //Time interval for visitor to queue

        print_action(counter, &proc, "queue", file); //Anounce visitor in queue
        sem_wait(queue_waiting); //Wait to board

        print_action(counter, &proc, "boarding", file); //Anounce boarding

        sem_post(confirmed_onboard); //Tell cart you are on board

        sem_wait(on_board_queue); //Cart confirms passanger on board

        sem_wait(leaving); //Wait for cart to enable leaving
        print_action(counter, &proc, "leaving", file); //Anounce leaving
        sem_post(confirmed_offboard); //Tell cart you are off board


        exit(0);
    }

    for (int i = 0; i < (nof_visitors + nof_carts + 1); i ++) {
        wait(NULL);
    }

    fclose(file);

    /*Clear shared memory*/
    destroy_shrd_sem(queue_waiting);
    destroy_shrd_sem(leaving);
    destroy_shrd_sem(boarding_entry);
    destroy_shrd_sem(on_board_queue);
    destroy_shrd_sem(leaving_entry);
    destroy_shrd_sem(confirmed_onboard);
    destroy_shrd_sem(confirmed_offboard);
    destroy_shrd_sem(next_cart);

    destroy_counter(counter);
    destroy_counter(dispatched);

    shmdt(is_closed);

    return 0;

}