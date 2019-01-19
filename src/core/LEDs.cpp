//
// Created by adam on 11/05/18.
//

#include "LEDs.h"
#include "EventLoopHandlerRegistration.h"

auto const null_arg0_handler = []() {};
auto const null_arg1_handler = [](auto) {};
auto const null_arg2_handler = [](auto, auto) {};
auto const null_arg4_handler = [](auto, auto, auto, auto) {};
char const *const leds_bus_name = "org.thinkglobally.Gemian.LEDs";
char const *const leds_object_path = "/org/thinkglobally/Gemian/LEDs";
char const *const leds_interface_name = "org.thinkglobally.Gemian.LEDs";
char const *const log_tag = "LEDs";

char const *const leds_service_introspection = R"(
<node>
    <interface name='org.thinkglobally.Gemian.LEDs'>
        <method name='SetCapsLock'>
            <arg type="b" name="state" direction='in'></arg>
        </method>
        <method name='SetLEDBlockStep'>
            <arg type="u" name="led" direction='in'></arg>
            <arg type="u" name="colour" direction='in'></arg>
            <arg type="u" name="type" direction='in'></arg>
            <arg type="u" name="value" direction='in'></arg>
        </method>
        <method name='ClearLEDBlockAnimation'>
        </method>
        <method name='PushLEDBlockAnimation'>
        </method>
        <method name='SetTorch'>
            <arg type="b" name="on" direction='in'></arg>
            <arg type="u" name="duration" direction='in'></arg>
        </method>
        <method name='SetCall'>
            <arg type="b" name="earpiece" direction='in'></arg>
            <arg type="b" name="leftUp" direction='in'></arg>
        </method>
    </interface>
</node>)";

LEDs::LEDs(std::shared_ptr<Log> const &log, std::string const &dbus_bus_address)
        : log{log},
          dbus_connection{dbus_bus_address},
          dbus_event_loop{"LEDs"},
          ledsCapsLockHandler{null_arg1_handler} {
}

void LEDs::start_processing() {
    log->log(log_tag, "start_processing");
    dbus_signal_handler_registration = dbus_event_loop.register_object_handler(
            dbus_connection,
            leds_object_path,
            leds_service_introspection,
            [this](
                    GDBusConnection *connection,
                    gchar const *sender,
                    gchar const *object_path,
                    gchar const *interface_name,
                    gchar const *method_name,
                    GVariant *parameters,
                    GDBusMethodInvocation *invocation) {
                try {
                    dbus_method_call(
                            connection, sender, object_path, interface_name,
                            method_name, parameters, invocation);
                }
                catch (std::invalid_argument const &e) {
                    g_dbus_method_invocation_return_error_literal(
                            invocation, G_DBUS_ERROR, G_DBUS_ERROR_INVALID_ARGS, e.what());
                }
                catch (std::exception const &e) {
                    g_dbus_method_invocation_return_error_literal(
                            invocation, G_DBUS_ERROR, G_DBUS_ERROR_FAILED, e.what());
                }
            });

    dbus_connection.request_name(leds_bus_name);
}

HandlerRegistration LEDs::registerLEDsCapsLockHandler(LEDsCapsLockHandler const &handler) {

    return EventLoopHandlerRegistration{
            dbus_event_loop,
            [this, &handler] { this->ledsCapsLockHandler = handler; },
            [this] { this->ledsCapsLockHandler = null_arg1_handler; }};
}

HandlerRegistration LEDs::registerLEDsClearBlockHandler(LEDsClearBlockHandler const &handler) {

    return EventLoopHandlerRegistration{
            dbus_event_loop,
            [this, &handler] { this->ledsClearBlockHandler = handler; },
            [this] { this->ledsClearBlockHandler = null_arg0_handler; }};
}

HandlerRegistration LEDs::registerLEDsPushBlockHandler(LEDsPushBlockHandler const &handler) {

    return EventLoopHandlerRegistration{
            dbus_event_loop,
            [this, &handler] { this->ledsPushBlockHandler = handler; },
            [this] { this->ledsPushBlockHandler = null_arg0_handler; }};
}

