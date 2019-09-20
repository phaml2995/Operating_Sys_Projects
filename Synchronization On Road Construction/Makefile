OBJS = main.cpp
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

all: $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o main -pthread
tar:
	tar -cvf p3.tar Readme.txt main.cpp Makefile car.log flagperson.log

submit:
	/home/fac/zhuy/class/SubmitHW/submit3500 p3 p3.tar

	