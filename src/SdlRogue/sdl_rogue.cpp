#include <cassert>
#include <map>
#include <vector>
#include <deque>
#include <mutex>
#include "SDL.h"
#include "sdl_rogue.h"
#include "utility.h"
#include "..\RogueVersions\pc_gfx_charmap.h"

#define CTRL(ch)  (ch&037)
#define ESCAPE    (0x1b)

namespace
{
    std::map<int, int> unix_chars = {
        { PASSAGE,   '#' },
        { DOOR,      '+' },
        { FLOOR,     '.' },
        { PLAYER,    '@' },
        { TRAP,      '^' },
        { STAIRS,    '%' },
        { GOLD,      '*' },
        { POTION,    '!' },
        { SCROLL,    '?' },
        { FOOD,      ':' },
        { STICK,     '/' },
        { ARMOR,     ']' },
        { AMULET,    ',' },
        { RING,      '=' },
        { WEAPON,    ')' },
        { VWALL,     '|' },
        { HWALL,     '-' },
        { ULWALL,    '-' },
        { URWALL,    '-' },
        { LLWALL,    '-' },
        { LRWALL,    '-' },
    };

    uint32_t char_text(uint32_t ch)
    {
        return ch & 0x0000ffff;
    }

    uint32_t char_color(uint32_t ch)
    {
        return (ch >> 24) & 0xff;
    }
}

struct SdlRogue::Impl
{
    const unsigned int MAX_QUEUE_SIZE = 1;

    Impl(const TextConfig& text, TileConfig* tiles, Options options);
    ~Impl();

    void SetDimensions(Coord dimensions);
    void UpdateRegion(uint32_t* info);
    void UpdateRegion(uint32_t* info, Region rect);
    void MoveCursor(Coord pos);
    void SetCursor(bool enable);

    void Run();
    void Quit();

private:
    void Render();
    void RenderRegion(uint32_t* info, Coord dimensions, Region rect);
    void RenderText(uint32_t info, SDL_Rect r, bool is_text, unsigned char color);
    void RenderTile(uint32_t info, SDL_Rect r);
    void RenderCursor(Coord pos);

    Coord get_screen_pos(Coord buffer_pos);

    int tile_index(unsigned char c, unsigned short attr);
    bool use_inverse(unsigned short attr);
    SDL_Rect get_tile_rect(int i, bool use_inverse);

    int get_text_index(unsigned short attr);
    SDL_Rect get_text_rect(unsigned char c, int i);

    Region shared_data_full_region();     //must have mutex before calling
    bool shared_data_is_narrow();         //must have mutex before calling

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    SDL_Texture* m_tiles = 0;
    SDL_Texture* m_text = 0;

    Coord m_block_size = { 0, 0 };
    int m_tile_states = 0;
    Coord m_text_dimensions = { 0, 0 };
    std::map<int, int> m_attr_index;

    Options m_options;

    struct ThreadData
    {
        uint32_t* m_data = 0;
        Coord m_dimensions = { 0, 0 };
        bool m_cursor = false;
        Coord m_cursor_pos = { 0, 0 };
        std::vector<Region> m_render_regions;
    };
    ThreadData m_shared_data;
    std::mutex m_mutex;

    std::map<int, int> m_index = {
        { PLAYER, 26 },
        { ULWALL, 27 },
        { URWALL, 28 },
        { LLWALL, 29 },
        { LRWALL, 30 },
        { HWALL,  31 },
        { VWALL,  32 },
        { FLOOR,  33 },
        { PASSAGE,34 },
        { DOOR,   35 },
        { STAIRS, 36 },
        { TRAP,   37 },
        { AMULET, 38 },
        { FOOD,   39 },
        { GOLD,   40 },
        { POTION, 41 },
        { RING,   42 },
        { SCROLL, 43 },
        { STICK,  44 },
        { WEAPON, 45 },
        { ARMOR,  55 },
        { MAGIC,  63 },
        { BMAGIC, 64 },
        { '\\',   65 },
        { '/',    66 },
        { '-',    67 },
        { '|',    68 },
        { '*',    77 },
    };    

    //todo: 2 classes
public:
    char GetNextChar(bool block, bool do_key_state);
    virtual std::string GetNextString(int size);
private:
    void HandleEventText(const SDL_Event& e);
    void HandleEventKeyDown(const SDL_Event& e);
    void HandleEventKeyUp(const SDL_Event& e);

