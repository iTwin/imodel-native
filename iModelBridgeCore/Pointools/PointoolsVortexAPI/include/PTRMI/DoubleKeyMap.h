#pragma once

#include <map>
#include <PTRMI/Name.h>
#include <PTRMI/URL.h>
#include <PTRMI/GUID.h>
#include <PTRMI/Mutex.h>
#include <PTRMI/Status.h>

namespace PTRMI
{

#define PTRMI_DOUBLE_MAP_TIMEOUT				1000 * 60
#define PTRMI_DOUBLE_MAP_DELETE_LOCK_TIMEOUT	1000 * 5
#define PTRMI_DOUBLE_MAP_LOCK_NUM_RETRIES		10
#define PTRMI_DOUBLE_MAP_LOCK_SLEEP				1000

template<typename KA, typename KB, typename T> class DoubleKeyMap
{

protected:

	typedef std::map<KA, T *>			KeyAMap;
	typedef std::map<KB, T *>			KeyBMap;

	typedef ObjectLockManager<T>		ItemLockManager;

public:

	typedef KA							KeyA;
	typedef KB							KeyB;
	typedef T							Type;

	typedef DoubleKeyMap<KA, KB, T>		thisType;

protected:

	Mutex				mutex;

	KeyAMap				keyAMap;
	KeyBMap				keyBMap;

	ItemLockManager		itemLockManager;

	unsigned int		numItems;

protected:

	template<typename M, typename K> T *getItem(M &map, K &key)
	{
		MutexScope	mutexScope(mutex);

		typename M::iterator	it;

		if((it = map.find(key)) != map.end())
		{
			return it->second;
		}

		return NULL;
	}

	template<typename M, typename K> T *removeIndex(M &map, K &key)
	{
        typename M::iterator			it;
		T				*	item = NULL;
															// Look up item and erase it's map entry
		MutexScope	mutexScope(mutex);

		if((it = map.find(key)) != map.end())
		{
			item = it->second;

			map.erase(it);
		}
															// Return item for which index was removed or NULL if not found
		return item;
	}

	template<typename M, typename K> Status newItem(M &map, const K &key, T **preLockedItem = NULL)
	{
		MutexScope	mutexScope(mutex);

		T		*	item;
		Status		status;
		bool		preLock = false;

		if(preLockedItem)
		{
			*preLockedItem = NULL;

			preLock = true;
		}

		if(getItem(map, key) != NULL)
		{
			return Status();
		}

		if((item = new T(key)) != NULL)
		{
			map[key] = item;
		}
		else
		{
			return Status(Status::Status_Error_Memory_Allocation);
		}
															// Create a new lock
		if((status = itemLockManager.newLock(item, PTRMI_DOUBLE_MAP_TIMEOUT, &mutex, preLock)).isFailed())
		{
			removeIndex(map, key);

			delete item;
		}
		else
		{
															// If pre-locking item, return pointer to the locked item
			if(preLockedItem)
			{
				*preLockedItem = item;
			}
		}

		setNumItems(getNumItems() + 1);

		return Status();
	}

	template<typename M, typename K> const K *getKey(M &map, T *item)
	{
        typename M::iterator it;

		MutexScope	mutexScope(mutex);

		for(it = map.begin(); it != map.end(); it++)
		{
			if(it->second == item)
			{
				return &(it->first);
			}
		}

		return NULL;
	}

	template<typename M, typename K, typename MS> Status deleteItem(M &map, const K &key, MS &mapS, bool preLocked = false)
	{
		T					*	item;
		Status					status;
		const MS::key_type	*	keyS;

		if((item = getItem(map, key)) == NULL)
		{
			return Status(Status::Status_Error_Failed);
		}

		if((status = itemLockManager.deleteLock(item, preLocked, PTRMI_DOUBLE_MAP_DELETE_LOCK_TIMEOUT, &mutex, PTRMI_DOUBLE_MAP_LOCK_NUM_RETRIES, PTRMI_DOUBLE_MAP_LOCK_SLEEP)).isFailed())
		{
			return status;
		}

		if(item == NULL)
		{
			return Status(Status::Status_Error_Failed);
		}

		MutexScope mutexScope(mutex);
															// Remove entry
		removeIndex(map, key);
															// Look up item's key in other map to see if it's also present there
		if(keyS = getKey<MS, MS::key_type>(mapS, item))
		{
															// Remove index if present
			removeIndex(mapS, *keyS);
		}
															// Delete the item
		delete item;

		setNumItems(getNumItems() - 1);
															// Return status
		return status;
	}

	template<typename M> unsigned int getNumItems(M &map)
	{
		MutexScope	mutexScope(mutex);

		return map.size();
	}

	template<typename M, typename K, typename MS, typename KS> Status addIndex(M &map, const K &existingKey, MS &mapS, const KS &newKey)
	{
		T *item;

		MutexScope	mutexScope(mutex);
															// Get existing item		
		if((item = getItem(map, existingKey)) == NULL)
		{
			return Status(Status::Status_Error_Failed);
		}
															// Add a new index to the item based on the second key
		mapS[newKey] = item;
															// Return OK
		return Status();
	}

	template<typename M, typename K> bool getFirstKey(M &map, K &key)
	{
		typename M::iterator	it;

		MutexScope	mutexScope(mutex);

		if((it = map.begin()) != map.end())
		{
			key = it->first;
			return true;
		}

		return false;
	}

	template<typename M, typename K> Status deleteAll(M &map)
	{
		Status		status;
		K			key;
		bool		terminate = false;
															// Always get first key to delete as next															
		while(getFirstKey(map, key))
		{
															// Delete item indexed by key and secondary key if present
			if((status = deleteItem(key)).isFailed())
			{
				return status;
			}
		}
															// Return OK
		return status;
	}

	template<typename M, typename K> T *lockItem(M &map, K &key, bool *itemFound, unsigned int timeOut = PTRMI_DOUBLE_MAP_TIMEOUT)
	{
		T	*	item;

		if(itemFound)
		{
			*itemFound = false;
		}

		if(item = getItem(map, key))
		{
			if(itemFound)
			{
				*itemFound = true;
			}

			if(itemLockManager.lock(item, timeOut, &mutex).isOK())
			{

				return item;
			}
		}

		return NULL;
	}

	template<typename M, typename K> Status releaseItem(M &map, K &key, bool *found = NULL)
	{
		T	*	item;

		if(item = getItem(map, key))
		{
			if(found)
			{
				*found = true;
			}

			return itemLockManager.release(item, PTRMI_DOUBLE_MAP_TIMEOUT, &mutex);
		}

		if(found)
		{
			*found = false;
		}

		return Status();
	}

	template<typename M, typename K> T *lockOrNewLockedItem(M &map, K &key)
	{
		T *	item;
															// Attempt to lock tiem
		if(item = lockItem(key))
		{
															// Locked, so return item
			return item;
		}
															// Return new pre-locked item
		if(newItem(key, &item).isFailed())
		{
			return NULL;
		}
															// Return pre-locekd item
		return item;
	}

	void setNumItems(unsigned int initNumItems)
	{
		numItems = initNumItems;
	}

public:

					DoubleKeyMap		(void);
				   ~DoubleKeyMap		(void);

	Status			newItem				(const KeyA &keyA, T **preLockedItem = NULL)		{return newItem(keyAMap, keyA, preLockedItem);}
	Status			newItem				(const KeyB &keyB, T **preLockedItem = NULL)		{return newItem(keyBMap, keyB, preLockedItem);}

	Status			addIndex			(const KeyA &existingKey, const KeyB &newKey)		{return addIndex(keyAMap, existingKey, keyBMap, newKey);}
	Status			addIndex			(const KeyB &existingKey, const KeyA &newKey)		{return addIndex(keyBMap, existingKey, keyAMap, newKey);}

	Status			updateIndex			(const KeyA &keyA, const KeyB &keyB);

	Status			removeIndex			(KeyA &keyA)										{if(removeIndex(keyAMap, keyA) != NULL) return Status(); else return Status(Status::Status_Error_Failed);}
	Status			removeIndex			(KeyB &keyB)										{if(removeIndex(keyBMap, keyB) != NULL) return Status(); else return Status(Status::Status_Error_Failed);}

	Status			deleteItem			(const KeyA &keyA, bool preLocked = false)			{return deleteItem(keyAMap, keyA, keyBMap, preLocked);}
	Status			deleteItem			(const KeyB &keyB, bool preLocked = false)			{return deleteItem(keyBMap, keyB, keyAMap, preLocked);}

	Status			deleteAllKeyA		(void)												{return deleteAll<KeyAMap, KeyA>(keyAMap);}
	Status			deleteAllKeyB		(void)												{return deleteAll<KeyBMap, KeyB>(keyBMap);};
	Status			deleteAll			(void);

	unsigned int	getNumKeyAItems		(void)												{return getNumItems(keyAMap);}
	unsigned int	getNumKeyBItems		(void)												{return getNumItems(keyAMap);};
	unsigned int	getNumItems			(void)												{return numItems;}

	T			*	lockItem			(const KeyA &keyA, bool *found = NULL, unsigned int timeOut = PTRMI_DOUBLE_MAP_TIMEOUT)	{return lockItem(keyAMap, keyA, found, timeOut);}
	T			*	lockItem			(const KeyB &keyB, bool *found = NULL, unsigned int timeOut = PTRMI_DOUBLE_MAP_TIMEOUT)	{return lockItem(keyBMap, keyB, found, timeOut);}

	T			*	lockOrNewLockedItem	(const KeyA &keyA)									{return lockOrNewLockedItem(keyAMap, keyA);}
	T			*	lockOrNewLockedItem	(const KeyB &keyB)									{return lockOrNewLockedItem(keyBMap, keyB);}

	Status			releaseItem			(const KeyA &keyA, bool *found = NULL)				{return releaseItem(keyAMap, keyA, found);}
	Status			releaseItem			(const KeyB &keyB, bool *found = NULL)				{return releaseItem(keyBMap, keyB, found);}
	Status			releaseItem			(T *item);


};

template<typename KA, typename KB, typename T>
Status PTRMI::DoubleKeyMap<KA, KB, T>::updateIndex(const KeyA &keyA, const KeyB &keyB)
{
	Host *hostA;
	Host *hostB;

	MutexScope mutexScope(mutex);

	hostA = getItem(keyAMap, keyA);
	hostB = getItem(keyBMap, keyB);

	if(hostA)
	{
		if(hostB)
		{
			if(hostA == hostB)
			{
				return Status();
			}
			else
			{
				return Status(Status::Status_Error_Failed);
			}
		}
		else
		{
			return addIndex(keyA, keyB);
		}
	}
	else
	{
		if(hostB)
		{
			return addIndex(keyB, keyA);
		}
	}

	return Status(Status::Status_Error_Failed);
}

template<typename KA, typename KB, typename T>
inline PTRMI::DoubleKeyMap<KA, KB, T>::DoubleKeyMap(void)
{
	setNumItems(0);
}


template<typename KA, typename KB, typename T>
inline PTRMI::DoubleKeyMap<KA, KB, T>::~DoubleKeyMap(void)
{
	deleteAll();
}


template<typename KA, typename KB, typename T> 
inline Status DoubleKeyMap<KA, KB, T>::deleteAll(void)
{
	Status	status;
														// Delete all map A entries including any items also in Map B
	if((status = deleteAll<KeyAMap, KeyA>(keyAMap)).isFailed())
	{
		return status;
	}
														// Delete all remaining entries in map B
	status = deleteAll<KeyBMap, KeyB>(keyBMap);
														// Return status
	return status;
}



template<typename KA, typename KB, typename T>
inline Status PTRMI::DoubleKeyMap<KA, KB, T>::releaseItem(T *item)
{
	return itemLockManager.release(item, PTRMI_DOUBLE_MAP_TIMEOUT, &mutex);
}


} // End PTRMI namespace
