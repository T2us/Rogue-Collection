//Code for handling the various special properties of the slime
//slime.c     1.0     (A.I. Design 1.42)      1/17/85

#include "rogue.h"
#include "slime.h"
#include "misc.h"
#include "thing.h"
#include "curses.h"
#include "io.h"
#include "monsters.h"
#include "chase.h"
#include "main.h"
#include "level.h"
#include "scrolls.h"

static Coord slimy;

//Slime_split: Called when it has been decided that A slime should divide itself

void slime_split(Agent *monster)
{
  Agent *nslime;

  if (new_slime(monster)==0) 
      return;
  msg("The %s divides.  Ick!", monster->get_monster_name());
  nslime = create_monster(monster->type, &slimy, get_level());
  if (can_see(slimy.y, slimy.x))
  {
    nslime->oldch = Level::get_tile(slimy);
    Screen::DrawChar(slimy, monster->type);
  }
  start_run(nslime);
}

int new_slime(Agent *slime)
{
  int y, x, ty, tx, ret;
  Agent *ntp;
  Coord sp;

  ret = 0;
  slime->set_dirty(true);
  if (plop_monster((ty = slime->pos.y), (tx = slime->pos.x), &sp)==0)
  {
    //There were no open spaces next to this slime, look for other slimes that might have open spaces next to them.
    for (y = ty-1; y<=ty+1; y++)
      for (x = tx-1; x<=tx+1; x++)
        if (get_tile_or_monster({x, y})==slime->type && (ntp = monster_at({x, y})))
        {
          if (ntp->is_dirty()) 
              continue; //Already done this one
          if (new_slime(ntp)) {y = ty+2; x = tx+2;}
        }
  }
  else {ret = 1; slimy = sp;}
  slime->set_dirty(false);
  return ret;
}

bool plop_monster(int r, int c, Coord *cp)
{
  int y, x;
  bool appear = 0;
  byte ch;

  for (y = r-1; y<=r+1; y++)
    for (x = c-1; x<=c+1; x++)
    {
      //Don't put a monster on top of the player.
      if ((y==player.pos.y && x==player.pos.x) || offmap(y,x)) continue;
      //Or anything else nasty
      if (step_ok(ch = get_tile_or_monster({x, y})))
      {
        if (ch==SCROLL && is_scare_monster_scroll(find_obj(y, x))) continue;
        if (rnd(++appear)==0) {cp->y = y; cp->x = x;}
      }
    }
    return appear;
}
