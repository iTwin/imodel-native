#include "stdafx.h"
#include <iostream>
#include "ActivitySemaphore.h"

ActivitySemaphore::ActivitySemaphore(void)
{
    setCounter(0);
}

ActivitySemaphore::ActivitySemaphore(CounterValue value)
{
    setCounter(value);
}

ActivitySemaphore::~ActivitySemaphore(void)
{
    //std::unique_lock<std::mutex> lock(mutex);
                                                            // Release all waiting threads
    condition.notify_all();
}

void ActivitySemaphore::setCounter(CounterValue value)
{
    counter = value;
}

ActivitySemaphore::CounterValue ActivitySemaphore::getCounter(void)
{
    return counter;
}

void ActivitySemaphore::increment(void)
{
                                                            // Atomic increment
    counter.fetch_add(1, std::memory_order_relaxed);
}

bool ActivitySemaphore::decrement(void)
{
    CounterValue    previousValue;
                                                            // Atomic decrement on counter    
    previousValue = counter.fetch_sub(1, std::memory_order_relaxed);
                                                            // If counter value is greater than zero (previously greater than 1)
    if(previousValue > 1)
    {
                                                            // Return not notified
        return false;
    }
                                                            // Notify all threads waiting for the zero counter condition
    condition.notify_all();
                                                            // Return notified
    return true;
}

void ActivitySemaphore::wait(void)
{
    std::unique_lock<std::mutex>    lock(mutex);
                                                            // Wait for counter zero condition indefinitely. Note: This is not good practice! Try timeout verison instead.
    condition.wait(lock);
}

ActivitySemaphore::Status ActivitySemaphore::waitFor(Timeout timeout)
{
    std::unique_lock<std::mutex>    lock(mutex);
                                                            // Wait for counter zero condition with specified timeout
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();

    auto cv_wakeup_status = condition.wait_for(lock, timeout);

    end = std::chrono::system_clock::now();
    std::chrono::duration<double> waiting_time = end - start;
    std::cout << "Activity semaphore waited: " << waiting_time.count() << std::endl;
    if (cv_wakeup_status == std::cv_status::no_timeout)
    {
                                                            // Successfully waited, with no timeout
        return Status_NoTimeout;
    }

                                                            // Timeout occured
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


