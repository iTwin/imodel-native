/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__
#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/Update.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
//! An interface for a class which listens for when ECClasses are used in specific ECDb.
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                07/2016
//=======================================================================================
struct IECDbUsedClassesListener
{
protected:
    //! Virtual destructor.
    virtual ~IECDbUsedClassesListener() {}

    //! Called when an ECClass is used in the context of the supplied connection.
    //! @param[in] db The ECDb where the class was used.
    //! @param[in] ecClass The ECClass that was used.
    //! @param[in] polymorphically Was the class used polymorphically.
    virtual void _OnClassUsed(ECDbCR db, ECClassCR ecClass, bool polymorphically) = 0;

public:
    //! @see _OnClassUsed
    void NotifyClassUsed(ECDbCR db, ECClassCR ecClass, bool polymorphically) {_OnClassUsed(db, ecClass, polymorphically);}
};

//=======================================================================================
//! Broadcasts ECInstanceChange events to registered handlers.
//!
//! @warning When a many-to-many relationship is inserted/updated/deleted, the source 
//! implementation is expected to notify about an update of instances on both ends of the 
//! relationship. In case of one-to-many relationship, notifying about an update of the
//! instance that has the foreign key is enough.
//!
//! @note This class doesn't broadcast ECInstanceChange events by itself, it just provides
//! means for subclasses to do that.
//!
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                01/2016
//=======================================================================================
struct ECInstanceChangeEventSource : RefCountedBase, IECDbUsedClassesListener
{    
    //===================================================================================
    //! Information about a changed ECInstance.
    // @bsiclass                                    Grigas.Petraitis            01/2016
    //===================================================================================
    struct ChangedECInstance
    {
    private:
        ECClassCP m_class;
        ECInstanceId m_instanceId;
        ChangeType m_changeType;
    public:
        //! Constructor. Creates an invalid object.
        ChangedECInstance() : m_class(nullptr) {}
        //! Copy constructor.
        ChangedECInstance(ChangedECInstance const& other) 
            : m_class(other.m_class), m_instanceId(other.m_instanceId), m_changeType(other.m_changeType) 
            {}
        //! Constructor.
        //! @param[in] ecClass The ECClass of the changed ECInstance.
        //! @param[in] instanceId The ID of the changed ECInstance.
        //! @param[in] changeType Type of the change.
        ChangedECInstance(ECClassCR ecClass, ECInstanceId instanceId, ChangeType changeType) 
            : m_class(&ecClass), m_instanceId(instanceId), m_changeType(changeType) 
            {}
        //! Compare operator for using in sets and maps
        bool operator<(ChangedECInstance const& other) const
            {
            return m_class < other.m_class
                || (m_class == other.m_class && m_instanceId < other.m_instanceId)
                || (m_class == other.m_class && m_instanceId == other.m_instanceId && (int)m_changeType < (int)other.m_changeType);
            }
        //! Is this object valid.
        bool IsValid() const {return nullptr != m_class;}
        //! Get the ECClass of the changed ECInstance.
        ECClassCP GetClass() const {return m_class;}
        //! Get the ID of the changed ECInstance.
        ECInstanceId GetInstanceId() const {return m_instanceId;}
        //! Get the type of the change.
        ChangeType GetChangeType() const {return m_changeType;}
    };
    
    //===================================================================================
    //! An interface for a class that listens for ECInstance changes.
    // @bsiclass                                    Grigas.Petraitis            01/2016
    //===================================================================================
    struct IEventHandler
    {
    friend struct ECInstanceChangeEventSource;
    protected:
        //! Virtual destructor.
        virtual ~IEventHandler() {}

        //! Called by ECInstanceChangeEventSource to notify about changed ECInstances.
        //! @param[in] db The ECDb where the change happened.
        //! @param[in] changes The list of changes that happened.
        virtual void _OnECInstancesChanged(ECDbCR db, bvector<ChangedECInstance> changes) = 0;
    };

private:
    bset<IEventHandler*> m_eventHandlers;
    
protected:
    //! @see IECDbUsedClassesListener::_OnClassUsed
    virtual void _OnClassUsed(ECDbCR, ECClassCR, bool polymorphically) override {}

    //! A method that subclasses should call to notify listeners about changed ECInstance.
    //! @note If more than one ECInstance changes at a time, it's recommended to call @ref NotifyECInstancesChanged
    //! instead.
    ECPRESENTATION_EXPORT void NotifyECInstanceChanged(ECDbCR db, ChangedECInstance change) const;

    //! A method that subclasses should call to notify listeners about changed ECInstances.
    ECPRESENTATION_EXPORT void NotifyECInstancesChanged(ECDbCR db, bvector<ChangedECInstance> changes) const;

public:
    //! Registers an event handler.
    void RegisterEventHandler(IEventHandler& handler) {m_eventHandlers.insert(&handler);}

    //! Unregisters an event handler.
    void UnregisterEventHandler(IEventHandler& handler) {m_eventHandlers.erase(&handler);}
};
typedef RefCountedPtr<ECInstanceChangeEventSource> ECInstanceChangeEventSourcePtr;

END_BENTLEY_ECPRESENTATION_NAMESPACE
