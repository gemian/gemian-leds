//
// Created by adam on 12/05/18.
//

#ifndef GEMIAN_LEDS_TESTBASE_H
#define GEMIAN_LEDS_TESTBASE_H


#include "../src/core/DaemonConfig.h"
#include "../src/core/Daemon.h"

struct TestBase {

    TestBase(std::shared_ptr<DaemonConfig> const& daemon_config);
    TestBase();
    ~TestBase();

    void runDaemon();

    std::shared_ptr<DaemonConfig> config_ptr;
    DaemonConfig& config;
    Daemon daemon{config};
    std::thread daemon_thread;

};


#endif //GEMIAN_LEDS_TESTBASE_H