    SDL_Keycode TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers);
    char TranslateKey(SDL_Keycode keycode, uint16_t modifiers);


    std::deque<unsigned char> m_buffer;
    std::mutex m_input_mutex;
    std::condition_variable m_input_cv;

    std::map<SDL_Keycode, SDL_Keycode> m_numpad = {
        { SDLK_KP_0, SDLK_INSERT },
        { SDLK_KP_1, SDLK_END },
        { SDLK_KP_2, SDLK_DOWN },
        { SDLK_KP_3, SDLK_PAGEDOWN },
        { SDLK_KP_4, SDLK_LEFT },
        { SDLK_KP_6, SDLK_RIGHT },
        { SDLK_KP_7, SDLK_HOME },
        { SDLK_KP_8, SDLK_UP },
        { SDLK_KP_9, SDLK_PAGEUP },
        { SDLK_KP_ENTER, SDLK_RETURN },
    };

    std::map<SDL_Keycode, unsigned char> m_keymap = {
        { SDLK_RETURN,   '\r' },
        { SDLK_BACKSPACE,'\b' },
        { SDLK_ESCAPE,    ESCAPE },
        { SDLK_HOME,     'y' },
        { SDLK_UP,       'k' },
        { SDLK_PAGEUP,   'u' },
        { SDLK_LEFT,     'h' },
        { SDLK_RIGHT,    'l' },
        { SDLK_END,      'b' },
        { SDLK_DOWN,     'j' },
        { SDLK_PAGEDOWN, 'n' },
        { SDLK_INSERT,   '>' },
        { SDLK_DELETE,   's' },
        { SDLK_F1,       '?' },
        { SDLK_F2,       '/' },
        { SDLK_F3,       'a' },
        { SDLK_F4,       CTRL('R') },
        { SDLK_F5,       'c' },
        { SDLK_F6,       'D' },
        { SDLK_F7,       'i' },
        { SDLK_F8,       '^' },
        { SDLK_F9,       CTRL('F') },
        { SDLK_F10,      '!' },
    };
};

SdlRogue::Impl::Impl(const TextConfig& text_cfg, TileConfig* tile_cfg, Options options)
    : m_options(options)
{
    m_window = SDL_CreateWindow("Rogue", 100, 100, 100, 100, SDL_WINDOW_HIDDEN);
    if (m_window == nullptr)
        throw_error("SDL_CreateWindow");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == nullptr)
        throw_error("SDL_CreateRenderer");

    SDL::Scoped::Surface text(load_bmp(getResourcePath("") + text_cfg.filename));
    m_text_dimensions.x = text->w / 256;
    m_text_dimensions.y = text->h / text_cfg.colors.size();
    for (size_t i = 0; i < text_cfg.colors.size(); ++i)
        m_attr_index[text_cfg.colors[i]] = i;
    m_block_size = m_text_dimensions;
    m_text = create_texture(text.get(), m_renderer).release();

    if (tile_cfg)
    {
        SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + tile_cfg->filename));
        m_block_size.x = tiles->w / tile_cfg->count;
        m_block_size.y = tiles->h / tile_cfg->states;
        m_tile_states = tile_cfg->states;
        m_tiles = create_texture(tiles.get(), m_renderer).release();
    }

    SDL_SetWindowSize(m_window, m_block_size.x * 80, m_block_size.y * 25);
    SDL_ShowWindow(m_window);
}

SdlRogue::Impl::~Impl()
{
    SDL_DestroyTexture(m_text);
    SDL_DestroyTexture(m_tiles);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

int SdlRogue::Impl::tile_index(unsigned char c, unsigned short attr)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';

    auto i = m_index.find(c);
    if (i == m_index.end())
        return -1;

    int index = i->second;
    if (index >= 65 && index <= 68) //different color bolts use different tiles
    {
        if (attr & 0x02) //yellow
            index += 8;
        else if (attr & 0x04) //red
            index += 4;
    }
    return index;
}

bool SdlRogue::Impl::use_inverse(unsigned short attr)
{
    return attr > 100 && attr != 160;
}

SDL_Rect SdlRogue::Impl::get_tile_rect(int i, bool use_inverse)
{
    SDL_Rect r;
    r.h = m_block_size.y;
    r.w = m_block_size.x;
    r.x = i*m_block_size.x;
    r.y = use_inverse ? m_block_size.y : 0;
    return r;
}

