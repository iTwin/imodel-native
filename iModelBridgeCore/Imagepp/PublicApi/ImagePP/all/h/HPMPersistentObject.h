//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPMPersistentObject.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HPMPersistentObject
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HPMClassKey.h"

BEGIN_IMAGEPP_NAMESPACE

class HPMObjectStore;
class HFCExclusiveKey;


/**--------------------------------------------------------------------------

 Ancestor class for all persistent objects, acting as a replacement for the
 HFCShareableObject class to make them usable by HFCPtr and HPMLoader while
 having some behavior specific to persistence.

 Use this class as second ancestor everywhere HPMPersistentObject is
 declared as ancestor.  Type "T" must be the name of the class being defined.

*/


template<class T> class HPMShareableObject
    {
public:

    typedef HPMShareableObject<T>  ShareableAncestor;
    typedef HPMShareableObject<T>* ShareableAncestorPtr;

    HPMShareableObject();
    HPMShareableObject(const HPMShareableObject<T>& pi_rObj);
    ~HPMShareableObject();
    const HPMShareableObject<T>& operator=(const HPMShareableObject<T>& pi_rObj);


    //:> These methods should be protected, since they are used only by
    //:> smart pointers.  But we cannot give friendship to template
    //:> classes.  So we use names that are longer than usual :)

    void                         _internal_NotifyAdditionOfASmartPointer();
    void                         _internal_NotifyRemovalOfASmartPointer();

    //:> These methods are sometimes required to protect the object

    void                         IncrementRef();
    void                         DecrementRef();
    uint32_t                     GetRefCount() const;

private:

    BeAtomic<uint32_t>    m_RefCount;
    //HFCExclusiveKey              m_Key;
    };

/**--------------------------------------------------------------------------

    This class is one of the basic classes of HPM library.  Its purpose is
    to give a common interface to all kinds of objects that can be
    persistent and that will use HPM as persistence model.  In other words,
    this class is the root of all the class hierarchy for persistent objects
    of a given system.  All the HPM model is based on this class, and does
    not know any other type for persistent objects.

    The interface of this class defines some services that deal with object
    identification (including type identification), that manage relations
    with object stores (where persistent objects must persist, if they do)
    and that give some other informations about persistent objects.   These
    methods are required and used internally by HPM and the persistence
    libraries built upon HPM, but some can be useful for programs using a
    child class.

    A part of this interface is implemented at this level and does not need
    to be redefined elsewhere.  The other part is only "interface
    definition", forcing all classes that inherits this one to give an
    implementation.

    Thus, to make a class to be able to create "persistable instances", all
    the programmer have to do is to make sure that the class inherits from
    HPMPersistentObject, either directly or not (through another class).
    That class must also provide a default constructor (one without
    parameters).  Then he has to provide implementation for a certain set of
    methods: all of this is done by using a pair of macros provided with
    HPM:

    @list{@k{HPM_DECLARE_CLASS_DLL} or @k{HPM_DECLARE_CLASS_DLL(} :  a call to this macro
          must be placed inside the class declaration (where members are
          declared);}

    @list{@k{HPM_REGISTER_CLASS} or @k{HPM_REGISTER_TEMPLATE} : a call to this macro
          must be placed in source code that is compiled once in a given program,
          like the .CPP file of class.}

    These macros declare and define overrides for all pure virtual methods,
    associates the class with a unique key that can identify the type of its
    instances at runtime, registers the class in the global factory and do
    some preparation work for the global dictionary to make possible its
    use.

    These macros define three @b{static public members} for each class:

    @list{@k{s_pDictionary} Pointer to the class dictionary that describes the
          contents of the class.  It is setted automatically as soon as one member
          of the class is registered.  Otherwise, no dictionary is associated to
          the class and this pointer points to NULL (contains zero).}

    @list{@k{CLASS_ID} The class key, an object that can be used to differentiate
          class types at runtime, and to know the type of an instance also at
          runtime.  It is unique for a given version of a given class in a given
          system.}

    @list{@k{s_InstanceSize} The size of an instance of the class, as the sizeof
          operator would give (size that excludes any dynamicly allocated
          buffers).}

    @h3{Relations between persistent objects and object stores}

    An instance of a class that derives from HPMPersistentObject is commonly
    labeled "persistent object", even it is not actually persistent (not
    saved in an object store).  A persistent object is an object that has
    the ability to persist, but it is not mandatory that the object is
    really persistent.  Because of this, the relation betwen a persistent
    object and an object store does not always exists.

    When an object is made persistent, it has a new lifetime and some kind
    of new identity.  From that point, the object is considered to be
    "transferred" into the store, and temporarily copied into program memory
    when it is needed.  An object store can be seen as "persistent memory"
    that is disjoint from program memory space.  When object is removed from
    the store, it is transferred back to memory, and the object returns back
    to non-persistent state.

    So when persistent object does really "persist", this means that the
    physical instance (in program memory) is linked to an object store.
    This link is established as soon as the object is saved in the store, or
    when the object's data is read from the store.  From that moment, the
    instance in program memory is considered to take the role of "being"
    that persistent object; but as soon as it is deleted from program
    memory, it is the data saved in the store for that object that becomes
    the persistent object.  Following this approach, a persistent object is
    either "living in memory" or "sleeping in object store", but never both
    at sime time, even when the data is still in the store when object is in
    memory (which is what really happens).

    To distinguish between these two states, the object store manager always
    know all instances in program memory that correspond to objects saved in
    it.

    At the other end of the relation, the persistent object need also to
    know if it is the "living" part of a persistent object existing in a
    store.  This relation is concretized as an "attachment to a store" for
    the persistent object.  When a persistent object is really persistent,
    it holds:

    @list{a smart pointer to an instance of HPMObjectStore-derived class that
          describes and manipulates the object store holding object's data;}

    @list{a numeric identifier, labeled object ID, that describes the identity
          of the object into the attached store.  This ID is choosen and
          understood only by the object store's management mechanism.}

    Each persistent object is identified by a numeric value that distinguish
    it from other objects saved in same store.  This object ID cannot be
    used to manage identity of objects in a given program, because objects
    may come from different stores (possibility of same ID occurring more
    than once) or can be non-persistent (having then no ID at all).  In
    program memory, object address is still the way to recognize an object
    from another.

    The object store manager remains in memory as long as there are
    persistent objects associated to it that are loaded in memory.  This is
    done by the use of smart pointers to establish the link between object
    and stores, since a store manager is a shareable object that is always
    managed in memory by using smart pointers.

    @h3{Runtime type identification}

    To help distinguish object types when they are saved in store, or
    between objects accessed through pointers in program memory (i.e. in
    polymorphism), each class is tagged by a kind of identifier that is
    associated to instances of that class.  This identifier, the class key,
    should be unique for a given system (a system can be composed of many
    programs and many object stores), this unicity enclosing the fact that
    there may be many versions of same class living in same system (i.e. : a
    single object store used by two versions of an application).  Class keys
    can be manipulated (copied, compared, etc.) as concrete data type
    objects.

    The programmer who create a class of persistent objects will be
    requested to provide a numeric class ID and a version ID that will be
    used to construct a class key for that class.

    @h3{Other notes}

    If a given class "A" define reference to object of another class "B",
    through aggregation, pointers or array of pointers, and that class "B"
    defines a pointer to the owner of type "A", then there is a situation of
    reference looping (two objects that refer each other).  The HPM
    persistence model supports this in all its variants (direct looping and
    indirect looping via intermediate agents), but some precaution must be
    done.   When loading objects, recursives loops are broken since once an
    object has been loaded, it is not loaded again.  This works well unless
    automatic loaders are used with too small memory space allocated to them:
    if the total size of objects loaded in a loop is bigger than memory size
    reserved by the object log, infinite loading will occur.

    On the other side, unlimited recursive saving should not occur but this
    responsability is given to each implementation of object store managers:
    be very careful that the version you use or define supports this.

    The programmer must provide a default constructor for each class that
    inherits from HPMPersistentObject.  This default constructor must place
    the object in a state that makes it to be copiable.  The object may
    still be in a kind of "invalid" state needing further initialization but
    copy construction from it must work, giving other objects in same state.
    This default constructor must also give default values for members that
    will not be persistent.


*/

