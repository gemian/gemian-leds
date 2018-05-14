//
// Created by adam on 12/05/18.
//

#include <fstream>
#include "LightState.h"

void LightState::handleCapsLock(bool state) {
    capsLock = state;

    Update();
}

void LightState::Update() {
    std::ofstream aw9120_operation;
    aw9120_operation.open("/proc/aw9210_operation");
    aw9120_operation << "7 " << capsLock << " " << powerState << " 0";
    aw9120_operation.close();
}
