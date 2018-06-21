OBJS = main.o utils.o
ALGO_OBJS = 
LIBS = -static-libstdc++ -static
CXXFLAGS = -I. 
CFLAGS = -std=gnu11 -Wall -Wno-unused-variable -Wno-unused-but-set-variable -O2 -I.
OUTPUT = xboverclock
CC = gcc

main: $(OBJS) $(ALGO_OBJS)
	$(CC) -o $(OUTPUT) $(LIBS) $(OBJS) $(ALGO_OBJS)

clean:
	rm -rf $(OUTPUT) $(OBJS) $(ALGO_OBJS)
