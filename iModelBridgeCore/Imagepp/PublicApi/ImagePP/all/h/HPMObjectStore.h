//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMObjectStore.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPMObjectStore
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HFCExclusiveKey.h>
#include <Imagepp/all/h/HPMClassKey.h>

BEGIN_IMAGEPP_NAMESPACE
class HPMPool;
class HPMPersistentObject;


/**

    This class is defined to give a common interface to manage @t{object stores}
    that will work with the HPM persistence model.  An object store is any
    kind of storage unit that can store data in some organized way; that
    object can live longer than duration of a program, and will be used
    through the HPM persistence mechanism to hold data for objects in
    programs that will become @t{persistent}, giving a new lifetime to these
    objects.  The class HPMObjectStore defines an interface that will be
    used to understand and use a storage unit as an object database.

    Under this class there will be many @t{implementation classes} that will act
    as @t{adaptors} between this "object database" concept and the actual
    implementation of storage units.  They will play the role of adaptors,
    because file formats usually do not give an interpretation of data and
    do not implement storage of data using an object oriented approach.
    Here, any kind of file can be used like an object database, sometimes
    using a restricted range of classes supported for persistence, that
    range changing between type of stores, but always using same interface.

    In the case of @t{versatile stores}, which are storage units that can hold
    any kind of objects (like relational databases and object databases),
    all classes of persistent objects that respect the HPM model are
    supported.  This gives the possibility to create applications that will
    do polymorphic usage of different database technologies without specific
    support for any type of database.

    Instances of classes that inherit from HPMObjectStore are called @t{store
    managers}.

    The interface defined here provide services in the following areas:

    @list{@b{Advanced content access} : methods and companion classes that provide
          advanced ways to access contents of stores, like scanning by type,
          finding by key, etc.}

    @list{@b{Concurrent access management} : methods for object locking, for
          synchronization between memory and store, and long transaction control.}

    @list{@b{Relation between the store manager and its persistent objects} :
          there is a relation between persistent objects in program memory and the
          store where they live outside of the program's life.  Part of this
          relation is managed by the HPMObjectStore class: a store always knows
          all objects in program memory that belong to it (that were saved and/or
          loaded from itself, we call this object tracking).  This topic is well
          covered in the presentation of the HPMPersistentObject class.}

    @list{@b{Utility methods} : miscellaneous methods helpful for HPM persistence
          model and its users: association to a default object log, persistent
          description of store for easy reuse, read-only detection.}

    Interface for object stores is not limited to this class.  It also
    includes the notion of store reader and some predefined kinds of readers
    that are provided in HPM.  See the documentation of HPMStoreReader for
    more information about this part of the persistence model.  It also
    includes the notion of @t{store descriptor} defined by base class
    HPMStoreDescriptor: see also its documentation for details.

    A store manager cannot be shared by many threads, no concurrent usage is
    allowed on a store manager.  To share a store, create many managers on
    the same store, one manager for each thread.

    This class implements some basic services but requires that most of the
    interface be defined by child classes.  See the following sections to
    discover the methods to override and how to override them.


    @see HPMPool
    @see HPMPersistentObject
*/

class HPMObjectStore : public HFCShareableObject<HPMObjectStore>
    {
public:

    // Class ID for this class.
    HDECLARE_BASECLASS_ID(HPMObjectStoreId_Base)

    //:> Primary methods

    IMAGEPP_EXPORT                         HPMObjectStore(HPMPool* pi_pDefaultPool);
    IMAGEPP_EXPORT virtual                 ~HPMObjectStore();

    //:> Persistence methods : load/save methods
    virtual void            Save(HPMPersistentObject* pi_pObj) = 0;

    //:> Store management methods
    virtual bool            IsReadOnly() const = 0;
    virtual void            ForceReadOnly(bool pi_ReadOnly) = 0;

    //:> Other methods

    IMAGEPP_EXPORT HPMPool*                GetPool() const;
    IMAGEPP_EXPORT void                    SaveAll();
    IMAGEPP_EXPORT void                    CleanUp();

protected:

    friend class HPMPersistentObject;

    //:> Object tracking

    IMAGEPP_EXPORT HPMPersistentObject*    GetLoaded(HPMObjectID pi_ObjectID) const;
    IMAGEPP_EXPORT void                    RegisterObject(HPMPersistentObject* pi_pObj);
    IMAGEPP_EXPORT void                    UnregisterObject(HPMPersistentObject* pi_pObj);

private:

    HPMPool*                m_pPool;

    // Local types

    typedef map<HPMObjectID, HPMPersistentObject*>
    RegisteredObjectList;

    // This map is used to find persistent objects actually loaded in
    // physical memory

    RegisteredObjectList    m_RegisteredObjects;

    // This stack is used by long transactions, to have the possibility to
    // restore the state of the list of registered objects when aborting a transaction

    mutable HFCExclusiveKey m_Key;
    };


END_IMAGEPP_NAMESPACE
