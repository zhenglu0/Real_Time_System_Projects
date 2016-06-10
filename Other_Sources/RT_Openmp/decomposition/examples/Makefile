
all: simple double rate

simple:
	g++ simple.cpp ../librtomp.a -o simple -I../src -I../gomp_src -lpthread -lrt

double:
	g++ double.cpp ../librtomp.a -o double -I../src -I../gomp_src -lpthread -lrt

rate:
	g++ rate.cpp ../librtomp.a -o rate -I../src -I../gomp_src -lpthread -lrt

clean:
	rm simple double rate
