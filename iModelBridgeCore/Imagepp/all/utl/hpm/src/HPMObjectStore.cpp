//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hpm/src/HPMObjectStore.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HPMObjectStore
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HPMObjectStore.h>
#include <Imagepp/all/h/HPMPersistentObject.h>
#include <Imagepp/all/h/HFCMonitor.h>

/**----------------------------------------------------------------------------
 This is the constructor for this class.  At this generic level, the
 constructor only establish a link between the store and a "default
 object pool".  This link is mandatory and is used by automatic loaders
 when they are initialized while not assigned to a specific pool:
 the default pool of the store used by a loader will be chosen by
 that loader for its memory management duties. See the documentation of
 HPMLoader to know more about automatic memory management.

 There is a global default log, that defines infinite memory space (so it
 does not cause object discarding) that can be used here, by taking its
 address:  @c{&g_DefaultPool}.

 @param pi_pDefaultPool Pointer to the pool that will be considered as the
                        @t{default pool} for loaders refering to objects
                        that persist in this store.

 @inheritance Must be called by child classes.  Usually the constructor
              of a child class will require a pointer to the default pool
              to be provided by class user and will be passed to this
              constructor. The destructor of each class that inherits
              from this one must call @k{CleanUp}.

 @see HPMPool
-----------------------------------------------------------------------------*/
HPMObjectStore::HPMObjectStore(HPMPool* pi_pDefaultPool)
    : m_pPool(pi_pDefaultPool)
    {
    HPRECONDITION(pi_pDefaultPool != 0);
    }

/**----------------------------------------------------------------------------
 Destructor for this class.

 @inheritance The destructor of each class that inherits
              from this one must call @k{CleanUp}.
-----------------------------------------------------------------------------*/
HPMObjectStore::~HPMObjectStore()
    {
    HDEBUGCODE(HFCMonitor Monitor(m_Key);)
    HASSERT(m_RegisteredObjects.size() == 0);
    }

/**----------------------------------------------------------------------------
 Save all objects that are persistent in this store and that are actually
 present in program memory.

 The main purpose of this method is to synchronize the store with the
 memory before performing clean-up.  This method must be called before
 calling CleanUp.

 You can call it for any other situation where you may want to
 synchronize in a single shot the state of the object store with states
 of objects in memory.  Beware that objects must already be persistent
 the store (being saved or loaded from it).  Objects that have their
 modification flag set to false (see @k{ToBeSaved}) are not saved.

 @inheritance Cannot be overriden.  Must be called by the destructor of
             derived classes, before calling CleanUp.

 @see CleanUp
-----------------------------------------------------------------------------*/
void HPMObjectStore::SaveAll()
    {
    HFCMonitor Monitor(m_Key);

    if (!IsReadOnly())
        {
        RegisteredObjectList::iterator itr(m_RegisteredObjects.begin());
        while (itr != m_RegisteredObjects.end())
            {
            HPMPersistentObject* pObj = (*itr).second;
            if (pObj->ToBeSaved())
                {
                Save(pObj);
                pObj->SetModificationState(false);
                }
            ++itr;
            }
        }
    }

/**----------------------------------------------------------------------------
 Performs clean-up before the store manager destruction, and resolves all
 virtual pointers in order to make them point to objects physically
 present in program memory.  All persistent objects using this store that
 are still present in memory are "disconnected" from it, becoming normal
 runtime objects.

 To be called at destruction time.

 @inheritance This method cannot be overriden but must be called by the
              destructor defined in each class that inherits from this one.
              NOTE: always call @k{SaveAll} before calling this method.

 @see SaveAll
-----------------------------------------------------------------------------*/
void HPMObjectStore::CleanUp()
    {
    HFCMonitor Monitor(m_Key);

    RegisteredObjectList::iterator itr(m_RegisteredObjects.begin());
    while (itr != m_RegisteredObjects.end())
        {
        HPMPersistentObject* pObj = (*itr).second;
        pObj->SetStore(0);
        pObj->SetModificationState();
        ++itr;
        }
    m_RegisteredObjects.erase(m_RegisteredObjects.begin(), m_RegisteredObjects.end());
    }

/**----------------------------------------------------------------------------
 Returns the default object pool that can be used with this store for
 memory management purposes.  This is the pool that is provided to loaders
 created from this store manager.

 @return A pointer to the default pool.

 @inheritance Cannot be overriden.

 @see HPMPool
-----------------------------------------------------------------------------*/
HPMPool* HPMObjectStore::GetPool() const
    {
    return m_pPool;
    }

