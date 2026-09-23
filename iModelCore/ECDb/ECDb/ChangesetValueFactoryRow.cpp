/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=============================================================================
// ChangesetValueFactory — high-level resolution helpers
// Per-property value construction is in ChangesetValueFactory.cpp.
//=============================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryResolveClassIdFromChangeset(
    DbTable const& dbTable, ColumnValueMap const& columnValues,
    ECDbCR conn, ECClassId& classIdOut) {

    DbColumn const& classIdCol = dbTable.GetECClassIdColumn();

    auto it = columnValues.find(classIdCol.GetName());
    if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull())
        return false;

    ECClassId candidate(it->second.GetValueUInt64());
    if (!candidate.IsValid())
        return false;

    classIdOut  = candidate;
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::TryResolveClassIdFromDbSeek(
    DbTable const& dbTable,
    ColumnValueMap const& columnValues,
    ECDbCR conn, ECClassId& classIdOut) {

    // A virtual class-id column has no physical storage — nothing to read from the DB.
    DbColumn const& classIdCol = dbTable.GetECClassIdColumn();
    if (classIdCol.IsVirtual())
        return false;

    // Validate all PK columns, build and bind the statement via shared helper.
    ECClassId classId;
    if(!TryFetchBeInt64IdFromDb(classId, conn, classIdCol, columnValues))
        return false;

    if (!classId.IsValid())
        return false;

    classIdOut  = classId;
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::ResolveInstanceId(
    ClassMap const& classMap,
    ColumnValueMap const& columnValues,
    ECDbCR conn, DbTable const& primaryDbTable,
    ECInstanceId& instanceIdOut, std::unique_ptr<IECSqlValue>& fieldOut) {

    for (auto& propertyMap : classMap.GetPropertyMaps()) {
        if (!propertyMap->IsSystem())
            continue;
        const auto prim = propertyMap->GetProperty().GetAsPrimitiveProperty();
        if (prim == nullptr)
            continue;
        const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extType != ExtendedTypeHelper::ExtendedType::Id)
            continue;
        if (!propertyMap->GetProperty().GetName().EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
            continue;

        const auto& sysMap = propertyMap->GetAs<SystemPropertyMap>();
        const SystemPropertyMap::PerTableIdPropertyMap* dataMap = sysMap.FindDataPropertyMap(primaryDbTable);
        if(dataMap == nullptr && primaryDbTable.GetType() == DbTable::Type::Overflow) {
            DbTable const& parentTable = primaryDbTable.GetLinkNode().GetParent()->GetTable();
            LOG.infov("ECInstanceId property map not found for overflow table '%s'; trying parent table '%s'.",
                      primaryDbTable.GetName().c_str(), parentTable.GetName().c_str());
            dataMap = sysMap.FindDataPropertyMap(parentTable);
        }
        if (dataMap == nullptr) {
            LOG.errorv("No ECInstanceId data property map found for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return ERROR;
        }
        Utf8StringCR colName = dataMap->GetColumn().GetName();
        auto it = columnValues.find(colName);
        if (it == columnValues.end() || !it->second.IsValid() || it->second.IsNull()) {
            LOG.errorv("ECInstanceId is absent or null in changeset for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return ERROR;
        }

        ECInstanceId instanceId(it->second.GetValueUInt64());
        if (!instanceId.IsValid()) {
            LOG.errorv("ECInstanceId resolved to an invalid (zero) id for table '%s'.",
                       primaryDbTable.GetName().c_str());
            return ERROR;
        }

        std::unique_ptr<IECSqlValue> sysVal;
        CreateFixedId(conn, *propertyMap, instanceId, sysVal);

        instanceIdOut = instanceId;
        fieldOut      = std::move(sysVal);
        LOG.debugv("Table '%s': resolved ECInstanceId %" PRIu64 " from changeset.",
                   primaryDbTable.GetName().c_str(), instanceId.GetValueUnchecked());
        return SUCCESS;
    }

    LOG.errorv("ECInstanceId property not found in ClassMap for table '%s'.",
               primaryDbTable.GetName().c_str());
    return ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::ResolveClassIdField(
    ClassMap const& classMap,
    ECClassId resolvedClassId,
    ECDbCR conn, DbTable const& primaryDbTable,
    std::unique_ptr<IECSqlValue>& out) {

    for (auto& propertyMap : classMap.GetPropertyMaps()) {
        if (!propertyMap->IsSystem())
            continue;
        const auto prim = propertyMap->GetProperty().GetAsPrimitiveProperty();
        if (prim == nullptr)
            continue;
        const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extType != ExtendedTypeHelper::ExtendedType::ClassId)
            continue;
        if (!propertyMap->GetProperty().GetName().EqualsIAscii(ECDBSYS_PROP_ECClassId))
            continue;

        CreateFixedId(conn, *propertyMap, resolvedClassId, out);
        return SUCCESS;
    }

    LOG.errorv("ECClassId property not found in ClassMap for table '%s'.",
               primaryDbTable.GetName().c_str());
    return ERROR;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::BuildPropertyFields(
    ClassMap const& classMap,
    ColumnValueMap const& columnValues,
    ECDbCR conn,
    DbTable const& dbTable,
    std::vector<std::unique_ptr<IECSqlValue>>& fieldsOut,
    std::vector<Utf8String>* changedProps) {

    for (auto& propertyMap : classMap.GetPropertyMaps()) {
        // ECInstanceId and ECClassId are emitted as fixed slots [0] and [1] by the caller.
        if (propertyMap->IsSystem()) {
            const auto prim = propertyMap->GetProperty().GetAsPrimitiveProperty();
            if (prim != nullptr) {
                const auto extType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
                Utf8StringCR propName = propertyMap->GetProperty().GetName();
                if (extType == ExtendedTypeHelper::ExtendedType::Id &&
                    propName.EqualsIAscii(ECDBSYS_PROP_ECInstanceId))
                    continue;
                if (extType == ExtendedTypeHelper::ExtendedType::ClassId &&
                    propName.EqualsIAscii(ECDBSYS_PROP_ECClassId))
                    continue;
            }
        }
        BentleyStatus status = CreateValueForProperty(conn, *propertyMap, columnValues, dbTable, fieldsOut, changedProps);
        if (status != SUCCESS)
            return status;
    }
    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ChangesetValueFactory::IsDerivedFromBisElement(ECClassId classId, ECDbCR conn) {
    const ECClass* cls = conn.Schemas().Main().GetClass(classId);
    if (cls == nullptr)
        return false;

    const ECClass* bisElementClass = conn.Schemas().Main().GetClass("BisCore", "Element", SchemaLookupMode::AutoDetect);
    if (bisElementClass == nullptr)
        return false;

    // If it's exactly BisCore.Element, return false because we just want children of biscore element class not the biscore elemnt class itself
    if(ECClass::ClassesAreEqualByName(cls, bisElementClass)) 
        return false;

    return cls->Is(bisElementClass);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ChangesetValueFactory::FillChangedPropIfApplicable(std::vector<Utf8String>* changedProps, Utf8String const& propName) {
    if (changedProps != nullptr)
        changedProps->push_back(propName);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::ResolveClassId(
    ECDbCR conn, DbTable const& tbl, ColumnValueMap const& columnValues, ECClassId& resolvedClassIdOut, bool& classIdFromChangesetOut) {
    resolvedClassIdOut.Invalidate();
    classIdFromChangesetOut = false;

    if (TryResolveClassIdFromChangeset(tbl, columnValues, conn, resolvedClassIdOut)) {
        classIdFromChangesetOut = true;
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " from changeset.",
                   tbl.GetName().c_str(), resolvedClassIdOut.GetValueUnchecked());
    } else if (TryResolveClassIdFromDbSeek(tbl, columnValues, conn, resolvedClassIdOut)) {
        LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " via DB seek.",
                   tbl.GetName().c_str(), resolvedClassIdOut.GetValueUnchecked());
    } else {
        const ClassMap* classMapOut = GetRootClassMap(tbl, conn);
        if (classMapOut != nullptr) {
            resolvedClassIdOut = classMapOut->GetClass().GetId();
            LOG.debugv("Table '%s': resolved ECClassId %" PRIu64 " via GetRootClassMap.",
                       tbl.GetName().c_str(), resolvedClassIdOut.GetValueUnchecked());
        }
    }

    if (!resolvedClassIdOut.IsValid()) {
        LOG.errorv("Could not resolve ECClassId for table '%s'.", tbl.GetName().c_str());
        return ERROR;
    }

    return SUCCESS;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ChangesetValueFactory::Create(
    ECDbCR conn, DbTable const& tbl, ColumnValueMap const& columnValues, ECN::ECClassId resolvedClassId, bool classIdFromChangeset,
    std::vector<std::unique_ptr<IECSqlValue>>& fields, ChangesetReader::PropertyFilter propertyFilter, DbOpcode opcode, std::vector<Utf8String>* changedProps) {

    const ECClass* cls = conn.Schemas().Main().GetClass(resolvedClassId);
    if (cls == nullptr) {
        LOG.errorv("Resolved ECClassId %" PRIu64 " could not be found in the schema.", resolvedClassId.GetValueUnchecked());
        return ERROR;
    }

    const ClassMap* classMap  = conn.Schemas().Main().GetClassMap(*cls);
    if (classMap == nullptr) {
        LOG.errorv("ClassMap for ECClassId %" PRIu64 " could not be found in the schema.", resolvedClassId.GetValueUnchecked());
        return ERROR;
    }

    // ECInstanceId and ECClassId name collection — handled here since they are
    // emitted as fixed slots and skipped by the BuildPropertyFields loop.
    FillChangedPropIfApplicable(changedProps, ECDBSYS_PROP_ECInstanceId);
    if (classIdFromChangeset)
        FillChangedPropIfApplicable(changedProps, ECDBSYS_PROP_ECClassId);

    // -----------------------------------------------------------------------
    // Step 2: Resolve ECInstanceId (slot [0]).
    // -----------------------------------------------------------------------
    ECInstanceId instanceId;
    std::unique_ptr<IECSqlValue> instanceIdField;
    BentleyStatus status = ResolveInstanceId(*classMap, columnValues, conn, tbl, instanceId, instanceIdField);
    if (status != SUCCESS)
        return status;

    // -----------------------------------------------------------------------
    // Step 3: Build ECClassId field (slot [1]).
    // -----------------------------------------------------------------------
    std::unique_ptr<IECSqlValue> classIdField;
    status = ResolveClassIdField(*classMap, resolvedClassId, conn, tbl, classIdField);
    if (status != SUCCESS)
        return status;

    // -----------------------------------------------------------------------
    // Step 4: Build remaining property fields (slots [2+]).
    // -----------------------------------------------------------------------
    fields.emplace_back(std::move(instanceIdField));
    fields.emplace_back(std::move(classIdField));

    if (propertyFilter == ChangesetReader::PropertyFilter::InstanceKey)
        return SUCCESS; // caller only needs the instance key — skip user properties

    if (propertyFilter == ChangesetReader::PropertyFilter::InstanceKeyAndIdentifiers) {
        // Whitelist of identifiers read only from the changeset; values absent from the current table and changeset are omitted.
        auto isBisCore = [&](Utf8CP className) {
            const ECClass* bisClass = conn.Schemas().Main().GetClass("BisCore", className, SchemaLookupMode::AutoDetect);
            return bisClass != nullptr && cls->Is(bisClass);
        };
        const bool isDelete = opcode == DbOpcode::Delete;
        const bool isAspect = isBisCore("ElementAspect");
        const bool isDeletedExternalSourceAspect = isDelete && isBisCore("ExternalSourceAspect");
        const bool isDeletedElement = isDelete && isBisCore("Element");
        const bool isDeletedLinkTableRelationship = isDelete && classMap->GetType() == ClassMap::Type::RelationshipLinkTable;
        for (auto const& propertyMap : classMap->GetPropertyMaps()) {
            ECPropertyCR prop = propertyMap->GetProperty();
            Utf8StringCR name = prop.GetName();
            if (prop.GetIsNavigation() && ((isAspect && name.EqualsIAscii("Element")) || (isDeletedExternalSourceAspect && name.EqualsIAscii("Scope"))))
                CreateNavIdOnly(conn, *propertyMap, columnValues, tbl, fields, changedProps);
            else if (!propertyMap->IsSystem() && prop.GetIsPrimitive() && ((isDeletedElement && name.EqualsIAscii("FederationGuid")) || (isDeletedExternalSourceAspect && (name.EqualsIAscii("Kind") || name.EqualsIAscii("Identifier"))))) {
                if (CreatePrimitive(conn, *propertyMap, columnValues, tbl, fields, changedProps) != SUCCESS)
                    return ERROR;
            } else if (propertyMap->IsSystem() && isDeletedLinkTableRelationship && (name.EqualsIAscii(ECDBSYS_PROP_SourceECInstanceId) || name.EqualsIAscii(ECDBSYS_PROP_TargetECInstanceId))) {
                if (CreateSystem(conn, *propertyMap, columnValues, tbl, fields, changedProps) != SUCCESS)
                    return ERROR;
            }
        }
        return SUCCESS;
    }

    if(propertyFilter == ChangesetReader::PropertyFilter::BisCoreElement && IsDerivedFromBisElement(resolvedClassId, conn) && !tbl.GetName().EqualsIAscii("bis_Element"))
        return SUCCESS; // caller only needs bis_element properties — skip rest   

    status = BuildPropertyFields(*classMap, columnValues, conn, tbl, fields, changedProps);

    if (status != SUCCESS) {
        fields.clear();
        return status;
    }

    return SUCCESS;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
