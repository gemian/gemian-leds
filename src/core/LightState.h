//
// Created by adam on 12/05/18.
//

#ifndef GEMIAN_LEDS_LIGHTSTATE_H
#define GEMIAN_LEDS_LIGHTSTATE_H

class Log;

static const int BLOCK_LED_COUNT = 5;
static const int BLOCK_COLOUR_COUNT = 3;
static const int BLOCK_COLOUR_RED = 0;
static const int BLOCK_COLOUR_GREEN = 1;
static const int BLOCK_COLOUR_BLUE = 2;

class LightState {

public:
    LightState(std::shared_ptr<Log> const& log);

    void handleCapsLock(bool state);
    void handleConnectivityWifi(bool state);
    void handleConnectivityBluetooth(bool state);
    void handleConnectivityCellular(bool state);
    void handleClearBlock();
    void handleSetBlockRGB(int led, int r, int g, int b);

    bool capsLock = false;
    int powerState = 0;
    bool connectivityWifi = false;
    bool connectivityBluetooth = false;
    bool connectivityCellular = false;
    int block[BLOCK_LED_COUNT][BLOCK_COLOUR_COUNT];

    void Update();

    std::shared_ptr<Log> const log;
};


#endif //GEMIAN_LEDS_LIGHTSTATE_H
