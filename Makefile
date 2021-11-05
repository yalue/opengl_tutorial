# This makefile is intended to be used on my Linux machine.
.PHONY: all clean

GLFW_DIR := /storage/other/glfw/install
GLFW_CFLAGS := -I$(GLFW_DIR)/include -L$(GLFW_DIR)/lib -lglfw3 -ldl -lm -lpthread
CFLAGS := -Wall -Werror -O3

all: opengl_tutorial

scapegoat_tree.o: scapegoat_tree.c scapegoat_tree.h
	gcc $(CFLAGS) -c -o scapegoat_tree.o scapegoat_tree.c

parse_obj.o: parse_obj.c parse_obj.h
	gcc $(CFLAGS) -c -o parse_obj.o parse_obj.c

model.o: model.c model.h
	gcc $(CFLAGS) -c -o model.o model.c

utilities.o: utilities.c utilities.h
	gcc $(CFLAGS) -c -o utilities.o utilities.c

opengl_tutorial: opengl_tutorial.c opengl_tutorial.h parse_obj.o \
	scapegoat_tree.o model.o utilities.o
	gcc $(CFLAGS) -o opengl_tutorial opengl_tutorial.c \
		glad/src/glad.c parse_obj.o scapegoat_tree.o -I glad/include \
		-I cglm/include $(GLFW_CFLAGS)

clean:
	rm -f *.o
	rm -f opengl_tutorial

