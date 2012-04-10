/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/uiitem.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjects.h>

EC_TYPEDEFS (IAUIItem);
EC_TYPEDEFS (UIECClass);
EC_TYPEDEFS (IAUICommandItem);
EC_TYPEDEFS (IUICommand);
EC_TYPEDEFS (UICommand);

BEGIN_BENTLEY_EC_NAMESPACE

typedef ECClass UIClass;

typedef RefCountedPtr<IAUIItem> IAUIItemPtr;

/*---------------------------------------------------------------------------------**//**
A Uiitem is an instance of the BE Display Schema. It has utility functions that helps with ease of use
by having schema specific functions. eg it has methods that allows you to evaluate relationships faster.
* @bsiclass                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAUIItem : public IECInstance
    {
        virtual IAUICommandItemCP    _GetAsCommandItem () const;

    public:
        //! Get the instance as a Command Item. NULL if the instance cannot be cast as one.
        ECOBJECTS_EXPORT IAUICommandItemCP GetAsCommandItem () const;
        
        //! Get the parent instance associated with this instance.
        ECOBJECTS_EXPORT IAUIItemCP        GetParent () const;

        //! Utility function to evaluate whether the given control is visible.
        //ECOBJECTS_EXPORT bool             IsVisible () const;

        //! Get the data instance bind with this ui instance.
        ECOBJECTS_EXPORT IECInstancePtr     GetDataInstance() const;
    };

/*---------------------------------------------------------------------------------**//**
//! A specialized UIitem that has a command action associated with it.
* @bsiclass                                     Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct IAUICommandItem: public IAUIItem
    {
    protected:
        
        virtual IAUICommandItemCP    _GetAsCommandItem () const override;
        virtual UICommandCR         _GetCommand () const = 0;

    public:
        
        //! Get the command associated with this ui instance.
        ECOBJECTS_EXPORT UICommandCR GetCommand () const;

        //! Do the action associated with the ui item using the bound data instance.
        ECOBJECTS_EXPORT BentleyStatus ExecuteAction () const;
    };

END_BENTLEY_EC_NAMESPACE