/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/uiitem.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

namespace Bentley { namespace DgnPlatform {
    struct ECQuery;
    typedef ECQuery const*      ECQueryCP;
    }}

#include <ECObjects/ECObjects.h>


EC_TYPEDEFS (IAUIItem);
EC_TYPEDEFS (IAUIItemInfo);
EC_TYPEDEFS (IAUIDataContext);

BEGIN_BENTLEY_EC_NAMESPACE

typedef RefCountedPtr<IAUIItem>         IAUIItemPtr;

/*---------------------------------------------------------------------------------**//**
A Uiitem is an instance of the BE Display Schema. It has utility functions that helps with ease of use
by having schema specific functions. eg it has methods that allows you to evaluate relationships faster.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAUIItem : public RefCountedBase
    {
    protected:
        
        virtual IAUIItemCP          _GetParent () const = 0;
        virtual IAUIDataContextCP   _GetDataInstance() const = 0;

    public:
        enum ItemType
            {
            MenuItem,
            };

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
struct  IAUIItemInfo
    {
    IAUIItem::ItemType  m_itemType;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct  IAUIDataContext
    {
    enum ContextType
        {
        Instance,
        ECQuery,
        Custom,
        };

    virtual ContextType             GetContextType() = 0;
    virtual IECInstancePtr          GetInstance () {return NULL;}
    virtual DgnPlatform::ECQueryCP  GetQuery () {return NULL;}
    virtual void*                   GetCustomData() {return NULL;}
    };

END_BENTLEY_EC_NAMESPACE