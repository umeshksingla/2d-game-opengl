all: sample2D

sample2D: game.cpp glad.c
	g++ -o sample game.cpp glad.c -framework OpenGL -I/usr/local/include -L/usr/local/lib -ldl -lglfw

clean:
	rm sample
