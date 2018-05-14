
#include <signal.h>
#include <memory.h>
#include "core/Daemon.h"
#include "core/Log.h"
#include "core/DaemonConfig.h"
#include "config.h"

char const* const log_tag = "main";

struct SignalHandler {

    SignalHandler(Daemon* daemon_ptr, Log* log_ptr) {
        SignalHandler::daemon_ptr = daemon_ptr;
        SignalHandler::log_ptr = log_ptr;

        struct sigaction new_action;
        new_action.sa_handler = stop_daemon;
        new_action.sa_flags = 0;
        sigfillset(&new_action.sa_mask);

        sigaction(SIGINT, &new_action, &old_sigint_action);
        sigaction(SIGTERM, &new_action, &old_sigterm_action);
    }

    ~SignalHandler() {
        sigaction(SIGINT, &old_sigint_action, nullptr);
        sigaction(SIGTERM, &old_sigterm_action, nullptr);
    }

    static void stop_daemon(int sig) {
        log_ptr->log(log_tag, "Received signal %s", strsignal(sig));
        daemon_ptr->stop();
    }

    static Daemon* daemon_ptr;
    static Log* log_ptr;

    struct sigaction old_sigint_action;
    struct sigaction old_sigterm_action;
};

Daemon* SignalHandler::daemon_ptr{nullptr};
Log* SignalHandler::log_ptr{nullptr};

int main() {

    DaemonConfig config;
    auto const log = config.the_log();

    log->log(log_tag, "Starting %s", PROJECTNAME);

    Daemon daemon{config};
    SignalHandler signal_handler{&daemon, log.get()};

    daemon.run();

    log->log(log_tag, "Exiting %s", PROJECTNAME);
}

