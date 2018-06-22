#include "ConnectionWatcher.h"
#include "EventLoopHandlerRegistration.h"
#include "ScopedGError.h"
#include "DBusMessageHandle.h"
#include "string.h"

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

    dbus_event_loop.enqueue([this] { dbus_query_connectivity_state(connection_object_path_wifi); }).get();
    dbus_event_loop.enqueue([this] { dbus_query_connectivity_state(connection_object_path_bluetooth); }).get();
    dbus_event_loop.enqueue([this] { dbus_query_connectivity_state(connection_object_path_cellular); }).get();
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

            saveStateForPath(object_path, powered);
        }

        g_variant_unref(value);
    }
}

void ConnectionWatcher::saveStateForPath(const gchar *object_path, bool powered) const {
    std::__cxx11::string const path = object_path;
    if (path.find("wifi") != std::string::npos) {
        lightState->handleConnectivityWifi(powered);
    } else if (path.find("bluetooth") != std::string::npos) {
        lightState->handleConnectivityBluetooth(powered);
    } else if (path.find("cellular") != std::string::npos) {
        lightState->handleConnectivityCellular(powered);
    }
}

void ConnectionWatcher::dbus_query_connectivity_state(char const *object_path) {
    log->log(log_tag, "dbus_query_connectivity_state()");

    int constexpr timeout_default = -1;
    auto constexpr null_cancellable = nullptr;
    ScopedGError error;

    auto const msg = g_dbus_message_new_method_call(connection_bus_name, object_path, connection_interface_name, "GetProperties");

    auto const result = g_dbus_connection_send_message_with_reply_sync(
            dbus_connection, msg, G_DBUS_SEND_MESSAGE_FLAGS_NONE, timeout_default, nullptr, null_cancellable, error);

    if (!result) {
        log->log(log_tag, "dbus_query_connectivity_state() failed to get Powered: %s", error.message_str().c_str());
        return;
    }

    auto body = g_dbus_message_get_body(result);

    if (strcmp(g_variant_get_type_string(body), "(s)") == 0) {
        char const* error_cstr{""};
        g_variant_get(body, "(s)", &error_cstr);
        log->log(log_tag, "dbus_query_connectivity_state() failed to get Powered: %s", error_cstr);
        return;
    }

    GVariantIter* properties_iter{nullptr};
    g_variant_get(body, "(a{sv})", &properties_iter);

    char const* key_cstr{""};
    GVariant* value{nullptr};

    while (g_variant_iter_next(properties_iter, "{&sv}", &key_cstr, &value))
    {
        auto const key_str = std::string{key_cstr ? key_cstr : ""};

        if (key_str == "Powered") {
            bool powered{false};

            g_variant_get(value, "b", &powered);

            saveStateForPath(object_path, powered);
        }

        g_variant_unref(value);
    }

    g_variant_iter_free(properties_iter);

}
