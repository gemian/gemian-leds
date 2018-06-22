#include "ConnectionWatcher.h"
#include "EventLoopHandlerRegistration.h"

auto const null_arg_handler = [](auto){};
char const* const connection_bus_name = "net.connman";
char const* const connection_object_path_wifi = "/net/connman/technology/wifi";
char const* const connection_object_path_bluetooth = "/net/connman/technology/bluetooth";
char const* const connection_object_path_cellular = "/net/connman/technology/cellular";
char const* const connection_interface_name = "net.connman.Technology";
char const* const log_tag = "ConnectionWatcher";

ConnectionWatcher::ConnectionWatcher(std::shared_ptr<Log> const& log, std::shared_ptr<LightState> const& lightState, std::string const& dbus_bus_address)
        : log{log},
          lightState{lightState},
          dbus_connection{dbus_bus_address},
          dbus_event_loop{"ConnectionWatcher"} {

    log->log(log_tag,"ConnectionWatcher::ConnectionWatcher");

    dbus_signal_handler_registration_wifi = getRegistration(connection_object_path_wifi);
    dbus_signal_handler_registration_bluetooth = getRegistration(connection_object_path_bluetooth);
    dbus_signal_handler_registration_cellular = getRegistration(connection_object_path_cellular);
}

HandlerRegistration ConnectionWatcher::getRegistration(char const *object_path) {
    return dbus_event_loop.register_signal_handler(
            dbus_connection,
            connection_bus_name,
            connection_interface_name,
            "PropertyChanged",
            object_path,
            [this] (
                    GDBusConnection* connection,
                    gchar const* sender,
                    gchar const* object_path,
                    gchar const* interface_name,
                    gchar const* signal_name,
                    GVariant* parameters)
            {
                handle_dbus_signal(connection, sender, object_path, interface_name, signal_name, parameters);
            });
}


void ConnectionWatcher::handle_dbus_signal(
        GDBusConnection* /*connection*/,
        gchar const* /*sender*/,
        gchar const* object_path,
        gchar const* /*interface_name*/,
        gchar const* signal_name_cstr,
        GVariant* parameters) {

    std::string const signal_name{signal_name_cstr ? signal_name_cstr : ""};

    if (signal_name == "PropertyChanged") {
        char const* key_cstr{""};
        GVariant* value{nullptr};
        g_variant_get(parameters, "(sv)", &key_cstr, &value, nullptr);

        auto const key_str = std::string{key_cstr ? key_cstr : ""};

        log->log(log_tag,"PropertyChanged %s (%s)", key_str.c_str(), object_path);
        if (key_str == "Powered") {
            bool powered{false};
            g_variant_get(value, "b", &powered);

            std::string const path = object_path;
            if (path.find("wifi") != std::string::npos) {
                lightState->handleConnectivityWifi(powered);
            } else if (path.find("bluetooth") != std::string::npos) {
                lightState->handleConnectivityBluetooth(powered);
            } else if (path.find("cellular") != std::string::npos) {
                lightState->handleConnectivityCellular(powered);
            }
        }

        g_variant_unref(value);
    }
}