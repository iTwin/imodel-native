/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/ecviewdefinition.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationUIItemCR IECPresentationViewDefinition::GetUIItem()
    {
    return _GetUIItem ();
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
    IECPresentationViewTransform                m_viewDefTransform;

    CompositeViewDefinition (bvector<IECPresentationViewDefinitionPtr> const& childDefs)
        :m_childDefs (childDefs)
        {}

    virtual IECPresentationUIItemCR         _GetUIItem () override;
    virtual ChildDefinitions                _GetChildDefinitions () override;
    virtual IECPresentationViewTransformCR  _GetViewTransform () override {return m_viewDefTransform;}

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
IECPresentationUIItemCR  CompositeViewDefinition::_GetUIItem () 
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
    for (bvector<IECPresentationViewDefinitionPtr>::const_iterator iter = viewDefs.begin(); iter != viewDefs.end(); ++iter)
        {
        if (beginVal.GetItemType() != (*iter)->GetUIItem().GetUIItemInfo().GetItemType())
            return NULL;
        }

    return CompositeViewDefinition::CreateViewDefs(viewDefs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECPresentationViewTransformCR  IECPresentationViewDefinition::GetViewTransform ()
    {
    return _GetViewTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         ECPresentationMenuItemInfo::GetToolTip() const
    {
    return m_tooltip.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         ECPresentationMenuItemInfo::GetLabel() const
    {
    return m_label.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationMenuItemInfoCR ECPresentationMenuItem::GetItemInfo() const
    {
    return m_itemInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECPresentationMenuItemSeperator : public ECPresentationMenuItem
    {
    virtual WCharCP             _GetLabel() const {return NULL;}
    virtual WCharCP             _GetToolTip() const {return NULL;}
    virtual IAUIDataContextCP   _GetDataInstance() const {return NULL;}
    virtual bool                _IsSeperator () const {return true;}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPresentationMenuItem* ECPresentationMenuItem::CreateSeperator()
    {
    return new ECPresentationMenuItemSeperator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECPresentationMenuItemInfo::IsSeperator () const
    {
    return _IsSeperator();
    }