HandlerRegistration LEDs::registerLEDsBlockHandler(LEDsBlockHandler const &handler) {

    return EventLoopHandlerRegistration{
            dbus_event_loop,
            [this, &handler] { this->ledsBlockHandler = handler; },
            [this] { this->ledsBlockHandler = null_arg4_handler; }};
}

HandlerRegistration LEDs::registerLEDsTorchHandler(LEDsTorchHandler const &handler) {

    return EventLoopHandlerRegistration{
            dbus_event_loop,
            [this, &handler] { this->ledsTorchHandler = handler; },
            [this] { this->ledsTorchHandler = null_arg1_handler; }};
}

HandlerRegistration LEDs::registerLEDsCallHandler(LEDsCallHandler const &handler) {

    return EventLoopHandlerRegistration{
            dbus_event_loop,
            [this, &handler] { this->ledsCallHandler = handler; },
            [this] { this->ledsCallHandler = null_arg2_handler; }};
}

void LEDs::dbus_method_call(
        GDBusConnection * /*connection*/,
        gchar const *sender_cstr,
        gchar const * /*object_path_cstr*/,
        gchar const * /*interface_name_cstr*/,
        gchar const *method_name_cstr,
        GVariant *parameters,
        GDBusMethodInvocation *invocation) {
    std::string const sender{sender_cstr ? sender_cstr : ""};
    std::string const method_name{method_name_cstr ? method_name_cstr : ""};

    if (method_name == "SetCapsLock") {
        gboolean state{FALSE};
        g_variant_get(parameters, "(b)", &state);
        ledsCapsLockHandler(static_cast<bool>(state));
        log->log(log_tag, "caps %d", state);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    } else if (method_name == "SetLEDBlockStep") {
        guint led{0}, colour{0}, type{0}, value{0};
        g_variant_get(parameters, "(uuuu)", &led, &colour, &type, &value);
        if (verifyLed(led) && verifyType(type) && verifyValue(type, value)) {
            ledsBlockHandler(led, static_cast<BlockColour>(colour), static_cast<BlockStepType>(type), value);
            log->log(log_tag, "block %u(%u,%u,%u)", led, colour, type, value);
        } else {
            log->log(log_tag, "rejected block %u(%u,%u,%u)", led, colour, type, value);
        }
        g_dbus_method_invocation_return_value(invocation, nullptr);
    } else if (method_name == "ClearLEDBlockAnimation") {
        ledsClearBlockHandler();
        log->log(log_tag, "clear animation");
        g_dbus_method_invocation_return_value(invocation, nullptr);
    } else if (method_name == "PushLEDBlockAnimation") {
        ledsPushBlockHandler();
        log->log(log_tag, "push animation");
        g_dbus_method_invocation_return_value(invocation, nullptr);
    } else if (method_name == "SetTorch") {
        gboolean on{FALSE};
        guint duration{0};
        g_variant_get(parameters, "(bu)", &on, &duration);
        ledsTorchHandler(static_cast<bool>(on));
        if (on && duration > 0) {
            std::chrono::milliseconds after(duration);
            dbus_event_loop.schedule_in(
                    after,
                    [this] {
                        ledsTorchHandler(false);
                    });
        }

        log->log(log_tag, "torch %d(%d)", on, duration);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    } else if (method_name == "SetCall") {
        gboolean earpiece{FALSE},leftUp{FALSE};
        g_variant_get(parameters, "(bb)", &earpiece, &leftUp);
        ledsCallHandler(static_cast<bool>(earpiece),static_cast<bool>(leftUp));
        log->log(log_tag, "call %d %d", earpiece, leftUp);
        g_dbus_method_invocation_return_value(invocation, nullptr);
    } else {
        log->log(log_tag, "dbus_unknown_method(%s,%s)", sender.c_str(), method_name.c_str());

        g_dbus_method_invocation_return_error_literal(
                invocation, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED, "");
    }
}

bool LEDs::verifyValue(guint type, guint value) const {
    return ((type == BlockStepDelay && value < 1024) || value < 256);
}

bool LEDs::verifyType(guint type) const { return type < BlockStepMax; }

bool LEDs::verifyLed(guint led) const { return led > 0 && led < 6; }
