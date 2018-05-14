#include "catch.hpp"
#include "FakeLog.h"
#include "DBusBus.h"
#include "DBusClient.h"
#include "WaitCondition.h"
#include "FakeShared.h"
#include "../src/core/LEDs.h"
#include "TestBase.h"

TEST_CASE("log") {

    FakeLog log;

    log.log("tag", "something %d",234);

    REQUIRE(log.contains_line({"tag: something 234"}));
}

static const char *const GEMINI_LEDS_DESTINATION = "org.thinkglobally.Gemian.LEDs";

struct LEDsDBusClient : DBusClient {
    LEDsDBusClient(std::string const& dbus_address) : DBusClient {
            dbus_address,
            GEMINI_LEDS_DESTINATION,
            "/org/thinkglobally/Gemian/LEDs"}
    {
    }

    void emitSetCaps(bool capsLock) {
        invoke_with_reply<DBusAsyncReplyVoid>(GEMINI_LEDS_DESTINATION, "SetCapsLock", g_variant_new("(b)", capsLock));
    }

};

std::chrono::seconds const default_timeout{3};

struct MockHandler {

    void capsLock(bool state) {
        caps = state;
    }

    bool caps;
};

struct ALEDs : TestBase {
    ALEDs() {
        registrations.push_back(
                leds.registerLEDsCapsLockHandler(
                        [this] (bool state)
                        {
                            mockHandler.capsLock(state);
                        }));
        leds.start_processing();
    }

    MockHandler mockHandler;

    DBusBus bus;
    FakeLog fake_log;
    LEDs leds{FakeShared(fake_log), bus.address()};
    LEDsDBusClient client{bus.address()};
    std::vector<HandlerRegistration> registrations;
};

TEST_CASE("set caps lock on") {
    ALEDs aLEDs;
    aLEDs.mockHandler.capsLock(false);

    WaitCondition request_processed;

    aLEDs.client.emitSetCaps(true);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.caps == true);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: caps 1"}));
}

TEST_CASE("set caps lock off") {
    ALEDs aLEDs;
    aLEDs.mockHandler.capsLock(true);

    WaitCondition request_processed;

    aLEDs.client.emitSetCaps(false);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.caps == false);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: caps 0"}));
}