/**----------------------------------------------------------------------------
 Checks if the object identified by the specified ID can be found in
 program memory.  If so, it returns its address, otherwise it returns
 zero.

 @param pi_ObjectID Object ID of object to look for in physical memory.

 @return A pointer to the requested object if found in memory, or zero (NULL)
         if the object is not loaded.

 @inheritance Cannot be overriden.
-----------------------------------------------------------------------------*/
HPMPersistentObject* HPMObjectStore::GetLoaded(HPMObjectID pi_ObjectID) const
    {
    HPRECONDITION(pi_ObjectID != INVALID_OBJECT_ID);

    // Get the object from the registered object list
    HFCMonitor Monitor(m_Key);
    RegisteredObjectList::const_iterator itr = m_RegisteredObjects.find(pi_ObjectID);

    HPMPersistentObject* pObj = (itr == m_RegisteredObjects.end()) ? 0 : (*itr).second;

    return pObj;
    }

/**----------------------------------------------------------------------------
 Registers the physical presence of an object attached to this store.
 This method must be called each time an object receives an object ID to
 maintain a list of loaded objects that can be used by the method
 @k{GetLoaded}.

 @param pi_pObj Pointer to an object loaded/saved in this store.

 @inheritance This method cannot be overriden.  Each class that derives
              from this one must call this method everytime an object in
              memory is linked to the store and identified (after loading,
              or when saving an object for the first time, or if an object
              gets a new ID).

 @see GetLoadedObject
 @see UnregisterObject
-----------------------------------------------------------------------------*/
void HPMObjectStore::RegisterObject(HPMPersistentObject* pi_pObj)
    {
    HPRECONDITION(pi_pObj != 0);
    HFCMonitor Monitor(m_Key);
    m_RegisteredObjects.insert(
        RegisteredObjectList::value_type(pi_pObj->GetID(), pi_pObj));
    }

/**----------------------------------------------------------------------------
 Protected method that is called by the destructor of the class
 HPMPersistentObject to remove the address of an object, being deleted,
 from the list of loaded objects maintained by this store.

 @param pi_pObj Pointer to the object that will be deleted.

 @inheritance This method cannot be overriden.

 @see RegisterObject
 @see GetLoadedObject
-----------------------------------------------------------------------------*/
void HPMObjectStore::UnregisterObject(HPMPersistentObject* pi_pObj)
    {
    HPRECONDITION(pi_pObj != 0);
    HFCMonitor Monitor(m_Key);
    RegisteredObjectList::iterator Itr(m_RegisteredObjects.find(pi_pObj->GetID()));
    if (Itr != m_RegisteredObjects.end())
        m_RegisteredObjects.erase(Itr);
    }

//:Ignore
#if 0          // Pure virtual methods that are not implemented for real
//:End Ignore  // For doc purposes

/**----------------------------------------------------------------------------
 This method "saves" the specified object in this store.

 The behavior will depend on the actual relation between this store and
 the object to save. If the object is already attached to this store,
 meaning that there is data for this object in the store, data will be
 updated with the data found in the specified object.  If no link exists
 at that time, an object entry will be created in the store, data will be
 saved in that entry and the store will establish the link between itself
 and the object (by attaching the object to itself and giving it an
 object ID).

 A simpler way is to call the method @k{SaveOn} or the method @k{Save} on the
 persistent object, which uses the method @k{Save} internally.

 If the object is already present in the store, it must not be locked in
 any way except by this manager.  If locked, the execution of the program
 is paused until the lock is removed.

 @param pi_pObj Pointer to object to save

 @inheritance This pure virtual method must be overriden for each store
              implementation.  If the object store supports only a
              limited range of classes, the implementation should ask the
              class key of the object to save and perform saving
              specifically for that kind of object.  If the store support
              all classes of persistent objects, the implementation will
              need to use the class dictionary for the object to save.

              If the object gets an ID for the first time (it wasn't
              persistent before), it must be linked to this store by
              calling its @k{SetStore} method, dentified by using
              @k{SetID}, then registered in the list of loaded objects by
              calling @k{RegisterObject}.

              If the object is already present in the store, and the
              manager doesn't own a lock for it, the object must be
              temporarily locked for the duration of writing.  This
              prevent unwanted effects on eventual concurrent loading
              that may occur from another thread or process.

 @see HPMPersistentOBject::Save
-----------------------------------------------------------------------------*/
void HPMObjectStore::Save(HPMPersistentObject* pi_pObj) { }

/**----------------------------------------------------------------------------
 Returns a boolean value that indicates if this store can only be read.
 This method is used by the memory management mechanism to know if an
 object can be saved when it is discarded.  This makes memory management
 of objects loaded from read-only stores possible.

 @return true if store can only be read, false if objects can be saved in it.

 @inheritance This pure virtual method must be overriden for each store
              implementation.
-----------------------------------------------------------------------------*/
bool HPMObjectStore::IsReadOnly() const { }

/**----------------------------------------------------------------------------
 This method is used to force the store to behave as a read-only store.
 It may be used to disable object storing .

 @param pi_ReadOnly Boolean indicating if we want to force the store as
                    read-only or not.

 @inheritance This pure virtual method must be overriden for each store
              implementation.

 @see IsReadOnly
-----------------------------------------------------------------------------*/
void HPMObjectStore::ForceReadOnly(bool pi_ReadOnly) { }

//:Ignore
#endif
//:End Ignore