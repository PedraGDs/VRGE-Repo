
#include "MainThreadRunner.h"
#include <future>

void MainThreadRunner::start( ) {

    if ( std::this_thread::get_id() != this->threadId ) {
        std::cout << "Attempted to call MainThreadRunner::start() from a different thread.";
        return;
    }

    std::queue<std::function<void()>> scheduleCopy;
    std::function<void()> task;
    this->shouldRun = true;

    while ( this->shouldRun ) {

        for ( auto& task : this->continuousTasks ) {
            task();
        }

        this->mtx.lock();

        while ( !scheduledTasks.empty() ) {
            scheduleCopy.push(std::move(scheduledTasks.front()));
            scheduledTasks.pop();
        }

        this->mtx.unlock();

        while ( !scheduleCopy.empty() ) {
            scheduleCopy.front()();
            scheduleCopy.pop();
        }

        std::this_thread::sleep_for(this->sleepTime);
        
    }

}

void MainThreadRunner::runEveryFrame ( std::function<void()> func ) {
    std::lock_guard<std::mutex> lock(mtx);

    if ( this->isRunning ) {
        std::cout << "Attempted to add a continuous function to an already executing thread.";
        return;
    }

    this->continuousTasks.push_back(func);
}

template<typename T>
T MainThreadRunner::scheduleAndWait ( std::function<T()> func ) {

    std::promise<T> done;
    std::future<T> fut = done.get_future();

    this->schedule([&func, &done]() -> void {
        done.set_value(func());
    });

    return fut.get();

}

void MainThreadRunner::schedule ( std::function<void()> func ) {
    std::lock_guard<std::mutex> lock(mtx);
    this->scheduledTasks.push(func);
}

void MainThreadRunner::stop () {
    std::lock_guard<std::mutex> lock(mtx);
    this->shouldRun = false;
}