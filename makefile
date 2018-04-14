EXEC=RATP
CFLAGS=-Wall -Wextra -g -std=c++11
CC =g++

all:$(EXEC)

$(EXEC): main.o Mapper.o
	$(CC) main.o Mapper.o -o $(EXEC)

main.o: main.cpp 
	$(CC) -c main.cpp $(CFLAGS)

Mapper.o: Mapper.cpp Mapper.hpp Generic_connection_parser.hpp Generic_mapper.hpp Generic_station_parser.hpp 
	$(CC) -c Mapper.cpp $(CFLAGS)


clean:
	rm -r *.o
	rm -r $(EXEC) 