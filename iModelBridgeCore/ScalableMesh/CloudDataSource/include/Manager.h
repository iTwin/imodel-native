#pragma once

#include "DataSourceDefs.h"
#include <string>
#include <map>
#include <chrono>
#include <mutex>
#include <functional>

#define    MANAGER_DEFAULT_ACCESS_TIMEOUT    30 * 1000


template<typename T> class Manager
{

public:

    typedef std::wstring                                        ItemName;
    typedef unsigned int                                        ItemCount;

protected:

    typedef std::map<ItemName, T *>                             ItemMap;

public:

    typedef std::function<bool(typename ItemMap::iterator)>     ApplyFunction;

    typedef typename ItemMap::iterator                          Iterator;
    typedef typename std::chrono::milliseconds                  Timeout;

protected:

    ItemMap                                         items;

    std::recursive_timed_mutex                      itemMutex;
    Timeout                                         accessTimeout;

protected:

protected:

                                                    Manager                 (void);
    virtual                                        ~Manager                 (void);

    T                *                              create                  (const ItemName &name, T *item);
    bool                                            destroy                 (const ItemName &name, bool deleteItem);
    bool                                            destroy                 (T *item, bool deleteItem);
    bool                                            destroyAll              (bool deleteItems);

    bool                                            apply                   (ApplyFunction f);

    ItemCount                                       getCount                (void);

    T                *                              get                     (const ItemName &name);
    Iterator                                        getEntry                (const ItemName &name);
    Iterator                                        getEntry                (T *item);

    void                                            setAccessTimeout        (Timeout timeMilliseconds);
    Timeout                                         getAccessTimeout        (void);

};


template<typename T>
inline Manager<T>::Manager(void)
{
    setAccessTimeout(Timeout(MANAGER_DEFAULT_ACCESS_TIMEOUT));
}

template<typename T>
inline Manager<T>::~Manager(void)
{

}

template<typename T>
inline T * Manager<T>::create(const ItemName & name, T * item)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return nullptr;
        
    items[name] = item;

    itemMutex.unlock();

    return item;
}

template<typename T>
inline bool Manager<T>::destroy(const ItemName & name, bool deleteItem)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;

    Iterator it = getEntry(name);
    if (it != items.end())
    {
        if (deleteItem && it->second)
        {
            delete it->second;
        }

        items.erase(it);

        itemMutex.unlock();

        return true;
    }

    itemMutex.unlock();

    return false;
}


template<typename T>
inline bool Manager<T>::destroy(T * item, bool deleteItem)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;

    Iterator it = getEntry(item);

    if (it != items.end())
    {
        if (deleteItem && it->second)
        {
            delete it->second;
        }

        items.erase(it);

        itemMutex.unlock();

        return true;
    }

    itemMutex.unlock();

    return false;
}

template<typename T>
inline bool Manager<T>::destroyAll(bool deleteItems)
{
    Iterator it;

    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;

    for (it = items.begin(); it != items.end(); it++)
    {
        if (deleteItems && it->second)
        {
            delete it->second;
        }
    }

    items.clear();

    itemMutex.unlock();

    return false;
}

template<typename T>
bool typename Manager<T>::apply(ApplyFunction f)
{
    bool traverse = true;

    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;
                                                            // Iterate over items
    for (auto it = items.begin(); traverse && it != items.end(); it++)
    {
        traverse = f(it);
    }

    itemMutex.unlock();
                                                            // Return whether last item returned to continue traversing
    return traverse;
}


template<typename T>
inline typename Manager<T>::ItemCount Manager<T>::getCount(void)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return 0;

    ItemCount numItems = items.count();

    itemMutex.unlock();

    return numItems;
}

template<typename T>
inline T * Manager<T>::get(const ItemName & name)
{
    Iterator it;

    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;

    if ((it = getEntry(name)) != items.end())
    {
        itemMutex.unlock();
        return it->second;
    }

    itemMutex.unlock();

    return nullptr;
}

template<typename T>
inline typename Manager<T>::Iterator Manager<T>::getEntry(const ItemName &name)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return Iterator();

    Iterator it = items.find(name);

    itemMutex.unlock();

    return it;
}

template<typename T>
inline typename Manager<T>::Iterator Manager<T>::getEntry(T * item)
{
    Iterator it;

    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return Iterator();

    if (item == nullptr)
    {
        it = items.end();
        itemMutex.unlock();
        return it;
    }

    for (it = items.begin(); it != items.end() && it->second != item; it++)
    {
    }

    itemMutex.unlock();

    return it;
}

template<typename T>
inline void Manager<T>::setAccessTimeout(Timeout timeMilliseconds)
{
    accessTimeout = timeMilliseconds;
}

template<typename T>
inline typename Manager<T>::Timeout Manager<T>::getAccessTimeout(void)
{
    return accessTimeout;
}





