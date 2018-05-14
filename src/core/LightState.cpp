//
// Created by adam on 12/05/18.
//

#include <fstream>
#include <memory>
#include "LightState.h"
#include "Log.h"

char const* const log_tag = "LightState";

LightState::LightState(std::shared_ptr<Log> const& the_log) : log{the_log} {

}

void LightState::handleCapsLock(bool state) {
    log->log(log_tag,"handleCapsLock %d", state);

    capsLock = state;

    Update();
}

void LightState::Update() {
    log->log(log_tag,"Update");

    std::ofstream aw9120_operation;
    aw9120_operation.open("/proc/aw9120_operation");
    aw9120_operation << "7 " << capsLock << " " << powerState << " 0";
    aw9120_operation.close();
}
