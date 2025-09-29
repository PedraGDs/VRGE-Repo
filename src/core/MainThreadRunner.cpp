
#include "MainThreadRunner.h"

void MainThreadRunner::waitForChildren ( ) {

    std::cout << "Waiting for child threads to shutdown" << std::endl;
    std::unique_lock<std::mutex> lock(this->mtx);

    while ( !this->childThreads.empty() ) {
        
        auto thread = *this->childThreads.begin();
        this->mtx.unlock();

        thread->join();

        this->mtx.lock();
        this->childThreads.erase(thread);
    
    }

}

void MainThreadRunner::start( ) {

    if ( std::this_thread::get_id() != this->threadId ) {
        std::cout << "Attempted to call MainThreadRunner::start() from a different thread.";
        return;
    }

    this->mtx.lock();
    
    std::queue<std::function<void()>> scheduleCopy{};
    std::function<void()> task;

    this->isShuttingDown = false;
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
            task = scheduleCopy.front();
            scheduleCopy.pop();
            task();
        }

        std::this_thread::sleep_for(this->sleepTime);
        
    }

    this->waitForChildren(); // wait for global shutdown
    
}

void MainThreadRunner::addRepeating ( std::function<void()> func ) {
    std::lock_guard<std::mutex> lock(this->mtx);

    if ( this->isRunning ) {
        std::cout << "Attempted to add a continuous function to an already executing thread.";
    }
    
    else {
        this->continuousTasks.push_back(func);
    }

}

void MainThreadRunner::schedule ( std::function<void()> func ) {

    this->mtx.lock();

    if ( this->isShuttingDown ) {
        std::cout << "Unable to schedule changes to MainThread during shutdown" << std::endl;
        return;
    }

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
    this->childThreads.insert(child);
}

void MainThreadRunner::removeChild (std::thread* child) {
    std::lock_guard<std::mutex> lock(mtx);
    this->childThreads.erase(child);
}

void MainThreadRunner::stop () {
    std::lock_guard<std::mutex> lock(mtx);
    this->isShuttingDown = true;
    this->isRunning = false;
}