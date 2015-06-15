/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapHintReader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "MapHintReader.h"


USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************************************************************
// RelationshipClassHintReader 
//************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr RelationshipClassHintReader::ReadHint (ECClassCR relClass)
    {
    return CustomAttributeReader::Read (relClass, BSCAC_ECDbRelationshipClassHint);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadAllowDuplicateRelationships (RelationshipClassMapInfo::TriState& allowDuplicateRelationships, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    ECValue v;
    relClassHint.GetValue (v, BSCAP_AllowDuplicateRelationships);
    if (!v.IsNull ())
        {
        allowDuplicateRelationships = v.GetBoolean () ? RelationshipClassMapInfo::TriState::True : RelationshipClassMapInfo::TriState::False;
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadSourceECInstanceIdColumnName (Utf8String& sourceECInstanceIdColumnName, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    return CustomAttributeReader::TryGetTrimmedValue (sourceECInstanceIdColumnName, relClassHint, BSCAP_SourceECInstanceIdColumn);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadSourceECClassIdColumnName (Utf8String& sourceECClassIdColumnName, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    return CustomAttributeReader::TryGetTrimmedValue (sourceECClassIdColumnName, relClassHint, BSCAP_SourceECClassIdColumn);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadTargetECInstanceIdColumnName (Utf8String& targetECInstanceIdColumnName, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    return CustomAttributeReader::TryGetTrimmedValue (targetECInstanceIdColumnName, relClassHint, BSCAP_TargetECInstanceIdColumn);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadTargetECClassIdColumnName (Utf8String& targetECClassIdColumnName, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    return CustomAttributeReader::TryGetTrimmedValue (targetECClassIdColumnName, relClassHint, BSCAP_TargetECClassIdColumn);
    }





//************************************************************************************
// CustomAttributeReader 
//************************************************************************************
//---------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr CustomAttributeReader::Read (IECCustomAttributeContainer const& caContainer, WCharCP customAttributeName)
    {
    return caContainer.GetCustomAttributeLocal(customAttributeName);
    }


//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomAttributeReader::TryGetTrimmedValue (Utf8StringR val, IECInstanceCR ca, WCharCP ecPropertyName)
    {
    ECValue v;

    if (ca.GetValue (v, ecPropertyName) != ECOBJECTS_STATUS_Success || v.IsNull ())
        return false;

    val = v.GetUtf8CP ();
    val.Trim ();
    return true;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
