#include "catch.hpp"
#include "FakeLog.h"
#include "DBusBus.h"
#include "DBusClient.h"
#include "WaitCondition.h"
#include "FakeShared.h"
#include "../src/core/ConnectionWatcher.h"
#include "TestBase.h"

char const* const connman_technology_introspection = R"(
<node>
  <interface name='net.connman.Technology'>
    <method name='GetProperties'>
      <arg type='a{sv}' name='properties' direction='out'/>
    </method>
    <signal name='PropertyChanged'>
      <arg type='s' name='name'/>
      <arg type='v' name='value'/>
    </signal>
  </interface>
</node>)";

class FakeConnectionWatcherDBusService
{
public:
    FakeConnectionWatcherDBusService(std::string const& bus_address) : dbus_connection{bus_address}, dbus_event_loop{"FakeConnectionWatcher"} {
        dbus_connection.request_name("net.connman");

        connman_handler_registation = dbus_event_loop.register_object_handler(
                dbus_connection,
                "/net/connman/technology/wifi",
                connman_technology_introspection,
                [this] (
                        GDBusConnection* connection,
                        gchar const* sender,
                        gchar const* object_path,
                        gchar const* interface_name,
                        gchar const* method_name,
                        GVariant* parameters,
                        GDBusMethodInvocation* invocation)
                {
                    dbus_method_call(
                            connection, sender, object_path, interface_name,
                            method_name, parameters, invocation);
                });
    }

    void emit_technology_powered(char const *technology_path, bool powered)
    {
        this->powered = powered;

        g_dbus_connection_emit_signal(
                dbus_connection,
                nullptr,
                technology_path,
                "net.connman.Technology",
                "PropertyChanged",
                g_variant_new_parsed(
                        "('Powered', <%b>)",
                        powered),
                nullptr);
    }

private:
    void dbus_method_call(
            GDBusConnection* /*connection*/,
            gchar const* /*sender*/,
            gchar const* /*object_path*/,
            gchar const* /*interface_name*/,
            gchar const* method_name_cstr,
            GVariant* /*parameters*/,
            GDBusMethodInvocation* invocation)
    {
        std::string const method_name{method_name_cstr ? method_name_cstr : ""};
        GVariant* reply{nullptr};

        if (method_name == "GetProperties") {
            reply = g_variant_new_parsed("({'Powered': <%b>},)", powered.load());
        }

        g_dbus_method_invocation_return_value(invocation, reply);
    }

    DBusConnectionHandle dbus_connection;
    DBusEventLoop dbus_event_loop;
    HandlerRegistration connman_handler_registation;
    std::atomic<bool> powered{false};
    std::string name{};
};

std::chrono::seconds const default_timeout{3};

struct AConnectionWatcherTest {

    DBusBus bus;
    FakeLog fake_log;
    LightState fake_lightState{FakeShared(fake_log)};
    FakeConnectionWatcherDBusService service{bus.address()};
    ConnectionWatcher connectionWatcher{FakeShared(fake_log), FakeShared(fake_lightState), bus.address()};
};

TEST_CASE("all connections off") {
    AConnectionWatcherTest test;
    REQUIRE(test.fake_lightState.connectivityWifi == false);
    REQUIRE(test.fake_lightState.connectivityBluetooth == false);
    REQUIRE(test.fake_lightState.connectivityCellular == false);
}

TEST_CASE("wifi on") {
    AConnectionWatcherTest test;
    WaitCondition request_processed;

    test.service.emit_technology_powered("/net/connman/technology/wifi", true);

    request_processed.wait_for(default_timeout);
    REQUIRE(test.fake_lightState.connectivityWifi == true);
    REQUIRE(test.fake_lightState.connectivityBluetooth == false);
    REQUIRE(test.fake_lightState.connectivityCellular == false);
}

TEST_CASE("bluetooth on") {
    AConnectionWatcherTest test;
    WaitCondition request_processed;

    test.service.emit_technology_powered("/net/connman/technology/bluetooth", true);

    request_processed.wait_for(default_timeout);
    REQUIRE(test.fake_lightState.connectivityWifi == false);
    REQUIRE(test.fake_lightState.connectivityBluetooth == true);
    REQUIRE(test.fake_lightState.connectivityCellular == false);
}

TEST_CASE("cellular on") {
    AConnectionWatcherTest test;
    WaitCondition request_processed;

    test.service.emit_technology_powered("/net/connman/technology/cellular", true);

    request_processed.wait_for(default_timeout);
    REQUIRE(test.fake_lightState.connectivityWifi == false);
    REQUIRE(test.fake_lightState.connectivityBluetooth == false);
    REQUIRE(test.fake_lightState.connectivityCellular == true);
}
