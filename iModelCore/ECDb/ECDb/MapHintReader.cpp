/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapHintReader.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "MapHintReader.h"

#define ECDBMAP_CLASSNAME_SchemaMap   L"SchemaMap"
#define ECDBMAP_CLASSNAME_ClassMap    L"ClassMap"
#define ECDBMAP_CLASSNAME_PropertyMap L"PropertyMap"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*******************************************************************************************
// CustomSchemaMapReader
//*******************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr CustomSchemaMapReader::Read (ECSchemaCR ecschema)
    {
    return CustomAttributeReader::Read(ecschema, ECDBMAP_CLASSNAME_SchemaMap);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomSchemaMapReader::TryReadTablePrefix(Utf8String& tablePrefix, IECInstanceCR customSchemaMap)
    {
    BeAssert(customSchemaMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_SchemaMap));

    Utf8String tablePrefixTemp;
    if (CustomAttributeReader::TryGetTrimmedValue(tablePrefixTemp, customSchemaMap, L"TablePrefix"))
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
// CustomClassMapReader
//*******************************************************************************************
//---------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                02/2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr CustomClassMapReader::Read (ECClassCR ecClass)
    {
    return CustomAttributeReader::Read(ecClass, ECDBMAP_CLASSNAME_ClassMap);
    }

//*******************************************************************************************
// ClassMapHintReader
//*******************************************************************************************

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomClassMapReader::TryReadMapStrategy(ECDbMapStrategy& mapStrategy, IECInstanceCR customClassMap)
    {
    BeAssert(customClassMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_ClassMap));
    mapStrategy.Reset ();

    Utf8String mapStrategyStr;
    CustomAttributeReader::TryGetTrimmedValue(mapStrategyStr, customClassMap, L"MapStrategy");

    Utf8String mapStrategyOptionsStr;
    CustomAttributeReader::TryGetTrimmedValue(mapStrategyOptionsStr, customClassMap, L"MapStrategyOptions");

    if (mapStrategyStr.empty() && mapStrategyOptionsStr.empty())
        return false;

    return mapStrategy.Parse(mapStrategy, mapStrategyStr.c_str(), mapStrategyOptionsStr.c_str()) == SUCCESS;
    }

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomClassMapReader::TryReadTableName(Utf8String& tableName, IECInstanceCR customClassMap)
    {
    BeAssert(customClassMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_ClassMap));
    return CustomAttributeReader::TryGetTrimmedValue(tableName, customClassMap, L"TableName");
    }

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomClassMapReader::TryReadECInstanceIdColumnName(Utf8String& ecInstanceIdColumnName, IECInstanceCR customClassMap)
    {
    BeAssert(customClassMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_ClassMap));
    return CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, customClassMap, L"ECInstanceIdColumn");
    }

#define ECDBMAP_PROPNAME_Indexes L"Indexes"
#define ECDBMAP_PROPNAME_Properties L"Properties"

