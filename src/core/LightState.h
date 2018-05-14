//
// Created by adam on 12/05/18.
//

#ifndef GEMIAN_LEDS_LIGHTSTATE_H
#define GEMIAN_LEDS_LIGHTSTATE_H


class LightState {

public:
    void handleCapsLock(bool state);

    bool capsLock;
    int powerState;

    void Update();
};


#endif //GEMIAN_LEDS_LIGHTSTATE_H
