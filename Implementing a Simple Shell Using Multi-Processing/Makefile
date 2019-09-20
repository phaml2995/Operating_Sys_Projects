OBJS = main.cpp
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o main

tar:
	tar -cvf p1.tar Readme main.cpp Makefile

submit:
	/home/fac/zhuy/class/SubmitHW/submit3500 p1_late p1.tar

	