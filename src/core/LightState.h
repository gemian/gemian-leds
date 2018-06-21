//
// Created by adam on 12/05/18.
//

#ifndef GEMIAN_LEDS_LIGHTSTATE_H
#define GEMIAN_LEDS_LIGHTSTATE_H

class Log;

class LightState {

public:
    LightState(std::shared_ptr<Log> const& log);

    void handleCapsLock(bool state);
    void handleConnectivityWifi(bool state);
    void handleConnectivityBluetooth(bool state);
    void handleConnectivityCellular(bool state);

    bool capsLock = 0;
    int powerState = 0;
    bool connectivityWifi = 0;
    bool connectivityBluetooth = 0;
    bool connectivityCellular = 0;

    void Update();

    std::shared_ptr<Log> const log;
};


#endif //GEMIAN_LEDS_LIGHTSTATE_H
