/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeThreadLocalStorage.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "Bentley.h"

BEGIN_BENTLEY_NAMESPACE

/*=================================================================================**//**
*
* Thread local storage
*
* Usage:
* /code
// When your program starts up, allocate a slot in the OS TLS registry.
s_tls = new BeThreadLocalStorage();

...start a thread ...

// At some point in a thread, store a value in thread-local storage.
s_tls->SetValue (new MyObject);

// At some later point in a thread, retrieve a value from thread-local storage.
MyObject* obj = s_tls->GetValue();
if (NULL != obj)
    { obj->MethodCall(); ... }

// Before the thread exits, clean up the stored value.
MyObject* obj = s_tls->GetValue();
if (NULL != obj)
    {
    delete obj;
    s_tls->SetValue (NULL);
    }

...end the thread...

// When your program is done, deallocate the slot from the OS TLS registry.
delete s_tls;
/code
*
* @bsiclass                                     sam.wilson                      06/2011
+===============+===============+===============+===============+===============+======*/
struct BeThreadLocalStorage
{
private:
    void*  m_key;

public:
    BENTLEYDLL_EXPORT static void* Create();
    BENTLEYDLL_EXPORT static void Delete(void*);
    BENTLEYDLL_EXPORT static void SetValue(void* key, void* val);
    BENTLEYDLL_EXPORT static void* GetValue(void* key);
    
    //! Allocate a slot for thread local storage
    BeThreadLocalStorage() : m_key(Create()) {}

    //! Delete a slot for thread local storage.
    //! @remarks It is your responsibility to free the stored value, if that is required, before calling this function.
    ~BeThreadLocalStorage() {Delete(m_key);}

    //! Store a pointer value.
    //! @remarks It is your responsibility to free the current stored value, if that is required, before calling this function.
    void SetValueAsPointer(void* val) {SetValue(m_key, val);}

    //! Store an integer value.
    void SetValueAsInteger(intptr_t v) {SetValueAsPointer((void*)v);}

    //! Retrieve the stored value as a pointer.
    //! @return the value stored by SetValueAsPointer or NULL if no value was ever stored.
    void* GetValueAsPointer() {return GetValue(m_key);}

    //! Retrieve a the stored value as an integer.
    //! @return the value stored by SetValue or 0 if no value was ever stored.
    intptr_t GetValueAsInteger() {return (intptr_t)GetValueAsPointer();}
};

END_BENTLEY_NAMESPACE
