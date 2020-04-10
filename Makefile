all:
	g++ -c test.cpp
	g++ test.o -o test.out -lsfml-graphics -lsfml-window -lsfml-system
	rm test.o

run:
	./test.out

clean:
	rm test.out
	rm *.o
