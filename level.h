#include <list>
#include "room.h"

struct Item;
struct Monster;

struct Level {
    Level();

    void new_level(int do_implode);

    //get_room_from_position: Find what room some coordinates are in. NULL means they aren't in any room.
    Room* get_room_from_position(Coord cp);

    //put_things: Put potions and scrolls on this level
    void put_things();

    //treas_room: Add a treasure room
    void treas_room();

    void clear_level();

    bool is_floor_or_passage(Coord p);
    byte get_tile(Coord p, bool consider_monsters = false, bool mimic_as_monster = false);
    byte get_tile_or_monster(Coord p, bool mimic_as_monster = false); //todo:remove default
    void set_tile(Coord p, byte c);

    byte get_flags(Coord p);
    void set_flag(Coord p, byte f);
    void unset_flag(Coord p, byte f);
    void copy_flags(Coord p, byte f);

    bool is_passage(Coord p);
    bool is_maze(Coord p);
    bool is_real(Coord p);
    int get_passage_num(Coord p);
    int get_trap_type(Coord p);

    void search(Coord pos);

    Room* get_passage(Coord pos);

    //monster_at: returns pointer to monster at coordinate. if no monster there return NULL
    Monster* monster_at(Coord p, bool include_disguised =true); //todo: remove default

    void draw_char(Coord p);

    void show_map();

    //do_rooms: Create rooms and corridors with a connectivity graph
    void do_rooms();

    //do_passages: Draw all the passages on a level.
    void do_passages();

    Room* rnd_room();

    std::list<Item*> items; //List of objects on this level
    std::list<Monster*> monsters; //List of monsters on the level
private:
    byte the_level[(MAXLINES - 3)*MAXCOLS];
    byte the_flags[(MAXLINES - 3)*MAXCOLS];

    Room rooms[MAXROOMS]; //One for each room -- A level
    Room passages[MAXPASS] =
    {
        { 0,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 1,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 2,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 3,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 4,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 5,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 6,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 7,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 8,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 9,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 10,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 },
        { 11,{ 0, 0 },{ 0, 0 },{ 0, 0 }, 0, IS_GONE | IS_DARK, 0, 0 }
    };

    //conn: Draw a corridor from a room in a certain direction.
    void Level::conn(int r1, int r2);

    //passnum: Assign a number to each passageway
    void passnum();

    //numpass: Number a passageway square and its brethren
    void numpass(Coord p);

    void psplat(Coord p);
};

int get_level();
int next_level();
int prev_level();
int max_level();

int rnd_gold();
int maxrow();
