/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHost.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define TERMINATE_HOST_OBJECT(obj, isProgramExit) {if (obj) {obj->OnHostTermination(isProgramExit); obj = NULL;}}

//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include "DgnPlatform.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// A DgnHost is an object that uniquely identifies a usage of the DgnPlatform libraries for a single purpose.
// For a given process, there can be more than one DgnHost, but each DgnHost must be on a different thread.
// DgnHost holds a collection of key/pointer pairs that are used to store and retrieve host-based data.
// @bsiclass                                                    Keith.Bentley   06/10
//=======================================================================================
struct DgnHost : NonCopyableClass
    {
public:
    //! Each "type" of data stored on a DgnHost must have a unique key to identify it. To add data to a DgnHost, create a single *static*
    //! instance of this class and pass it to the SetHostVariable method. The same static instance should be used to set and retrieve your data from
    //! the DgnHost.
    struct Key
        {
        friend struct DgnHost;
        private:
            size_t m_key;
        public:
            Key() {m_key=0;}
        };

    struct IHostObject : NonCopyableClass
        {
    protected:
        virtual ~IHostObject(){}
        virtual void _OnHostTermination (bool isProgramExit) {delete this;}
    public:
        void OnHostTermination (bool isProgramExit) {_OnHostTermination(isProgramExit);}
        };

    struct HostObjectBase : IHostObject
        {
        DEFINE_BENTLEY_NEW_DELETE_OPERATORS
        };

    template<typename T>
    struct HostValue : HostObjectBase
        {
        private:
        T   m_value;
        public:
        HostValue (T const& v) : m_value (v) {;}
        T const& GetValue () const {return m_value;}
        void     SetValue (T const& v) {m_value = v;}
        };

private:
    size_t GetKeyIndex (DgnHost::Key& key);

protected:
    struct VarEntry
        {
        friend struct DgnHost;
        void* m_val;
        VarEntry() {m_val=0;}
        void* GetValue() {return m_val;}
        void  SetValue(void* val) {m_val = val;}
        };

    struct ObjEntry
        {
        friend struct DgnHost;
        IHostObject* m_val;
        ObjEntry() {m_val=0;}
        IHostObject* GetValue() {return m_val;}
        void  SetValue(IHostObject* val) {m_val = val;}
        };

    bvector<VarEntry> m_hostVar;
    bvector<ObjEntry> m_hostObj;
    VarEntry& GetVarEntry(Key& key);
    ObjEntry& GetObjEntry(Key& key);

public:
    //! Get the value of a host-based variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The value of the host variable identified by key. If the variable has never been set on this instance of DgnHost,
    //! the value will be 0.
    DGNPLATFORM_EXPORT void* GetHostVariable (Key& key);

    //! Set the value of a host-based variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The value to be associated with key for this DgnHost.
    DGNPLATFORM_EXPORT void  SetHostVariable (Key& key, void* val);

    //! Get the value of a host-based integer variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The integer value of the host variable identified by key. If the variable has never been set on this instance of DgnHost,
    //! the value will be 0.
    template <typename INT_TYPE>
    INT_TYPE  GetHostIntVariable (Key& key) {return (INT_TYPE)(intptr_t)GetHostVariable (key);}

    //! Set the value of a host-based integer variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The integer value to be associated with key for this DgnHost.
    template <typename INT_TYPE>
    void SetHostIntVariable (Key& key, INT_TYPE val) {SetHostVariable (key, (void*)(intptr_t)val);}

    //! Get the value of a host-based boolean variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The boolean value of the host variable identified by key. If the variable has never been set on this instance of DgnHost,
    //! the value will be 0.
    bool  GetHostBoolVariable (Key& key) {return 0 != (intptr_t)GetHostVariable (key);}

    //! Set the value of a host-based boolean variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The boolean value to be associated with key for this DgnHost.
    void SetHostBoolVariable (Key& key, bool val) {SetHostVariable (key, (void*)(intptr_t)val);}

    //! Get the value of a host-based variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @return The value of the host variable identified by key. If the variable has never been set on this instance of DgnHost,
    //! the value will be 0.
    DGNPLATFORM_EXPORT IHostObject* GetHostObject (Key& key);

    //! Set the value of a host-based variable identified by key.
    //! @param[in] key The key that identifies this variable. Keys must always be static variables, as their values are assigned when they are first used
    //! and must remain the same for the entire run of a host program.
    //! @param[in] val The value to be associated with key for this DgnHost.
    DGNPLATFORM_EXPORT void  SetHostObject (Key& key, IHostObject* val);
    };

/*=================================================================================**//**
* A host object that holds a string
* @bsiclass                                     Sam.Wilson                      10/2010
+===============+===============+===============+===============+===============+======*/
struct          DgnHostWString : DgnHost::IHostObject
{
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

private:
    WString m_string;

public:
    DgnHostWString (wchar_t const* s) : m_string(s) {;}
    DgnHostWString (WString const& ws) : m_string(ws) {;}

    WString&        GetWString ()       {return m_string;}
    WString const&  GetWString () const {return m_string;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

/** @endcond */
