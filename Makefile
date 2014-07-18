CFLAGS=
CPP=
OBJS=

snake:	
	cc snake.c set_ticker.c -g -lrt -lcurses -o snake

snake.o:		snake.c
		$(CPP) $(CFLAGS) -c snake.c

clean:
		rm -f $(OBJS) snake *~
