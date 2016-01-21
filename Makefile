all: sample2D

#sample3D: Sample_GL3_3D.cpp glad.c
#	g++ -o sample3D Sample_GL3.cpp glad.c -framework OpenGL -lglfw -I/usr/local/include -L/usr/local/lib

sample2D: Sample_GL3_2D.cpp glad.c
	g++ -o sample Sample_GL3_2D.cpp glad.c -framework OpenGL -I/usr/local/include -L/usr/local/lib -lglfw

clean:
	rm sample2D sample3D
