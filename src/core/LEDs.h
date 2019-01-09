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
#include "LightState.h"

using LEDsCapsLockHandler = std::function<void(bool)>;
using LEDsClearBlockHandler = std::function<void()>;
using LEDsPushBlockHandler = std::function<void()>;
using LEDsBlockHandler = std::function<void(int, BlockColour, BlockStepType, unsigned int)>;
using LEDsTorchHandler = std::function<void(bool)>;

class LEDs {
public:
    LEDs(std::shared_ptr<Log> const &log, std::string const &dbus_bus_address);

    ~LEDs() = default;

    void start_processing();

    HandlerRegistration registerLEDsCapsLockHandler(LEDsCapsLockHandler const &handler);

    HandlerRegistration registerLEDsClearBlockHandler(LEDsClearBlockHandler const &handler);

    HandlerRegistration registerLEDsPushBlockHandler(LEDsPushBlockHandler const &handler);

    HandlerRegistration registerLEDsBlockHandler(LEDsBlockHandler const &handler);

    HandlerRegistration registerLEDsTorchHandler(LEDsTorchHandler const &handler);

protected:
    LEDs() = default;

    LEDs(LEDs const &) = default;

    LEDs &operator=(LEDs const &) = default;

private:
    void dbus_method_call(
            GDBusConnection *connection,
            gchar const *sender_cstr,
            gchar const *object_path_cstr,
            gchar const *interface_name_cstr,
            gchar const *method_name_cstr,
            GVariant *parameters,
            GDBusMethodInvocation *invocation);

    std::shared_ptr<Log> const log;
    DBusConnectionHandle dbus_connection;
    DBusEventLoop dbus_event_loop;
    HandlerRegistration dbus_signal_handler_registration;

    LEDsCapsLockHandler ledsCapsLockHandler;
    LEDsClearBlockHandler ledsClearBlockHandler;
    LEDsPushBlockHandler ledsPushBlockHandler;
    LEDsBlockHandler ledsBlockHandler;
    LEDsTorchHandler ledsTorchHandler;

    bool verifyLed(guint led) const;

    bool verifyType(guint type) const;

    bool verifyValue(guint type, guint value) const;
};


#endif //GEMIAN_LEDS_LEDS_H
