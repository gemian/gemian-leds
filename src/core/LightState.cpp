//
// Created by adam on 12/05/18.
//

#include <fstream>
#include <memory>
#include "LightState.h"
#include "Log.h"

char const *const log_tag = "LightState";

static const uint64_t POWER_LEVEL_20b001s = 0x249249249249249;
static const uint64_t POWER_LEVEL_20b111s = 0xfffffffffffffff;

LightState::LightState(std::shared_ptr<Log> const &the_log) : log{the_log} {
    handleClearBlock();
    powerLevels = POWER_LEVEL_20b001s;
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

void LightState::handleTorch(bool on) {
    powerLevels = on ? POWER_LEVEL_20b111s : POWER_LEVEL_20b001s;
    handleClearBlock();
    if (on) {
        steps.emplace_back(0x1f, BlockColourRed, BlockStepSetPWM, 0xff);
    }
    handlePushBlock();

    Update();
}

void LightState::handleCall(bool earpiece, bool leftUp) {
    callEarpiece = earpiece;
    callLeftUp = leftUp;
    Update();
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
#define WAIT_PRE_05MS 0x000
#define WAIT_PRE_16MS 0x400
#define MAX_WAIT 0x3ff
#define SET_STEP_TMRI 0x8000 // Pre 0=0.5ms, 1=16ms. Ch: LED, Im: step time (Im+1)*Pre
#define SET_STEP_PRE_05MS 0x00
#define SET_STEP_PRE_16MS 0x80
#define RAMPI_IN 0xE000
#define RAMPI_OUT 0xC000
#define CH_ALL_LEDS 0x1f00
#define CH_6_RED 0x0f00
#define CH_6_GREEN 0x1000
#define CH_6_BLUE 0x1100
#define CH_7_RED 0x1200
#define CH_7_BLUE 0x1300

void LightState::WriteAW9120(unsigned int addr, unsigned int reg_data) {
    std::ofstream aw9120_reg;

    aw9120_reg.open("/proc/aw9120_reg");
    aw9120_reg << "0x" << std::hex << addr << " 0x" << std::hex << reg_data << std::endl;
    aw9120_reg.close();
}

void LightState::WriteClassLEDGreen(bool on) {
    std::ofstream led_green;

    led_green.open("/sys/class/leds/green/brightness");
    led_green << on << std::endl;
    led_green.close();
}

void LightState::WriteClassLEDRed(bool on) {
    std::ofstream led_red;

    led_red.open("/sys/class/leds/red/brightness");
    led_red << on << std::endl;
    led_red.close();
}

void LightState::DisableAW9120() {
    WriteAW9120(GCR, 0);
}

void LightState::EnableAW9120() {
    WriteAW9120(GCR, 0x1);
}

void LightState::SetupAW9120() {
    WriteAW9120(IMAX1, maxLED(4) + maxLED(3) + maxLED(2) + maxLED(1));    // IMAX1-LED1~LED4 Current
    WriteAW9120(IMAX2, maxLED(8) + maxLED(7) + maxLED(6) + maxLED(5));    // IMAX2-LED5~LED8 Current
    WriteAW9120(IMAX3, maxLED(12) + maxLED(11) + maxLED(10) + maxLED(9));    // IMAX3-LED9~LED12 Current
    WriteAW9120(IMAX4, maxLED(16) + maxLED(15) + maxLED(14) + maxLED(13));    // IMAX4-LED13~LED16 Current
    WriteAW9120(IMAX5, maxLED(20) + maxLED(19) + maxLED(18) + maxLED(17));    // IMAX5-LED17~LED20 Current
    WriteAW9120(LER1, 0x0FFF);    // LER1-LED1~LED12 Enable
    WriteAW9120(LER2, 0x00FF);    // LER2-LED13~LED20 Enable
    WriteAW9120(CTRS1, 0x0000);    // CTRS1-LED1~LED12: SRAM Control
    WriteAW9120(CTRS2, 0x0000);    // CTRS2-LED13~LED20: SRAM Control
}

unsigned int LightState::maxLED(int led) const {
    return static_cast<unsigned int>(((powerLevels >> ((led - 1) * 3)) & 0x7) << (led % 4) * 4);
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
    WriteSramProgAW9120(SET_STEP_TMRI + CH_ALL_LEDS + SET_STEP_PRE_05MS + 0x3); //step 2ms
    WriteSramProgAW9120(SETPWMI + CH_ALL_LEDS + 0x00); //all off

    if (callEarpiece) {
        if (callLeftUp) {
            WriteSramProgAW9120(SETPWMI + CH_6_RED + 0xa0);
            WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x00);
            WriteClassLEDRed(false);
            WriteClassLEDGreen(true);
        } else {
            WriteSramProgAW9120(SETPWMI + CH_6_RED + 0x00);
            WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x50);
            WriteClassLEDRed(true);
            WriteClassLEDGreen(false);
        }
    } else {
        WriteClassLEDRed(false);
        WriteClassLEDGreen(false);
        if (connectivityCellular) {
            WriteSramProgAW9120(SETPWMI + CH_6_RED + 0xa0);
        }
        if (connectivityWifi) {
            WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x50);
        }
        if (connectivityBluetooth) {
            if (connectivityWifi) {
                WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0xc0);
            } else {
                WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0xff);
            }
        }
    }
    if (capsLock) {
        WriteSramProgAW9120(SETPWMI + CH_7_RED + 0xa0);
    }
    if (powerState) {
        WriteSramProgAW9120(SETPWMI + CH_7_BLUE + 0xff);
    }
    unsigned int loopStartPC = programCounter;
    bool delaySet = false;
    for (auto step : steps) {
        if (step.type == BlockStepDelay) {
            //ignore led,colour
            WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + step.value);
            delaySet = true;
        } else {
            int awLed = AWLEDCodeForStep(step);

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
<<<<<<< HEAD
        WriteSramProgAW9120(SETPWMI + CH_6_RED + 0x00);
        WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x00);
        WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0x00);
        if (connectivityCellular) {
            WriteSramProgAW9120(SETPWMI + CH_6_RED + 0xa0);
            WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + 0x80);
            WriteSramProgAW9120(SETPWMI + CH_6_RED + 0x00);
        }
        if (connectivityWifi) {
            WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x50);
            WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + 0x80);
            WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x00);
        }
        if (connectivityBluetooth) {
            WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0xff);
            WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + 0x80);
            WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0x00);
        }
        if (!connectivityCellular && !connectivityWifi && !connectivityBluetooth) {
            WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + MAX_WAIT);
