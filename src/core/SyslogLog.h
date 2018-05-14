#pragma once

#include "Log.h"

class SyslogLog : public Log
{
public:
    SyslogLog();
    ~SyslogLog();

    void log(char const* tag, char const* format, ...) override;
};

