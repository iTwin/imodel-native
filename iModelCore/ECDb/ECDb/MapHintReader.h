/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapHintReader.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ClassMap.h"
#include "ClassMapInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct RelationshipClassHintReader
    {
private:
    RelationshipClassHintReader ();
    ~RelationshipClassHintReader ();

public:
    static ECN::IECInstancePtr ReadHint (ECN::ECClassCR relClass);
    static bool TryReadAllowDuplicateRelationships (RelationshipClassMapInfo::TriState& allowDuplicateRelationships, ECN::IECInstanceCR relClassHint);
    static bool TryReadSourceECInstanceIdColumnName (Utf8String& sourceECInstanceIdColumnName, ECN::IECInstanceCR relClassHint);
    static bool TryReadSourceECClassIdColumnName (Utf8String& sourceECClassIdColumnName, ECN::IECInstanceCR relClassHint);
    static bool TryReadTargetECInstanceIdColumnName (Utf8String& targetECInstanceIdColumnName, ECN::IECInstanceCR relClassHint);
    static bool TryReadTargetECClassIdColumnName (Utf8String& targetECClassIdColumnName, ECN::IECInstanceCR relClassHint);
    };


//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct CustomAttributeReader
    {
private:
    CustomAttributeReader ();
    ~CustomAttributeReader ();
        
public:
    static ECN::IECInstancePtr Read (ECN::IECCustomAttributeContainer const&, WCharCP customAttributeName);
    static bool TryGetTrimmedValue (Utf8StringR val, ECN::IECInstanceCR ca, WCharCP ecPropertyName);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