class HNOVTABLEINIT HPMPersistentObject
    {
public:

    // Class ID for this class.

    enum { CLASS_ID = 0 };

    //:> Primary methods

    IMAGEPP_EXPORT                      HPMPersistentObject();
    IMAGEPP_EXPORT                      HPMPersistentObject(const HPMPersistentObject& pi_rObj);
    IMAGEPP_EXPORT virtual              ~HPMPersistentObject();
    IMAGEPP_EXPORT HPMPersistentObject& operator=(const HPMPersistentObject& pi_rObj);

    //:> Identification of object

    HPMObjectID                         GetID() const;
    void                                SetID(HPMObjectID pi_ObjectID);
    virtual HCLASS_ID                   GetClassID() const;
    IMAGEPP_EXPORT virtual bool                 IsCompatibleWith(HCLASS_ID pi_ClassID) const;
    
    //:> Link to a store

    IMAGEPP_EXPORT HPMObjectStore* GetStore() const;
    IMAGEPP_EXPORT void           SetStore(HPMObjectStore* pi_pStore);

    //:> Persistence methods
    IMAGEPP_EXPORT void           Save();

    //:> State management
    IMAGEPP_EXPORT virtual HPMPersistentObject* Clone() const;

    //:> Utility
    virtual size_t        GetObjectSize() const = 0;
    IMAGEPP_EXPORT virtual size_t GetAdditionalSize() const;
    void                  SetModificationState(bool pi_State = true);
    bool                  ToBeSaved() const;
    void                  SetTimestamp(uint32_t pi_Timestamp);
    uint32_t             GetTimestamp() const;

    //:> To make possible HFCPtr<HPMPersistentObject>

    virtual void          AddHFCPtr() = 0;
    virtual void          RemoveHFCPtr() = 0;

private:

    friend class HPMObjectStore;

    HFCPtr<HPMObjectStore>  m_pStore;
    HPMObjectID             m_ObjectID;
    uint32_t               m_Timestamp;
    bool                    m_ToBeSaved;
    };

END_IMAGEPP_NAMESPACE

//---------------------------------------------------------------------------
// These inclusions made here allow including only this file where
// persistence is required without including all other files that
// are needed.

#include "HPMMacros.h"
#include "HPMPersistentObject.hpp"


