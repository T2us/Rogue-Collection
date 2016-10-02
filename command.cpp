//Read and execute the user commands
//command.c   1.44    (A.I. Design)   2/14/85

#include <map>
#include <ctype.h>
#include "rogue.h"
#include "game_state.h"
#include "daemons.h"
#include "daemon.h"
#include "command.h"
#include "main.h"
#include "io.h"
#include "wizard.h"
#include "misc.h"
#include "save.h"
#include "armor.h"
#include "weapons.h"
#include "sticks.h"
#include "output_interface.h"
#include "passages.h"
#include "mach_dep.h"
#include "move.h"
#include "rings.h"
#include "things.h"
#include "potions.h"
#include "pack.h"
#include "scrolls.h"
#include "level.h"
#include "food.h"
#include "hero.h"
#include "commands.h"

namespace
{
    std::map<int, bool(*)()> s_commands =
    {
        //proper actions
        { 'e', do_eat },
        { 'w', do_wield },
        { 'W', do_wear_armor },
        { 'T', do_take_off_armor },
        { 'P', do_put_on_ring },
        { 'R', do_remove_ring },
        { 't', do_throw_item },
        { 'z', do_zap },
        { 'q', do_quaff },
        { 'r', do_read_scroll },
        { 'd', do_drop },
        { 's', do_search },
        { '.', do_rest },
        { '>', do_go_down_stairs },
        { '<', do_go_up_stairs },

        //informational/utility actions
        { 'i', do_inventory },
        { 'c', do_call },
        { 'D', do_discovered },
        { '^', do_id_trap },
        { 'F', do_record_macro },
        { 'S', do_save_game },
        { 'Q', do_quit },
        { 'v', do_print_version },
        { '/', do_object_help },
        { '?', do_command_help },
        { '!', do_fakedos },
        { 'o', do_options },
        { CTRL('T'), do_toggle_terse },
        { CTRL('F'), do_play_macro },
        { CTRL('R'), do_repeat_msg },
        { CTRL('L'), do_clear_screen },
        { CTRL('W'), do_toggle_wizard },
    };

    //commands that are recognized only in wizard mode
    std::map<int, bool(*)()> s_wizard_commands = {
        { 'C', do_summon_object },
        { 'X', do_show_map },
        { CTRL('P'), do_toggle_powers },
    };
}


bool Command::is_move() const
{
    switch (ch) {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
        return true;
    }
    return false;
}

bool Command::is_run() const
{
    switch (ch) {
    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N':
        return true;
    }
    return false;
}


void command()
{
    int ntimes;

    if (game->hero().is_fast())
        ntimes = rnd(2) + 2;
    else ntimes = 1;
    while (ntimes--)
    {
        update_status_bar();
        SIG2();
        if (game->sleep_timer)
        {
            if (--game->sleep_timer <= 0) {
                msg("you can move again");
                game->sleep_timer = 0;
            }
        }
        else execcom();
        do_fuses();
        do_daemons();
        for (ntimes = LEFT; ntimes <= RIGHT; ntimes++)
            if (game->hero().get_ring(ntimes))
                switch (game->hero().get_ring(ntimes)->m_which)
                {
                case R_SEARCH:
                    do_search();
                    break;
                case R_TELEPORT:
                    if (rnd(50) == 17)
                        game->hero().teleport();
                    break;
                }
    }
}

//allow multiple keys to be mapped to the same command
int translate_command(int ch)
{
    switch (ch)
    {
    case '\b': 
        return 'h';
    case '+': 
        return 't';
    case '-':
        return 'z';
    }
    return ch;
}

int com_char()
{
    int ch;
    ch = readchar();
    clear_msg();
    return translate_command(ch);
}

