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

//*******************************************************************************************
// SchemaHintReader
//*******************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr SchemaHintReader::ReadHint (ECSchemaCR ecschema)
    {
    return HintReaderHelper::ReadHint (ecschema, BSCAC_ECDbSchemaHint);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool SchemaHintReader::TryReadTablePrefix (Utf8String& tablePrefix, IECInstanceCR hint)
    {
    BeAssert (hint.GetClass ().GetName ().Equals (BSCAC_ECDbSchemaHint));

    Utf8String tablePrefixTemp;
    if (HintReaderHelper::TryGetTrimmedValue (tablePrefixTemp, hint, BSCAP_TablePrefix))
        {
        if (ECNameValidation::Validate (WString (tablePrefixTemp.c_str (), BentleyCharEncoding::Utf8).c_str ()) == ECNameValidation::RESULT_Valid)
            {
            tablePrefix = tablePrefixTemp;
            return true;
            }

        LOG.warningv ("'%s' is not a valid table prefix. A default one will be used. The table prefix should be a few characters long and can only contain [a-zA-Z_0-9] and must start with a non-numeric character.", 
            tablePrefixTemp.c_str ());
        }

    return false;
    }


//*******************************************************************************************
// ClassMapHintReader
//*******************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr ClassHintReader::ReadHint (ECClassCR ecClass)
    {
    return HintReaderHelper::ReadHint (ecClass, BSCAC_ECDbClassHint);
    }

//*******************************************************************************************
// ClassMapHintReader
//*******************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr ClassHintReader::ReadClassHasTimeStamp(ECClassCR ecClass)
    {
    return HintReaderHelper::ReadHint(ecClass, BSCAC_ECDbClassHasTimeStamp);
    }
//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassHintReader::TryReadMapStrategy (MapStrategy& mapStrategy, IECInstanceCR classHint)
    {
    BeAssert (classHint.GetClass ().GetName ().Equals (BSCAC_ECDbClassHint));

    mapStrategy = MapStrategy::DoNotMap;

    WString hintMapStrategyName;
    ECValue v;
    if (classHint.GetValue (v, BSCAP_MapStrategy) == ECOBJECTS_STATUS_Success && !v.IsNull ())
        {
        hintMapStrategyName = v.GetString ();
        if (TryConvertToMapStrategy (mapStrategy, hintMapStrategyName.c_str ()))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassHintReader::TryReadTableName (Utf8String& tableName, IECInstanceCR classHint)
    {
    BeAssert (classHint.GetClass ().GetName ().Equals (BSCAC_ECDbClassHint));

    return HintReaderHelper::TryGetTrimmedValue (tableName, classHint, BSCAP_TableName);
    }

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassHintReader::TryReadECInstanceIdColumnName (Utf8String& ecInstanceIdColumnName, IECInstanceCR classHint)
    {
    BeAssert (classHint.GetClass ().GetName ().Equals (BSCAC_ECDbClassHint));

    return HintReaderHelper::TryGetTrimmedValue (ecInstanceIdColumnName, classHint, BSCAP_ECInstanceIdColumn);
    }

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassHintReader::TryReadMapToExistingTable (bool& mapToExistingTable, IECInstanceCR classHint)
    {
    BeAssert (classHint.GetClass ().GetName ().Equals (BSCAC_ECDbClassHint));

    ECValue v;
    if (classHint.GetValue (v, BSCAP_MapToExistingTable) == ECOBJECTS_STATUS_Success && !v.IsNull ())
        {
        mapToExistingTable = v.GetBoolean ();
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassHintReader::TryReadReplaceEmptyTableWithEmptyView (bool& replaceEmptyTableWithEmptyView, IECInstanceCR classHint)
    {
    BeAssert (classHint.GetClass ().GetName ().Equals (BSCAC_ECDbClassHint));

    ECValue v;
    if (classHint.GetValue (v, BSCAP_ReplaceEmptyTableWithEmptyView) == ECOBJECTS_STATUS_Success && !v.IsNull ())
        {
        replaceEmptyTableWithEmptyView = v.GetBoolean ();
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ClassHintReader::TryReadIndices (bvector<ClassIndexInfoPtr>& indices, IECInstanceCR classHint, ECClassCR ecClass)
    {
    BeAssert (classHint.GetClass ().GetName ().Equals (BSCAC_ECDbClassHint));

    indices.clear ();
    ECValue v;
    if (classHint.GetValue (v, BSCAP_Indexes) != ECOBJECTS_STATUS_Success || v.IsNull ())
        return false;

    uint32_t indexesPropIdx;
    if (classHint.GetEnablerR ().GetPropertyIndex (indexesPropIdx, BSCAP_Indexes) != ECOBJECTS_STATUS_Success)
        {
        LOG.errorv ("Failed to get property index for 'ECDbClassHint.Indexes'.");
        return false;
        }

    uint32_t indexCount = v.GetArrayInfo ().GetCount ();
    for (uint32_t i = 0; i < indexCount; i++)
        {
        bool errorEncountered = false;
        ClassIndexInfoPtr info = ClassIndexInfo::Create ();
        if (classHint.GetValue (v, indexesPropIdx, i) != ECOBJECTS_STATUS_Success)
            {
            LOG.errorv ("Failed to get %dth array element for ECDbClassHint.Indexes.", i);
            return false;
            }

        IECInstancePtr ecDbIndex = v.GetStruct ();
        if (ecDbIndex.IsNull ())
            continue;

        //optional
        if (ecDbIndex->GetValue (v, BSCAP_Name) == ECOBJECTS_STATUS_Success && !v.IsNull ())
            info->SetName (Utf8String (v.GetString ()).c_str ());

        //optional
        if (ecDbIndex->GetValue (v, BSCAP_IsUnique) == ECOBJECTS_STATUS_Success && !v.IsNull ())
            info->SetIsUnique (v.GetBoolean ());

        //mandatory. Reject Index if it doesn't have any property
        if (ecDbIndex->GetValue (v, BSCAP_Properties) == ECOBJECTS_STATUS_Success && !v.IsNull ())
            {
            uint32_t propertiesPropIdx;
            if (ecDbIndex->GetEnablerR ().GetPropertyIndex (propertiesPropIdx, BSCAP_Properties) != ECOBJECTS_STATUS_Success)
                {
                //error
                LOG.errorv ("Fail to get property index for 'ECDbClassHint.Indexes.Properties'");
                return false; //Do not enumerate anymore
                }

            uint32_t propertyCount = v.GetArrayInfo ().GetCount ();
            if (propertyCount == 0)
                {
                LOG.errorv (L"Rejecting user specified index[%d] specified in ECDbClassHint on class %ls because it has no properties.", i, ecClass.GetFullName ());
                errorEncountered = true; // skip this index and continue with rest.
                }

            //process properties specified in index
            for (uint32_t j = 0; j < propertyCount && !errorEncountered; j++)
                {
                if (ecDbIndex->GetValue (v, propertiesPropIdx, j) == ECOBJECTS_STATUS_Success)
                    {
                    Utf8String propertyName (v.GetString ());
                    propertyName.Trim ();
                    if (Utf8String::IsNullOrEmpty (propertyName.c_str ()))
                        {
                        LOG.errorv (L"Rejecting index[%d] specified in ECDbClassHint on class %ls because on of its property is empty string.", i, ecClass.GetFullName ());
                        errorEncountered = true; // skip this index and continue with rest
                        break;
                        }

                    if (!errorEncountered)
                        info->GetProperties ().push_back (propertyName);
                    }
                else
                    {
                    LOG.errorv (L"Rejecting user specified index[%d] specified in ECDbClassHint on class %ls because array element didnt returned a value", i, ecClass.GetFullName ());
                    errorEncountered = true; // skip this index and continue with rest
                    break;
                    }
                }

            if (!errorEncountered)
                indices.push_back (info);
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bool ClassHintReader::TryConvertToMapStrategy (MapStrategy& mapStrategy, WCharCP mapStrategyName)
    {
    bool success = true;
    if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_TablePerHierarchy))
        mapStrategy = MapStrategy::TablePerHierarchy;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_DoNotMapHierarchy))
        mapStrategy = MapStrategy::DoNotMapHierarchy;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_DoNotMap))
        mapStrategy = MapStrategy::DoNotMap;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_TablePerClass))
        mapStrategy = MapStrategy::TablePerClass;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_TableForThisClass))
        mapStrategy = MapStrategy::TableForThisClass;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_NoHint))
        mapStrategy = MapStrategy::NoHint;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_RelationshipSourceTable))
        mapStrategy = MapStrategy::RelationshipSourceTable;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_RelationshipTargetTable))
        mapStrategy = MapStrategy::RelationshipTargetTable;
    else if (0 == BeStringUtilities::Wcsicmp (mapStrategyName, BSCAV_SharedTableForThisClass))
        mapStrategy = MapStrategy::SharedTableForThisClass;
    else
        success = false;

    return success;
    }


