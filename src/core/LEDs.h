//
// Created by adam on 11/05/18.
//

#ifndef GEMIAN_LEDS_LEDS_H
#define GEMIAN_LEDS_LEDS_H


#include <string>
#include <memory>
#include <gio/gio.h>
#include "Log.h"
#include "HandlerRegistration.h"
#include "DBusConnectionHandle.h"
#include "DBusEventLoop.h"

using LEDsCapsLockHandler = std::function<void(bool)>;


class LEDs {
public:
    LEDs(std::shared_ptr<Log> const& log, std::string const& dbus_bus_address);
    ~LEDs() = default;

    void start_processing();
    HandlerRegistration registerLEDsCapsLockHandler(LEDsCapsLockHandler const& handler);

protected:
    LEDs() = default;
    LEDs(LEDs const&) = default;
    LEDs& operator=(LEDs const&) = default;

private:
    void dbus_method_call(
            GDBusConnection* connection,
            gchar const* sender_cstr,
            gchar const* object_path_cstr,
            gchar const* interface_name_cstr,
            gchar const* method_name_cstr,
            GVariant* parameters,
            GDBusMethodInvocation* invocation);

    std::shared_ptr<Log> const log;
    DBusConnectionHandle dbus_connection;
    DBusEventLoop dbus_event_loop;
    HandlerRegistration dbus_signal_handler_registration;

    LEDsCapsLockHandler ledsCapsLockHandler;
};


#endif //GEMIAN_LEDS_LEDS_H
