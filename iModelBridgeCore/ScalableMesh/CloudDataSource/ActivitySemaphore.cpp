#include "stdafx.h"
#include <iostream>
#include "ActivitySemaphore.h"

ActivitySemaphore::ActivitySemaphore(void)
{
    setCounter(0);
}

ActivitySemaphore::ActivitySemaphore(Counter value)
{
    //std::unique_lock<std::mutex> lock(mutex);

    setCounter(value);
}

ActivitySemaphore::~ActivitySemaphore(void)
{
    //std::unique_lock<std::mutex> lock(mutex);
                                                            // Release all waiting threads
    condition.notify_all();
}

void ActivitySemaphore::setCounter(Counter value)
{
    //std::unique_lock<std::mutex> lock(mutex);

    counter = value;
}

ActivitySemaphore::Counter ActivitySemaphore::getCounter(void)
{
    //std::unique_lock<std::mutex> lock(mutex);

    return counter;
}

void ActivitySemaphore::increment(void)
{
    //std::unique_lock<std::mutex> lock(mutex);

    setCounter(getCounter() + 1);
}

bool ActivitySemaphore::decrement(void)
{
    //std::unique_lock<std::mutex> lock(mutex);

    setCounter(getCounter() - 1);

    if(counter > 0)
    {
        return false;
    }

    condition.notify_all();

    return true;
}

void ActivitySemaphore::wait(void)
{
    std::unique_lock<std::mutex>    lock(mutex);

    condition.wait(lock);
}

ActivitySemaphore::Status ActivitySemaphore::waitFor(Timeout timeout)
{
    std::unique_lock<std::mutex>    lock(mutex);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    auto cv_wakeup_status = condition.wait_for(lock, timeout);

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> waiting_time = end - start;
    std::cout << "Activity semaphore waited: " << waiting_time.count() << std::endl;
    if (cv_wakeup_status == std::cv_status::no_timeout)
        return Status_NoTimeout;

    return Status_Timeout;
}

void ActivitySemaphore::releaseAll(void)
{
    //std::unique_lock<std::mutex> lock(mutex);
                                                            // Set counter back to zero
    setCounter(0);
                                                            // Notify all blocked threads
    condition.notify_all();
}


