/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentation/ecviewdefinition.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_EC_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IECViewDefinition : public RefCountedBase
    {
    typedef bvector <IECViewDefinitionPtr>  ChildDefinitions;
    
    protected:
        virtual IAUIItemInfoCR      _GetUIInfo () = 0;
        virtual IAUIDataContextP    _GetDataContext () = 0;
        virtual ChildDefinitions    _GetChildDefinitions () = 0;

    public:
        //! The UI information associated with this view. It describes the control that needs to 
        //! to be instantiated to represent the data.
        ECOBJECTS_EXPORT IAUIItemInfoCR GetUIInfo();

        //!Get the data context relevant for the control represented in IAUIItemInfo
        ECOBJECTS_EXPORT IAUIDataContextP GetDataContext();

        //Get the child view definitions if there any. Its used by composite controls.
        ECOBJECTS_EXPORT ChildDefinitions GetChildDefinitions();

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IECViewDefinitionProvider : public IAUIProvider
    {
    protected:
        virtual             IECViewDefinitionPtr _GetViewDefinition () = 0;

    public:

        ECOBJECTS_EXPORT    IECViewDefinitionPtr GetViewDefinition ();

    };


END_BENTLEY_EC_NAMESPACE