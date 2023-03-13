ALL: csample csamplets bigsample

CFLAGS = -Wall -O2 -g -DDEBUG
LOADLIBES = -lpthread -lCAEN_FELib

libbabies.o : libbabies.h

csample : csample.o libbabies.o

csamplets : csamplets.o libbabies.o

bigsample : bigsample.o libbabies.o

clean :
	rm -f *.o
