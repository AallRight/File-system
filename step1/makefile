all: BDS BDC BDC_random
clean:
	rm *.o
BDS:	BDS.o
	gcc BDS.o -o BDS
BDC:	BDC.o
	gcc BDC.o -o BDC
BDC_random:	BDC_random.o
	gcc BDC_random.o -o BDC_random
BDS.o:	BDS.c
	gcc -c BDS.c -o BDS.o
BDC.o:	BDC.c
	gcc -c BDC.c -o BDC.o
BDC_random.o:	BDC_random.c
	gcc -c BDC_random.c -o BDC_random.o