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
struct          IECPresentationViewDefinition : public RefCountedBase
    {
    typedef bvector <IECPresentationViewDefinitionPtr>  ChildDefinitions;
    
    protected:
        virtual IAUIItemCR          _GetUIItem () = 0;
        virtual IAUIDataContextP    _GetDataContext () = 0;
        virtual ChildDefinitions    _GetChildDefinitions () = 0;

    public:
        //! The UI information associated with this view. It describes the control that needs to 
        //! to be instantiated to represent the data.
        ECOBJECTS_EXPORT IAUIItemCR GetUIItem();

        //!Get the data context relevant for the control represented in IAUIItemInfo
        ECOBJECTS_EXPORT IAUIDataContextP GetDataContext();

        //Get the child view definitions if there any. Its used by composite controls.
        ECOBJECTS_EXPORT ChildDefinitions GetChildDefinitions();

        static IECPresentationViewDefinitionPtr CreateCompositeViewDef (bvector<IECPresentationViewDefinitionPtr> const& viewDefs);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IECPresentationViewProvider : public IECPresentationProvider
    {
    protected:
        virtual IECPresentationViewDefinitionPtr    _GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR dataContext) = 0;
        virtual ProviderType                        _GetProviderType() const override {return ViewService;}

    public:
        //!Get the view definition associated with a particular data context.
        //!@param[in] itemInfo      A hint to provide the context in which the view definition will be used. eg. MenuItem
        //!@param[in] dataContext   The data context for which the view definition is requested.
        ECOBJECTS_EXPORT    IECPresentationViewDefinitionPtr GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR dataContext);

    };


END_BENTLEY_EC_NAMESPACE