//---------------------------------------------------------------------------------
//@bsimethod                                                    casey.mullen      11 / 2012
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomClassMapReader::TryReadIndices(bvector<ClassIndexInfoPtr>& indices, IECInstanceCR customClassMap, ECClassCR ecClass)
    {
    BeAssert(customClassMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_ClassMap));

    indices.clear ();
    ECValue v;
    if (customClassMap.GetValue(v, ECDBMAP_PROPNAME_Indexes) != ECOBJECTS_STATUS_Success || v.IsNull())
        return false;

    uint32_t indexesPropIdx;
    if (customClassMap.GetEnablerR().GetPropertyIndex(indexesPropIdx, ECDBMAP_PROPNAME_Indexes) != ECOBJECTS_STATUS_Success)
        {
        LOG.errorv ("Failed to get property index for 'ClassMap.Indexes'.");
        return false;
        }

    uint32_t indexCount = v.GetArrayInfo ().GetCount ();
    for (uint32_t i = 0; i < indexCount; i++)
        {
        bool errorEncountered = false;
        ClassIndexInfoPtr info = ClassIndexInfo::Create ();
        if (customClassMap.GetValue(v, indexesPropIdx, i) != ECOBJECTS_STATUS_Success)
            {
            LOG.errorv ("Failed to get %dth array element for ClassMap.Indexes.", i);
            return false;
            }

        IECInstancePtr ecDbIndex = v.GetStruct ();
        if (ecDbIndex.IsNull ())
            continue;

        //optional
        if (ecDbIndex->GetValue (v, L"Name") == ECOBJECTS_STATUS_Success && !v.IsNull ())
            info->SetName (Utf8String (v.GetString ()).c_str ());
        //optional
        if (ecDbIndex->GetValue(v, L"Where") == ECOBJECTS_STATUS_Success && !v.IsNull())
            {
            Utf8String whereFlag =  Utf8String(v.GetString());
            whereFlag.Trim();
            if (whereFlag.EqualsI("ECDB_NOTNULL"))
                {
                info->SetWhere(EC::ClassIndexInfo::WhereConstraint::NotNull);
                }
            else
                {
                LOG.errorv("Invalid where flag in ClassMap.Indexes.");
                return false;
                }
            }
        //optional
        if (ecDbIndex->GetValue(v, L"IsUnique") == ECOBJECTS_STATUS_Success && !v.IsNull())
            info->SetIsUnique (v.GetBoolean ());

        //mandatory. Reject Index if it doesn't have any property
        if (ecDbIndex->GetValue(v, ECDBMAP_PROPNAME_Properties) == ECOBJECTS_STATUS_Success && !v.IsNull())
            {
            uint32_t propertiesPropIdx;
            if (ecDbIndex->GetEnablerR().GetPropertyIndex(propertiesPropIdx, ECDBMAP_PROPNAME_Properties) != ECOBJECTS_STATUS_Success)
                {
                //error
                LOG.errorv ("Fail to get property index for 'ClassMap.Indexes.Properties'");
                return false; //Do not enumerate anymore
                }

            uint32_t propertyCount = v.GetArrayInfo ().GetCount ();
            if (propertyCount == 0)
                {
                LOG.errorv (L"Rejecting user specified index[%d] specified in custom attribute ClassMap on class %ls because it has no properties.", i, ecClass.GetFullName ());
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
                        LOG.errorv (L"Rejecting index[%d] specified in custom attribute ClassMap on class %ls because on of its property is empty string.", i, ecClass.GetFullName ());
                        errorEncountered = true; // skip this index and continue with rest
                        break;
                        }

                    if (!errorEncountered)
                        info->GetProperties ().push_back (propertyName);
                    }
                else
                    {
                    LOG.errorv (L"Rejecting user specified index[%d] specified in custom attribute ClassMap on class %ls because array element didn't returned a value", i, ecClass.GetFullName ());
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


//************************************************************************************
// CustomPropertyMapReader 
//************************************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstancePtr CustomPropertyMapReader::Read(ECN::ECPropertyCR ecProperty)
    {
    return CustomAttributeReader::Read(ecProperty, ECDBMAP_CLASSNAME_PropertyMap);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomPropertyMapReader::TryReadColumnName(Utf8String& columnName, ECN::IECInstanceCR customPropMap)
    {
    BeAssert(customPropMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_PropertyMap));

    return CustomAttributeReader::TryGetTrimmedValue(columnName, customPropMap, L"ColumnName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomPropertyMapReader::TryReadIsNullable(bool& isNullable, ECN::IECInstanceCR customPropMap)
    {
    BeAssert(customPropMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_PropertyMap));

    ECValue v;
    if (customPropMap.GetValue(v, L"IsNullable") == ECOBJECTS_STATUS_Success && !v.IsNull())
        {
        isNullable = v.GetBoolean();
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomPropertyMapReader::TryReadIsUnique(bool& isUnique, ECN::IECInstanceCR customPropMap)
    {
    BeAssert(customPropMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_PropertyMap));

    ECValue v;
    if (customPropMap.GetValue(v, L"IsUnique") == ECOBJECTS_STATUS_Success && !v.IsNull())
        {
        isUnique = v.GetBoolean();
        return true;
        }

    return false;

    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool CustomPropertyMapReader::TryReadCollation(ECDbSqlColumn::Constraint::Collation& collation, ECN::IECInstanceCR customPropMap)
    {
    BeAssert(customPropMap.GetClass().GetName().Equals(ECDBMAP_CLASSNAME_PropertyMap));

    bool found = false;
    ECValue v;
    if (customPropMap.GetValue(v, L"Collation") == ECOBJECTS_STATUS_Success && !v.IsNull())
        {
        found = true;
        Utf8CP collationValStr = v.GetUtf8CP();
        if (BeStringUtilities::Stricmp(collationValStr, "Binary") == 0)
            collation = ECDbSqlColumn::Constraint::Collation::Binary;
        else if (BeStringUtilities::Stricmp(collationValStr, "NoCase") == 0)
            collation = ECDbSqlColumn::Constraint::Collation::NoCase;
        else if (BeStringUtilities::Stricmp(collationValStr, "RTrim") == 0)
            collation = ECDbSqlColumn::Constraint::Collation::RTrim;
        else
            {
            LOG.warningv(L"Custom attribute " ECDBMAP_CLASSNAME_PropertyMap L" has an invalid value for the property 'Collation': %ls",
                         WString(collationValStr, BentleyCharEncoding::Utf8).c_str());
            found = false;
            }
        }

    return found;
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
    return CustomAttributeReader::Read (relClass, BSCAC_ECDbRelationshipClassHint);
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
