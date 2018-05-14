#pragma once

#include "Log.h"

class NullLog : public Log
{
public:
    void log(char const* tag, char const* format, ...) override;
};

