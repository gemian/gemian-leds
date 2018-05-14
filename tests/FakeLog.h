#pragma once

#include <string>
#include <vector>
#include <mutex>
#include "../src/core/Log.h"

class FakeLog : public Log
{
public:

    void log(char const* tag, char const* format, ...) override;

    bool contains_line(std::vector<std::string> const& words);

private:
    std::mutex contents_mutex;
    std::vector<std::string> contents;
};

