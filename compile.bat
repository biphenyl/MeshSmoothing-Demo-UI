gcc -DGLEW_STATIC -I. -c glew.c -o glew.o
g++ -O0 -g3 -Wall -c -std=c++0x -o MyGui.o MyGui.cpp
g++ -o CG_project.exe -static -O2 -std=c++0x -DGLEW_STATIC -Wall -Wextra main.cpp tiny_obj_loader.cc glew.o MyGui.o -I. -lglfw3 -lopengl32 -lgdi32