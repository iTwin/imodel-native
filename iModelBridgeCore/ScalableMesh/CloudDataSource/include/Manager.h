#pragma once

#include "DataSourceDefs.h"
#include <string>
#include <map>
#include <chrono>
#include <mutex>
#include <functional>

#define    MANAGER_DEFAULT_ACCESS_TIMEOUT    30 * 1000

#define T_MANAGER   template<typename T, bool managePointered>


template<typename T, bool managePointered> class Manager
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

    bool                                            deletePointered;

protected:

    void                                            setDeletePointered      (bool del);
    bool                                            getDeletePointered      (void);

public:

                                                    Manager                 (void);
    virtual                                        ~Manager                 (void);

	virtual void									shutdown				(void);

    T                *                              create                  (const ItemName &name, T *item);
    bool                                            destroy                 (const ItemName &name);
    virtual bool                                    destroy                 (T *item);
    virtual bool                                    destroyAll              (void);

    bool                                            apply                   (ApplyFunction f);

    ItemCount                                       getCount                (void);

    T                *                              get                     (const ItemName &name);
    Iterator                                        getEntry                (const ItemName &name);
    Iterator                                        getEntry                (T *item);

    void                                            setAccessTimeout        (Timeout timeMilliseconds);
    Timeout                                         getAccessTimeout        (void);

};


T_MANAGER
inline Manager<T, managePointered>::Manager(void)
{
    setDeletePointered(managePointered);

    setAccessTimeout(Timeout(MANAGER_DEFAULT_ACCESS_TIMEOUT));
}

T_MANAGER
inline Manager<T, managePointered>::~Manager(void)
{

}

T_MANAGER
inline void Manager<T, managePointered>::setDeletePointered(bool del)
{
    deletePointered = del;
}


T_MANAGER
inline bool Manager<T, managePointered>::getDeletePointered(void)
{
    return deletePointered;
}


T_MANAGER
inline void Manager<T, managePointered>::shutdown(void)
{
	destroyAll();
}

T_MANAGER
inline T * Manager<T, managePointered>::create(const ItemName & name, T * item)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return nullptr;
        
    items[name] = item;

    itemMutex.unlock();

    return item;
}

T_MANAGER
inline bool Manager<T, managePointered>::destroy(const ItemName & name)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;

    Iterator it = getEntry(name);
    if (it != items.end())
    {
        T *i = it->second;

        if (i)
		{
            i->destroyAll();

            if (getDeletePointered())
            {
                delete i;
            }
        }

        items.erase(it);

        itemMutex.unlock();

        return true;
    }

    itemMutex.unlock();

    return false;
}


T_MANAGER
inline bool Manager<T, managePointered>::destroy(T * item)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;

    Iterator it = getEntry(item);

    if (it != items.end())
    {
        T *i = it->second;

        if (i)
        {
            if (getDeletePointered())
            {
                delete i;
            }
        }

        items.erase(it);

        itemMutex.unlock();

        return true;
    }

    itemMutex.unlock();

    return false;
}

T_MANAGER
inline bool Manager<T, managePointered>::destroyAll(void)
{
    Iterator it;

    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;

    for (it = items.begin(); it != items.end(); it++)
    {
        T *i = it->second;

        if (i)
        {
            i->destroyAll();

            if (getDeletePointered())
            {
                delete i;
            }
        }
    }

    items.clear();

    itemMutex.unlock();

    return true;
}

T_MANAGER
bool Manager<T, managePointered>::apply(ApplyFunction f)
{
    bool traverse = true;

    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return false;
                                                            // Iterate over items
    for (auto it = items.begin(); traverse && it != items.end();)
    {
        auto current = it++;
        traverse = f(current);
    }

    itemMutex.unlock();
                                                            // Return whether last item returned to continue traversing
    return traverse;
}


T_MANAGER
inline typename Manager<T, managePointered>::ItemCount Manager<T, managePointered>::getCount(void)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return 0;

    ItemCount numItems = items.count();

    itemMutex.unlock();

    return numItems;
}

T_MANAGER
inline T * Manager<T, managePointered>::get(const ItemName & name)
{
    Iterator it;

    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return nullptr;

    if ((it = getEntry(name)) != items.end())
    {
        itemMutex.unlock();
        return it->second;
    }

    itemMutex.unlock();

    return nullptr;
}

T_MANAGER
inline typename Manager<T, managePointered>::Iterator Manager<T, managePointered>::getEntry(const ItemName &name)
{
    if (itemMutex.try_lock_for(Timeout(getAccessTimeout())) == false)
        return Iterator();

    Iterator it = items.find(name);

    itemMutex.unlock();

    return it;
}

T_MANAGER
inline typename Manager<T, managePointered>::Iterator Manager<T, managePointered>::getEntry(T * item)
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

T_MANAGER
inline void Manager<T, managePointered>::setAccessTimeout(Timeout timeMilliseconds)
{
    accessTimeout = timeMilliseconds;
}

T_MANAGER
inline typename Manager<T, managePointered>::Timeout Manager<T, managePointered>::getAccessTimeout(void)
{
    return accessTimeout;
}





