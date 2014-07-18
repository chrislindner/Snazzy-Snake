// Chris Lindner
// snake.c - a snake game
// NOTE: setup_aio_buffer() and set_ticker.c are 
//       from bounce_aio.c by Bruce Molay

#define ROWS 20
#define HCOLS 60
#define HEADCHAR "="
#define TAILCHAR "0"
#define FOODCHAR "F"
#define BORDER "H"
#define DELAY 175
#define FOOD_VALUE 1
#define DELAY_MULT .95

#include <stdio.h>
#include <signal.h>
#include <curses.h>
#include <aio.h>
#include <sys/time.h>
#include <stdlib.h>

struct aiocb kbcbuf;

int foodCount = 0;
int score = 0;
char scorestr[4];
int head[2] = {4,5};
int tail[2] = {5,5};
int direction = 2;
int foodLoc[2] = {10,10};
int start = 0;
int end = 0;
int masterArray[ROWS+2][HCOLS+2];
int delay = DELAY;
int size = 2;
int lastdir = 2;
 
sigset_t sigs, prevsigs;

void inthndl(int);

int main()
{
  signal(SIGINT, inthndl);
  
  void menu(int);
  void setup_aio_buffer();
  void startGame();
  void inthndl(int);

  initscr();
  crmode();
  noecho();
  clear();
  
  sigemptyset(&sigs);
  sigprocmask(SIG_BLOCK,&sigs,&prevsigs);
  
  signal(SIGIO, menu);
  setup_aio_buffer();
  aio_read(&kbcbuf);

  mvaddstr(0,0,"SNAZZY SNAKE\n");  
  mvaddstr(1,0,"Press 'b' to begin\n");
  mvaddstr(2,0,"Press 'i' for instructions\n");
  mvaddstr(3,0,"Print 'q' to quit\n");

  refresh();

  while(!start)
    pause();

  startGame();

  while(!end)
    pause();
  
  endwin();

  printf("Your score was %d.\n",score);
  return 0;
}

void menu(int unused)
{
  signal(SIGINT, inthndl);
  sigemptyset(&sigs);  
  sigprocmask(SIG_BLOCK,&sigs,&prevsigs);

  int c;
  char *cp = (char *) kbcbuf.aio_buf;
  void instructions();

  if (aio_error(&kbcbuf) != 0)
    perror("keyboard read failed");
  else
    if (aio_return(&kbcbuf) == 1)
      {
	c = *cp;
	if (c == 'Q' || c == 'q' || c == EOF)
	  {start = 1; end = 1;}
	else if (c == 'I' || c == 'i')
	  instructions();
	else if (c == 'B' || c == 'b')
	  start = 1;
      }
  aio_read(&kbcbuf);
}

void instructions()
{
  signal(SIGINT, inthndl);
  mvaddstr(4,0,"Welcome to SNAZZY SNAKE!\n");
  mvaddstr(5,0,"You are a snake!  You look like this:\n");
  mvaddstr(6,0,TAILCHAR);
  mvaddstr(6,1,TAILCHAR);
  mvaddstr(6,2,HEADCHAR);
  mvaddstr(7,0,"You want to eat food.  It looks like this:\n");
  mvaddstr(8,0,FOODCHAR);
  mvaddstr(9,0,"Use the WADS keys to move up, left, right, and down respectively.\n");
  mvaddstr(10,0,"Eat food.  Don't eat yourself or the walls.");
  mvaddstr(11,0,"Your score will be displayed when you finish.");
  mvaddstr(12,0,"Press 'b' to begin.\n\n");

  refresh();
}

void startGame()
{
  signal(SIGINT, inthndl);
  sigemptyset(&sigs);
  sigprocmask(SIG_BLOCK,&sigs,&prevsigs);

  scorestr[0]=0;
  scorestr[1]=0;
  scorestr[2]=0;
  scorestr[3]='\n';

  clear();

  void advanceGame(int);
  void gameInput(int);

  //moveList[0] = 1;
  int icol, irow;
  for (icol=0;icol<HCOLS+2;icol++){
    mvaddstr(0,icol,BORDER);
    mvaddstr(ROWS+1,icol,BORDER);
  }
  for (irow=1;irow<ROWS+1;irow++){
    mvaddstr(irow,0,BORDER);
    mvaddstr(irow,HCOLS+1,BORDER);
  }

  for (irow=0;irow<ROWS+2;irow++)
    for (icol=0;icol<HCOLS+2;icol++)
      if(irow != 0 && icol != 0 && irow != ROWS+1 && icol != HCOLS+1)
	masterArray[irow][icol] = 0;
      else
	masterArray[irow][icol] = -1;

  mvaddstr(head[0],head[1],HEADCHAR);
  mvaddstr(tail[0],tail[1],TAILCHAR);
  mvaddstr(foodLoc[0],foodLoc[1],FOODCHAR);

  masterArray[head[0]][head[1]] = 1;
  masterArray[tail[0]][tail[1]] = 2;
  masterArray[foodLoc[0]][foodLoc[1]] = -2;

  crmode();
  signal(SIGIO,gameInput);
  signal(SIGALRM,advanceGame);
  set_ticker(delay);

  refresh();

  setup_aio_buffer();
  aio_read(&kbcbuf);

  while(!end)
    pause();
}

