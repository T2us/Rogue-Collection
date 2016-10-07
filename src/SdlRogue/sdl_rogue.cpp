#include <cassert>
#include <map>
#include <vector>
#include <deque>
#include <mutex>
#include <Windows.h> //todo: change interface to avoid char_info
#include "SDL.h"
#include "sdl_rogue.h"
#include "utility.h"
#include "RogueCore/rogue.h"
#include "RogueCore/coord.h"
#include "RogueCore/io.h"
#include "RogueCore/mach_dep.h"
#include "RogueCore/game_state.h"
#include "RogueCore/curses.h"

struct SdlRogue::Impl
{
    const Coord EMPTY_COORD = { 0, 0 };
    const unsigned int MAX_QUEUE_SIZE = 50;

    const int TILE_COUNT = 78;
    const int TILE_STATES = 2;

    const int TEXT_COUNT = 256;
    const int TEXT_STATES = 16;

    Impl();
    ~Impl();

    void SetDimensions(Coord dimensions);
    void Draw(_CHAR_INFO * info);
    void Draw(_CHAR_INFO * info, Region rect);
    void Run();
    void Quit();

private:
    void Render();
    void RenderRegion(CHAR_INFO* data, Coord dimensions, Region rect);

    Coord get_screen_pos(Coord buffer_pos);

    bool render_as_text(int c);

    int tile_index(unsigned char c);
    bool use_inverse(unsigned short attr);
    SDL_Rect get_tile_rect(int i, bool use_inverse);

    int get_text_index(unsigned short attr);
    SDL_Rect get_text_rect(int c, int i);

    int shared_data_size() const;         //must have mutex before calling
    Region shared_data_full_region(); //must have mutex before calling

private:
    SDL_Window* m_window = 0;
    SDL_Renderer* m_renderer = 0;
    SDL_Texture* m_tiles = 0;
    SDL_Texture* m_text = 0;

    int m_tile_height = 0;
    int m_tile_width = 0;
    bool m_force_text = false;

    struct ThreadData
    {
        CHAR_INFO* m_data = 0;
        Coord m_dimensions = { 0, 0 };
        std::vector<Region> m_render_regions;
    };
    ThreadData m_shared_data;
    std::mutex m_mutex;

    std::map<int, int> m_attr_index = {
        {  7,  0 },
        {  2,  1 },
        {  3,  2 },
        {  4,  3 },
        {  5,  4 },
        {  6,  5 },
        {  8,  6 },
        {  9,  7 },
        { 10,  8 },
        { 12,  9 },
        { 13, 10 },
        { 14, 11 },
        { 15, 12 },
        {  1, 13 },
        {112, 14 },
        { 15, 15 },
    };

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
    };    

    //todo: 2 classes
public:
    char GetNextChar(bool do_key_state);
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

SdlRogue::Impl::Impl()
{
    SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + "tiles.bmp"));
    //SDL::Scoped::Surface tiles(load_bmp(getResourcePath("") + "sprites.bmp"));
    m_tile_height = tiles->h / TILE_STATES;
    m_tile_width = tiles->w / TILE_COUNT;

    SDL::Scoped::Surface text(load_bmp(getResourcePath("") + "text.bmp"));
    assert(m_tile_height == text->h / TEXT_STATES);
    assert(m_tile_width == text->w / TEXT_COUNT);

    m_window = SDL_CreateWindow("Rogue", 100, 100, m_tile_width * 80, m_tile_height * 25, SDL_WINDOW_SHOWN);
    if (m_window == nullptr)
        throw_error("SDL_CreateWindow");

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (m_renderer == nullptr)
        throw_error("SDL_CreateRenderer");

    m_tiles = create_texture(tiles.get(), m_renderer).release();
    m_text = create_texture(text.get(), m_renderer).release();
}

SdlRogue::Impl::~Impl()
{
    SDL_DestroyTexture(m_text);
    SDL_DestroyTexture(m_tiles);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
}

int SdlRogue::Impl::tile_index(unsigned char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    auto i = m_index.find(c);
    if (i != m_index.end())
        return i->second;
    return -1;
}

bool SdlRogue::Impl::use_inverse(unsigned short attr)
{
    return attr > 100 && attr != 160;
}

inline SDL_Rect SdlRogue::Impl::get_tile_rect(int i, bool use_inverse)
{
    SDL_Rect r;
    r.h = m_tile_height;
    r.w = m_tile_width;
    r.x = i*m_tile_width;
    r.y = use_inverse ? m_tile_height : 0;
    return r;
}

void SdlRogue::Impl::Render()
{
    std::vector<Region> regions;
    Coord dimensions;
    CHAR_INFO* temp = 0;
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        dimensions = m_shared_data.m_dimensions;
        if (dimensions == EMPTY_COORD)
            return;

        if (m_shared_data.m_render_regions.empty())
            return;

        temp = new CHAR_INFO[dimensions.x*dimensions.y];
        memcpy(temp, m_shared_data.m_data, shared_data_size());

        regions = m_shared_data.m_render_regions;
        m_shared_data.m_render_regions.clear();
    }
    std::unique_ptr<CHAR_INFO[]> data(temp);

    for (auto i = regions.begin(); i != regions.end(); ++i)
    {
        RenderRegion(data.get(), dimensions, *i);
    }

    SDL_RenderPresent(m_renderer);
}

