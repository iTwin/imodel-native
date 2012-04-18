/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/auiitem.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
A Uiitem is an instance of the BE Display Schema. 
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAUIItem : public RefCountedBase // Content Service Element
    {
    protected:
        
        virtual IAUIItemCP          _GetParent () const = 0;
        virtual IAUIDataContextCP   _GetDataInstance() const = 0;

    public:

        //! Get the parent instance associated with this instance.
        ECOBJECTS_EXPORT IAUIItemCP        GetParent () const;

        //! Utility function to evaluate whether the given control is visible.
        //ECOBJECTS_EXPORT bool             IsVisible () const;

        //! Get the data instance bind with this ui instance.
        ECOBJECTS_EXPORT IAUIDataContextCP GetDataInstance() const;

        //! Get the command associated with this ui instance if any
        ECOBJECTS_EXPORT UICommandPtr GetCommand () const;

        //! Do the action associated with the ui item using the bound data instance.
        ECOBJECTS_EXPORT BentleyStatus ExecuteAction () const;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IAUIItemInfo // View
    {
    enum ItemType
        {
        MenuItem,
        DataGrid,
        TreeView,
        ListView,
        Panel,
        DgnViewPort,
        };
    
    private:
        
        ItemType  m_itemType;

    public:
    IAUIItemInfo (ItemType itemType)
        :m_itemType (itemType)
        {}

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IAUIDataContext // Query
    {
    enum ContextType
        {
        Instance,
        ECQuery,
        Custom,
        };

    virtual ContextType             GetContextType() const = 0;
    virtual IECInstanceP            GetInstance () const {return NULL;}
    virtual DgnPlatform::ECQueryCP  GetQuery () const {return NULL;}
    virtual void*                   GetCustomData() const {return NULL;}
    virtual ~IAUIDataContext () {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  AUIInstanceDataContext : public IAUIDataContext
    {
    private:
        IECInstancePtr m_instance;
        
    public:
        AUIInstanceDataContext (IECInstanceR instance)
            :m_instance(&instance)
            {}
        
        virtual ContextType GetContextType() const override {return IAUIDataContext::Instance;}
        virtual IECInstanceP GetInstance () const override {return m_instance.get();}
    };

END_BENTLEY_EC_NAMESPACE