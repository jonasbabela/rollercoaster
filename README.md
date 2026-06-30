Popis Úlohy (Horská dráha)
V systému máme 3 typy procesů: (0) dispečer, (1) vozík a (2) návštěvník. V rámci horské dráhy máme
několik vozíků, které jezdí v pevném pořadí po dráze (nemohou se předjet). Návštěvník přichází do
zábavního parku s cílem absolvovat jednu jízdu. Návštěvníci tvoří frontu před nástupem na atrakci. V
případě, že od odjezdu posledního vozíku uplynul bezpečný čas dá dispečer pokyn k nástupu do dalšího
vozíku. Po naplnění kapacity vozíku vyráží vozík na trať. Po dojezdu vozíku do cílové stanice všichni
bezpečně vystoupí a vozík je zařazen do fronty volných vozíků. V případě, že počet zájemců o svezení
je menší než kapacita vozíku (žádný další návštěvník již dnes neplánuje přijít) vyjíždí i částečně
zaplněný vozík, který odveze všechny zbývající návštěvníky. Dispečer pak atrakci uzavře.
Zadání je inspirováno knihou The Little Book of Semaphores.

Podrobná specifikace úlohy
Spuštění:
$ ./proj2 V N K TV TN O
• V: počet vozíků, 0<V<10
• N: počet návštěvníků, 0<N<10000
• K: kapacita vozíku, 4<=K<=40
• TV: Doba jízdy vozíku po dráze v mikrosekundách, 0<=TV<=1000
• TN: Maximální doba, než návštěvník dojde do fronty v mikrosekundách, 0<=TN<=1000
• O: Minimální časový odstup vozíků, 0<O<=100

Chybové stavy:
• Pokud některý ze vstupů nebude odpovídat očekávanému formátu nebo bude mimo povolený
rozsah, program vytiskne chybové hlášení na standardní chybový výstup, uvolní všechny dosud
alokované zdroje a ukončí se s kódem (exit code) 1.
• Pokud selže některá z operací se semafory, sdílenou pamětí, nebo volání fork, postupujte
stejně--program vytiskne chybové hlášení na standardní chybový výstup, uvolní všechny dosud
alokované zdroje a ukončí se s kódem (exit code) 1.

Implementační detaily:
• Každý proces vykonává své akce a současně zapisuje informace o akcích do souboru s názvem
proj2.out. Součástí výstupních informací o akci je pořadové číslo „A“ prováděné akce (viz
popis výstupů). Akce se číslují od jedničky.
• Celkový počet spuštěných procesů bude 2+V+N (hlavní proces, dispečer, V vozíků a N
návštěvníků). Nespouštějte žádné další pomocné procesy.
• Použijte sdílenou paměť pro implementaci čítače akcí a sdílených proměnných nutných pro
synchronizaci.
• Použijte semafory pro synchronizaci procesů.
• Nepoužívejte aktivní čekání (včetně cyklického časového uspání procesu) pro účely
synchronizace.
• Pracujte s procesy, ne s vlákny.
• Frontu na atrakci a frontu volných vozíků implementujte semafory.
• Skupina jedoucí jedním vozíkem může nastupovat a vystupovat v libovolném pořadí.
• Nástup a výstup je možný pouze v době, kdy je povolen--výpis N X: boarding (N X: leaving)
bude ve výstupním souboru pouze mezi příslušnými výpisy vozíku: V X: bording started (V X:
leaving started) a V X: bording complete (V X: leaving complete).
• Vozíky se nemohou na dráze předjet - přijíždějí do výstupní stanice ve stejném pořadí jako
odjely z nástupní.
• V nástupní a ve výstupní stanici může být pouze jeden vozík.

Hlavní proces
• Proces vytváří ihned po spuštění proces dispečer.
• Dále vytvoří procesy pro V vozíků a N návštěvníků.
• Poté čeká na ukončení všech procesů, které aplikace vytváří. Jakmile jsou tyto procesy
ukončeny, ukončí se i hlavní proces s kódem (exit code) 0.

Proces dispečer
• Po spuštění vypíše: A: D: started
• (*) V případě, že ve frontě na atrakci již nikdo není, ani nemůže přijít, pokračuje bodem (**)
• Vypíše: A: D: next cart
• Dá pokyn k nástupu do dalšího vozíku
• Po odjezdu vozíku vyčká po dobu bezpečnostního odstupu B a pokračuje bodem (*)
• (**) Po odjezdu posledního vozíku vypíše: A: D: closing
• Dá pokyn k ukončení všech procesů vozíků.

Vozík
• Každý vozík je jednoznačně identifikován číslem idV, 0<idV<=V
• Po spuštění vypíše: A: V idV: started
• (*) Zařadí se do fronty připravených vozíků ve stanici a čeká na pokyn dispečera. Pořadí vozíků
v této frontě nemusí odpovídat idV.
• Na pokyn dispečera najede na nástupní místo, nebo v případě uzavření atrakce pokračuje bodem
(**)
• Vypíše A: V idV: boarding started
• Dá pokyn k nástupu K návštěvníkům (v případě poslední jízdy všem zbývajícím).
• Čeká na dokončení nástupu
• Vypíše A: V idV: boarding complete
• Vozík vyjede na trať---čeká v intervalu <TV/2,TV> mikrosekund
• Čeká na uvolnění místa ve výstupní stanici (pokud tam ještě probíhá výstup z předchozího
vozíku)
• Vypíše: A: N idV: leaving started
• Dá pokyn k výstupu a čeká až všichni vystoupí
• Vypíše: A: N idV: leaving complete
• Pokračuje bodem (*)
• (**) Vypíše: A: N idV: closed
• Proces končí.

