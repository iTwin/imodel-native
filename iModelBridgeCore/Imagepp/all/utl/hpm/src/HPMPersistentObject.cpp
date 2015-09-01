//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpm/src/HPMPersistentObject.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPMPersistentObject
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPMPersistentObject.h>
#include <Imagepp/all/h/HPMObjectStore.h>


/**----------------------------------------------------------------------------
 Default constructor for this class.
-----------------------------------------------------------------------------*/
HPMPersistentObject::HPMPersistentObject()
    {
    m_ObjectID = INVALID_OBJECT_ID;
    m_pStore = 0;
    m_ToBeSaved = true;
    m_Timestamp = 0;
    }

/**----------------------------------------------------------------------------
 Copy constructor for this class.  Copying an object does not copy the object
 ID or the store reference because persistence is not a property that can be
 copied.
-----------------------------------------------------------------------------*/
HPMPersistentObject::HPMPersistentObject(const HPMPersistentObject& pi_rObj)
    {
    m_ObjectID = INVALID_OBJECT_ID;  // Nothing to copy! Because persistence
    m_pStore = 0;                    // is not a property that can be copied.
    m_ToBeSaved = true;
    m_Timestamp = 0;
    }


/**----------------------------------------------------------------------------
 Destructor for this class.  Objects are rarely explicitely deleted because
 smart pointers should be used most of the time.
-----------------------------------------------------------------------------*/
HPMPersistentObject::~HPMPersistentObject()
    {
    if (m_pStore)
        m_pStore->UnregisterObject(this);
    }

/**----------------------------------------------------------------------------
 Assignment operator for this class.  Copying an object does not copy the object
 ID or the store reference because persistence is not a property that can be
 copied.

 @param pi_rObj Constant reference to object to duplicate.

 @return Reference to self that allow using this expression as an l-value.

 @inheritance To be called by same operation definition of child classes.
-----------------------------------------------------------------------------*/
HPMPersistentObject& HPMPersistentObject::operator=(
    const HPMPersistentObject& pi_rObj)
    {
    if (&pi_rObj != this)
        {
        if (m_pStore)
            m_pStore->UnregisterObject(this);
        m_ObjectID = INVALID_OBJECT_ID;  // Nothing to copy!
        m_pStore = 0;
        }
    return *this;
    }

/**----------------------------------------------------------------------------
 Returns true if the specified class key identifies the class of this
 object or a class among its ancestors.  Works by using recursive call of
 itself through inheritance tree until answer is found or until the
 topmost ancestor has been queried.

 @param pi_ClassID Class key that identifies the class that user want to
                    know if this object belongs to.

 @return true if object belongs to the specified class (directly or not),
         false otherwise.

 @inheritance This virtual method is automatically overriden by HPM macros.
-----------------------------------------------------------------------------*/
bool HPMPersistentObject::IsCompatibleWith(HCLASS_ID pi_ClassID) const
    {
    // Usually a recursive call to run the inheritance path, it stops here.
    return false;
    }

/**----------------------------------------------------------------------------
 Returns the class key that identify the class of this object.

 To get the class key of an object of a known type, get it directly in its
 static public member @k{CLASS_ID}.

 @return The exact class key of class of this object.

 @inheritance This virtual method is automatically overriden by HPM macros.
-----------------------------------------------------------------------------*/
HCLASS_ID HPMPersistentObject::GetClassID() const
    {
    return HCLASS_ID(0);
    }

/**----------------------------------------------------------------------------
 Returns a pointer to the object store where this object has a persistent
 copy, and where its ID have a specific meaning.  As soon as an object
 was saved to or loaded from an object store, a link between this object
 and that store (where the object continues its life outside program
 execution) is maintained and can be known using this method.

 If this object is not persistent (not attached to a store), this method
 returns zero (NULL).

 @return Pointer to the object store associated to this object, of zero (NULL)
         if there is none.

 @inheritance Cannot be overriden.

 @see HPMObjectStore
-----------------------------------------------------------------------------*/
HPMObjectStore* HPMPersistentObject::GetStore() const
    {
    return m_pStore;
    }

/**----------------------------------------------------------------------------
 Used internally to establish the link between this object and the store
 manager, when the object becomes persistent or when it is loaded from the
 store.

 To dissociate, call this method again with null value.

 @param pi_pStore Pointer to the store manager to link to.

 @inheritance Cannot be overrriden.
-----------------------------------------------------------------------------*/
void HPMPersistentObject::SetStore(HPMObjectStore* pi_pStore)
    {
    HPRECONDITION(pi_pStore ? ((m_pStore == 0) || (m_pStore.operator ==(pi_pStore))) : true);
    m_pStore = pi_pStore;
    }

/**----------------------------------------------------------------------------
 This methods updates the record of this object in its object store.  It saves
 the object in the store where it is persistent (the object must already be
 persistent).

 @inheritance Cannot be overriden.
-----------------------------------------------------------------------------*/
void HPMPersistentObject::Save()
    {
    HPRECONDITION(m_pStore != 0);
    m_pStore->Save(this);
    }


/**----------------------------------------------------------------------------
 Allocates on the heap a new object of exactly the same type than this
 object and copies its state to that new object, making it to be a clone
 of this object, without persistence.  The new object is not linked to
 any store : this method can be used to transfer and object from a store
 to another, by cloning it first then saving its clone to a new store.

 @return A pointer to a newly allocated object whose type and state is the same
         than for this object, excepting the persistence property.   Returns 0
         if this object cannot be cloned.

 @inheritance Default behavior is to return 0.  Ideally all classes that
              inherits from this one should reimplement this method to make
              it create a clone.
-----------------------------------------------------------------------------*/
HPMPersistentObject* HPMPersistentObject::Clone() const
    {
    return 0;
    }

/**----------------------------------------------------------------------------
 Returns the additional memory consumed, in bytes, by this object.
 "Additional memory" corresponds to memory used by data defined outside
 this object, like buffers allocated on the heap, that cannot be found
 with the sizeof operator, and that can change during program execution.
 This additional size does not include size of data that can be
 associated to more than one object.

 If this method is overriden, consider also the use of UpdateUsedSize in
 your code to give real-time information about size changes.

 @return Size, in bytes, of memory used by this object outside of its instance.

 @inheritance Default behavior is to return zero.  If the overriding
              class make use of external buffers and allocated data that is
              not used by more than one instance, this method must be overriden
              to return the correct value.

 @see UpdateUsedSize
-----------------------------------------------------------------------------*/
size_t HPMPersistentObject::GetAdditionalSize() const {
    return 0;
    }

//:Ignore
#if 0 // Pure virtual methods, implementation fake for doc purposes
//:End Ignore

/**----------------------------------------------------------------------------
 This is equivalent of using the sizeof operator on the exact type of
 this object.  This method is useful because it is virtual: it can be
 called for an object of an unknown type (accessed through a pointer) in
 polymorphism situations.  The size obtained excludes any space occupied
 outside the instance, like buffers allocated on the heap: to get the
 total size of this object, additionate results of a call to this method
 and of a call to the method @k{GetAdditionalSize}.

 @return Instance size of this object, like doing sizeof on its exact type.

 @inheritance This virtual method is automatically overriden by HPM macros.

 @see GetAdditionalSize
 @see UpdateUsedSize
-----------------------------------------------------------------------------*/
size_t HPMPersistentObject::GetObjectSize() const {
    return 0;
    }

//:Ignore
#endif
//:End Ignore
