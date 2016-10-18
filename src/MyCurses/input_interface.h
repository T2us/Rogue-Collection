#pragma once
#include <string>

struct InputInterface
{
    virtual ~InputInterface();

    virtual char GetChar(bool block, bool *is_replay) = 0;
    virtual void Flush() = 0;
};
