/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include <future>
#include "Daemon.h"

char const* const log_tag = "Daemon";

Daemon::Daemon(DaemonConfig& config)
        : the_log{config.the_log()},
          leds{config.the_leds()},
          connectionWatcher{config.the_connectionWatcher()},
          lightState{config.the_lightState()},
          running{false}
{
}

void Daemon::run() {
    auto const registrations = register_event_handlers();
    start_event_processing();

    running = true;

    while (running)
    {
        auto const ev = dequeue_action();
        ev();
    }
}

void Daemon::stop() {
    enqueue_priority_action([this] { running = false; });
}

void Daemon::flush() {
    std::promise<void> flushed_promise;
    auto flushed_future = flushed_promise.get_future();

    enqueue_action([&flushed_promise] { flushed_promise.set_value(); });

    flushed_future.wait();
}

std::vector<HandlerRegistration> Daemon::register_event_handlers() {
    std::vector<HandlerRegistration> registrations;

    registrations.push_back(
            leds->registerLEDsCapsLockHandler(
                    [this] (bool state)
                    {
                        enqueue_action(
                                [this, state] {
                                    lightState->handleCapsLock(state);
                                });
                    }));
    registrations.push_back(
            leds->registerLEDsBlockHandler(
                    [this](int led, BlockColour colour, BlockStepType type, unsigned int value)
                    {
                        enqueue_action(
                                [this, led, colour, type, value] {
                                    lightState->handleSetBlockRGB(led, colour, type, value);
                                });
                    }));
    registrations.push_back(
            leds->registerLEDsClearBlockHandler(
                    [this]()
                    {
                        enqueue_action(
                                [this] {
                                    lightState->handleClearBlock();
                                });
                    }));
    registrations.push_back(
            leds->registerLEDsPushBlockHandler(
                    [this]()
                    {
                        enqueue_action(
                                [this] {
                                    lightState->handlePushBlock();
                                });
                    }));
    registrations.push_back(
            leds->registerLEDsTorchHandler(
                    [this](bool on)
                    {
                        enqueue_action(
                                [this, on] {
                                    lightState->handleTorch(on);
                                });
                    }));
    registrations.push_back(
            leds->registerLEDsCallHandler(
                    [this](bool earpiece, bool leftUp)
                    {
                        enqueue_action(
                                [this, earpiece, leftUp] {
                                    lightState->handleCall(earpiece, leftUp);
                                });
                    }));
    return  registrations;
}

void Daemon::start_event_processing() {
    leds->start_processing();
}

void Daemon::enqueue_action(const Daemon::Action &action) {
    std::lock_guard<std::mutex> lock{action_queue_mutex};

    action_queue.push_back(action);
    action_queue_cv.notify_one();
}

void Daemon::enqueue_priority_action(const Daemon::Action &action) {
    std::lock_guard<std::mutex> lock{action_queue_mutex};

    action_queue.push_front(action);
    action_queue_cv.notify_one();
}

Daemon::Action Daemon::dequeue_action() {
    std::unique_lock<std::mutex> lock{action_queue_mutex};

    action_queue_cv.wait(lock, [this] { return !action_queue.empty(); });
    auto ev = action_queue.front();
    action_queue.pop_front();
    return ev;
}
