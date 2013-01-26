// ###   ###   ###  #   # #####
// #  # #   # #   # #   # #
// #  # #   # #   # #   # #
// ###  #   # #     #   # ###
// #  # #   # #  ## #   # #
// #  # #   # #   # #   # #
// #  #  ###   ###   ###  #####
//
//Exploring the Dungeons of Doom
//Copyright (C) 1981 by Michael Toy, Ken Arnold, and Glenn Wichman
//Copyright (C) 1983 by Mel Sibony, Jon Lane (AI Design update for the IBMPC)
//All rights reserved
//main.c      1.4 (A.I. Design) 11/28/84

#include <stdlib.h>
#include <time.h>

#include "rogue.h"
#include "main.h"
#include "daemons.h"
#include "daemon.h"
#include "chase.h"
#include "mach_dep.h"
#include "curses.h"
#include "io.h"
#include "init.h"
#include "new_leve.h"
#include "misc.h"
#include "rip.h"
#include "save.h"
#include "env.h"
#include "command.h"

int bwflag = FALSE;

//main: The main program, of course
int main(int argc, char **argv)
{
  char *curarg, *savfile = 0;

  init_ds();
  setenv("rogue.opt");
  //Parse the screen environment variable.  if the string starts with "bw", then we force black and white mode.
  if (strncmp(s_screen, "bw", 2)==0) bwflag = TRUE;
  dnum = 0;
  while (--argc)
  {
    curarg = *(++argv);
    if (*curarg=='-' || *curarg=='/')
    {
      switch (curarg[1])
      {
      case 'R': case 'r': savfile = s_save; break;
      case 's': case 'S': winit(); noscore = TRUE; score(0, 0, 0); fatal(""); break;
      }
    }
    else if (savfile==0) savfile = curarg;
  }
  if (savfile==0)
  {
    savfile = 0;
    winit();
    if (bwflag) forcebw();
    credits();
    if (dnum==0) dnum = srand2();
    seed = dnum;
    init_player(); //Set up initial player stats
    init_things(); //Set up probabilities of things
    init_names(); //Set up names of scrolls
    init_colors(); //Set up colors of potions
    init_stones(); //Set up stone settings of rings
    init_materials(); //Set up materials of wands
    setup();
    drop_curtain();
    new_level(); //Draw current level
    //Start up daemons and fuses
    daemon(doctor, 0);
    fuse(swander, 0, WANDERTIME);
    daemon(stomach, 0);
    daemon(runners, 0);
    msg("Hello %s%s.", whoami, noterse(".  Welcome to the Dungeons of Doom"));
    raise_curtain();
  }
  playit(savfile);
}

//endit: Exit the program abnormally.
void endit()
{
  fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

//Random number generator - adapted from the FORTRAN version in "Software Manual for the Elementary Functions" by W.J. Cody, Jr and William Waite.
long ran()
{
  seed *= 125;
  seed -= (seed/2796203)*2796203;
  return seed;
}

//rnd: Pick a very random number.
int rnd(int range)
{
  return range<1?0:((ran()+ran())&0x7fffffffl)%range;
}

int srand2()
{
  int t = (int)time(0);
  srand(t);
  return t;
}

//roll: Roll a number of dice
int roll(int number, int sides)
{
  int dtotal = 0;

  while (number--) dtotal += rnd(sides)+1;
  return dtotal;
}

//playit: The main loop of the program.  Loop until the game is over, refreshing things and looking at the proper times.
void playit(char *sname)
{
  if (sname)
  {
    restore_game(sname);
    if (bwflag) forcebw();
    setup();
    cursor(FALSE);
  }
  else {oldpos.x = hero.x; oldpos.y = hero.y; oldrp = roomin(&hero);}
  while (playing) command(); //Command execution
  endit();
}

//quit: Have player make certain, then exit.
void quit()
{
  int oy, ox;
  byte answer;
  static int qstate = FALSE;

  //if they try to interrupt with a control C while in this routine blow them away!
  if (qstate==TRUE) leave();
  qstate = TRUE;
  mpos = 0;
  getrc(&oy, &ox);
  move(0, 0);
  clrtoeol();
  move(0, 0);
  if (!terse) addstr("Do you wish to ");
  str_attr("end your quest now (%Yes/%No) ?");
  look(FALSE);
  answer = readchar();
  if (answer=='y' || answer=='Y')
  {
    clear();
    move(0, 0);
    printw("You quit with %u gold pieces\n", purse);
    score(purse, 1, 0);
    fatal("");
  }
  else
  {
    move(0, 0);
    clrtoeol();
    status();
    move(oy, ox);
    mpos = 0;
    count = 0;
  }
  qstate = FALSE;
}

//leave: Leave quickly, but courteously
void leave()
{
  look(FALSE);
  move(LINES-1, 0);
  clrtoeol();
  move(LINES-2, 0);
  clrtoeol();
  move(LINES-2, 0);
  fatal("Ok, if you want to leave that badly\n");
}

//fatal: exit with a message
void fatal(char *msg, int arg)
{
  printw(msg, arg);
  exit(0);
}