
#include "MainThreadRunner.h"

void MainThreadRunner::start( ) {

    if ( std::this_thread::get_id() != this->threadId ) {
        std::cout << "Attempted to call MainThreadRunner::start() from a different thread.";
        return;
    }

    this->mtx.lock();
    
    std::queue<std::function<void()>> scheduleCopy;
    std::function<void()> task;
    this->isRunning = true;

    this->mtx.unlock();

    while ( this->isRunning || !this->scheduledTasks.empty() ) {

        for ( auto& task : this->continuousTasks ) {
            task();
        }

        if ( scheduledTasks.empty() ) {
            continue;
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

    std::cout << "Waiting for child threads to shutdown" << std::endl;

    for ( auto thread : this->childThreads ) {
        thread->join();
    }

}

void MainThreadRunner::addRepeating ( std::function<void()> func ) {
    std::lock_guard<std::mutex> lock(this->mtx);

    if ( this->isRunning ) {
        std::cout << "Attempted to add a continuous function to an already executing thread.";
    } else {
        this->continuousTasks.push_back(func);
    }
}

void MainThreadRunner::schedule ( std::function<void()> func ) {

    this->mtx.lock();

    if ( std::this_thread::get_id() != this->threadId ) {
        this->scheduledTasks.push(func);
        this->mtx.unlock();
    } 
    
    else {
        this->mtx.unlock();
        func();
    }
}

void MainThreadRunner::addChild (std::thread* child) {
    std::lock_guard<std::mutex> lock(mtx);
    this->childThreads.push_back(child);
}

void MainThreadRunner::stop () {
    std::lock_guard<std::mutex> lock(mtx);
    this->isRunning = false;
}