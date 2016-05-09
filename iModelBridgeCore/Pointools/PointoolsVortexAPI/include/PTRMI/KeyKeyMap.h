#pragma once

#include <map>

#include <PTRMI/Status.h>


namespace PTRMI
{

#define T_KEY_KEY_MAP	template<typename GK, typename IK, typename I> inline 

template<typename GK, typename IK, typename I> class KeyKeyMap
{
public:

	typedef KeyKeyMap<GK, IK, I>				this_type;

	typedef	I									Item;
	typedef GK									GroupKey;
	typedef IK									ItemKey;

public:

	typedef std::map<ItemKey, I>				Group;

protected:

	typedef std::map<GroupKey, Group>			Groups;

protected:

	Groups					groups;

protected:

	Group				*	newGroup			(GroupKey &groupKey);

	I					*	getItem				(Group *group, ItemKey &itemKey);
	Status					deleteItem			(Group *group, GroupKey &groupKey, ItemKey &itemKey, bool deleteEmptyGroup);

	Status					getNumGroupItems	(Group &group, unsigned int &numItems);

public:

	void					clear				(void);

	Status					addItem				(GroupKey &groupKey, ItemKey &itemKey, Item &item, bool *groupCreated = NULL);
	Status					deleteItem			(GroupKey &groupKey, ItemKey &itemKey, bool deleteEmptyGroup = true);
	Status					deleteItem			(ItemKey &itemKey, bool deleteEmptyGroup = true);

	Item					getItem				(GroupKey &groupKey, ItemKey &itemKey);
	Item					getItem				(ItemKey &itemKey);

	Item				*	getItemPtr			(GroupKey &groupKey, ItemKey &itemKey);
	Item				*	getItemPtr			(ItemKey &itemKey);

	Group				*	getGroup			(const GroupKey &GroupKey);
	GroupKey				getItemGroup		(ItemKey &item);
	Status					deleteGroup			(GroupKey &GroupKey);

	unsigned int			getNumGroups		(void);
	unsigned int			getNumItems			(void);
	unsigned int			getNumGroupItems	(GroupKey &groupKey);

	template<typename T> unsigned int getGroupItemsOfType(GroupKey &groupKey, std::vector<T *> &resultSet)
	{
		applyGroupItems<this_type, std::vector<T *> &>(groupKey, this, &this_type::getItemsOfType<T>, resultSet);

		return static_cast<unsigned int>(resultSet.size());
	}

	template<typename T> Status getItemsOfType(Item &item, std::vector<T *> &resultSet)
	{
		T *i;
															// If item is of required type
		if(i = dynamic_cast<T *>(item))
		{
															// Add it to the result set
			resultSet.push_back(i);
		}

		return Status();
	}

	template<typename C> Status applyAllItems(C *obj, Status (C::*func)(Item), bool exitFirstGroupError = true, bool exitFirstError = true)
	{
		Groups::iterator	it, itNext;
		Status				status;
		bool				terminate = false;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return status;
		}

		for(it = groups.begin(); terminate == false; it = itNext)
		{
			itNext = it;
			itNext++;
															// Decide whether this is the last item. This allows underlying group to be erased after function call
			terminate = (itNext == group->end());

			if((status = applyGroupItems(it->first, obj, func, exitFirstError)).isFailed() && exitFirstGroupError)
			{
				return status;
			}
		}

		return status;
	}


	template<typename C> Status applyGroupKeys(C *obj, Status (C::*func)(const GroupKey &), bool exitFirstGroupError = true)
	{
		Groups::iterator	it, itNext;
		Status				status;
		bool				terminate = false;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return status;
		}

		for(it = groups.begin(); terminate == false; it = itNext)
		{
			itNext = it;
			itNext++;
															// Decide whether this is the last item. This allows underlying group to be erased after function call
			terminate = (itNext == groups.end());

			if((status = (obj->*func)(it->first)).isFailed() && exitFirstGroupError)
			{
				return status;
			}
		}		

