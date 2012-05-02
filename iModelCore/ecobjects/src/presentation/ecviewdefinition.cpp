/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ecviewdefinition.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemCR      IECPresentationViewDefinition::GetUIItem()
    {
    return _GetUIItem ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIDataContextP IECPresentationViewDefinition::GetDataContext()
    {
    return _GetDataContext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationViewDefinition::ChildDefinitions IECPresentationViewDefinition::GetChildDefinitions()
    {
    return _GetChildDefinitions();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationViewDefinitionPtr    IECPresentationViewProvider::GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR dataContext)
    {
    return _GetViewDefinition(itemInfo, dataContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CompositeViewDefinition : public IECPresentationViewDefinition
    {
    private:
    bvector<IECPresentationViewDefinitionPtr>   m_childDefs;

    CompositeViewDefinition (bvector<IECPresentationViewDefinitionPtr> const& childDefs)
        :m_childDefs (childDefs)
        {}

    virtual IAUIItemCR          _GetUIItem () override;
    virtual IAUIDataContextP    _GetDataContext () override
        {
        return NULL;
        }

    virtual ChildDefinitions    _GetChildDefinitions () override;

    public:
    static IECPresentationViewDefinitionPtr CreateViewDefs (bvector<IECPresentationViewDefinitionPtr> const& viewDefs)
        {
        return new CompositeViewDefinition(viewDefs);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationViewDefinition::ChildDefinitions CompositeViewDefinition::_GetChildDefinitions ()
    {
    return m_childDefs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemCR  CompositeViewDefinition::_GetUIItem () 
    {
    return m_childDefs.front()->GetUIItem ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationViewDefinitionPtr IECPresentationViewDefinition::CreateCompositeViewDef (bvector<IECPresentationViewDefinitionPtr> const& viewDefs)
    {
    if (viewDefs.empty())
        return NULL;

    IAUIItemInfoCR beginVal (viewDefs.front()->GetUIItem().GetUIItemInfo());
    if (viewDefs.end() != std::find_if(viewDefs.begin(), viewDefs.end(), [&] (IECPresentationViewDefinitionPtr const &x) {return beginVal.GetItemType() != x->GetUIItem().GetUIItemInfo().GetItemType();}))
        return NULL;

    return CompositeViewDefinition::CreateViewDefs(viewDefs);
    }