void process_prefixes(int ch, Command* command, bool* fast_mode)
{
    switch (ch)
    {
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
    {
        int junk = game->repeat_cmd_count * 10;
        if ((junk += ch - '0') > 0 && junk < 10000)
            game->repeat_cmd_count = junk;
        show_count();
        break;
    }
    case 'f': // f: toggle fast mode for this turn
        *fast_mode = !*fast_mode;
        break;
    case 'g': // g + direction: move onto an item without picking it up
        command->m_can_pick_up = false;
        break;
    case 'a': //a: repeat last command
        *command = game->last_turn.command;
        game->repeat_cmd_count = game->last_turn.count; //todo: should Command have count?
        game->repeat_last_action = true;
        break;
    case ' ':
        break;
    case ESCAPE:
        game->m_stop_at_door = false;
        game->reset_command_count();
        show_count();
        break;
    default:
        command->ch = ch;
        break;
    }
}

//Read a command, setting things up according to prefix like devices. Return the command character to be executed.
Command get_command()
{
    Command command;

    bool fast_mode = false;
    look(true);
    if (!game->is_running())
        game->m_stop_at_door = false;
    command.m_can_pick_up = true;
    game->repeat_last_action = false;

    --game->repeat_cmd_count;
    //only update the screen when the count has changed
    if (game->repeat_cmd_count || game->last_turn.count)
        show_count();

    if (game->repeat_cmd_count > 0) {
        command = game->last_turn.command;
    }
    else
    {
        game->reset_command_count();
        if (game->is_running()) {
            command.ch = game->run_character;
            command.m_can_pick_up = game->last_turn.command.m_can_pick_up;
        }
        else
        {
            for (command.ch = 0; command.ch == 0;)
            {
                int ch = com_char();
                process_prefixes(ch, &command, &fast_mode);
            }
            fast_mode ^= game->fast_play();
        }
    }
    if (game->repeat_cmd_count)
        fast_mode = false;

    switch (command.ch)
    {
    case 'h': case 'j': case 'k': case 'l': case 'y': case 'u': case 'b': case 'n':
        if (fast_mode && !game->is_running())
        {
            if (!game->hero().is_blind()) {
                game->m_stop_at_door = true;
                game->m_first_move = true;
            }
            command.ch = toupper(command.ch);
        }

    case 'H': case 'J': case 'K': case 'L': case 'Y': case 'U': case 'B': case 'N':
        if (game->options.stop_running_at_doors() && !game->is_running())
        {
            if (!game->hero().is_blind()) {
                game->m_stop_at_door = true;
                game->m_first_move = true;
            }
        }
    
    case 'q': case 'r': case 's': case 'z': case 't': case '.':

    case CTRL('D'): case 'C':

        break;

    default:
        game->reset_command_count();
    }

    game->last_turn.count = game->repeat_cmd_count;
    game->last_turn.command = command;

    return command;
}

void show_count()
{
    const int COLS = game->screen().columns();
    const int LINES = game->screen().lines();

    game->screen().move(LINES - 2, COLS - 4);
    if (game->repeat_cmd_count > 0)
        game->screen().printw("%-4d", game->repeat_cmd_count);
    else
        game->screen().addstr("    ");
}

bool dispatch_command(Command c)
{
    //handle directional movement commands
    if (c.is_move())
        return do_move(c);
    else if (c.is_run())
        return do_run(c);

    //try executing the command from the map
    auto i = s_commands.find(c.ch);
    if (i != s_commands.end()) {
        return (*i->second)();
    }

    //if we're a wizard look at wizard commands too
    if (game->wizard().enabled()) {
        auto i = s_wizard_commands.find(c.ch);
        if (i != s_wizard_commands.end()) {
            return (*i->second)();
        }
    }

    msg("illegal command '%s'", unctrl(c.ch));
    game->reset_command_count();
    return false;
}


void execcom()
{
    bool counts_as_turn;
    do
    {
        Command c = get_command();
        counts_as_turn = dispatch_command(c);

        //todo: why is this here?
        if (!game->is_running())
            game->m_stop_at_door = false;

    } while (!counts_as_turn);
}
