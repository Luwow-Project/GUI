#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>

namespace Luwow::Gui {

// Work another thread hands to the main thread's message loop, then waits on. If the loop ends before
// taking it, the waiter gives up and the work is skipped, whenever the message would still arrive.
class MainThreadCall {
public:
    explicit MainThreadCall(const std::function<void()>& work) : work(work) {}

    // On the main thread
    void run() {
        {
            std::lock_guard<std::mutex> guard(mutex);
            if (abandoned) return;
            started = true;
        }
        work();
        {
            std::lock_guard<std::mutex> guard(mutex);
            finished = true;
        }
        done.notify_all();
    }

    // On the calling thread, false if the loop ended without taking the work
    bool wait(const std::atomic<bool>& loopEnded) {
        std::unique_lock<std::mutex> guard(mutex);
        while (!finished) {
            if (!started && loopEnded.load()) {
                abandoned = true;
                return false;
            }
            done.wait_for(guard, std::chrono::milliseconds(50));
        }
        return true;
    }

private:
    const std::function<void()>& work;
    std::mutex mutex;
    std::condition_variable done;
    bool started = false;
    bool finished = false;
    bool abandoned = false;
};

} // namespace Luwow::Gui