void SdlRogue::Impl::RenderRegion(CHAR_INFO* data, Coord dimensions, Region rect)
{
    for (int x = rect.Left; x <= rect.Right; ++x) {
        for (int y = rect.Top; y <= rect.Bottom; ++y) {
            auto p = get_screen_pos({ x, y });
            auto info = data[y*dimensions.x + x];
            int c = (unsigned char)info.Char.AsciiChar;

            //todo: how to correctly determine text or monster/passage/wall?
            //the code below works for original tile set, but not others
            if (render_as_text(c))
            {
                short attr = info.Attributes;
                if (use_inverse(attr))
                    attr = 112;
                int i = get_text_index(attr);
                auto r = get_text_rect(c, i);
                render_texture_at(m_text, m_renderer, p, r);
            }
            else {
                auto i = tile_index(info.Char.AsciiChar);
                if (i == -1)
                {
                    continue;
                }
                bool inv = use_inverse(info.Attributes);
                auto r = get_tile_rect(i, inv);
                render_texture_at(m_tiles, m_renderer, p, r);
            }
        }
    }
}

inline Coord SdlRogue::Impl::get_screen_pos(Coord buffer_pos)
{
    Coord p;
    p.x = buffer_pos.x * m_tile_width;
    p.y = buffer_pos.y * m_tile_height;
    return p;
}

bool SdlRogue::Impl::render_as_text(int c)
{ 
    if (m_force_text)
        return true;

    return (c >= 0x20 && c < 128 ||
        c == PASSAGE || c == HWALL || c == VWALL || c == LLWALL || c == LRWALL || c == URWALL || c == ULWALL ||
        c == 0xcc || c == 0xb9 || //double line drawing
        c == 0xda || c == 0xb3 || c == 0xc0 || c == 0xc4 || c == 0xbf || c == 0xd9 || //line drawing
        c == 0x11 || c == 0x19 || c == 0x1a || c == 0x1b);
}

inline int SdlRogue::Impl::get_text_index(unsigned short attr)
{
    auto i = m_attr_index.find(attr);
    if (i != m_attr_index.end())
        return i->second;
    return 0;
}

inline SDL_Rect SdlRogue::Impl::get_text_rect(int c, int i)
{
    SDL_Rect r;
    r.h = m_tile_height;
    r.w = m_tile_width;
    r.x = c*m_tile_width;
    r.y = i*m_tile_height;
    return r;
}

void SdlRogue::Impl::SetDimensions(Coord dimensions)
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_shared_data.m_dimensions = dimensions;
        m_shared_data.m_data = new CHAR_INFO[dimensions.x*dimensions.y];
    }
    SDL_SetWindowSize(m_window, m_tile_width*dimensions.x, m_tile_height*dimensions.y);
}

void SdlRogue::Impl::Draw(_CHAR_INFO * info)
{
    Region r;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        //If we're adding a full render to the queue, we can ignore any previous regions.
        m_shared_data.m_render_regions.clear();
        r = shared_data_full_region();
    }
    Draw(info, r);
}

inline void SdlRogue::Impl::Draw(_CHAR_INFO * info, Region rect)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    //If we're behind on rendering, clear the queue and do a single full render.
    if (m_shared_data.m_render_regions.size() > MAX_QUEUE_SIZE)
    {
        m_shared_data.m_render_regions.clear();
        m_shared_data.m_render_regions.push_back(shared_data_full_region());
    }
    else {
        m_shared_data.m_render_regions.push_back(rect);
    }

    memcpy(m_shared_data.m_data, info, shared_data_size());
}

void SdlRogue::Impl::Run()
{
    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
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
        }
        Render();
    }
}

void SdlRogue::Impl::Quit()
{
    SDL_Event sdlevent;
    sdlevent.type = SDL_QUIT;
    SDL_PushEvent(&sdlevent);
}

int SdlRogue::Impl::shared_data_size() const
{
    return sizeof(CHAR_INFO) * m_shared_data.m_dimensions.x * m_shared_data.m_dimensions.y;
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






SdlRogue::SdlRogue() :
    m_impl(new Impl())
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

void SdlRogue::Draw(_CHAR_INFO * info)
{
    m_impl->Draw(info);
}

void SdlRogue::Draw(_CHAR_INFO * info, Region r)
{
    m_impl->Draw(info, r);
}

void SdlRogue::MoveCursor(Coord pos)
{
}

void SdlRogue::SetCursor(bool enable)
{
}

bool SdlRogue::HasMoreInput()
{
    return true;
}

char SdlRogue::GetNextChar()
{
    return m_impl->GetNextChar(true);
}

std::string SdlRogue::GetNextString(int size)
{
    return m_impl->GetNextString(size);
}

bool SdlRogue::IsCapsLockOn()
{
    return is_caps_lock_on();
}

bool SdlRogue::IsNumLockOn()
{
    return is_num_lock_on();
}

bool SdlRogue::IsScrollLockOn()
{
    return is_scroll_lock_on();
}

void SdlRogue::Serialize(std::ostream & out)
{
}

char SdlRogue::Impl::GetNextChar(bool do_key_state)
{
    std::unique_lock<std::mutex> lock(m_input_mutex);
    while (m_buffer.empty()) {
        if (do_key_state)
            handle_key_state();
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
        int x, y;
        game->screen().getrc(&x, &y);
        if (--y < 0) y = 0;
        game->screen().move(x, y);
        game->screen().addch(' ');
        game->screen().move(x, y);
    }
}

std::string SdlRogue::Impl::GetNextString(int size)
{
    std::string s;

    while (true)
    {
        char c = GetNextChar(false);
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
                beep();
                break;
            }
            game->screen().addch(c);
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
    auto keycode = TranslateNumPad(e.key.keysym.sym, e.key.keysym.mod);
    char c = TranslateKey(keycode, e.key.keysym.mod);
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