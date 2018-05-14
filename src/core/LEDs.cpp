//
// Created by adam on 11/05/18.
//

#include "LEDs.h"
#include "EventLoopHandlerRegistration.h"

auto const null_arg_handler = [](auto){};
char const* const leds_bus_name = "org.thinkglobally.Gemian.LEDs";
char const* const leds_object_path = "/org/thinkglobally/Gemian/LEDs";
char const* const leds_interface_name = "org.thinkglobally.Gemian.LEDs";
char const* const log_tag = "LEDs";

char const* const leds_service_introspection = R"(
<node>
    <interface name='org.thinkglobally.Gemian.LEDs'>
        <method name='SetCapsLock'>
            <arg type="b" name="state" direction='in'></arg>
        </method>
    </interface>
</node>)";

LEDs::LEDs(std::shared_ptr<Log> const& log, std::string const& dbus_bus_address)
        : log{log},
          dbus_connection{dbus_bus_address},
          dbus_event_loop{"LEDs"},
          ledsCapsLockHandler{null_arg_handler} {
}

void LEDs::start_processing() {
    log->log(log_tag,"start_processing");
    dbus_signal_handler_registration = dbus_event_loop.register_object_handler(
            dbus_connection,
            leds_object_path,
            leds_service_introspection,
            [this] (
                    GDBusConnection* connection,
                    gchar const* sender,
                    gchar const* object_path,
                    gchar const* interface_name,
                    gchar const* method_name,
                    GVariant* parameters,
                    GDBusMethodInvocation* invocation)
            {
                try
                {
                    dbus_method_call(
                            connection, sender, object_path, interface_name,
                            method_name, parameters, invocation);
                }
                catch (std::invalid_argument const& e)
                {
                    g_dbus_method_invocation_return_error_literal(
                            invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, e.what());
                }
                catch (std::exception const& e)
                {
                    g_dbus_method_invocation_return_error_literal(
                            invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, e.what());
                }
            });

    dbus_connection.request_name(leds_bus_name);
}

HandlerRegistration LEDs::registerLEDsCapsLockHandler(LEDsCapsLockHandler const &handler) {

    return EventLoopHandlerRegistration {
            dbus_event_loop,
            [this, &handler] { this->ledsCapsLockHandler = handler; },
            [this] { this->ledsCapsLockHandler = null_arg_handler; }};
}

void LEDs::dbus_method_call(
        GDBusConnection* /*connection*/,
        gchar const* sender_cstr,
        gchar const* /*object_path_cstr*/,
        gchar const* /*interface_name_cstr*/,
        gchar const* method_name_cstr,
        GVariant* parameters,
        GDBusMethodInvocation* invocation)
{
    std::string const sender{sender_cstr ? sender_cstr : ""};
    std::string const method_name{method_name_cstr ? method_name_cstr : ""};

    if (method_name == "SetCapsLock") {
        gboolean state{FALSE};
        g_variant_get(parameters, "(b)", &state);
        ledsCapsLockHandler(state);
        log->log(log_tag,"caps %d", state);
        g_dbus_method_invocation_return_value(invocation, NULL);
    } else if (method_name == "Power") {
    } else {
        log->log(log_tag, "dbus_unknown_method(%s,%s)", sender.c_str(), method_name.c_str());

        g_dbus_method_invocation_return_error_literal(
                invocation, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "");
    }
}
