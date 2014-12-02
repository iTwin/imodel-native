/*----------------------------------------------------------------------+
|
|     $Source: DgnHandlers/Dimension/DimStyleTable.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    "DimStyleInternal.h"

#define     MS_DIMSTYLE_TABLE_LEVEL                             5

/*---------------------------------------------------------------------------------**//**
* Tests whether given element is a valid dimension style.
*
* @param        pCandidate      => element to tedt
* @return       true if is valid style element.
* @bsimethod                                                    petri.niiranen  11/00
+---------------+---------------+---------------+---------------+---------------+------*/
Public int      dimStyleEntry_isStyleElement
(
const DgnElement   *pCandidate
)
    {
    return  (pCandidate &&
             pCandidate->GetLegacyType()  == TABLE_ENTRY_ELM &&
             pCandidate->GetLevelValue() == MS_DIMSTYLE_TABLE_LEVEL);
    }

