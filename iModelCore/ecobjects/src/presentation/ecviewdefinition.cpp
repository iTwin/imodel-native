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
IAUIItemInfoCR  IECViewDefinition::GetUIInfo()
    {
    return _GetUIInfo ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIDataContextP IECViewDefinition::GetDataContext()
    {
    return _GetDataContext();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECViewDefinition::ChildDefinitions IECViewDefinition::GetChildDefinitions()
    {
    return _GetChildDefinitions();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECViewDefinitionPtr    IECViewDefinitionProvider::GetViewDefinition (IAUIItemInfoCR itemInfo, IAUIDataContextCR dataContext)
    {
    return _GetViewDefinition(itemInfo, dataContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct CompositeViewDefinition : public IECViewDefinition
    {
    private:
    bvector<IECViewDefinitionPtr>   m_childDefs;

    CompositeViewDefinition (bvector<IECViewDefinitionPtr> const& childDefs)
        :m_childDefs (childDefs)
        {}

    virtual IAUIItemInfoCR      _GetUIInfo () override;
    virtual IAUIDataContextP    _GetDataContext () override
        {
        return NULL;
        }

    virtual ChildDefinitions    _GetChildDefinitions () override;

    public:
    static IECViewDefinitionPtr CreateViewDefs (bvector<IECViewDefinitionPtr> const& viewDefs)
        {
        return new CompositeViewDefinition(viewDefs);
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECViewDefinition::ChildDefinitions CompositeViewDefinition::_GetChildDefinitions ()
    {
    return m_childDefs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemInfoCR  CompositeViewDefinition::_GetUIInfo () 
    {
    return m_childDefs.front()->GetUIInfo ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECViewDefinitionPtr IECViewDefinition::CreateCompositeViewDef (bvector<IECViewDefinitionPtr> const& viewDefs)
    {
    if (viewDefs.empty())
        return NULL;

    IAUIItemInfoCR beginVal (viewDefs.front()->GetUIInfo());
    if (viewDefs.end() != std::find_if(viewDefs.begin(), viewDefs.end(), [&] (IECViewDefinitionPtr const &x) {return beginVal.GetItemType() != x->GetUIInfo().GetItemType();}))
        return NULL;

    return CompositeViewDefinition::CreateViewDefs(viewDefs);
    }
