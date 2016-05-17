#pragma once

#include <string>
#include <map>


template<typename T> class Manager
{

public:

	typedef std::wstring				ItemName;
	typedef unsigned int				ItemCount;

protected:

	typedef std::map<ItemName, T *>		ItemMap;

public:


	typedef typename ItemMap::iterator	Iterator;

protected:

	ItemMap								items;

protected:

protected:

	virtual						 	   ~Manager			(void);

	T				*					create			(const ItemName &name, T *item);
	bool								destroy			(const ItemName &name);
	bool								destroy			(T *item, bool deleteItem);
	bool								destroyAll		(void);

	ItemCount							getCount		(void);

	T				*					get				(const ItemName &name);
	Iterator							getEntry		(const ItemName &name);
	Iterator							getEntry		(T *item);
};


template<typename T>
inline Manager<T>::~Manager(void)
{

}

template<typename T>
inline T * Manager<T>::create(const ItemName & name, T * item)
{
	items[name] = item;

	return item;
}

template<typename T>
inline bool Manager<T>::destroy(const ItemName & name)
{
	return false;
}

template<typename T>
inline bool Manager<T>::destroy(T * item, bool deleteItem)
{
	Iterator it = getEntry(item);

	if (it != items.end())
	{
		if (deleteItem && it->second)
		{
			delete it->second;
		}

		items.erase(it);

		return true;
	}

	return false;
}

template<typename T>
inline bool Manager<T>::destroyAll(void)
{
	return false;
}

template<typename T>
inline typename Manager<T>::ItemCount Manager<T>::getCount(void)
{
	return items.count();
}

template<typename T>
inline T * Manager<T>::get(const ItemName & name)
{
	Iterator it;
	
	if ((it = getEntry(name)) != items.end())
	{
		return it->second;
	}

	return NULL;
}

template<typename T>
inline typename Manager<T>::Iterator Manager<T>::getEntry(const ItemName &name)
{
	return items.find(name);
}

template<typename T>
inline typename Manager<T>::Iterator Manager<T>::getEntry(T * item)
{
	if (item == NULL)
		return items.end();

	Iterator it;

	for (it = items.begin(); it != items.end() && it->second != item; it++)
	{
	}

	return it;
}



