//
// Created by adam on 10/05/18.
//

#include "DaemonConfig.h"
#include "ConsoleLog.h"
#include "NullLog.h"
#include "SyslogLog.h"

std::string DaemonConfig::the_dbus_bus_address() {
    auto const address = std::unique_ptr<gchar, decltype(&g_free)>{
            g_dbus_address_get_for_bus_sync(G_BUS_TYPE_SYSTEM, nullptr, nullptr),
            g_free};

    return address ? address.get() : std::string{};
}

std::shared_ptr<Log> DaemonConfig::the_log() {
    if (!log) {
        auto const log_env_cstr = getenv("LEDS_LOG");
        std::string const log_env{log_env_cstr ? log_env_cstr : ""};
        if (log_env == "console")
            log = std::make_shared<ConsoleLog>();
        else if (log_env == "null")
            log = std::make_shared<NullLog>();
        else
            log = std::make_shared<SyslogLog>();
    }
    return log;
}

std::shared_ptr<LEDs> DaemonConfig::the_leds() {
    if (!leds) {
        leds = std::make_shared<LEDs>(the_log(), the_dbus_bus_address());
    }
    return leds;
}

std::shared_ptr<LightState> DaemonConfig::the_lightState() {
    if (!lightState) {
        lightState = std::make_shared<LightState>(the_log());
    }
    return lightState;
}
