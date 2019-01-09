//
// Created by adam on 12/05/18.
//

#ifndef GEMIAN_LEDS_LIGHTSTATE_H
#define GEMIAN_LEDS_LIGHTSTATE_H

#include <fstream>
#include <vector>

class Log;

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

    void handleTorch(bool on);

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

    int AWLEDCodeForStep(const BlockAnimStep &step) const;

    unsigned int maxLED(int led) const;

    unsigned int programCounter;

    uint64_t powerLevels;
};


#endif //GEMIAN_LEDS_LIGHTSTATE_H
