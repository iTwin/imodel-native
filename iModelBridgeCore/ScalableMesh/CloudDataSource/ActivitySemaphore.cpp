#include "stdafx.h"
#include "ActivitySemaphore.h"

ActivitySemaphore::ActivitySemaphore(void)
{
    setCounter(0);
}

ActivitySemaphore::ActivitySemaphore(Counter value)
{
    std::unique_lock<std::mutex> lock(mutex);

    setCounter(value);
}

ActivitySemaphore::~ActivitySemaphore(void)
{
    std::unique_lock<std::mutex> lock(mutex);
                                                            // Release all waiting threads
    condition.notify_all();
}

void ActivitySemaphore::setCounter(Counter value)
{
    std::unique_lock<std::mutex> lock(mutex);

    counter = value;
}

ActivitySemaphore::Counter ActivitySemaphore::getCounter(void)
{
    std::unique_lock<std::mutex> lock(mutex);

    return counter;
}

void ActivitySemaphore::increment(void)
{
    std::unique_lock<std::mutex> lock(mutex);

    setCounter(getCounter() + 1);
}

bool ActivitySemaphore::decrement(void)
{
    std::unique_lock<std::mutex> lock(mutex);

//    assert(counter > 0);

    counter--;

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

    if (condition.wait_for(lock, timeout) == std::cv_status::no_timeout)
        return Status_NoTimeout;

    return Status_Timeout;
}




