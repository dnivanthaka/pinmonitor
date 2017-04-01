all: pinmon

pinmon: pinmon.o conf.o gpio.o log.o
	gcc pinmon.o conf.o gpio.o log.o -o pinmon 

pinmon.o: pinmon.c
	gcc -c pinmon.c

conf.o: conf.c
	gcc -c conf.c

gpio.o: gpio.c
	gcc -c gpio.c

log.o: log.c
	gcc -c log.c 


clean:
	rm -f *.o pinmon


