//save and restore routines
//save.c      1.32    (A.I. Design)   12/13/84

#include "rogue.h"
#include "save.h"
#include "io.h"
#include "curses.h"

//save_game: Implement the "save game" command
void save_game()
{
  char savename[20];

  msg("");
  mpos = 0;
  if (terse) addstr("Save file ? ");
  else printw("Save file (press enter (\x11\xd9) to default to \"%s\") ? ", s_save);
  getinfo(savename, 19);
  if (*savename==0) strcpy(savename, s_save);
  msg("Save not implemented");
}

void restore(char *savefile)
{
}