=======
        if (callEarpiece) {
            WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + MAX_WAIT);
        } else {
            WriteSramProgAW9120(SETPWMI + CH_6_RED + 0x00);
            WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x00);
            WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0x00);
            if (connectivityCellular) {
                WriteSramProgAW9120(SETPWMI + CH_6_RED + 0xa0);
                WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + 0x80);
                WriteSramProgAW9120(SETPWMI + CH_6_RED + 0x00);
            }
            if (connectivityWifi) {
                WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x50);
                WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + 0x80);
                WriteSramProgAW9120(SETPWMI + CH_6_GREEN + 0x00);
            }
            if (connectivityBluetooth) {
                WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0xff);
                WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + 0x80);
                WriteSramProgAW9120(SETPWMI + CH_6_BLUE + 0x00);
            }
            if (!connectivityCellular && !connectivityWifi && !connectivityBluetooth) {
                WriteSramProgAW9120(WAITI + WAIT_PRE_16MS + MAX_WAIT);
            }
>>>>>>> master
        }
    }
    WriteSramProgAW9120(loopStartPC);

    RunSramAW9120();
}

int LightState::AWLEDCodeForStep(const BlockAnimStep &step) const {
    int awLed = 0;
    if (step.led == 2) {
        awLed = 0xc00;
    } else if (step.led > 2 && step.led < 6) {
        awLed = ((step.led - 2) * 3) << 8;
    } else if (step.led == 0x1f && step.colour == BlockColourRed) {
        awLed = CH_ALL_LEDS;
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
    return awLed;
}
