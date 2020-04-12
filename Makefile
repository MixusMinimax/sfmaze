all:
	g++ maze.cpp -o maze.out -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system

run:
	./maze.out --verbose -dg -x 4 -y 4

clean:
	rm *.out
	rm *.o
