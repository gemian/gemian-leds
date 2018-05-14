#pragma once

#include "Log.h"

class ConsoleLog : public Log {

public:
    void log(char const* tag, char const* format, ...) override;

};
