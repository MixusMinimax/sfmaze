all:
	g++ test.cpp -o test.out -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system

run:
	./test.out

clean:
	rm test.out
	rm *.o