void SdlRogue::Impl::Render()
{
    std::vector<Region> regions;
    Coord dimensions;
    std::unique_ptr<uint32_t[]> data;
    Coord cursor_pos;
    bool show_cursor;

    //locked region
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        dimensions = m_shared_data.m_dimensions;
        if (dimensions.x == 0 || dimensions.y == 0)
            return;

        if (m_shared_data.m_render_regions.empty())
            return;

        uint32_t* temp = new uint32_t[dimensions.x*dimensions.y];
        memcpy(temp, m_shared_data.m_data, dimensions.x*dimensions.y*sizeof(uint32_t));
        data.reset(temp);

        regions = m_shared_data.m_render_regions;
        m_shared_data.m_render_regions.clear();

        show_cursor = m_shared_data.m_cursor;
        cursor_pos = m_shared_data.m_cursor_pos;
    }

    for (auto i = regions.begin(); i != regions.end(); ++i)
    {
        RenderRegion(data.get(), dimensions, *i);
    }

    if (show_cursor) {
        RenderCursor(cursor_pos);
    }

    SDL_RenderPresent(m_renderer);
}

void SdlRogue::Impl::RenderRegion(uint32_t* data, Coord dimensions, Region rect)
{
    for (int x = rect.Left; x <= rect.Right; ++x) {
        for (int y = rect.Top; y <= rect.Bottom; ++y) {
            Coord p = get_screen_pos({ x, y });

            // We always render using the tile size.  Text will be scaled if it doesn't match
            SDL_Rect r;
            r.x = p.x;
            r.y = p.y;
            r.w = m_block_size.x;
            r.h = m_block_size.y;

            uint32_t info = data[y*dimensions.x+x];

            //todo: how to correctly determine text vs monster/passage/wall?
            if (!m_tiles)
            {
                //int color = (y >= 23) ? 0x0e : 0;
                RenderText(info, r, false, 0);
            }
            else {
                RenderTile(info, r);
            }
        }
    }
}

unsigned int GetColor(int chr, int attr)
{
    //if it is inside a room
    if (attr == 0x07 || attr == 0) switch (chr)
    {
    case DOOR:
    case VWALL: case HWALL:
    case ULWALL: case URWALL: case LLWALL: case LRWALL:
        return 0x06; //brown
    case FLOOR:
        return 0x0a; //light green
    case STAIRS:
        return 0xa0; //black on light green
    case TRAP:
        return 0x05; //magenta
    case GOLD:
    case PLAYER:
        return 0x0e; //yellow
    case POTION:
    case SCROLL:
    case STICK:
    case ARMOR:
    case AMULET:
    case RING:
    case WEAPON:
        return 0x09; //light blue
    case FOOD:
        return 0x04; //red
    }
    //if inside a passage or a maze
    else if (attr == 0x70) switch (chr)
    {
    case FOOD:
        return 0x74; //red on grey
    case GOLD: case PLAYER:
        return 0x7e; //yellow on grey
    case POTION: case SCROLL: case STICK: case ARMOR: case AMULET: case RING: case WEAPON:
        return 0x71; //blue on grey
    }

    return attr;
}


void SdlRogue::Impl::RenderText(uint32_t info, SDL_Rect r, bool is_text, unsigned char color)
{
    unsigned char c = char_text(info);
    if (!color) {
        color = GetColor(c, char_color(info));
    }
    if (!m_options.use_colors) {
        color = 0;
    }

    if (!is_text && m_options.use_unix_gfx)
    {
        auto i = unix_chars.find(c);
        if (i != unix_chars.end())
            c = i->second;
    }
    int i = get_text_index(color);
    SDL_Rect clip = get_text_rect(c, i);

    SDL_RenderCopy(m_renderer, m_text, &clip, &r);
}

void SdlRogue::Impl::RenderTile(uint32_t info, SDL_Rect r)
{
    auto i = tile_index(char_text(info), char_color(info));
    if (i == -1)
    {
        //draw a black tile if we don't have a tile for this character
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(m_renderer, &r);
        return;
    }
    bool inv = (m_tile_states > 1 && char_color(info));
    SDL_Rect clip = get_tile_rect(i, inv);
    SDL_RenderCopy(m_renderer, m_tiles, &clip, &r);
}

void SdlRogue::Impl::RenderCursor(Coord pos)
{
    pos = get_screen_pos(pos);

    SDL_Rect r;
    r.x = pos.x;
    r.y = pos.y + (m_block_size.y * 3 / 4);
    r.w = m_block_size.x;
    r.h = m_block_size.y/4;

    int color = 0x0f;
    int i = get_text_index(color);
    SDL_Rect clip = get_text_rect(0xdb, i);

    SDL_RenderCopy(m_renderer, m_text, &clip, &r);
}