Návštěvník
• Každý návštěvník je jednoznačně identifikován číslem idN, 0<idN<=N
• Vypíše A: N IdN: started
• Čeká v intervalu <0,TN> mikrosekund
• Vypíše A: N IdN: queue
• Zařadí se do fronty na atrakci
• Na výzvu k nástupu vypíše A: N idN: boarding
• Po skončení jízdy na pokyn vystoupí a vypíše: A: N idN: leaving
• Proces končí

Obecné informace
• Projekt implementujte v jazyce C. Komentujte zdrojové kódy, programujte přehledně. Součástí
hodnocení bude i kvalita zdrojového kódu.
• Kontrolujte, zda se všechny procesy ukončují korektně a zda při ukončování správně uvolňujete
všechny alokované zdroje.
• Dodržujte syntax zadaných jmen, formát souborů a formát výstupních dat. Použijte základní
skript pro ověření korektnosti výstupního formátu (dostupný z webu se zadáním).
• Dotazy k zadání: Veškeré nejasnosti a dotazy řešte pouze prostřednictvím diskuzního fóra k
projektu 2.
• Poznámka k testování: Můžete si nasimulovat častější přepínání procesů například vložením
krátkého uspání po uvolnění semaforů apod. Pouze pro testovací účely, do finálního řešení
nevkládejte!

Překlad
• Pro překlad používejte nástroj make. Součástí odevzdání bude soubor Makefile.
• Překlad se provede příkazem „make“ v adresáři, kde je umístěn soubor Makefile.
• Po překladu vznikne spustitelný soubor se jménem proj2, který bude umístěn ve stejném
adresáři jako soubor Makefile
• Spustitelný soubor může být závislý pouze na systémových knihovnách—nesmí předpokládat
existenci žádného dalšího studentem vytvořeného souboru (např. spustitelný soubor
cart/dispacher/person , konfigurační soubor, dynamická knihovna, ...).
• Zdrojové kódy překládejte s přepínači -std=gnu99 -Wall -Wextra -Werror -pedantic
• Pokud to vaše řešení vyžaduje, lze přidat další přepínače pro linker (např. kvůli semaforům či
sdílené paměti, -pthread, -lrt , . . . ).
• Vaše řešení musí být možné přeložit a spustit na serveru merlin.
Odevzdání
• Součástí odevzdání budou pouze soubory se zdrojovými kódy (*.c , *.h ) a soubor Makefile.
Tyto soubory zabalte pomocí nástroje zip do archivu s názvem proj2.zip.
• Archiv vytvořte tak, aby po rozbalení byl soubor Makefile umístěn ve stejném adresáři, jako je
archiv.
• Archiv proj2.zip odevzdejte prostřednictvím informačního systému—termín Projekt 2.
• Pokud nebude dodržena forma odevzdání nebo projekt nepůjde přeložit, bude projekt hodnocen
0 body.
• Archiv odevzdejte pomocí informačního systému v dostatečném předstihu—odevzdaný soubor
můžete před vypršením termínu snadno nahradit jeho novější verzí, kdykoliv budete potřebovat.

Příklad výstupu
Příklad výstupního souboru proj2.out pro následující příkaz:
$ ./proj2 2 10 4 10 10 10
1: D: started
2: D: next cart
3: V 1: started
4: V 1: boarding started
5: V 2: started
6: N 1: started
7: N 2: started
8: N 3: started
9: N 1: queue
10: N 1: boarding
11: N 2: queue
12: N 2: boarding
13: N 4: started
14: N 5: started
15: N 3: queue
16: N 3: boarding
17: N 4: queue
18: N 4: boarding
19: N 6: started
20: V 1: boarding complete
21: N 7: started
22: N 5: queue
23: N 8: started
24: V 1: leaving started
25: N 6: queue
26: D: next cart
27: N 7: queue
28: N 2: leaving
29: N 4: leaving
30: N 1: leaving
31: V 2: boarding started
32: N 3: leaving
33: N 9: started
34: V 1: leaving complete
35: N 10: started
36: N 6: boarding
37: N 9: queue
38: N 9: boarding
39: N 8: queue
40: N 5: boarding
41: N 7: boarding
42: V 2: boarding complete
43: D: next cart
44: V 1: boarding started
45: N 10: queue
46: N 10: boarding
47: N 8: boarding
48: V 1: boarding complete
49: V 2: leaving started
50: D: closing
51: N 9: leaving
52: N 6: leaving
53: N 5: leaving
54: N 7: leaving
55: V 2: leaving complete
56: V 2: closed
57: V 1: leaving started
58: N 10: leaving
59: N 8: leaving
60: V 1: leaving complete
61: V 1: closed
