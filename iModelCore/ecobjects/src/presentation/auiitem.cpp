/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auiitem.cpp $
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
BentleyStatus   IAUIItem::ExecuteActions () const
    {
    IAUIDataContextCP instance = GetDataInstance();
    if (NULL == instance)
        return ERROR;

    bvector<UICommandPtr> cmds = UIPresentationManager::GetManager().GetCommands(*instance);
    for (bvector<UICommandPtr>::const_iterator iter = cmds.begin(); iter != cmds.end(); ++iter)
        {
        BentleyStatus status = (*iter)->Execute (instance);
        if (SUCCESS != status)
            return status;
        }
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIDataContextCP    IAUIItem::GetDataInstance() const
    {
    return _GetDataInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemCP      IAUIItem::GetParent () const
    {
    return _GetParent();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemInfoCR  IAUIItem::GetUIItemInfo () const
    {
    return _GetUIItemInfo ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIItemInfo::ItemType  IAUIItemInfo::GetItemType() const
    {
    return m_itemType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAUIItemInfo::IsAggregatable () const
    {
    return _IsAggregatable();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAUIItemInfo::_IsAggregatable () const
    {
    switch (GetItemType())
        {
        case Menu:
        case ToolBar:
            return true;
        default:
            return false;
        }
    }