Coord SdlRogue::Impl::get_screen_pos(Coord buffer_pos)
{
    Coord p;
    p.x = buffer_pos.x * m_block_size.x;
    p.y = buffer_pos.y * m_block_size.y;
    return p;
}

int SdlRogue::Impl::get_text_index(unsigned short attr)
{
    auto i = m_attr_index.find(attr);
    if (i != m_attr_index.end())
        return i->second;
    return 0;
}

SDL_Rect SdlRogue::Impl::get_text_rect(unsigned char c, int i)
{
    SDL_Rect r;
    r.h = m_text_dimensions.y;
    r.w = m_text_dimensions.x;
    r.x = c*m_text_dimensions.x;
    r.y = i*m_text_dimensions.y;
    return r;
}

void SdlRogue::Impl::SetDimensions(Coord dimensions)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shared_data.m_dimensions = dimensions;
        m_shared_data.m_data = new uint32_t[dimensions.x*dimensions.y];
    }
    SDL_SetWindowSize(m_window, m_block_size.x*dimensions.x, m_block_size.y*dimensions.y);
}

void SdlRogue::Impl::UpdateRegion(uint32_t * info)
{
    UpdateRegion(info, shared_data_full_region());
}

void SdlRogue::Impl::UpdateRegion(uint32_t* info, Region rect)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    //todo: If we're adding a full render to the queue, we can ignore any previous regions.

    //If we're behind on rendering, clear the queue and do a single full render.
    if (m_shared_data.m_render_regions.size() > MAX_QUEUE_SIZE)
    {
        m_shared_data.m_render_regions.clear();
        m_shared_data.m_render_regions.push_back(shared_data_full_region());
    }
    else {
        m_shared_data.m_render_regions.push_back(rect);
    }

    memcpy(m_shared_data.m_data, info, m_shared_data.m_dimensions.x * m_shared_data.m_dimensions.y *sizeof(int32_t));
}

void SdlRogue::Impl::MoveCursor(Coord pos)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shared_data.m_cursor_pos = pos;
}

void SdlRogue::Impl::SetCursor(bool enable)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_shared_data.m_cursor = enable;
}

void SdlRogue::Impl::Run()
{
    SDL_Event e;
    while (SDL_WaitEvent(&e)) {
        if (e.type == SDL_QUIT) {
            return;
        }
        else if (e.type == SDL_TEXTEDITING) {
            continue;
        }
        else if (e.type == SDL_TEXTINPUT) {
            HandleEventText(e);
        }
        else if (e.type == SDL_KEYDOWN) {
            HandleEventKeyDown(e);
        }
        else if (e.type == SDL_KEYUP) {
            HandleEventKeyUp(e);
        }
        else if (e.type == SDL_USEREVENT) {
            Render();
        }
    }
}

void SdlRogue::Impl::Quit()
{
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
}

Region SdlRogue::Impl::shared_data_full_region()
{
    Region r;
    r.Left = 0;
    r.Top = 0;
    r.Right = short(m_shared_data.m_dimensions.x - 1);
    r.Bottom = short(m_shared_data.m_dimensions.y - 1);
    return r;
}

bool SdlRogue::Impl::shared_data_is_narrow()
{
    return m_shared_data.m_dimensions.x == 40;
}

SdlRogue::SdlRogue(const TextConfig& text, TileConfig* tiles, Options options) :
    m_impl(new Impl(text, tiles, options))
{
}

SdlRogue::~SdlRogue()
{
}

void SdlRogue::Run()
{
    m_impl->Run();
}

void SdlRogue::Quit()
{
    m_impl->Quit();
}

void SdlRogue::SetDimensions(Coord dimensions)
{
    m_impl->SetDimensions(dimensions);
}

void SdlRogue::UpdateRegion(uint32_t * buf)
{
    m_impl->UpdateRegion(buf);
    SDL_Event sdlevent;
    sdlevent.type = SDL_USEREVENT;
    SDL_PushEvent(&sdlevent);
}

void SdlRogue::UpdateRegion(uint32_t* info, Region r)
{
    m_impl->UpdateRegion(info, r);
    SDL_Event sdlevent;
    sdlevent.type = SDL_USEREVENT;
    SDL_PushEvent(&sdlevent);
}

void SdlRogue::MoveCursor(Coord pos)
{
    m_impl->MoveCursor(pos);
}

void SdlRogue::SetCursor(bool enable)
{
    m_impl->SetCursor(enable);
}

