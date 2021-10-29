# This makefile is intended to be used on my Linux machine.
.PHONY: all clean

GLFW_DIR := /storage/other/glfw/install
GLFW_CFLAGS := -I$(GLFW_DIR)/include -L$(GLFW_DIR)/lib -lglfw3 -ldl -lm -lpthread

all: opengl_tutorial

scapegoat_tree.o: scapegoat_tree.c scapegoat_tree.h
	gcc -Wall -Werror -O3 -c -o scapegoat_tree.o scapegoat_tree.c

parse_obj.o: parse_obj.c parse_obj.h
	gcc -Wall -Werror -O3 -c -o parse_obj.o parse_obj.c

opengl_tutorial: opengl_tutorial.c opengl_tutorial.h parse_obj.o \
	scapegoat_tree.o
	gcc -Wall -Werror -O3 -o opengl_tutorial opengl_tutorial.c \
		glad/src/glad.c parse_obj.o scapegoat_tree.o -I glad/include \
		-I cglm/include $(GLFW_CFLAGS)

clean:
	rm -f parse_obj.o
	rm -f scapegoat_tree.o
	rm -f opengl_tutorial

