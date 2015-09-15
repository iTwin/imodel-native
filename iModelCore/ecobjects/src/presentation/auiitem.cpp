/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auiitem.cpp $
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
    return _GetItemType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            IAUIItemInfo::IsAggregatable () const
    {
    return _IsAggregatable();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIDataContext::ContextType     IAUIDataContext::GetContextType() const
    {
    return _GetContextType();
    }