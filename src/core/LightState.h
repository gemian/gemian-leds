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

    bool capsLock = 0;
    int powerState = 0;

    void Update();

    std::shared_ptr<Log> const log;
};


#endif //GEMIAN_LEDS_LIGHTSTATE_H
