#pragma once
#include <memory>
#include <vector>
#include <SDL.h>
#include <coord.h>
#include "game_config.h"

struct DisplayInterface;
struct InputInterface;
struct SdlDisplay;
struct SdlInput;
struct Environment;

struct SdlRogue
{
    SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> env, int index);
    SdlRogue(SDL_Window* window, SDL_Renderer* renderer, std::shared_ptr<Environment> env, const std::string& filename);
    ~SdlRogue();

    void Run();
    void PostQuit();

    void SaveGame(std::string path, bool notify);
    void RestoreGame(const std::string& filename);

    DisplayInterface* Display() const;
    InputInterface* Input() const;
    Environment* GameEnv() const;
    GameConfig Options() const;

    static const char* kWindowTitle;
    static const unsigned char kSaveVersion;

private:
    void SetGame(const std::string& name);
    void SetGame(int i);

    std::unique_ptr<SdlDisplay> m_display;
    std::unique_ptr<SdlInput> m_input;
    std::shared_ptr<Environment> m_current_env;
    std::shared_ptr<Environment> m_game_env;

    GameConfig m_options;
    uint16_t m_restore_count = 0;
};
