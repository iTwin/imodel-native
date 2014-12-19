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
    static void         DuplicateProperties (IECInstanceR target, ECValuesCollectionCR source);
    static UInt32       GetPropertyIndex (ECClassCR ecClass, ECPropertyCR ecProperty);
    static ECPropertyP  GetPropertyFromIndex (ECClassCR ecClass, UInt32 propertyIndex);
    static bool         IsECValueEmpty (ECValueCR value);
};

END_BENTLEY_SQLITE_EC_NAMESPACE

