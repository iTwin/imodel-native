/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auiitem.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#include <ECObjects\ECObjectsAPI.h>
BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
A Uiitem is an instance of a control facing a user. 
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAUIItem : public RefCountedBase // Content Service Element
    {
    protected:
        
        virtual IAUIItemCP          _GetParent () const {return NULL;}
        virtual IAUIDataContextCP   _GetDataInstance() const = 0;
        virtual IAUIItemInfoCR      _GetUIItemInfo () const = 0;
    public:
        //! Get the data instance bind with this ui instance.
        ECOBJECTS_EXPORT IAUIDataContextCP GetDataInstance() const;

        //! Get the command associated with this ui instance if any
        ECOBJECTS_EXPORT IAUIItemInfoCR GetUIItemInfo () const;

        //! Do all the actions associated with the ui item using the bound data context.
        ECOBJECTS_EXPORT BentleyStatus ExecuteActions() const;

        //! Get the parent instance associated with this instance.
        ECOBJECTS_EXPORT IAUIItemCP     GetParent () const;
    };

/*---------------------------------------------------------------------------------**//**
A AUIITemInfo describes the type of a control in a platform independent fashion.
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IAUIItemInfo 
    {
    //! Describes the primitive UI type an item info represents.
    enum ItemType
        {
        Menu,
        MenuItem,
        ToolBar,
        ToolBarItem,
        DataGrid,
        TreeView,
        ListView,
        Panel,
        DgnViewPort,
        };
    
    protected:
        virtual bool        _IsAggregatable () const = 0;
        virtual ItemType    _GetItemType() const = 0;

    public:
    //Constructor that initializes an item from a primitive type.
    virtual ~IAUIItemInfo ()
        {}

    ECOBJECTS_EXPORT bool IsAggregatable() const;
    ECOBJECTS_EXPORT ItemType GetItemType () const;

    };

/*---------------------------------------------------------------------------------**//**
//! A variant class which describes the data that is represented in the UI. The view definition
//! provider, and data context work in unison to describe the UI.
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IAUIDataContext // Query
    {
    //!Describes the different type of context available to the user
    enum ContextType
        {
        Custom                  = 0,
        InstanceID              = 1,
        Instance                = 1<<1,
        ECInstanceCollection    = 1<<2,
        //TODO Move this enum to a string for repository dependant values
        DgnECInstanceCollection = 1<<3,
        DgnHitPathInfo          = 1<<4,
        DgnActionItemInfo       = DgnECInstanceCollection | DgnHitPathInfo,
        ECGroupingNode          = 1<<5,
        NodeCollection          = 1<<6,

        };
    
    //! Get context type which can be used to call the appropriate Get function
    virtual ContextType             GetContextType() const = 0;

    //!Get the data instance that this datacontext stores.
    virtual IECInstanceP            GetInstance () const {return NULL;}
    virtual WString                 GetMoniker () const {return NULL;}
    virtual void*                   GetCustomData() const {return NULL;}
    virtual ECInstanceIterableCP    GetInstanceIterable () const {return NULL;}
    //!Virtual destructor
    virtual ~IAUIDataContext () {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceIterableDataContext: public IAUIDataContext
    {
    private:
    ECInstanceIterable m_data;

    public:
        ECInstanceIterableDataContext (ECInstanceIterable const& data)
            :m_data(data)
            {}
        virtual ContextType             GetContextType() const {return ECInstanceCollection;}
        virtual ECInstanceIterableCP    GetInstanceIterable () const {return &m_data;}
    };

/*---------------------------------------------------------------------------------**//**
//! A class which describes the data that is backed by a single ECInstance in the UI.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename InstanceType>
struct  AUIInstanceDataContext : public IAUIDataContext
    {
    private:
        RefCountedPtr<InstanceType> m_instancePtr;
        
    public:
        AUIInstanceDataContext (InstanceType& instance)
            :m_instancePtr(&instance)
            {}
        
        virtual ContextType GetContextType() const override {return IAUIDataContext::Instance;}
        virtual IECInstanceP    GetInstance () const override {return m_instancePtr.get();}
        InstanceType*  GetDataInstnce () const {return m_instancePtr.get();}
    };

/*---------------------------------------------------------------------------------**//**
//! A class which describes the data that is backed by a single ECInstance in the UI.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ECGroupingNodeDataContext : public IAUIDataContext
    {
    private:
        WString m_nodeMoniker;
        
    public:
        ECGroupingNodeDataContext (WString moniker)
            :m_nodeMoniker (moniker)
            {}
        
        virtual ContextType GetContextType() const override { return IAUIDataContext::ECGroupingNode; }
        virtual WString     GetMoniker () const override { return m_nodeMoniker; }
    };

/*---------------------------------------------------------------------------------**//**
//! A class which describes the data that is backed by a single ECInstance in the UI.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ECNodeCollectionDataContext : public IAUIDataContext
    {
    private:
        ECInstanceIterable m_data;
        WString m_labels;
        void* m_customData;
        
    public:
        ECNodeCollectionDataContext (WString labels, ECInstanceIterable const& data, void *customData)
            :m_labels (labels), m_data (data), m_customData (customData)
            {}
        
        virtual ContextType GetContextType() const override { return IAUIDataContext::NodeCollection; }
        virtual WString     GetMoniker () const override { return m_labels; }
        virtual ECInstanceIterableCP    GetInstanceIterable () const { return &m_data; }
        virtual void*                   GetCustomData() const {return m_customData;}
    };

END_BENTLEY_EC_NAMESPACE