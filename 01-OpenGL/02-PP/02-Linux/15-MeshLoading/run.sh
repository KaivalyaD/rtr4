gcc -o cvector.o -c meshloader/cvector/cvector.c
gcc -o meshloader.o -c meshloader/meshloader.c
g++ -o OGL.o -c OGL.cpp
g++ *.o -lm -lX11 -lGLEW -lGL -lSOIL
./*.out
rm ./*.out ./*.o
