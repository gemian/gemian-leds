//
// Created by adam on 10/05/18.
//

#ifndef GEMIAN_LEDS_DAEMONCONFIG_H
#define GEMIAN_LEDS_DAEMONCONFIG_H

#include <memory>

#include "Log.h"
#include "LEDs.h"

class DaemonConfig {

public:
    std::shared_ptr<Log> the_log();
    std::shared_ptr<LEDs> the_leds();

    std::string the_dbus_bus_address();

private:
    std::shared_ptr<Log> log;
    std::shared_ptr<LEDs> leds;

};


#endif //GEMIAN_LEDS_DAEMONCONFIG_H