//************************************************************************************
// RelationshipClassHintReader 
//************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr RelationshipClassHintReader::ReadHint (ECClassCR relClass)
    {
    return HintReaderHelper::ReadHint (relClass, BSCAC_ECDbRelationshipClassHint);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadPreferredDirection (RelationshipClassMapInfo::PreferredDirection& preferredDirection, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    ECValue v;
    relClassHint.GetValue (v, BSCAP_PreferredDirection);
    bool found = false;
    if (!v.IsNull ())
        {
        found = true;
        WCharCP val = v.GetString ();
        if (0 == BeStringUtilities::Wcsicmp (val, BSCAV_SourceToTarget))
            preferredDirection = RelationshipClassMapInfo::PreferredDirection::SourceToTarget;
        else if (0 == BeStringUtilities::Wcsicmp (val, BSCAV_TargetToSource))
            preferredDirection = RelationshipClassMapInfo::PreferredDirection::TargetToSource;
        else if (0 == BeStringUtilities::Wcsicmp (val, BSCAV_Bidirectional))
            preferredDirection = RelationshipClassMapInfo::PreferredDirection::Bidirectional;
        else
            {
            LOG.errorv (L"Unrecognized " BSCAP_PreferredDirection L" '%ls' in " BSCAC_ECDbRelationshipClassHint,
                val);
            preferredDirection = RelationshipClassMapInfo::PreferredDirection::Unspecified;
            found = false;
            }
        }

    return found;
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

    return HintReaderHelper::TryGetTrimmedValue (sourceECInstanceIdColumnName, relClassHint, BSCAP_SourceECInstanceIdColumn);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadSourceECClassIdColumnName (Utf8String& sourceECClassIdColumnName, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    return HintReaderHelper::TryGetTrimmedValue (sourceECClassIdColumnName, relClassHint, BSCAP_SourceECClassIdColumn);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadTargetECInstanceIdColumnName (Utf8String& targetECInstanceIdColumnName, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    return HintReaderHelper::TryGetTrimmedValue (targetECInstanceIdColumnName, relClassHint, BSCAP_TargetECInstanceIdColumn);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Ramanujam.Raman                07/2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool RelationshipClassHintReader::TryReadTargetECClassIdColumnName (Utf8String& targetECClassIdColumnName, IECInstanceCR relClassHint)
    {
    BeAssert (relClassHint.GetClass ().GetName ().Equals (BSCAC_ECDbRelationshipClassHint));

    return HintReaderHelper::TryGetTrimmedValue (targetECClassIdColumnName, relClassHint, BSCAP_TargetECClassIdColumn);
    }

//************************************************************************************
// PropertyHintReader 
//************************************************************************************

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr PropertyHintReader::ReadHint (ECN::ECPropertyCR ecProperty)
    {
    return HintReaderHelper::ReadHint (ecProperty, BSCAC_ECDbPropertyHint);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool PropertyHintReader::TryReadColumnName (Utf8String& columnName, ECN::IECInstanceCR hint)
    {
    BeAssert (hint.GetClass ().GetName ().Equals (BSCAC_ECDbPropertyHint));

    return HintReaderHelper::TryGetTrimmedValue (columnName, hint, BSCAP_ColumnName);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool PropertyHintReader::TryReadIsNullable (bool& isNullable, ECN::IECInstanceCR hint)
    {
    BeAssert (hint.GetClass ().GetName ().Equals (BSCAC_ECDbPropertyHint));

    ECValue v;
    if (hint.GetValue (v, BSCAP_IsNullable) == ECOBJECTS_STATUS_Success && !v.IsNull ())
        {
        isNullable = v.GetBoolean ();
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool PropertyHintReader::TryReadIsUnique (bool& isUnique, ECN::IECInstanceCR hint)
    {
    BeAssert (hint.GetClass ().GetName ().Equals (BSCAC_ECDbPropertyHint));

    ECValue v;
    if (hint.GetValue (v, BSCAP_IsUnique) == ECOBJECTS_STATUS_Success && !v.IsNull ())
        {
        isUnique = v.GetBoolean ();
        return true;
        }

    return false;

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool PropertyHintReader::TryReadCollate (ECDbSqlColumn::Constraint::Collate& collate, ECN::IECInstanceCR hint)
    {
    BeAssert (hint.GetClass ().GetName ().Equals (BSCAC_ECDbPropertyHint));

    bool found = false;
    ECValue v;
    if (hint.GetValue (v, BSCAP_Collate) == ECOBJECTS_STATUS_Success && !v.IsNull ())
        {
        found = true;
        WString collateHintStr (v.GetString ());
        if (collateHintStr.EqualsI (BSCAP_Collate_Binary))
            collate = ECDbSqlColumn::Constraint::Collate::Binary;
        else if (collateHintStr.EqualsI (BSCAP_Collate_NoCase))
            collate = ECDbSqlColumn::Constraint::Collate::NoCase;
        else if (collateHintStr.EqualsI (BSCAP_Collate_RTrim))
            collate = ECDbSqlColumn::Constraint::Collate::RTrim;
        else
            {
            LOG.warningv (L"Unrecognized value '%ls' for 'Collate' property in " BSCAC_ECDbPropertyHint L". The value is ignored.",
                collateHintStr.c_str ());
            found = false;
            }
        }

    return found;
    }





//************************************************************************************
// HintReaderHelper 
//************************************************************************************
//---------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr HintReaderHelper::ReadHint (IECCustomAttributeContainer const& caContainer, WCharCP hintCustomAttributeName)
    {
    return caContainer.GetCustomAttributeLocal (hintCustomAttributeName);
    }


//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool HintReaderHelper::TryGetTrimmedValue (Utf8StringR val, IECInstanceCR ca, WCharCP ecPropertyName)
    {
    ECValue v;

    if (ca.GetValue (v, ecPropertyName) != ECOBJECTS_STATUS_Success || v.IsNull ())
        return false;

    val = v.GetUtf8CP ();
    val.Trim ();
    return true;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
