//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMPersistentObject.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Inline methods for class HPMPersistentObject
//-----------------------------------------------------------------------------

#include <Imagepp/all/h/HPMObjectStore.h>

BEGIN_IMAGEPP_NAMESPACE
/**----------------------------------------------------------------------------
 Returns the object ID of this object.  An object have an object ID only
 if it is "attached" to an object store, in other words if it has been
 loaded from an object store or saved in it.  If object doesn't have an
 ID, this method returns INVALID_OBJECT_ID.  The meaning of an object ID
 is relative only to the associated store.

 @return Object ID of this object, or INVALID_OBJECT_ID if it has none.

 @inheritance Cannot be overriden.
-----------------------------------------------------------------------------*/
inline HPMObjectID HPMPersistentObject::GetID() const
    {
    return m_ObjectID;
    }

/**----------------------------------------------------------------------------
 This method is called by object store when an unidentified object had just
 been saved to it and received a new object ID.

 @param pi_ObjectID New ID for this object.

 @inheritance Cannot be overriden.
-----------------------------------------------------------------------------*/
inline void HPMPersistentObject::SetID(HPMObjectID pi_ObjectID)
    {
    m_ObjectID = pi_ObjectID;
    }

/**----------------------------------------------------------------------------
 Sets an internal flag that indicates if this object needs to be saved or
 not because of alteration of its state since the last save/load.  This
 flag may be used by object log or by object store to know if an object
 to be saved really needs so.

 The flag indicates the alteration of the state of the first level of the
 data members of this object.  This means that it changes only if a
 member directly embedded in the object is changed (numerical value,
 aggregation, array) but not if an associated object (accessed through a
 pointer that is a member) changes (this state indication is not
 "recursive").

 This flag is set to true as default value, so the object can be saved
 anytime even if this flag is not managed.

 @param pi_State Value to give to the flag: true to indicate that the state
                 of the object is altered, false for inverse situation.

 @inheritance Cannot be overriden. If used, this method should be
              called in local implementations of methods @k{UpdateAfterSave}
              and @k{UpdateInternalState}, and in all methods that alter the
              state of the object.

 @see ToBeSaved
-----------------------------------------------------------------------------*/
inline void HPMPersistentObject::SetModificationState(bool pi_State)
    {
    m_ToBeSaved = pi_State;
    }

/**----------------------------------------------------------------------------
 Indicates if the state of this object has changed since the latest save
 or load operation performed with it.  Changes mades to associated
 objects (those accessed through members that are pointers) are not taken
 in consideration.

 This method is used by object log when discarding an object accessed by
 virtual pointers, to know if the save operation can be skipped (if this
 method returns false).  It is also used at the destruction of an object
 store manager for objects still in memory, to save modified objects
 still unsaved.

 @return true if the state of this object has been altered since it was
         loaded or since the latest time it was saved.

 @inheritance Cannot be overriden.
-----------------------------------------------------------------------------*/
inline bool HPMPersistentObject::ToBeSaved() const
    {
    return m_ToBeSaved;
    }

/**----------------------------------------------------------------------------
 Sets a new timestamp value for this object.  The timestamp value is a numeric
 data that represent a point in time that corresponds to the last time the
 state of this object was modified.

 @param pi_Timestamp Timestamp value that identifies a point in time, like a
                     sequence number or a clock value.

 @inheritance Cannot be overriden. To be used by implementations of object
              stores to help implement synchronization routines.

 @see GetTimestamp
 @see Synchronize
-----------------------------------------------------------------------------*/
inline void HPMPersistentObject::SetTimestamp(uint32_t pi_Timestamp)
    {
    m_Timestamp = pi_Timestamp;
    }

/**----------------------------------------------------------------------------
 Returns the latest timestamp of this object.  A timestamp is a numeric
 data that represent a point in time that corresponds to the last time
 when the state of this object was modified.  It can be set for each
 persistent object with method SetTimestamp.  Interpretation of the
 timestamp value is left to the programmer.

 @return The last timstamp value set for this object.

 @inheritance Cannot be overriden.  To be used by implentations of object
              stores to help implement synchronization routines.

 @see SetTimestamp
 @see Synchronize
-----------------------------------------------------------------------------*/
inline uint32_t HPMPersistentObject::GetTimestamp() const
    {
    return m_Timestamp;
    }

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Methods for class HPMShareableObject

//-----------------------------------------------------------------------------
// Default constructor
//-----------------------------------------------------------------------------
template<class T>
inline HPMShareableObject<T>::HPMShareableObject()
    {
    m_RefCount.store(0);
    }

//-----------------------------------------------------------------------------
// Copy constructor.
//-----------------------------------------------------------------------------
template<class T>
inline HPMShareableObject<T>::HPMShareableObject(const HPMShareableObject<T>& pi_rObj)
    {
    m_RefCount.store(0);
    }

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
template<class T>
inline HPMShareableObject<T>::~HPMShareableObject()
    {
    HASSERT(m_RefCount.load() == 0);
    }

//-----------------------------------------------------------------------------
// Assignment operator
//-----------------------------------------------------------------------------
template<class T>
inline const HPMShareableObject<T>&
HPMShareableObject<T>::operator=(const HPMShareableObject<T>& pi_rObj)
    {
    // Nothing to do... which is different from default behavior!
    return *this;
    }

//-----------------------------------------------------------------------------
// Always called by HFCPtr.
//-----------------------------------------------------------------------------
template<class T>
inline void HPMShareableObject<T>::_internal_NotifyAdditionOfASmartPointer()
    {
    //m_Key.ClaimKey();
    ++m_RefCount;
    //m_Key.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// Always called by HFCPtr.
//-----------------------------------------------------------------------------
template<class T>
inline void HPMShareableObject<T>::_internal_NotifyRemovalOfASmartPointer()
    {
    //m_Key.ClaimKey();
    HASSERT(m_RefCount.load() != 0);
    uint32_t RefCount = --m_RefCount;
    //m_Key.ReleaseKey();

    // Delete here because we don't want to keep the key
    // locked in the destruction. Anyways, since the object
    // would have been deleted, there will surely be no other
    // try to use it...

    // Before deleting we may need to save the object, since T
    // is always derived from HPMPersistentObject.

    if (RefCount == 0)
        {
        T* This = (T*)this;
        if (This->GetStore() != 0)
            if (!(This->GetStore()->IsReadOnly()) && (This->ToBeSaved()))
                {
                IncrementRef();
                This->Save();
                DecrementRef();
                }
        if (RefCount == 0)
            delete This;
        }
    }

//-----------------------------------------------------------------------------
// For special circumstances
//-----------------------------------------------------------------------------
template<class T>
inline void HPMShareableObject<T>::IncrementRef()
    {
    //m_Key.ClaimKey();
    ++m_RefCount;
    //m_Key.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// For special circumstances
//-----------------------------------------------------------------------------
template<class T>
inline void HPMShareableObject<T>::DecrementRef()
    {
    //m_Key.ClaimKey();
    HASSERT(m_RefCount.load() != 0);
    --m_RefCount;
    //m_Key.ReleaseKey();
    }

//-----------------------------------------------------------------------------
// For special circumstances
//-----------------------------------------------------------------------------
template<class T>
inline uint32_t HPMShareableObject<T>::GetRefCount() const
    {
    return m_RefCount.load();
    }

END_IMAGEPP_NAMESPACE