/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbUtility.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      08/2012
+===============+===============+===============+===============+===============+======*/
struct ECDbUtility
{
public:
    static void         DuplicateProperties (ECN::IECInstanceR target, ECN::ECValuesCollectionCR source);
    static uint32_t       GetPropertyIndex (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty);
    static ECN::ECPropertyP  GetPropertyFromIndex (ECN::ECClassCR ecClass, uint32_t propertyIndex);
    static bool         IsECValueEmpty (ECN::ECValueCR value);
};

END_BENTLEY_SQLITE_EC_NAMESPACE