		return status;
	}


	template<typename C, typename P> Status applyGroupKeys(C *obj, Status (C::*func)(const GroupKey &, P param), P param, bool exitFirstGroupError = true)
	{
		Groups::iterator	it, itNext;
		Status				status;
		bool				terminate = false;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return status;
		}

		for(it = groups.begin(); it != groups.end(); it = itNext)
		{
			itNext = it;
			itNext++;

			if((status = (obj->*func)(it->first, param)).isFailed() && exitFirstGroupError)
			{
				return status;
			}
		}		

		return status;
	}


	template<typename C> Status applyGroups(C *obj, Status (C::*func)(Group &), bool exitFirstGroupError = true)
	{
		Groups::iterator	it, itNext;
		Status				status;
		bool				terminate = false;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return status;
		}

		for(it = groups.begin(); terminate == false; it = itNext)
		{
			itNext = it;
			itNext++;
															// Decide whether this is the last item. This allows underlying group to be erased after function call
			terminate = (itNext == groups.end());

			if((status = (obj->*func)(it->second)).isFailed() && exitFirstGroupError)
			{
				return status;
			}
		}		

		return status;
	}


	template<typename C, typename P> Status applyGroups(C *obj, Status (C::*func)(Group &, P param), P param, bool exitFirstGroupError = true)
	{
		Groups::iterator	it, itNext;
		Status				status;
		bool				terminate = false;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return status;
		}

		for(it = groups.begin(); it != groups.end(); it = itNext)
		{
			itNext = it;
			itNext++;

			if((status = (obj->*func)(it->second, param)).isFailed() && exitFirstGroupError)
			{
				return status;
			}
		}		

		return status;
	}


	template<typename C> Status applyGroupItems(GroupKey groupKey, C *obj, Status (C::*func)(Item), bool exitFirstError = true)
	{
		Group			*	group;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return Status();
		}

		if((group = getGroup(groupKey)) == NULL)
		{
			return Status(Status::Status_Warning_Failed_To_Find_Group); 
		}

		return applyGroupItems<C>(group, obj, func, exitFirstError);
	}


	template<typename C, typename P> Status applyGroupItems(const GroupKey groupKey, C *obj, Status (C::*func)(Item &, P param), P param, bool exitFirstError = true)
	{
		Group			*	group;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return Status();
		}

		if((group = getGroup(groupKey)) == NULL)
		{
			return Status(Status::Status_Warning_Failed_To_Find_Group); 
		}

		return applyGroupItems<C, P>(group, obj, func, param, exitFirstError);
	}


	template<typename C> Status applyGroupItems(Group *group, C *obj, Status (C::*func)(Item), bool exitFirstError = true)
	{
		Group::iterator		it, itNext;
		bool				terminate = false;
		Status				status;

		if(group == NULL)
		{
			return status;
		}

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}

		for(it = group->begin(); terminate == false; it = itNext)
		{
			itNext = it;
			itNext++;
															// Decide whether this is the last item. This allows underlying group to be erased after function call
			terminate = (itNext == group->end());

			if((status = (obj->*func)(it->second)).isFailed() && exitFirstError)
			{
				return status;
			}
		}

		return status;
	}


	template<typename C, typename P> Status applyGroupItems(Group *group, C *obj, Status (C::*func)(Item &, P param), P param, bool exitFirstError = true)
	{
		Group::iterator		it, itNext;
		bool				terminate = false;
		Status				status;

		if(group == NULL)
		{
			return status;
		}

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}

		for(it = group->begin(); terminate == false; it = itNext)
		{
			itNext = it;
			itNext++;
															// Decide whether this is the last item. This allows underlying group to be erased after function call
			terminate = (itNext == group->end());

			if((status = (obj->*func)(it->second, param)).isFailed() && exitFirstError)
			{
				return status;
			}
		}

		return status;
	}


	template<typename C> Status applyAllItemKeys(C *obj, Status (C::*func)(ItemKey &), bool exitFirstGroupError = true, bool exitFirstError = true)
	{
		Groups::iterator	it, itNext;
		Status				status;
		bool				terminate = false;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return status;
		}

		for(it = groups.begin(); terminate == false; it = itNext)
		{
			itNext = it;
			itNext++;
															// Decide whether this is the last item. This allows underlying groups to be erased after function call
			terminate = (itNext == groups.end());

			if((status = applyGroupItemKeys(it->first, obj, func, exitFirstError)).isFailed() && exitFirstGroupError)
			{
				return status;
			}
		}

		return Status();
	}


	template<typename C> Status applyGroupItemKeys(GroupKey groupKey, C *obj, Status (C::*func)(ItemKey &), bool exitFirstError = true)
	{
		Group::iterator		it, itNext;
		Group			*	group;
		bool				terminate = false;
		Status				status;

		if(obj == NULL || func == NULL)
		{
			return Status(Status::Status_Error_Bad_Parameter);
		}
															// Test early to allow loop that permits deletion during callback
		if(groups.empty())
		{
			return status;
		}

		if((group = getGroup(groupKey)) == NULL)
		{
			return Status(Status::Status_Warning_Failed_To_Find_Group); 
		}

		for(it = group->begin(); terminate == false; it = itNext)
		{
			itNext = it;
			itNext++;
															// Decide whether this is the last item. This allows underlying group to be erased after function call
			terminate = (itNext == group->end());

			if((status = (obj->*func)(it->first)).isFailed() && exitFirstError)
			{
				return status;
			}
		}

		return status;
	}


};



