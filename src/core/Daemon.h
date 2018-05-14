#pragma once

#include <vector>
#include <deque>
#include "DaemonConfig.h"
#include "HandlerRegistration.h"
#include "LEDs.h"
#include "LightState.h"

class Daemon {

public:
    explicit Daemon(DaemonConfig& config);

    void run();
    void stop();
    void flush();

private:

    using Action = std::function<void()>;

    std::vector<HandlerRegistration> register_event_handlers();
    void start_event_processing();
    void enqueue_action(Action const& action);
    void enqueue_priority_action(Action const& action);
    Action dequeue_action();

    std::shared_ptr<Log> const the_log;
    std::shared_ptr<LEDs> const leds;
    std::shared_ptr<LightState> const lightState;

    bool running;

    std::mutex action_queue_mutex;
    std::condition_variable action_queue_cv;
    std::deque<Action> action_queue;

};
