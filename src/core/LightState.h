//
// Created by adam on 12/05/18.
//

#ifndef GEMIAN_LEDS_LIGHTSTATE_H
#define GEMIAN_LEDS_LIGHTSTATE_H

#include <fstream>
#include <vector>

class Log;

static const int BLOCK_LED_COUNT = 5;
static const int BLOCK_COLOUR_COUNT = 3;
typedef enum {
    BlockColourRed,
    BlockColourGreen,
    BlockColourBlue,
} BlockColour;
typedef enum {
    BlockStepSetPWM,
    BlockStepFadeIn,
    BLockStepFadeOut,
    BlockStepDelay, //ignores led/colour components
    BlockStepMax
} BlockStepType;

class BlockAnimStep {
public:
    BlockAnimStep(int led, BlockColour colour, BlockStepType type, unsigned int value) :
            led(led), colour(colour), type(type), value(value) {}

    int led;
    BlockColour colour;
    BlockStepType type;
    unsigned int value;
};

class LightState {

public:
    LightState(std::shared_ptr<Log> const &log);

    void handleCapsLock(bool state);

    void handleConnectivityWifi(bool state);

    void handleConnectivityBluetooth(bool state);

    void handleConnectivityCellular(bool state);

    void handleClearBlock();

    void handlePushBlock();

    void handleSetBlockRGB(int led, BlockColour colour, BlockStepType type, unsigned int value);

    void Update();

    bool capsLock = false;
    int powerState = 0;
    bool connectivityWifi = false;
    bool connectivityBluetooth = false;
    bool connectivityCellular = false;
    std::vector<BlockAnimStep> steps;

    std::shared_ptr<Log> const log;

private:
    void WriteAW9120(unsigned int addr, unsigned int reg_data);

    void DisableAW9120();

    void EnableAW9120();

    void SetupAW9120();

    void HoldSramAW9120();

    void RunSramAW9120();

    void ResetSramLoadAddrAW9120();

    void WriteSramProgAW9120(unsigned int reg_data);

    int programCounter;

};


#endif //GEMIAN_LEDS_LIGHTSTATE_H
