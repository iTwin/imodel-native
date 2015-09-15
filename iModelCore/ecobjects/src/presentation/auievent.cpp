/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auievent.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_BENTLEY_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void const*     ECSelectionListener::GetEventHub () const
    {
    return _GetEventHub();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int             ECSelectionListener::GetPriority () const
    {
    return _GetPriority ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSelectionListener::OnSelection (ECSelectionEventCR selectionEvent)
    {
    return _OnSelection(selectionEvent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSelectionListener::OnSubSelection (ECSelectionEventCR selectionEvent)
    {
    return _OnSubSelection(selectionEvent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IAUIDataContextCP ECSelectionListener::GetSelection (bool subSelection)
    {
    return _GetSelection (subSelection);
    }