T_KEY_KEY_MAP
typename PTRMI::KeyKeyMap<GK, IK, I>::GroupKey PTRMI::KeyKeyMap<GK, IK, I>::getItemGroup(ItemKey &itemKey)
{
	Groups::iterator		it;
	Item				*	item;

	for(it = groups.begin(); it != groups.end(); it++)
	{
		if((item = getItem(&(it->second), itemKey)) != NULL)
		{
			return it->first;
		}
	}
															// Return NULL
	return GroupKey();
}


T_KEY_KEY_MAP
void PTRMI::KeyKeyMap<GK, IK, I>::clear(void)
{
	groups.clear();
}


T_KEY_KEY_MAP
unsigned int PTRMI::KeyKeyMap<GK, IK, I>::getNumGroups(void)
{
	return groups.size();
}


T_KEY_KEY_MAP
unsigned int PTRMI::KeyKeyMap<GK, IK, I>::getNumItems(void)
{
	unsigned int	numItems = 0;

	applyGroups<this_type, unsigned int &>(this, &this_type::getNumGroupItems, numItems, false);

	return numItems;
}


T_KEY_KEY_MAP
unsigned int PTRMI::KeyKeyMap<GK, IK, I>::getNumGroupItems(GroupKey &groupKey)
{
	Group	*group;

	if((group = getGroup(groupKey)) == NULL)
	{
		return 0;
	}

	return group->size();
}


T_KEY_KEY_MAP
Status PTRMI::KeyKeyMap<GK, IK, I>::getNumGroupItems(Group &group, unsigned int &numItems)
{
	numItems += group.size();

	return Status();	
}


T_KEY_KEY_MAP
Status PTRMI::KeyKeyMap<GK, IK, I>::addItem(GroupKey &groupKey, ItemKey &itemKey, Item &item, bool *groupCreated)
{
	Group	*	group;
	Status		status;
	bool		newGroupCreated = false;

	if(getItemPtr(groupKey, itemKey) != NULL)
	{
		return Status(Status::Status_Warning_Item_Exists);
	}

	if((group = getGroup(groupKey)) == NULL)
	{
		if((group = newGroup(groupKey)) == NULL)
		{
			return Status(Status::Status_Error_Failed_To_Create_Group);
		}

		newGroupCreated = true;
	}

															// Create item
	(*group)[itemKey] = item;

															// If group creation info requested, return whether group was created
	if(groupCreated)
	{
		*groupCreated = newGroupCreated;
	}

	return status;
}


T_KEY_KEY_MAP
typename KeyKeyMap<GK, IK, I>::Item * KeyKeyMap<GK, IK, I>::getItem(Group *group, ItemKey &itemKey)
{
	if(group == NULL)
	{
		return NULL;
	}

	Group::iterator	it;

	if((it = group->find(itemKey)) != group->end())
	{
		return &(it->second);
	}

	return NULL;
}


