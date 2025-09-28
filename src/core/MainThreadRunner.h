
#pragma once

#include <functional>
#include <iostream>
#include <thread>
#include <chrono>
#include <mutex>
#include <queue>

static MainThreadRunner* mainThreadRunner = nullptr; // gets initialized on the main function.

class MainThreadRunner {

    private:
        std::chrono::duration<double> sleepTime { 1.0 / 120.0 };
        std::thread::id threadId = std::this_thread::get_id();
        std::vector<std::function<void()>> continuousTasks;
        std::queue<std::function<void()>> scheduledTasks;
        bool isRunning = false;
        bool shouldRun = true;
        std::mutex mtx;

    public:
        MainThreadRunner ( ) { }

        void runEveryFrame ( std::function<void()> func );
        void schedule ( std::function<void()> func );

        template<typename T>
        T scheduleAndWait ( std::function<T()> func );

        void start ();
        void stop ();

};