char SdlRogue::GetChar(bool block)
{
    return m_impl->GetNextChar(block, true);
}

std::string SdlRogue::GetString(int size)
{
    return m_impl->GetNextString(size);
}

char SdlRogue::Impl::GetNextChar(bool block, bool do_key_state)
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    while (m_buffer.empty()) {
        if (!block) 
            return 0;
        //if (do_key_state)
          //  handle_key_state();
        m_input_cv.wait(lock);
    }

    char c = m_buffer.front();
    m_buffer.pop_front();

    return c;
}

namespace 
{
    void backspace()
    {
        //int x, y;
        //game->screen().getrc(&x, &y);
        //if (--y < 0) y = 0;
        //game->screen().move(x, y);
        //game->screen().add_text(' ');
        //game->screen().move(x, y);
    }
}

std::string SdlRogue::Impl::GetNextString(int size)
{
    std::string s;

    while (true)
    {
        char c = GetNextChar(true, false);
        switch (c)
        {
        case ESCAPE:
            s.clear();
            s.push_back(ESCAPE);
            return s;
        case '\b':
            if (!s.empty()) {
                backspace();
                s.pop_back();
            }
            break;
        default:
            if (s.size() >= unsigned int(size)) {
                //sound_beep();
                break;
            }
            //game->screen().add_text(c);
            s.push_back(c);
            break;
        case '\n':
        case '\r':
            return s;
        }
    }
}

void SdlRogue::Impl::HandleEventText(const SDL_Event & e)
{
    std::lock_guard<std::mutex> lock(m_input_mutex);
    //todo: when does string have more than 1 char?
    m_buffer.push_back(e.text.text[0]);
    m_input_cv.notify_all();
}

char ApplyShift(char c, uint16_t modifiers)
{
    bool caps((modifiers & KMOD_CAPS) != 0);
    bool shift((modifiers & KMOD_SHIFT) != 0);
    if (caps ^ shift)
        return toupper(c);
    return c;
}

bool IsLetterKey(SDL_Keycode keycode)
{
    return (keycode >= 'a' && keycode <= 'z');
}

bool IsDirectionKey(SDL_Keycode keycode)
{
    switch (keycode)
    {
    case 'h':
    case 'j':
    case 'k':
    case 'l':
    case 'y':
    case 'u':
    case 'b':
    case 'n':
        return true;
    }
    return false;
}

SDL_Keycode SdlRogue::Impl::TranslateNumPad(SDL_Keycode keycode, uint16_t modifiers)
{
    if ((modifiers & KMOD_NUM) == 0){
        auto i = m_numpad.find(keycode);
        if (i != m_numpad.end())
        {
            return i->second;
        }
    }
    return keycode;
}

char SdlRogue::Impl::TranslateKey(SDL_Keycode keycode, uint16_t modifiers)
{
    if (modifiers & KMOD_CTRL) {
        auto i = m_keymap.find(keycode);
        if (i != m_keymap.end())
            keycode = i->second;
        if (IsDirectionKey(keycode) && m_options.emulate_alt_controls)
        {
            m_buffer.push_back('f');
            m_buffer.push_back(keycode);
            m_input_cv.notify_all();
            return 0;
        }
        if (IsLetterKey(keycode)) {
            return CTRL(keycode);
        }
    }
    else if (modifiers & KMOD_ALT) {
        if (keycode == SDLK_F9) {
            return 'F';
        }
    }
    else {
        auto i = m_keymap.find(keycode);
        if (i != m_keymap.end())
        {
            char c = i->second;
            if (IsDirectionKey(c))
                return ApplyShift(c, modifiers);
            return c;
        }
    }

    return 0;
}

void SdlRogue::Impl::HandleEventKeyDown(const SDL_Event & e)
{
    auto c = TranslateNumPad(e.key.keysym.sym, e.key.keysym.mod);
    c = TranslateKey(c, e.key.keysym.mod);
    if (c == 0)
        return;

    std::lock_guard<std::mutex> lock(m_input_mutex);
    m_buffer.push_back(c);
    m_input_cv.notify_all();
}

void SdlRogue::Impl::HandleEventKeyUp(const SDL_Event & e)
{
    if (e.key.keysym.sym == SDLK_SCROLLLOCK ||
        e.key.keysym.sym == SDLK_CAPSLOCK ||
        e.key.keysym.sym == SDLK_NUMLOCKCLEAR){
        m_input_cv.notify_all();
    }
}
