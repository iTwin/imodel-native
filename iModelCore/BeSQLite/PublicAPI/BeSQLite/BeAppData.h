/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/BeAppData.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/bvector.h>

BEGIN_BENTLEY_SQLITE_NAMESPACE

//=======================================================================================
//! A unique (for this session) key to identify this AppData type. Create a static instance
//! of this class to identify each subclass of AppData.
//! @bsiclass                                                     Keith.Bentley   10/07
//=======================================================================================
struct   AppDataKey : NonCopyableClass
{
private:
    AppDataKey (AppDataKey const&);                  // illegal
    AppDataKey const& operator= (AppDataKey const&); // illegal
public:
    AppDataKey() {}
};

//=======================================================================================
//! @bsiclass                                                     Keith.Bentley   10/07
//=======================================================================================
template <typename APPDATA, typename KEY, typename HOST> struct AppDataList
{
    struct  AppDataEntry
        {
        KEY const*   m_key;
        APPDATA*     m_obj;

        AppDataEntry (APPDATA* entry, KEY const& key) : m_key(&key) {m_obj = entry;}
        void ChangeValue (APPDATA* obj, HOST host) {APPDATA* was = m_obj; m_obj=obj; if (was) was->_OnCleanup (host);}
        void Clear (HOST host) {ChangeValue (NULL, host); m_key = 0;}
        };

    typedef bvector<AppDataEntry> T_List;
    bool    m_locked;
    T_List  m_list;

    AppDataList() {m_locked=false;}

//__PUBLISH_SECTION_END__
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
APPDATA* FindAppData (KEY const& key) const
    {
    for (typename T_List::const_iterator entry = m_list.begin(); entry != m_list.end(); ++entry)
        {
        if (&key == entry->m_key)
            return entry->m_obj;
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
//! It is NOT legal to call AddAppData from within a callback.
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  AddAppData (KEY const& key, APPDATA* obj, HOST host)
    {
    if (m_locked)
        {
        BeAssert (0);
        return ERROR;
        }

    for (typename T_List::iterator entry = m_list.begin(); entry != m_list.end(); ++entry)
        {
        if (&key == entry->m_key)
            {
            entry->ChangeValue (obj, host);
            return  SUCCESS;
            }
        }

    m_list.push_back (AppDataEntry (obj, key));
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
//! It IS legal to call DropAppData from within a callback.
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DropAppData (KEY const& key, HOST host)
    {
    for (typename T_List::iterator entry = m_list.begin(); entry != m_list.end(); ++entry)
        {
        if (&key == entry->m_key)
            {
            entry->Clear(host);        // doesn't get removed until next traversal
            return  SUCCESS;
            }
        }
    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CALLER> void CallAllDroppable (CALLER const& caller, HOST host)
    {
    m_locked = true;
    for (typename T_List::iterator entry = m_list.begin(); entry != m_list.end(); )
        {
        if (NULL == entry->m_obj)      // was previously dropped
            {
            entry = m_list.erase (entry);
            }
        else if (caller.CallHandler (*entry->m_obj))
            {
            entry->ChangeValue(NULL, host);
            entry = m_list.erase (entry);
            }
        else
            ++entry;
        }
    m_locked = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename CALLER> void CallAll (CALLER const& caller)
    {
    m_locked = true;
    for (typename T_List::iterator entry = m_list.begin(); entry != m_list.end(); )
        {
        if (NULL == entry->m_obj)   // was previously dropped
            {
            entry = m_list.erase (entry);
            }
        else
            {
            caller.CallHandler (*entry->m_obj);
            ++entry;
            }
        }
    m_locked = false;
    }
//__PUBLISH_SECTION_START__
};


END_BENTLEY_SQLITE_NAMESPACE
