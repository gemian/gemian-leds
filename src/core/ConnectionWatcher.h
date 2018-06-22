#ifndef GEMIAN_LEDS_CONNECTIONWATCHER_H
#define GEMIAN_LEDS_CONNECTIONWATCHER_H

#include <string>
#include <memory>
#include <gio/gio.h>
#include "Log.h"
#include "LightState.h"
#include "HandlerRegistration.h"
#include "DBusConnectionHandle.h"
#include "DBusEventLoop.h"

class ConnectionWatcher {
public:
    ConnectionWatcher(std::shared_ptr<Log> const& log, std::shared_ptr<LightState> const& lightState, std::string const& dbus_bus_address);
    ~ConnectionWatcher() = default;

protected:
    ConnectionWatcher() = default;
    ConnectionWatcher(ConnectionWatcher const&) = default;
    ConnectionWatcher& operator=(ConnectionWatcher const&) = default;

private:
    void handle_dbus_signal(
            GDBusConnection* connection,
            gchar const* sender,
            gchar const* object_path,
            gchar const* interface_name,
            gchar const* signal_name,
            GVariant* parameters);

    std::shared_ptr<Log> const log;
    std::shared_ptr<LightState> const lightState;
    DBusConnectionHandle dbus_connection;
    DBusEventLoop dbus_event_loop;
    HandlerRegistration dbus_signal_handler_registration_wifi;
    HandlerRegistration dbus_signal_handler_registration_bluetooth;
    HandlerRegistration dbus_signal_handler_registration_cellular;

    HandlerRegistration getRegistration(const char *object_path);
};


#endif //GEMIAN_LEDS_CONNECTIONWATCHER_H
