/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/auievent.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include <EcPresentation/auipresentationapi.h>

USING_NAMESPACE_EC

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void const*     ECSelectionListener::GeteventHub () const
    {
    return _GeteventHub();
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
 void           ECSelectionListener::OnSubSelection (ECSelectionEventCR selectionEvent)
     {
     return _OnSubSelection(selectionEvent);
     }
