#include "catch.hpp"
#include "FakeLog.h"
#include "DBusBus.h"
#include "DBusClient.h"
#include "WaitCondition.h"
#include "FakeShared.h"
#include "../src/core/ConnectionWatcher.h"
#include "TestBase.h"

class FakeConnectionWatcherDBusService
{
public:
    FakeConnectionWatcherDBusService(std::string const& bus_address) : dbus_connection{bus_address}, dbus_event_loop{"FakeConnectionWatcher"} {
        dbus_connection.request_name("net.connman.Technology");
    }

    void emit_technology_powered(char const *technology_path, bool powered)
    {
        this->powered = powered;

        g_dbus_connection_emit_signal(
                dbus_connection,
                nullptr,
                technology_path,
                "org.freedesktop.DBus.Properties",
                "PropertiesChanged",
                g_variant_new_parsed(
                        "(@s 'net.connman.Technology',"
                        " @a{sv} { 'Powered' : <%b> },"
                        " @as [])",
                        powered),
                nullptr);
    }

private:
    DBusConnectionHandle dbus_connection;
    DBusEventLoop dbus_event_loop;
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
