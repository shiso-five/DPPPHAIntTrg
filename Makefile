ALL: DPPPHAIntTrg DPPPHAIntTrgNOEvtDivide

#CFLAGS = -Wall -O2 -g -DDEBUG
#CFLAGS = -Wall -O2 -g
CFLAGS = -Wall -O3 -g
#CFLAGS = -Wall -O3 -g  -DDEBUG
LOADLIBES = -lpthread -lCAEN_FELib

libbabies.o : libbabies.h

DPPPHAIntTrg : DPPPHAIntTrg.o libbabies.o

DPPPHAIntTrgNOEvtDivide : DPPPHAIntTrgNOEvtDivide.o libbabies.o

clean :
	rm -f *.o
