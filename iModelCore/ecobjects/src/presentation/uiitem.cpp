/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/uiitem.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/uipresentationmgr.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UICommandPtr    IAUIItem::GetCommand() const
    {
    IAUIDataContextCP instance = GetDataInstance();
    if (NULL == instance)
        return NULL;

    return UIPresentationManager::GetManager().GetCommand(*instance);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   IAUIItem::ExecuteAction () const
    {
    UICommandPtr cmd = GetCommand();
    if (cmd.IsNull())
        return ERROR;
    
    return cmd->Execute (GetDataInstance());
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