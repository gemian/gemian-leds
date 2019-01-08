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

    log.log("tag", "something %d", 234);

    REQUIRE(log.contains_line({"tag: something 234"}));
}

static const char *const GEMINI_LEDS_DESTINATION = "org.thinkglobally.Gemian.LEDs";

struct LEDsDBusClient : DBusClient {
    LEDsDBusClient(std::string const &dbus_address) : DBusClient{
            dbus_address,
            GEMINI_LEDS_DESTINATION,
            "/org/thinkglobally/Gemian/LEDs"} {
    }

    void emitSetCaps(bool capsLock) {
        invoke_with_reply<DBusAsyncReplyVoid>(GEMINI_LEDS_DESTINATION, "SetCapsLock", g_variant_new("(b)", capsLock));
    }

    void emitSetBlock(int i, int r, int g, int b) {
        invoke_with_reply<DBusAsyncReplyVoid>(GEMINI_LEDS_DESTINATION, "SetLEDBlock",
                                              g_variant_new("(uuuu)", i, r, g, b));
    }

    void emitClearBlock() {
        invoke_with_reply<DBusAsyncReplyVoid>(GEMINI_LEDS_DESTINATION, "ClearLEDBlock", nullptr);
    }
};

std::chrono::seconds const default_timeout{3};

struct MockHandler {

    void capsLock(bool state) {
        caps = state;
    }

    void block(int led, int r, int g, int b) {
        leds[led][BLOCK_COLOUR_RED] = r;
        leds[led][BLOCK_COLOUR_GREEN] = g;
        leds[led][BLOCK_COLOUR_BLUE] = b;
    }

    void clearBlock() {
        for (int i=0; i<BLOCK_COLOUR_COUNT; i++) {
            for (int c=0; c<BLOCK_COLOUR_COUNT; c++) {
                leds[i][c] = 0;
            }
        }
    }

    int leds[BLOCK_LED_COUNT][BLOCK_COLOUR_COUNT];
    bool caps;
};

struct ALEDs : TestBase {
    ALEDs() {
        registrations.push_back(
                leds.registerLEDsCapsLockHandler(
                        [this](bool state) {
                            mockHandler.capsLock(state);
                        }));
        registrations.push_back(
                leds.registerLEDsClearBlockHandler(
                        [this]() {
                            mockHandler.clearBlock();
                        }));
        registrations.push_back(
                leds.registerLEDsBlockHandler(
                        [this](int led, int r, int g, int b) {
                            mockHandler.block(led, r, g, b);
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

TEST_CASE("clear leds") {
    ALEDs aLEDs;
    aLEDs.mockHandler.block(0, 1, 1, 1);

    WaitCondition request_processed;

    aLEDs.client.emitClearBlock();

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.leds[0][BLOCK_COLOUR_RED] == 0);
    REQUIRE(aLEDs.mockHandler.leds[0][BLOCK_COLOUR_BLUE] == 0);
    REQUIRE(aLEDs.mockHandler.leds[0][BLOCK_COLOUR_GREEN] == 0);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: clear block"}));
}

TEST_CASE("set led 1 to red") {
    ALEDs aLEDs;
    aLEDs.mockHandler.block(0, 0, 0, 0);

    WaitCondition request_processed;

    aLEDs.client.emitSetBlock(0, 1, 0, 0);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.leds[0][BLOCK_COLOUR_RED] == 1);
    REQUIRE(aLEDs.mockHandler.leds[0][BLOCK_COLOUR_BLUE] == 0);
    REQUIRE(aLEDs.mockHandler.leds[0][BLOCK_COLOUR_GREEN] == 0);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: block 0(1,0,0)"}));
}
