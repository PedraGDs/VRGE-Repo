
#pragma once

#include <unordered_set>
#include <functional>
#include <iostream>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <queue>

class MainThreadRunner {

    private:
        std::chrono::duration<double> sleepTime { 1.0 / 120.0 };
        std::vector<std::function<void()>> continuousTasks{};
        std::queue<std::function<void()>> scheduledTasks{};
        std::unordered_set<std::thread*> childThreads{};
        std::atomic<bool> isShuttingDown = false;
        std::atomic<bool> isRunning = false;
        std::thread::id threadId;
        std::mutex mtx{};

        void waitForChildren();

    public:
        MainThreadRunner ( ) {
            this->threadId = std::this_thread::get_id();
        }

        void addRepeating ( std::function<void()> func );
        void schedule ( std::function<void()> func );

        void start ();
        void stop ();

        void addChild (std::thread* child);
        void removeChild (std::thread* child);

        template<typename T> 
        T scheduleAndWait ( std::function<T ()> func ) {

            std::promise<T> done;
            std::future<T> fut = done.get_future();

            this->schedule([&func, &done]() -> void {
                if constexpr (std::is_void_v<T>) {
                    func();
                    done.set_value();
                } 
                
                else {
                    done.set_value(func());
                }
            });

            return fut.get();
        }

};

extern MainThreadRunner* mainThreadRunner; // gets initialized on the main function.
