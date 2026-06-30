# Roller Coaster Simulation (C, Processes & Semaphores)

## Overview

This project simulates a roller coaster system using multiple concurrent processes and POSIX synchronization primitives. It models coordination between a dispatcher, ride cars, and visitors using shared memory and semaphores.

The main goal is correct synchronization of multiple processes without race conditions or busy waiting.

## System model

The system consists of three types of processes:

* Dispatcher
* Ride cars
* Visitors

All components run concurrently and coordinate their actions through shared memory and semaphores.

## Behavior summary

* Visitors arrive, queue, board cars, and leave after the ride
* Cars wait for dispatcher signals, board visitors up to capacity, simulate the ride, and unload
* The dispatcher controls ride scheduling and system shutdown
* Cars operate in a strict order and cannot pass each other
* The system terminates once all visitors are processed and all cars are closed

## Synchronization

The implementation uses:

* POSIX semaphores for coordination and mutual exclusion
* Shared memory for global state and counters
* Strict ordering constraints to ensure correct execution flow

Busy waiting is not used anywhere in the solution.

## Output

All processes write synchronized log messages to `proj2.out`, including lifecycle events and state transitions. A global sequence counter ensures deterministic ordering of events.

## What I learned

This project significantly improved my understanding of low-level concurrency and operating system behavior.

I gained hands-on experience with:

* Designing and coordinating multiple concurrent processes using `fork`
* Inter-process communication (IPC) using shared memory
* Synchronization using POSIX semaphores in real synchronization problems
* Avoiding race conditions and ensuring correct ordering of events
* Translating a complex specification into a working concurrent system
* Debugging timing-dependent and non-deterministic behavior
* Thinking in terms of critical sections, shared state, and process interaction

Beyond the technical implementation, this project taught me how difficult it is to reason about correctness in concurrent systems and how important careful design is before writing any code.

## Build

```bash id="x9p21m"
make
```

## Requirements

* C (gnu99)
* POSIX-compliant system (Linux recommended)
* shared memory + semaphore support

## Disclaimer

This is a student project created for educational purposes and demonstrates process synchronization techniques in C using POSIX IPC mechanisms.
