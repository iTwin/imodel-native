//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCPtr.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:> Inline method for class HFCShareableObject
//:> Template/Inline methods for class HFCPtr<T>
//-----------------------------------------------------------------------------
BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Default constructor.
//-----------------------------------------------------------------------------
template<class T>
inline HFCShareableObject<T>::HFCShareableObject()
    {
    m_RefCount.store(0);
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
template<class T>
inline HFCShareableObject<T>::HFCShareableObject(const HFCShareableObject<T>& pi_rObj)
    {
    m_RefCount.store(0);
    }

//-----------------------------------------------------------------------------
// Destructor.
//-----------------------------------------------------------------------------
template<class T>
inline HFCShareableObject<T>::~HFCShareableObject()
    {
    HASSERT(m_RefCount.load() == 0);
    }

//-----------------------------------------------------------------------------
// Assignment operator.
//-----------------------------------------------------------------------------
template<class T>
inline const HFCShareableObject<T>&
HFCShareableObject<T>::operator=(const HFCShareableObject<T>& pi_rObj)
    {
    // Nothing to do... which is different from default behavior!
    return *this;
    }

//-----------------------------------------------------------------------------
// Always called by HFCPtr.
//-----------------------------------------------------------------------------
template<class T>
inline void HFCShareableObject<T>::_internal_NotifyAdditionOfASmartPointer()
    {
    //m_Key.ClaimKey();
    ++m_RefCount;
    //m_Key.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// Always called by HFCPtr.
//-----------------------------------------------------------------------------
template<class T>
inline void HFCShareableObject<T>::_internal_NotifyRemovalOfASmartPointer()
    {
    //m_Key.ClaimKey();
    HASSERT(m_RefCount.load() != 0);
    uint32_t RefCount = --m_RefCount;
    //m_Key.ReleaseKey();

    // Delete here because we don't want to keep the key
    // locked in the destruction. Anyways, since the object
    // would have been deleted, there will surely be no other
    // try to use it...
    if (RefCount == 0)
        delete ((T*)this);
    }

//-----------------------------------------------------------------------------
// For special circumstances.
//-----------------------------------------------------------------------------
template<class T>
inline void HFCShareableObject<T>::IncrementRef()
    {
    //m_Key.ClaimKey();
    ++m_RefCount;
    //m_Key.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// For special circumstances.
//-----------------------------------------------------------------------------
template<class T>
inline void HFCShareableObject<T>::DecrementRef()
    {
    //m_Key.ClaimKey();
    HASSERT(m_RefCount.load() != 0);
    --m_RefCount;
    //m_Key.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// For special circumstances.
//-----------------------------------------------------------------------------
template<class T>
inline uint32_t HFCShareableObject<T>::GetRefCount() const
    {
    return m_RefCount.load();
    }


///////////////////////////////////////////////////////////////////////////////

/**---------------------------------------------------------------------------
 Default constructor.  It makes a smart pointer pointing to nothing, in
 other terms it points to "NULL" (value zero).
----------------------------------------------------------------------------*/
template<class T> inline HFCPtr<T>::HFCPtr()
    {
    m_pObject = 0;
    }


/**---------------------------------------------------------------------------
 Constructor from plain pointer.  It acts like a type converter from type
 T* to type "smart pointer to T".  It constructs a smart pointer that will
 point to object pointed to by @r{pi_ptr}.

 There is no problem to create separately many smart pointers on same
 object, without using copy methods : they will still share the same
 reference count.

 @param pi_Ptr Pointer to the object to be pointed to by this smart pointer.

 @see HFCPtr<T>::operator=(const HFCPtr<T>& pi_rObj)
-----------------------------------------------------------------------------*/
template<class T> inline HFCPtr<T>::HFCPtr(T* pi_Ptr)
    : m_pObject(pi_Ptr)
    {
    if (m_pObject)
        m_pObject->_internal_NotifyAdditionOfASmartPointer();
    }


/**----------------------------------------------------------------------------
 Copy constructor.  It duplicates another smart pointer specified by pi_rObj.
 Reference count for pointed object is incremented accordingly.

 There is no problem to create separately many smart pointers on same
 object, without using copy methods : they will still share the same
 reference count.

 @param pi_rObj Reference to the smart pointer to duplicate.

 @see HFCPtr<T>::operator=(const HFCPtr<T>& pi_rObj)
-----------------------------------------------------------------------------*/
template<class T> inline HFCPtr<T>::HFCPtr(const HFCPtr<T>& pi_rObj)
    : m_pObject(pi_rObj.m_pObject)
    {
    if (m_pObject)
        m_pObject->_internal_NotifyAdditionOfASmartPointer();
    }

/**----------------------------------------------------------------------------
 Assignment operator. It duplicates another smart pointer specified by
 @r{pi_rObj}; in other word, it changes the object to be pointed to by
 this smart pointer.  If the pointer was already refering to another
 object, it "disconnect" itself from that object by updating reference
 counting for it, and deleting it if it was the last pointer to
 that object.  Then it takes the address of another object and updates
 (or begin) reference counting for that new one.

 @param pi_rObj Reference to the smart pointer to duplicate.

 @return Reference to self that can be used as l-value.
-----------------------------------------------------------------------------*/
template<class T>
inline HFCPtr<T>& HFCPtr<T>::operator=(const HFCPtr<T>& pi_rObj)
    {
    if (m_pObject != pi_rObj.m_pObject)
        {
        if (m_pObject)
            m_pObject->_internal_NotifyRemovalOfASmartPointer();
        m_pObject = pi_rObj.m_pObject;
        if (m_pObject)
            m_pObject->_internal_NotifyAdditionOfASmartPointer();
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Assignment operator. It makes the current smart pointer pointing to a
 new object.  Reference to previously pointed object is decremented, this
 object is deleted if necessary, and a new reference count is set for the
 new object to point to. If the pointer was already refering to another
 object, it "disconnect" itself from that object by updating reference
 counting for it, and deleting it if it was the last pointer to that
 object.  Then it takes the address of another object and updates (or
 begin) reference counting for that new one.

 There is no problem to assign separately many smart pointers on same
 object, without using copy methods : they will still share the same
 reference count.

 @param pi_Ptr Pointer to the object to be pointed to by this smart pointer.

 @return Reference to self that can be used as l-value.
-----------------------------------------------------------------------------*/
template<class T> inline HFCPtr<T>& HFCPtr<T>::operator=(T* pi_Ptr)
    {
    if (m_pObject != pi_Ptr)
        {
        if (m_pObject)
            m_pObject->_internal_NotifyRemovalOfASmartPointer();
        m_pObject = pi_Ptr;
        if (m_pObject)
            m_pObject->_internal_NotifyAdditionOfASmartPointer();
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Less-than comparator.  Used internally for sorting in a list.
-----------------------------------------------------------------------------*/
template<class T> inline bool HFCPtr<T>::operator<(const HFCPtr<T>& pi_rObj) const
    {
    return m_pObject < pi_rObj.m_pObject;
    }

/**----------------------------------------------------------------------------
 More-than comparator.  Used internally for sorting in a list.
-----------------------------------------------------------------------------*/
template<class T> inline bool HFCPtr<T>::operator>(const HFCPtr<T>& pi_rObj) const
    {
    return m_pObject > pi_rObj.m_pObject;
    }

/**----------------------------------------------------------------------------
 Destructor.  Just before elimination of the smart pointer, the
 destructor updates reference counting for object pointed to by itself.
 If it discovers that it is the last pointer to that object, it deletes
 that object.
-----------------------------------------------------------------------------*/
template<class T> inline HFCPtr<T>::~HFCPtr()
    {
    if (m_pObject)
        m_pObject->_internal_NotifyRemovalOfASmartPointer();
    }

/**----------------------------------------------------------------------------
 Equality evaluation operator.  Smart pointers are considered to be
 "equal" if they point to the same object.

 @param pi_rObj Reference to the smart pointer to compare to this one.

 @return Boolean value that is true if the pointers are considered
         equal, or the opposite value if they're not.

 @see HFCPtr<T>::operator!=(const HFCPtr<T>& pi_rObj)
-----------------------------------------------------------------------------*/
template<class T> inline bool HFCPtr<T>::operator==(const HFCPtr<T>& pi_rObj) const
    {
    return (m_pObject == pi_rObj.m_pObject);
    }

/**----------------------------------------------------------------------------
 Equality evaluation operator.  Smart pointers are considered to be
 "equal" if they point to the same object.

 @param pi_pObj Pointer to the object that the user want to know if it
                is the object pointed to by this smart pointer.

 @return Boolean value that is true if the pointers are considered
         equal, or the opposite value if they're not.

 @see HFCPtr<T>::operator!=(const T* pi_Value) const
-----------------------------------------------------------------------------*/
template<class T> inline bool HFCPtr<T>::operator==(const T* pi_Value) const
    {
    return (m_pObject == pi_Value);
    }

/**----------------------------------------------------------------------------
 Non-equality evaluation operator.  Smart pointers are not equal if they point
 to different objects.

 @param pi_rObj Reference to the smart pointer to compare to this one.

 @return Boolean value that is false if the pointers are considered
         equal, or the opposite value if they're not.

 @see HFCPtr<T>::operator==(const HFCPtr<T>& pi_rObj)
-----------------------------------------------------------------------------*/
template<class T> inline bool HFCPtr<T>::operator!=(const HFCPtr<T>& pi_rObj) const
    {
    return (m_pObject != pi_rObj.m_pObject);
    }

/**----------------------------------------------------------------------------
 Non-equality evaluation operator.  Smart pointers are not equal if they point
 to different objects.

 @param pi_pObj Pointer to the object that the user want to know if it
                is the object pointed to by this smart pointer.

 @return Boolean value that is false if the pointers are considered
         equal, or the opposite value if they're not.

 @see HFCPtr<T>::operator==(const HFCPtr<T>& pi_rObj)
-----------------------------------------------------------------------------*/
template<class T> inline bool HFCPtr<T>::operator!=(const T* pi_Value) const
    {
    return (m_pObject != pi_Value);
    }

/**----------------------------------------------------------------------------
 Dereference operator.  This operator is overloaded to simulate the
 behavior of a plain C++ pointer.   It "transforms" the smart pointer in
 a l-value that represent the pointed object.

 @return A reference to the object pointed to by this smart pointer.
-----------------------------------------------------------------------------*/
template<class T> inline T& HFCPtr<T>::operator*() const
    {
    return *m_pObject;
    }

/**----------------------------------------------------------------------------
 Arrow operator. This operator is overloaded to simulate the
 behavior of a plain C++ pointer.

 @return A plain pointer to the object pointed to by this smart pointer.
-----------------------------------------------------------------------------*/
template<class T> inline T* HFCPtr<T>::operator->() const
    {
    HPRECONDITION(m_pObject != 0);
    return m_pObject;
    }

/**----------------------------------------------------------------------------
 Pointer-to-member operator.  This operator is overloaded to simulate the
 behavior of a plain C++ pointer.

 @return A plain pointer to the object pointed to by this smart pointer.
-----------------------------------------------------------------------------*/
template<class T> inline T* HFCPtr<T>::operator->*(int) const
    {
    HPRECONDITION(m_pObject != 0);
    return m_pObject;
    }

/**----------------------------------------------------------------------------
 Type cast operator.  It defines type conversion from smart pointer to
 plain pointer.

 @return A plain pointer to the object pointed to by this smart pointer.
-----------------------------------------------------------------------------*/
template<class T> inline HFCPtr<T>::operator T* () const
    {
    return m_pObject;
    }

/**----------------------------------------------------------------------------
 Returns a plain pointer to the object pointed to by this smart pointer.

 @return C++ typed pointer to the object that this object is refering to.
-----------------------------------------------------------------------------*/
template<class T> inline T* HFCPtr<T>::GetPtr() const
    {
    return m_pObject;
    }



/**----------------------------------------------------------------------------
static cast operator
-----------------------------------------------------------------------------*/
template<class T, class U> inline HFCPtr<T> static_pcast(const HFCPtr<U>& pi_rPtr)
{
    return static_cast<T*>(pi_rPtr.GetPtr());
}

/**----------------------------------------------------------------------------
const cast operator
-----------------------------------------------------------------------------*/
template<class T, class U> inline HFCPtr<T> const_pcast(const HFCPtr<U>& pi_rPtr)
{
    return const_cast<T*>(pi_rPtr.GetPtr());
}

/**----------------------------------------------------------------------------
dynamic cast operator
-----------------------------------------------------------------------------*/
template<class T, class U> inline HFCPtr<T> dynamic_pcast(const HFCPtr<U>& pi_rPtr)
{
    return dynamic_cast<T*>(pi_rPtr.GetPtr());
}
END_IMAGEPP_NAMESPACE