void advanceGame(int unused)
{
  signal(SIGINT, inthndl);
  int irow, icol;

  sigemptyset(&sigs);
  sigprocmask(SIG_BLOCK,&sigs,&prevsigs);

  void gameInput(int);
  void eat();

  signal(SIGIO,gameInput);
  setup_aio_buffer();
  aio_read(&kbcbuf);

  for (irow=1;irow<ROWS+1;irow++)
    for (icol=1;icol<HCOLS+1;icol++)
      if (masterArray[irow][icol] > 0)
	{
	  masterArray[irow][icol]++;
	  if(masterArray[irow][icol] == size+1)
	    {
	      masterArray[irow][icol] = 0;
	      mvaddstr(irow,icol," ");
	    }
	}

  int newHead[2]={head[0],head[1]};
  
  lastdir = direction;
  // find next space to move
  if(direction == 0)
    newHead[0]--;
  else if(direction == 1)
    newHead[1]++;
  else if(direction == 2)
    newHead[0]++;
  else if(direction == 3)
    newHead[1]--;
  
  if(masterArray[newHead[0]][newHead[1]])
    if (masterArray[newHead[0]][newHead[1]] == -2)
      eat();
    else
      end = 1;

  // draw the new head and tail tiles
  mvaddstr(head[0],head[1],TAILCHAR);
  mvaddstr(newHead[0],newHead[1],HEADCHAR);
  
  masterArray[newHead[0]][newHead[1]] = 1;

  head[0] = newHead[0];
  head[1] = newHead[1];

  refresh();
}

void gameInput(int unused)
{
  signal(SIGINT, inthndl);
  void gameInput(int);
  
  int c;
  char *cp = (char *) kbcbuf.aio_buf;

  if (aio_error(&kbcbuf) != 0)
    perror("keyboard read failed");
  else
    {
	c = *cp;
	if ((c == 'w' || c == 'W') && lastdir != 2)
	  direction = 0;
	else if ((c == 'a' || c == 'A') && lastdir != 1)
	  direction = 3;
	else if ((c == 'd' || c == 'D') && lastdir != 3)
	  direction = 1;
	else if ((c == 's' || c == 'S') && lastdir != 2)
	  direction = 2;
      }
  aio_read(&kbcbuf);
}

void eat()
{
  void win();
  long mabs(long);
  
  signal(SIGINT, inthndl);
  score++;
  delay = (int) ((float) delay * DELAY_MULT);
  set_ticker(delay);
  size += FOOD_VALUE;

  int trow, tcol;

  if(size == ROWS*HCOLS)
    win();
  else
    do{
      trow = abs(random())%ROWS+2;
      tcol = abs(random())%HCOLS+2;
    }while(masterArray[trow][tcol]);
  
  foodLoc[0] = trow;
  foodLoc[1] = tcol;
  
  mvaddstr(trow,tcol,FOODCHAR);
  masterArray[trow][tcol] = -2;
  
  refresh();
}

long mabs(long a)
{
  if (a<0)
    return -a;
 
  return a;
}

void win()
{
  mvaddstr(5,5,"YOU WON!  PRESS CTRL+C TO EXIT");
}

void setup_aio_buffer()
{
  signal(SIGINT, inthndl);
  static char input[1];		      /* 1 char of input */
  
  /* describe what to read */
  kbcbuf.aio_fildes     = 0;	      /* standard intput */
  kbcbuf.aio_buf        = input;	      /* buffer          */
  kbcbuf.aio_nbytes     = 1;             /* number to read  */
  kbcbuf.aio_offset     = 0;             /* offset in file  */
  
  /* describe what to do when read is ready */
  kbcbuf.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
  kbcbuf.aio_sigevent.sigev_signo  = SIGIO;  /* send SIGIO   */
}

void inthndl(int unused)
{
  start = 1;
  end = 1;
}
