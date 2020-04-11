all:
	g++ maze.cpp -o maze.out -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system

run:
	./maze.out --verbose -di maze.txt

clean:
	rm *.out
	rm *.o
