//
// Created by adam on 12/05/18.
//

#include <fstream>
#include <memory>
#include "LightState.h"
#include "Log.h"

char const *const log_tag = "LightState";

LightState::LightState(std::shared_ptr<Log> const &the_log) : log{the_log} {
    handleClearBlock();
}

void LightState::handleCapsLock(bool state) {
    log->log(log_tag, "handleCapsLock %d", state);

    capsLock = state;

    Update();
}

void LightState::handleConnectivityWifi(bool state) {
    log->log(log_tag, "handleConnectivityWifi %d", state);

    connectivityWifi = state;

    Update();
}

void LightState::handleConnectivityBluetooth(bool state) {
    log->log(log_tag, "handleConnectivityBluetooth %d", state);

    connectivityBluetooth = state;

    Update();
}

void LightState::handleConnectivityCellular(bool state) {
    log->log(log_tag, "handleConnectivityCellular %d", state);

    connectivityCellular = state;

    Update();
}

void LightState::handleClearBlock() {
    steps.clear();
}

void LightState::handlePushBlock() {
    Update();
}

void LightState::handleSetBlockRGB(int led, BlockColour colour, BlockStepType type, unsigned int value) {
    steps.emplace_back(led, colour, type, value);
}

#define GCR   0x01
#define LER1  0x50
#define LER2  0x51
#define PMR   0x53
#define RMR   0x54
#define CTRS1 0x55
#define CTRS2 0x56
#define IMAX1 0x57
#define IMAX2 0x58
#define IMAX3 0x59
#define IMAX4 0x5a
#define IMAX5 0x5b
#define SADDR 0x5f
#define WADDR 0x7e
#define WDATA 0x7f

#define SETPWMI 0xA000
#define WAITI 0x3800 // Pre 0=0.5ms, 1=16ms, T time=Pre*T
#define SET_STEP_TMRI 0x8000 // Pre 0=0.5ms, 1=16ms. Ch: LED, Im: step time (Im+1)*Pre
#define RAMPI_IN 0xE000
#define RAMPI_OUT 0xC000

void LightState::WriteAW9120(unsigned int addr, unsigned int reg_data) {
    std::ofstream aw9120_reg;

    aw9120_reg.open("/proc/aw9120_reg");
    aw9120_reg << "0x" << std::hex << addr << " 0x" << std::hex << reg_data << std::endl;
    aw9120_reg.close();
}

void LightState::DisableAW9120() {
    WriteAW9120(GCR, 0);
}

void LightState::EnableAW9120() {
    WriteAW9120(GCR, 0x1);
}

void LightState::SetupAW9120() {
    WriteAW9120(IMAX1, 0x1111);    // IMAX1-LED1~LED4 Current
    WriteAW9120(IMAX2, 0x1111);    // IMAX2-LED5~LED8 Current
    WriteAW9120(IMAX3, 0x1111);    // IMAX3-LED9~LED12 Current
    WriteAW9120(IMAX4, 0x1111);    // IMAX4-LED13~LED16 Current
    WriteAW9120(IMAX5, 0x1111);    // IMAX5-LED17~LED20 Current
    WriteAW9120(LER1, 0x0FFF);    // LER1-LED1~LED12 Enable
    WriteAW9120(LER2, 0x00FF);    // LER2-LED13~LED20 Enable
    WriteAW9120(CTRS1, 0x0000);    // CTRS1-LED1~LED12: SRAM Control
    WriteAW9120(CTRS2, 0x0000);    // CTRS2-LED13~LED20: SRAM Control
}

void LightState::HoldSramAW9120() {
    WriteAW9120(PMR, 0x0000);        // PMR-Load SRAM with I2C
    WriteAW9120(RMR, 0x0000);        // RMR-Hold Mode
}

void LightState::RunSramAW9120() {
    WriteAW9120(SADDR, 0x0000);    // SADDR-SRAM Run Start Addr:0
    WriteAW9120(PMR, 0x0001); // PMR-Reload and Excute SRAM
    WriteAW9120(RMR, 0x0002); // RMR-Run
}

void LightState::ResetSramLoadAddrAW9120() {
    WriteAW9120(WADDR, 0x0000);
    programCounter = 0;
}

void LightState::WriteSramProgAW9120(unsigned int reg_data) {
    if (programCounter < 256) {
        WriteAW9120(WDATA, reg_data);
    }
    programCounter++;
}

void LightState::Update() {
    std::ofstream aw9120_operation;
    log->log(log_tag, "Update");

    DisableAW9120();
    SetupAW9120();
    EnableAW9120();
    HoldSramAW9120();

    //prog - max 256 instructions
    ResetSramLoadAddrAW9120();
    WriteSramProgAW9120(SET_STEP_TMRI + 0x0000 + 0x1f00 + 0x3); //step 2ms
    WriteSramProgAW9120(SETPWMI + 0x1F00 + 0x00); //all off

    if (connectivityCellular) {
        WriteSramProgAW9120(SETPWMI + 0x0f00 + 0xa0);
    }
    if (connectivityWifi) {
        WriteSramProgAW9120(SETPWMI + 0x1000 + 0x50);
    }
    if (connectivityBluetooth) {
        if (connectivityWifi) {
            WriteSramProgAW9120(SETPWMI + 0x1100 + 0xc0);
        } else {
            WriteSramProgAW9120(SETPWMI + 0x1100 + 0xff);
        }
    }
    if (capsLock) {
        WriteSramProgAW9120(SETPWMI + 0x1200 + 0xa0);
    }
    if (powerState) {
        WriteSramProgAW9120(SETPWMI + 0x1300 + 0xff);
    }
    int loopStartPC = programCounter;
    bool delaySet = false;
    for (auto step : steps) {
        if (step.type == BlockStepDelay) {
            //ignore led,colour
            WriteSramProgAW9120(WAITI + 0x400 + step.value);
            delaySet = true;
        } else {
            int awLed = 0;
            if (step.led == 2) {
                awLed = 0xc00;
            } else if (step.led > 2) {
                awLed = ((step.led - 2) * 3) << 8;
            }
            switch (step.colour) {
                case BlockColourBlue:
                    awLed += 0x200;
                    break;
                case BlockColourGreen:
                    awLed += 0x100;
                    break;
                default:
                    break;
            }

            switch (step.type) {
                case BlockStepSetPWM:
                    WriteSramProgAW9120(SETPWMI + awLed + step.value);
                    break;
                case BlockStepFadeIn:
                    WriteSramProgAW9120(RAMPI_IN + awLed + step.value);
                    break;
                case BLockStepFadeOut:
                    WriteSramProgAW9120(RAMPI_OUT + awLed + step.value);
                    break;
                default:
                    break;
            }
        }
    }
    if (!delaySet) {
        WriteSramProgAW9120(WAITI + 0x400 + 0x20);
    }
    WriteSramProgAW9120(loopStartPC);

    RunSramAW9120();
}
