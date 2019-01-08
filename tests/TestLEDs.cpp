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

    void emitSetBlockLEDForCurrentFrame(int led, BlockColour colour, BlockStepType type, unsigned int value) {
        invoke_with_reply<DBusAsyncReplyVoid>(GEMINI_LEDS_DESTINATION, "SetLEDBlockStep",
                                              g_variant_new("(uuuu)", led, colour, type, value));
    }

    void emitClearAnimation() {
        invoke_with_reply<DBusAsyncReplyVoid>(GEMINI_LEDS_DESTINATION, "ClearLEDBlockAnimation", nullptr);
    }

    void emitPushAnimation() {
        invoke_with_reply<DBusAsyncReplyVoid>(GEMINI_LEDS_DESTINATION, "PushLEDBlockAnimation", nullptr);
    }
};

std::chrono::seconds const default_timeout{3};

struct MockHandler {

    void capsLock(bool state) {
        caps = state;
    }

    void block(int led, BlockColour colour, BlockStepType type, unsigned int value) {
        steps.emplace_back(led, colour, type, value);
    }

    void clearBlock() {
        steps.clear();
    }

    void pushBlock() {
        push = true;
    }

    std::vector<BlockAnimStep> steps;
    bool caps;
    bool push;
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
                leds.registerLEDsPushBlockHandler(
                        [this]() {
                            mockHandler.pushBlock();
                        }));
        registrations.push_back(
                leds.registerLEDsBlockHandler(
                        [this](int led, BlockColour colour, BlockStepType type, unsigned int value) {
                            mockHandler.block(led, colour, type, value);
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
    REQUIRE(aLEDs.mockHandler.caps);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: caps 1"}));
}

TEST_CASE("set caps lock off") {
    ALEDs aLEDs;
    aLEDs.mockHandler.capsLock(true);

    WaitCondition request_processed;

    aLEDs.client.emitSetCaps(false);

    request_processed.wait_for(default_timeout);
    REQUIRE(!aLEDs.mockHandler.caps);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: caps 0"}));
}

TEST_CASE("clear leds") {
    ALEDs aLEDs;
    aLEDs.mockHandler.block(0, BlockColourRed, BlockStepSetPWM, 1);

    WaitCondition request_processed;

    aLEDs.client.emitClearAnimation();

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps.empty());
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: clear animation"}));
}

TEST_CASE("push leds") {
    ALEDs aLEDs;
    aLEDs.mockHandler.push = false;

    WaitCondition request_processed;

    aLEDs.client.emitPushAnimation();

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.push);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: push animation"}));
}

TEST_CASE("set led 1 to full red") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(1, BlockColourRed, BlockStepSetPWM, 255);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps[0].led == 1);
    REQUIRE(aLEDs.mockHandler.steps[0].colour == BlockColourRed);
    REQUIRE(aLEDs.mockHandler.steps[0].type == BlockStepSetPWM);
    REQUIRE(aLEDs.mockHandler.steps[0].value == 255);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: block 1(0,0,255)"}));
}

TEST_CASE("set led 1 to fade in green") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(1, BlockColourGreen, BlockStepFadeIn, 20);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps[0].led == 1);
    REQUIRE(aLEDs.mockHandler.steps[0].colour == BlockColourGreen);
    REQUIRE(aLEDs.mockHandler.steps[0].type == BlockStepFadeIn);
    REQUIRE(aLEDs.mockHandler.steps[0].value == 20);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: block 1(1,1,20)"}));
}

TEST_CASE("set invalid led ignored") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(0, BlockColourGreen, BlockStepFadeIn, 20);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps.empty());
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: rejected block 0(1,1,20)"}));
}

TEST_CASE("set invalid type ignored") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(1, BlockColourGreen, BlockStepMax, 20);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps.empty());
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: rejected block 1(1,4,20)"}));
}

TEST_CASE("set invalid PWM value ignored") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(1, BlockColourGreen, BlockStepSetPWM, 256);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps.empty());
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: rejected block 1(1,0,256)"}));
}

TEST_CASE("set invalid fade time value ignored") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(1, BlockColourGreen, BlockStepFadeIn, 256);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps.empty());
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: rejected block 1(1,1,256)"}));
}

TEST_CASE("set invalid delay value ignored") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(1, BlockColourRed, BlockStepDelay, 1024);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps.empty());
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: rejected block 1(0,3,1024)"}));
}

TEST_CASE("set valid delay 1023") {
    ALEDs aLEDs;
    aLEDs.mockHandler.clearBlock();

    WaitCondition request_processed;

    aLEDs.client.emitSetBlockLEDForCurrentFrame(1, BlockColourRed, BlockStepDelay, 1023);

    request_processed.wait_for(default_timeout);
    REQUIRE(aLEDs.mockHandler.steps[0].led == 1);
    REQUIRE(aLEDs.mockHandler.steps[0].colour == BlockColourRed);
    REQUIRE(aLEDs.mockHandler.steps[0].type == BlockStepDelay);
    REQUIRE(aLEDs.mockHandler.steps[0].value == 1023);
    REQUIRE(aLEDs.fake_log.contains_line({"LEDs: block 1(0,3,1023)"}));
}
