all: BDS FS FC
clean:
	rm *.o
BDS:	BDS.o
	gcc BDS.o -o BDS
FS:	FS.o
	gcc FS.o -o FS
FC:	FC.o
	gcc FC.o -o FC
BDS.o:	BDS.c
	gcc -c BDS.c -o BDS.o
FS.o:	FS.c
	gcc -c FS.c -o FS.o
FC.o:	FC.c
	gcc -c FC.c -o FC.o
