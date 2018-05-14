//
// Created by adam on 12/05/18.
//

#include "TestBase.h"
#include "RunDaemon.h"

TestBase::TestBase(): TestBase{std::make_shared<DaemonConfig>()}
{
    runDaemon();
}

TestBase::TestBase(std::shared_ptr<DaemonConfig> const& daemon_config_ptr)
    : config_ptr{daemon_config_ptr}, config{*daemon_config_ptr} {
}

TestBase::~TestBase() {
    daemon.flush();
    daemon.stop();
    if (daemon_thread.joinable())
        daemon_thread.join();
}

void TestBase::runDaemon() {
    daemon_thread = test::runDaemon(daemon);
}

