/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMapSchemaHelper.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECDBMAP_SCHEMANAME "ECDbMap"

//*****************************************************************
//ECDbMapHelper
//*****************************************************************
struct CustomAttributeReader final
    {
    private:
        CustomAttributeReader();
        ~CustomAttributeReader();

        static BentleyStatus TryGetTrimmedValue(Nullable<Utf8String>& strVal, ECValueCR val);

    public:
        static IECInstanceCP Read(IECCustomAttributeContainer const& caContainer, Utf8CP customAttributeSchemaName, Utf8CP customAttributeName);

        static BentleyStatus TryGetTrimmedValue(Nullable<Utf8String>& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString);
        static BentleyStatus TryGetTrimmedValue(Nullable<Utf8String>& val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex);
        static BentleyStatus TryGetIntegerValue(Nullable<int>&, IECInstanceCR ca, Utf8CP ecPropertyAccessString);
        static BentleyStatus TryGetBooleanValue(Nullable<bool>&, IECInstanceCR ca, Utf8CP ecPropertyAccessString);
    };



//*****************************************************************
//CustomAttributeReader
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECInstanceCP CustomAttributeReader::Read(IECCustomAttributeContainer const& caContainer, Utf8CP customAttributeSchemaName, Utf8CP customAttributeName)
    {
    for (IECInstancePtr const& ca : caContainer.GetCustomAttributes(false))
        {
        ECClassCR caClass = ca->GetClass();
        if (caClass.GetName().Equals(customAttributeName) && caClass.GetSchema().GetName().Equals(customAttributeSchemaName))
            return ca.get();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetTrimmedValue(Nullable<Utf8String>& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return ERROR;
        }

    return TryGetTrimmedValue(val, v);
    }


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetTrimmedValue(Nullable<Utf8String>& val, IECInstanceCR ca, uint32_t propIndex, uint32_t arrayIndex)
    {
    ECValue v;
    if (ECObjectsStatus::Success != ca.GetValue(v, propIndex, arrayIndex))
        {
        LOG.errorv("Could not retrieve array element #%d value of ECProperty with property index %d of the '%s' custom attribute ECInstance.",
                   arrayIndex, propIndex, ca.GetClass().GetFullName());
        return ERROR;
        }

    return TryGetTrimmedValue(val, v);
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetTrimmedValue(Nullable<Utf8String>& strVal, ECValueCR val)
    {
    if (!val.IsNull())
        {
        strVal = Nullable<Utf8String>(val.GetUtf8CP());
        strVal.ValueR().Trim();
        
        //if the string property was specified but without a string, we consider this null as well
        if (strVal.Value().empty())
            strVal = Nullable<Utf8String>();
        }
    else
        strVal = Nullable<Utf8String>();

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   02 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetIntegerValue(Nullable<int>& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    if (ECObjectsStatus::Success != ca.GetValue(v, ecPropertyAccessString))
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return ERROR;
        }

    if (!v.IsNull())
        val = Nullable<int>(v.GetInteger());
    else
        val = Nullable<int>();


    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static 
BentleyStatus CustomAttributeReader::TryGetBooleanValue(Nullable<bool>& val, IECInstanceCR ca, Utf8CP ecPropertyAccessString)
    {
    ECValue v;
    ECObjectsStatus stat = ca.GetValue(v, ecPropertyAccessString);
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Could not retrieve value of ECProperty '%s' of the '%s' custom attribute ECInstance.",
                   ecPropertyAccessString, ca.GetClass().GetFullName());
        return ERROR;
        }

    if (!v.IsNull())
        val = Nullable<bool>(v.GetBoolean());
    else
        val = Nullable<bool>();

    return SUCCESS;
    }



//*****************************************************************
//ECDbMapCustomAttributeHelper
//*****************************************************************


//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetSchemaMap(ECDbSchemaMap& schemaMap, ECSchemaCR schema)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(schema, ECDBMAP_SCHEMANAME, "SchemaMap");
    if (ca == nullptr)
        return false;

    schemaMap = ECDbSchemaMap(schema, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetClassMap(ECDbClassMap& classMap, ECClassCR ecClass)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "ClassMap");
    if (ca == nullptr)
        return false;

    classMap = ECDbClassMap(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetShareColumns(ShareColumns& shareColumns, ECClassCR ecClass)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "ShareColumns");
    if (ca == nullptr)
        return false;

    shareColumns = ShareColumns(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::HasJoinedTablePerDirectSubclass(ECEntityClassCR ecClass)
    {
    return ecClass.GetCustomAttributeLocal(ECDBMAP_SCHEMANAME, "JoinedTablePerDirectSubclass") != nullptr;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   10 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetDbIndexList(DbIndexList& dbIndexList, ECClassCR ecClass)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecClass, ECDBMAP_SCHEMANAME, "DbIndexList");
    if (ca == nullptr)
        return false;

    dbIndexList = DbIndexList(ecClass, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetPropertyMap(ECDbPropertyMap& propertyMap, PrimitiveECPropertyCR ecProperty)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecProperty, ECDBMAP_SCHEMANAME, "PropertyMap");
    if (ca == nullptr)
        return false;

    propertyMap = ECDbPropertyMap(ecProperty, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetLinkTableRelationshipMap(ECDbLinkTableRelationshipMap& linkTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecRelationship, ECDBMAP_SCHEMANAME, "LinkTableRelationshipMap");
    if (ca == nullptr)
        return false;

    linkTableRelationshipMap = ECDbLinkTableRelationshipMap(ecRelationship, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::TryGetForeignKeyConstraint(ECDbForeignKeyConstraint& foreignKeyTableRelationshipMap, ECRelationshipClassCR ecRelationship)
    {
    IECInstanceCP ca = CustomAttributeReader::Read(ecRelationship, ECDBMAP_SCHEMANAME, "ForeignKeyConstraint");
    if (ca == nullptr)
        return false;

    foreignKeyTableRelationshipMap = ECDbForeignKeyConstraint(ecRelationship, ca);
    return true;
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   12 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECDbMapCustomAttributeHelper::HasUseECInstanceIdAsForeignKey(ECRelationshipClassCR relClass)
    {
    return relClass.GetCustomAttributeLocal(ECDBMAP_SCHEMANAME, "UseECInstanceIdAsForeignKey") != nullptr;
    }

//*****************************************************************
//ECDbSchemaMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbSchemaMap::ECDbSchemaMap(ECSchemaCR schema, IECInstanceCP ca) : m_schema(&schema), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbSchemaMap::TryGetTablePrefix(Nullable<Utf8String>& tablePrefix) const
    {
    if (m_ca == nullptr)
        return ERROR;

    if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(tablePrefix, *m_ca, "TablePrefix"))
        return ERROR;

    if (tablePrefix.IsNull() || tablePrefix.Value().empty() || ECNameValidation::Validate(tablePrefix.Value().c_str()) == ECNameValidation::RESULT_Valid)
        return SUCCESS;

    LOG.errorv("Custom attribute '%s' on the ECSchema '%s' has an invalid value for the property 'TablePrefix': %s. "
               "The table prefix should be a few characters long and can only contain [a-zA-Z_0-9] and must start with a non-numeric character.",
               m_ca->GetClass().GetName().c_str(),
               m_schema->GetName().c_str(), tablePrefix.Value().c_str());
    return ERROR;
    }


//*****************************************************************
//ECDbClassMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbClassMap::ECDbClassMap(ECClassCR ecClass, IECInstanceCP ca) : m_class(&ecClass), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbClassMap::TryGetMapStrategy(Nullable<Utf8String>& mapStrategy) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(mapStrategy, *m_ca, "MapStrategy");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbClassMap::TryGetTableName(Nullable<Utf8String>& tableName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(tableName, *m_ca, "TableName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbClassMap::TryGetECInstanceIdColumn(Nullable<Utf8String>& ecInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(ecInstanceIdColumnName, *m_ca, "ECInstanceIdColumn");
    }

//*****************************************************************
//DbIndexList
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   10 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
DbIndexList::DbIndexList(ECClassCR ecClass, IECInstanceCP ca) : m_class(&ecClass), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   10 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DbIndexList::GetIndexes(bvector<DbIndex>& indices) const
    {
    if (m_ca == nullptr)
        return ERROR;

    uint32_t propIx;
    ECObjectsStatus stat = m_ca->GetEnablerR().GetPropertyIndex(propIx, "Indexes");
    if (ECObjectsStatus::Success != stat)
        {
        LOG.errorv("Failed to get property index for property 'List' of custom attribute '%s' on ECClass '%s'.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return ERROR;
        }

    ECValue indexesVal;
    stat = m_ca->GetValue(indexesVal, propIx);
    if (ECObjectsStatus::Success != stat)
        return ERROR;

    indices.clear();
    const uint32_t indexCount = indexesVal.IsNull() ? 0 : indexesVal.GetArrayInfo().GetCount();
    if (indexCount == 0)
        {
        LOG.errorv("Failed to read %s custom attribute' on ECClass '%s'. Its property 'Indexes' must be defined and at contain at least one 'DbIndex' element.",
                   m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
        return ERROR;
        }

    for (uint32_t i = 0; i < indexCount; i++)
        {
        ECValue indexVal;
        if (ECObjectsStatus::Success != m_ca->GetValue(indexVal, propIx, i))
            {
            LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s could not be retrieved.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return ERROR;
            }

        IECInstancePtr dbIndexCA = indexVal.GetStruct();
        if (dbIndexCA == nullptr)
            continue;

        //optional
        Nullable<Utf8String> indexName;
        if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(indexName, *dbIndexCA, "Name"))
            return ERROR;

        Nullable<Utf8String> whereClause;
        if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(whereClause, *dbIndexCA, "Where"))
            return ERROR;

        Nullable<bool> isUnique;
        if (SUCCESS != CustomAttributeReader::TryGetBooleanValue(isUnique, *dbIndexCA, "IsUnique"))
            return ERROR;

        //Properties are mandatory for the index to be valid, so fail if there are none
        uint32_t propertiesPropIdx;
        if (ECObjectsStatus::Success != dbIndexCA->GetEnablerR().GetPropertyIndex(propertiesPropIdx, "Properties"))
            {
            LOG.errorv("Failed to get property index for property 'Indexes.Properties' of custom attribute '%s' on ECClass '%s'.",
                       m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return ERROR;
            }

        ECValue propertiesVal;
        uint32_t propertiesCount = 0;
        if (ECObjectsStatus::Success != dbIndexCA->GetValue(propertiesVal, propertiesPropIdx))
            return ERROR;

        if (propertiesVal.IsNull() || (propertiesCount = propertiesVal.GetArrayInfo().GetCount()) == 0)
            {
            LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s is invalid. At least one property must be specified.",
                       i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
            return ERROR;
            }

        DbIndex dbIndex(indexName, isUnique, whereClause);
        for (uint32_t j = 0; j < propertiesCount; j++)
            {
            Nullable<Utf8String> propName;
            if (SUCCESS != CustomAttributeReader::TryGetTrimmedValue(propName, *dbIndexCA, propertiesPropIdx, j) ||
                propName.IsNull())
                {
                LOG.errorv("DbIndex #%d in custom attribute %s on ECClass %s is invalid. Array element of the Properties property is an empty string.",
                           i, m_ca->GetClass().GetName().c_str(), m_class->GetFullName());
                return ERROR;
                }

            BeAssert(!propName.Value().empty());
            dbIndex.AddProperty(propName.Value());
            }

        indices.push_back(std::move(dbIndex));
        }

    BeAssert(indices.size() == indexCount);
    return SUCCESS;
    }


//*****************************************************************
//ECDbPropertyMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbPropertyMap::ECDbPropertyMap(ECPropertyCR ecProperty, IECInstanceCP ca) : m_property(&ecProperty), m_ca(ca) {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbPropertyMap::TryGetColumnName(Nullable<Utf8String>& columnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(columnName, *m_ca, "ColumnName");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbPropertyMap::TryGetIsNullable(Nullable<bool>& isNullable) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(isNullable, *m_ca, "IsNullable");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbPropertyMap::TryGetIsUnique(Nullable<bool>& isUnique) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(isUnique, *m_ca, "IsUnique");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbPropertyMap::TryGetCollation(Nullable<Utf8String>& collation) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(collation, *m_ca, "Collation");
    }


//*****************************************************************
//ShareColumns
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ShareColumns::TryGetApplyToSubclassesOnly(Nullable<bool>& applyToSubclassesOnly) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(applyToSubclassesOnly, *m_ca, "ApplyToSubclassesOnly");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   08 / 2016
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ShareColumns::TryGetSharedColumnCount(Nullable<uint32_t>& sharedColumnCount) const
    {
    if (m_ca == nullptr)
        return ERROR;
    Nullable<int> intVal;
    if (SUCCESS != CustomAttributeReader::TryGetIntegerValue(intVal, *m_ca, "SharedColumnCount"))
        return ERROR;

    if (intVal.IsNull())
        {
        sharedColumnCount = Nullable<uint32_t>();
        return SUCCESS;
        }

    if (intVal.Value() < 0)
        return ERROR;

    sharedColumnCount = Nullable<uint32_t>((uint32_t) intVal.Value());
    return SUCCESS;
    }


//*****************************************************************
//ECDbLinkTableRelationshipMap
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbLinkTableRelationshipMap::ECDbLinkTableRelationshipMap(ECRelationshipClassCR relClass, IECInstanceCP ca)
    : m_relClass(&relClass), m_ca(ca)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbLinkTableRelationshipMap::TryGetSourceECInstanceIdColumn(Nullable<Utf8String>& sourceECInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(sourceECInstanceIdColumnName, *m_ca, "SourceECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbLinkTableRelationshipMap::TryGetTargetECInstanceIdColumn(Nullable<Utf8String>& targetECInstanceIdColumnName) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(targetECInstanceIdColumnName, *m_ca, "TargetECInstanceIdColumn");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbLinkTableRelationshipMap::TryGetAllowDuplicateRelationships(Nullable<bool>& allowDuplicateRelationshipsFlag) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetBooleanValue(allowDuplicateRelationshipsFlag, *m_ca, "AllowDuplicateRelationships");
    }

//*****************************************************************
//ECDbForeignKeyConstraint
//*****************************************************************
//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
ECDbForeignKeyConstraint::ECDbForeignKeyConstraint(ECRelationshipClassCR relClass, IECInstanceCP ca)
    : m_relClass(&relClass), m_ca(ca)
    {}

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbForeignKeyConstraint::TryGetOnDeleteAction(Nullable<Utf8String>& onDeleteAction) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(onDeleteAction, *m_ca, "OnDeleteAction");
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                               Krischan.Eberle   06 / 2015
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbForeignKeyConstraint::TryGetOnUpdateAction(Nullable<Utf8String>& onUpdateAction) const
    {
    if (m_ca == nullptr)
        return ERROR;

    return CustomAttributeReader::TryGetTrimmedValue(onUpdateAction, *m_ca, "OnUpdateAction");
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