T_KEY_KEY_MAP
typename KeyKeyMap<GK, IK, I>::Item * KeyKeyMap<GK, IK, I>::getItemPtr(GroupKey &groupKey, ItemKey &itemKey)
{
	Group			*	group;
	Group::iterator		it;

	if((group = getGroup(groupKey)) != NULL)
	{
		if((it = group->find(itemKey)) != group->end())
		{
			return &(it->second);
		}
	}

	return NULL;
}



T_KEY_KEY_MAP
typename KeyKeyMap<GK, IK, I>::Item * KeyKeyMap<GK, IK, I>::getItemPtr(ItemKey &itemKey)
{
	Groups::iterator		it;
	Item				*	item;

	for(it = groups.begin(); it != groups.end(); it++)
	{
		if((item = getItem(&(it->second), itemKey)) != NULL)
		{
			return item;
		}
	}

	return NULL;
}


T_KEY_KEY_MAP
typename KeyKeyMap<GK, IK, I>::Item KeyKeyMap<GK, IK, I>::getItem(GroupKey &groupKey, ItemKey &itemKey)
{
	Item *item;
	
	if(item = getItemPtr(groupKey, itemKey))
	{
		return *item;
	}

	Item result = NULL;

	return result;
}



T_KEY_KEY_MAP
typename KeyKeyMap<GK, IK, I>::Item KeyKeyMap<GK, IK, I>::getItem(ItemKey &itemKey)
{
	Item *item;

	if(item = getItemPtr(itemKey))
	{
		return *item;
	}

	Item result = NULL;

	return result;
}


T_KEY_KEY_MAP
typename KeyKeyMap<GK, IK, I>::Group *KeyKeyMap<GK, IK, I>::getGroup(const GroupKey &groupKey)
{
	Groups::iterator	it;

	if((it = groups.find(groupKey)) != groups.end())
	{
		return &(it->second);
	}

	return NULL;
}


T_KEY_KEY_MAP
typename KeyKeyMap<GK, IK, I>::Group *KeyKeyMap<GK, IK, I>::newGroup(GroupKey &groupKey)
{
	groups[groupKey] = Group();

	return getGroup(groupKey);
}



T_KEY_KEY_MAP
Status PTRMI::KeyKeyMap<GK, IK, I>::deleteGroup(GroupKey &groupKey)
{
	Groups::iterator	it;

	if((it = groups.find(groupKey)) != groups.end())
	{
		groups.erase(it);

		return Status();
	}

	return Status(Status::Status_Warning_Failed_To_Find_Group);	
}


T_KEY_KEY_MAP
Status PTRMI::KeyKeyMap<GK, IK, I>::deleteItem(Group *group, GroupKey &groupKey, ItemKey &itemKey, bool deleteEmptyGroup)
{
	Group::iterator it;

	if(group == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	if((it = group->find(itemKey)) != group->end())
	{
		group->erase(it);

		if(group->empty())
		{
			return deleteGroup(groupKey);
		}

		return Status();
	}

	return Status(Status::Status_Warning_Failed_To_Find_Group);
}


T_KEY_KEY_MAP
Status PTRMI::KeyKeyMap<GK, IK, I>::deleteItem(GroupKey &groupKey, ItemKey &itemKey, bool deleteEmptyGroup)
{
	Group *	group;

	if((group = getGroup(groupKey)) == NULL)
	{
		return Status(Status::Status_Error_Bad_Parameter);
	}

	return deleteItem(group, groupKey, itemKey, deleteEmptyGroup);
}



T_KEY_KEY_MAP
Status PTRMI::KeyKeyMap<GK, IK, I>::deleteItem(ItemKey &itemKey, bool deleteEmptyGroup)
{
	GroupKeyGroupMap::iterator		it;
	Item						*	item;

	for(it = groups.begin(); it != groups.end(); it++)
	{
		if(deleteItem(&(it->second), it->first, itemKey, deleteEmptyGroup).isOK())
		{
			return Status();
		}
	}

	return Status(Status::Status_Warning_Failed_To_Find_Item);
}


} // End PTRMI