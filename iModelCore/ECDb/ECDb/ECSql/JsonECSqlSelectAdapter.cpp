/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECObjects/ECJsonUtilities.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct JsonECSqlSelectAdapter::CacheImpl
    {
    mutable bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>> m_memberNames;
    mutable bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>> m_uniqueMemberNames;
    };
JsonECSqlSelectAdapter::JsonECSqlSelectAdapter(ECSqlStatement const& ecsqlStatement, FormatOptions const& formatOptions, bool copyMemberNames )
    : m_ecsqlStatement(ecsqlStatement), m_formatOptions(formatOptions), m_copyMemberNames(copyMemberNames), m_ecsqlHash(ecsqlStatement.GetHashCode()), m_cacheImpl(new CacheImpl())
    {}

JsonECSqlSelectAdapter::~JsonECSqlSelectAdapter()
    {
    delete m_cacheImpl;
    }

//=======================================================================================
//! Helper class for the JsonECSqlSelectAdapter
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct AdapterHelper final
    {
    private:
        AdapterHelper() = delete;
        ~AdapterHelper() = delete;

        static BentleyStatus DetermineMemberNames(bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&);

        static BentleyStatus SelectClauseItemToJson(BeJsValue, IECSqlValue const&, ECSqlSystemPropertyInfo const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&, bool abbreviateBlobs = false, bool classIdToClassNames = true);
        static BentleyStatus PropertyValueToJson(BeJsValue, IECSqlValue const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&, bool abbreviateBlobs = false);
        static BentleyStatus NavigationToJson(BeJsValue, IECSqlValue const&, SchemaManager const&);
        static BentleyStatus StructToJson(BeJsValue, IECSqlValue const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus PrimitiveArrayToJson(BeJsValue, IECSqlValue const&, ECN::PrimitiveType, JsonECSqlSelectAdapter::FormatOptions const&, bool abbreviateBlobs = false);
        static BentleyStatus StructArrayToJson(BeJsValue, IECSqlValue const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus PrimitiveToJson(BeJsValue, IECSqlValue const&, ECN::PrimitiveType, JsonECSqlSelectAdapter::FormatOptions const&, bool abbreviateBlobs = false);

        static ECSqlSystemPropertyInfo const& DetermineTopLevelSystemPropertyInfo(SchemaManager const&, ECSqlColumnInfoCR, JsonECSqlSelectAdapter::FormatOptions const&);
        static Utf8String MemberNameFromSelectClauseItemCustom(ECDbSystemSchemaHelper const&, ECSqlColumnInfoCR, ECSqlSystemPropertyInfo const&, JsonECSqlSelectAdapter::MemberNameCasing);
        static Utf8String MemberNameFromSelectClauseItemIModelJs(ECDbSystemSchemaHelper const&, ECSqlColumnInfo const& colInfo, ECSqlSystemPropertyInfo const& sysPropInfo, JsonECSqlSelectAdapter::MemberNameCasing);
        static void FormatMemberName(Utf8String&, JsonECSqlSelectAdapter::MemberNameCasing);
        static bool NeedsMemberNameReformatting(JsonECSqlSelectAdapter::MemberNameCasing);

        static Utf8StringCR GetMemberName(bvector<bpair<Utf8String, Utf8String>> const& memberNames, size_t colIndex)
            {
            BeAssert(colIndex < memberNames.size());
            return memberNames[colIndex].first;
            }

        static Utf8StringCR GetUniqueMemberName(bvector<bpair<Utf8String, Utf8String>> const& memberNames, size_t colIndex)
            {
            BeAssert(colIndex < memberNames.size());
            bpair<Utf8String, Utf8String> const& memberName = memberNames[colIndex];
            if (memberName.second.empty())
                return memberName.first;

            return memberName.second;
            }

        static bool IsValid(ECSqlStatement const&);

    public:
        static BentleyStatus DetermineUniqueMemberNames(bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus GetRow(BeJsValue, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, bool appendToRow, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&, bool copyMemberNames, bool abbreviateBlobs);
        static BentleyStatus GetRowInstance(BeJsValue, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, ECN::ECClassId, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus GetRowAsArray(BeJsValue rowJson, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& uniqueMemberNames, bool appendToJson, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool copyMemberNames, bool abbreviateBlobs, bool classIdToClassNames);
    };

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetJsonPropertyNames(std::vector<Utf8String>& props) const {
    m_cacheImpl->m_uniqueMemberNames.clear();
    if (AdapterHelper::DetermineUniqueMemberNames(m_cacheImpl->m_uniqueMemberNames, m_ecsqlStatement, m_ecsqlHash, m_formatOptions) != SUCCESS)
        return ERROR;
    
    for(auto & prop: m_cacheImpl->m_uniqueMemberNames) {
        props.push_back(prop.first);
    }
    return SUCCESS;
}
//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRow(BeJsValue rowJson, bool appendToJson, bool abbreviateBlobs) const
    {
    return AdapterHelper::GetRow(rowJson, m_cacheImpl->m_uniqueMemberNames, appendToJson, m_ecsqlStatement, m_ecsqlHash, m_formatOptions, m_copyMemberNames, abbreviateBlobs);
    }
//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRowAsArray(BeJsValue rowJson, bool appendToJson, bool abbreviateBlobs, bool classIdToClassNames) const
    {
    return AdapterHelper::GetRowAsArray(rowJson, m_cacheImpl->m_uniqueMemberNames, appendToJson, m_ecsqlStatement, m_ecsqlHash, m_formatOptions, m_copyMemberNames, abbreviateBlobs, classIdToClassNames);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::GetRow(BeJsValue rowJson, ECSqlStatement const& stmt, bool appendToJson, bool abbreviateBlobs) const
    {
    return AdapterHelper::GetRow(rowJson, m_cacheImpl->m_uniqueMemberNames, appendToJson, stmt, stmt.GetHashCode(), m_formatOptions, m_copyMemberNames, abbreviateBlobs);
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRowInstance(BeJsValue rowJson, ECClassId classId) const
    {
    return AdapterHelper::GetRowInstance(rowJson, m_cacheImpl->m_memberNames, classId, m_ecsqlStatement, m_ecsqlHash, m_formatOptions);
    }

//******************************************************************************************
// AdapterHelper
//******************************************************************************************
//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool AdapterHelper::IsValid(ECSqlStatement const& stmt)
    {
    if (!stmt.IsPrepared())
        {
        LOG.error("ECSqlStatement passed to JsonECSqlSelectAdapter is not prepared yet.");
        return false;
        }

    if (stmt.GetColumnCount() == 0)
        {
        LOG.errorv("ECSqlStatement '%s' passed to JsonECSqlSelectAdapter has no SELECT clause.", stmt.GetECSql());
        return false;
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::GetRow(BeJsValue rowJson, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& uniqueMemberNames, bool appendToJson, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool copyMemberNames, bool abbreviateBlobs)
    {
    if (SUCCESS != DetermineUniqueMemberNames(uniqueMemberNames, stmt, ecsqlHash, formatOptions))
        return ERROR;

    if (appendToJson)
        {
        if (!rowJson.isObject())
            return ERROR;
        }
    else
        rowJson.SetEmptyObject();

    const int count = stmt.GetColumnCount();
    BeAssert(count == (int) uniqueMemberNames.size());
    SchemaManager const& schemaManager = stmt.GetECDb()->Schemas();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
        if (ecsqlValue.IsNull())
            continue;

        auto const& member = uniqueMemberNames[(size_t) columnIndex];
        Utf8CP memberName = member.first.c_str();
        ECSqlSystemPropertyInfo const& sysPropInfo = member.second;
        if (SUCCESS != SelectClauseItemToJson(copyMemberNames ? rowJson[memberName] : rowJson[Json::StaticString(memberName)], ecsqlValue, sysPropInfo, schemaManager, formatOptions, abbreviateBlobs))
            return ERROR;
        }

    return SUCCESS;
    }
//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::GetRowAsArray(BeJsValue rowJson, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& uniqueMemberNames, bool appendToJson, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool copyMemberNames, bool abbreviateBlobs, bool classIdToClassNames) {
    const auto trimNulls = true;
    if (SUCCESS != DetermineUniqueMemberNames(uniqueMemberNames, stmt, ecsqlHash, formatOptions))
        return ERROR;
    
    if (appendToJson) {
        if (!rowJson.isArray())
            return ERROR;
    } else {
        rowJson.SetEmptyArray();
    }
    const int count = stmt.GetColumnCount();
    SchemaManager const& schemaManager = stmt.GetECDb()->Schemas();
    int nulls = 0;
    for (int columnIndex = 0; columnIndex < count; columnIndex++) {
        IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
        if (ecsqlValue.IsNull()) {
            if (trimNulls) {
                ++nulls;
            }
            continue;
        }
        if (trimNulls) {
            while(nulls  > 0) {
                rowJson.appendValue().SetNull();
                --nulls;
            }
        }
        auto const& member = uniqueMemberNames[(size_t) columnIndex];
        ECSqlSystemPropertyInfo const& sysPropInfo = member.second;
        if (SUCCESS != SelectClauseItemToJson(rowJson.appendValue(), ecsqlValue, sysPropInfo, schemaManager, formatOptions, abbreviateBlobs, classIdToClassNames))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::GetRowInstance(BeJsValue rowJson, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& memberNames, ECClassId classId, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    if (!classId.IsValid())
        return ERROR;

    if (SUCCESS != DetermineMemberNames(memberNames, stmt, ecsqlHash, formatOptions))
        return ERROR;

    rowJson.SetEmptyObject();

    SchemaManager const& schemaManager = stmt.GetECDb()->Schemas();
    bool foundMatchingSelectClauseItems = false;
    const int count = stmt.GetColumnCount();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
        if (ecsqlValue.IsNull())
            continue;

        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        ECSqlColumnInfo::RootClass const& rootClass = colInfo.GetRootClass();
        if (colInfo.IsGeneratedProperty() || rootClass.GetClass().GetId() != classId)
            continue;

        if (colInfo.GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
            {
            LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", stmt.GetECSql());
            return ERROR;
            }

        foundMatchingSelectClauseItems = true;

        BeAssert(colInfo.GetProperty() != nullptr);
        auto const& member = memberNames[(size_t) columnIndex];
        ECSqlSystemPropertyInfo const& sysPropInfo = member.second;
        Utf8StringCR memberName = member.first;
        if (SUCCESS != SelectClauseItemToJson(rowJson[memberName], ecsqlValue, sysPropInfo, schemaManager, formatOptions))
            return ERROR;
        }

    if (!foundMatchingSelectClauseItems)
        {
        LOG.errorv("No properties of ECClass with Id '%s' found in ECSqlStatement '%s' passed to JsonECSqlSelectAdapter.", classId.ToString().c_str(), stmt.GetECSql());
        return ERROR;
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::DetermineUniqueMemberNames(bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& uniqueMemberNames, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    if (!IsValid(stmt))
        return ERROR;

    if (ecsqlHash == stmt.GetHashCode() && !uniqueMemberNames.empty())
        return SUCCESS;

    if (!uniqueMemberNames.empty())
        uniqueMemberNames.clear();

    const int count = stmt.GetColumnCount();
    bmap<Utf8String, int> columnNameCollisions;
    SchemaManager const& schemaManager = stmt.GetECDb()->Schemas();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        if (colInfo.GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
            {
            uniqueMemberNames.clear();
            LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", stmt.GetECSql());
            return ERROR;
            }

        BeAssert(colInfo.GetProperty() != nullptr);
        ECSqlSystemPropertyInfo const& sysPropInfo = DetermineTopLevelSystemPropertyInfo(schemaManager, colInfo, formatOptions);
        if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::Custom)
            uniqueMemberNames.push_back(make_bpair(MemberNameFromSelectClauseItemCustom(schemaManager.Main().GetSystemSchemaHelper(), colInfo, sysPropInfo, formatOptions.GetMemberCasingMode()), sysPropInfo));
        else
            uniqueMemberNames.push_back(make_bpair(MemberNameFromSelectClauseItemIModelJs(schemaManager.Main().GetSystemSchemaHelper(), colInfo, sysPropInfo, formatOptions.GetMemberCasingMode()), sysPropInfo));
        Utf8StringR memberName = uniqueMemberNames.back().first;
        if (memberName.empty())
            {
            uniqueMemberNames.clear();
            return ERROR;
            }

        int occurrenceCount = 0;
        auto it = columnNameCollisions.find(memberName);
        if (it == columnNameCollisions.end())
            occurrenceCount = 1;
        else
            occurrenceCount = it->second + 1;

        columnNameCollisions[memberName] = occurrenceCount;

        if (occurrenceCount > 1)
            {
            Utf8String suffix;
            suffix.Sprintf("_%d", occurrenceCount - 1);
            memberName.append(suffix);
            }
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::DetermineMemberNames(bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& memberNames, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    if (!IsValid(stmt))
        return ERROR;

    if (ecsqlHash == stmt.GetHashCode() && !memberNames.empty())
        return SUCCESS;

    if (!memberNames.empty())
        memberNames.clear();

    const int count = stmt.GetColumnCount();
    SchemaManager const& schemaManager = stmt.GetECDb()->Schemas();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
        ECSqlColumnInfoCR colInfo = ecsqlValue.GetColumnInfo();
        if (colInfo.GetPropertyPath().GetLeafEntry().GetKind() == ECSqlPropertyPath::Entry::Kind::ArrayIndex)
            {
            memberNames.clear();
            LOG.errorv("JsonECSqlSelectAdapter does not support array accessors in an ECSQL select clause: %s", stmt.GetECSql());
            return ERROR;
            }

        BeAssert(colInfo.GetProperty() != nullptr);
        ECSqlSystemPropertyInfo const& sysPropInfo = DetermineTopLevelSystemPropertyInfo(schemaManager, colInfo, formatOptions);
        if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::Custom)
            memberNames.push_back(make_bpair(MemberNameFromSelectClauseItemCustom(schemaManager.Main().GetSystemSchemaHelper(), colInfo, sysPropInfo, formatOptions.GetMemberCasingMode()), sysPropInfo));
        else
            memberNames.push_back(make_bpair(MemberNameFromSelectClauseItemIModelJs(schemaManager.Main().GetSystemSchemaHelper(), colInfo, sysPropInfo, formatOptions.GetMemberCasingMode()), sysPropInfo));

        Utf8StringR memberName = memberNames.back().first;
        if (memberName.empty())
            {
            memberNames.clear();
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::SelectClauseItemToJson(BeJsValue json, IECSqlValue const& ecsqlValue, ECSqlSystemPropertyInfo const& sysPropInfo, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool abbreviateBlobs, bool classIdToClassNames)
    {
    BeAssert(!ecsqlValue.IsNull() && "Should have been caught before");

    if (!sysPropInfo.IsSystemProperty())
        return PropertyValueToJson(json, ecsqlValue, schemaManager, formatOptions, abbreviateBlobs);

    BeAssert(ecsqlValue.GetId<BeInt64Id>().IsValid());

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class)
        {
        switch (sysPropInfo.GetClass())
            {
                case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                 return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Class::ECClassId: {
                    if (classIdToClassNames) {
                        ECClassCP cls = schemaManager.GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
                        if (cls == nullptr)
                            return ERROR;

                        ECJsonUtilities::ClassNameToJson(json, *cls);
                        return SUCCESS;
                    }
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<ECClassId>());
                }
                default:
                    BeAssert(false);
                    return ERROR;
            }
        }
    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Navigation)
        {
        switch (sysPropInfo.GetNavigation())
            {
                case ECSqlSystemPropertyInfo::Navigation::Id:
                     return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Navigation::RelECClassId: {
                    if (classIdToClassNames) {
                        ECClassCP cls = schemaManager.GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
                        if (cls == nullptr)
                            return ERROR;

                        ECJsonUtilities::ClassNameToJson(json, *cls);
                        return SUCCESS;
                    }
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<ECClassId>());
                }

                default:
                    BeAssert(false);
                    return ERROR;
            }
        }

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Relationship)
        {
        switch (sysPropInfo.GetRelationship())
            {
                case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                case ECSqlSystemPropertyInfo::Relationship::TargetECClassId: {
                    if (classIdToClassNames) {
                        ECClassCP cls = schemaManager.GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
                        if (cls == nullptr)
                            return ERROR;

                        ECJsonUtilities::ClassNameToJson(json, *cls);
                        return SUCCESS;
                    }
                    return ECJsonUtilities::IdToJson(json, ecsqlValue.GetId<ECClassId>());
                }

                default:
                    BeAssert(false);
                    return ERROR;
            }
        }

    BeAssert(false && "Other system properties are not expected to show up as top-level props in an ECSQL select clause");
    return ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::PropertyValueToJson(BeJsValue jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool abbreviateBlobs)
    {
    BeAssert(!ecsqlValue.IsNull() && "Should have been caught before to avoid that a null JSON value is created");

    ECN::ECPropertyCP prop = ecsqlValue.GetColumnInfo().GetProperty();
    BeAssert(prop != nullptr);

    if (prop->GetIsPrimitive())
        return PrimitiveToJson(jsonValue, ecsqlValue, prop->GetAsPrimitiveProperty()->GetType(), formatOptions, abbreviateBlobs);

    if (prop->GetIsStruct())
        return StructToJson(jsonValue, ecsqlValue, schemaManager, formatOptions);

    if (prop->GetIsNavigation())
        return NavigationToJson(jsonValue, ecsqlValue, schemaManager);

    if (prop->GetIsPrimitiveArray())
        return PrimitiveArrayToJson(jsonValue, ecsqlValue, prop->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType(), formatOptions, abbreviateBlobs);

    if (prop->GetIsStructArray())
        return StructArrayToJson(jsonValue, ecsqlValue, schemaManager, formatOptions);

    BeAssert(false && "Unhandled ECProperty type. Adjust the code for this new ECProperty type");
    return ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::PrimitiveArrayToJson(BeJsValue jsonValue, IECSqlValue const& ecsqlValue, PrimitiveType arrayElementType, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool abbreviateBlobs)
    {
    jsonValue.SetEmptyArray();
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != PrimitiveToJson(jsonValue.appendValue(), arrayElementValue, arrayElementType, formatOptions, abbreviateBlobs))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::StructArrayToJson(BeJsValue jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    jsonValue.SetEmptyArray();
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        if (SUCCESS != StructToJson(jsonValue.appendValue(), arrayElementValue, schemaManager, formatOptions))
            return ERROR;
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::NavigationToJson(BeJsValue jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager)
    {
    IECSqlValue const& navIdVal = ecsqlValue[ECDBSYS_PROP_NavPropId];
    if (navIdVal.IsNull())
        return SUCCESS;

    const ECInstanceId navId = navIdVal.GetId<ECInstanceId>();

    ECClassCP relClass = nullptr;
    IECSqlValue const& relClassIdVal = ecsqlValue[ECDBSYS_PROP_NavPropRelECClassId];
    if (!relClassIdVal.IsNull())
        {
        relClass = schemaManager.GetClass(relClassIdVal.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
        if (relClass == nullptr)
            return ERROR;
        }

    jsonValue.SetEmptyObject();
    if (SUCCESS !=  ECJsonUtilities::IdToJson(jsonValue[Json::StaticString(ECJsonSystemNames::Navigation::Id())], navId))
        return ERROR;

    if (relClass == nullptr)
        return SUCCESS;

    ECJsonUtilities::ClassNameToJson(jsonValue[Json::StaticString(ECJsonSystemNames::Navigation::RelClassName())], *relClass);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::StructToJson(BeJsValue jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    jsonValue.SetEmptyObject();
    for (IECSqlValue const& structMemberValue : ecsqlValue.GetStructIterable())
        {
        if (structMemberValue.IsNull())
            continue;

        ECPropertyCP memberProp = structMemberValue.GetColumnInfo().GetProperty();
        Utf8String memberName  = memberProp->GetName();
        if (NeedsMemberNameReformatting(formatOptions.GetMemberCasingMode()))
            FormatMemberName(memberName, formatOptions.GetMemberCasingMode());

        if (SUCCESS != PropertyValueToJson(jsonValue[memberName], structMemberValue, schemaManager, formatOptions))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::PrimitiveToJson(BeJsValue jsonValue, IECSqlValue const& ecsqlValue, ECN::PrimitiveType primType, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool abbreviateBlobs)
    {
    BeAssert(!ecsqlValue.IsNull());
    switch (primType)
        {
        case PRIMITIVETYPE_Binary:
            {
            int size = 0;
            Byte const* data = (Byte const*) ecsqlValue.GetBlob(&size);
            if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::IModelJs && ecsqlValue.GetColumnInfo().GetProperty())
                {
                const auto prop = ecsqlValue.GetColumnInfo().GetProperty()->GetAsPrimitiveProperty();
                const bool isGuid = !prop->GetExtendedTypeName().empty() && prop->GetExtendedTypeName().EqualsIAscii("BeGuid");
                if (isGuid)
                    {
                    if (size != sizeof(BeGuid))
                        return SUCCESS;

                    BeGuid guid;
                    memcpy(&guid, data, sizeof(guid));
                    jsonValue = guid.ToString().c_str();

                    return BentleyStatus::SUCCESS;
                    }
                }

            if (abbreviateBlobs && size > 1)
                size = 1;
                // There should be a way to communicate this in the value...

            // not a guid
            if (formatOptions.GetBlobMode() == JsonECSqlSelectAdapter::BlobMode::Base64String) {
                jsonValue.SetBinary(data, (size_t) size);
                return BentleyStatus::SUCCESS;
            }

            // render as int[]
            jsonValue.SetEmptyArray();
            for (Json::ArrayIndex i = 0; i < (Json::ArrayIndex)size; i++)
                jsonValue[i] = data[i];
            return BentleyStatus::SUCCESS;
            }
            case PRIMITIVETYPE_Boolean:
            {
            jsonValue =  ecsqlValue.GetBoolean();

            return SUCCESS;
            }

            case PRIMITIVETYPE_DateTime:
            {
            DateTime dt = ecsqlValue.GetDateTime();
            ECJsonUtilities::DateTimeToJson(jsonValue, dt);

            return SUCCESS;
            }
            case PRIMITIVETYPE_Double:
            {
            jsonValue = ecsqlValue.GetDouble();

            return SUCCESS;
            }
            case PRIMITIVETYPE_Integer:
            {
            if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::Custom)
                {
                jsonValue = ecsqlValue.GetInt();
                }
            else
                {
                jsonValue = std::trunc(ecsqlValue.GetDouble());
                }
            return SUCCESS;
            }
            case PRIMITIVETYPE_Long:
            {
            // following is also called for array type property and ecsqlValue.GetColumnInfo().GetProperty() will yield null for that case
            const auto primitiveProprety = ecsqlValue.GetColumnInfo().GetProperty() ? ecsqlValue.GetColumnInfo().GetProperty()->GetAsPrimitiveProperty() : nullptr;
            const auto isId = primitiveProprety ? primitiveProprety->GetExtendedTypeName().EqualsIAscii("Id") : false;
            if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::Custom || isId )
                {
                ECJsonUtilities::Int64ToJson(jsonValue, ecsqlValue.GetInt64(), formatOptions.GetInt64Format());
                }
            else
                {
               jsonValue = std::trunc(ecsqlValue.GetDouble());
                }
            return SUCCESS;
            }
            case PRIMITIVETYPE_Point2d:
            {
            return ECJsonUtilities::Point2dToJson(jsonValue, ecsqlValue.GetPoint2d());
            }

            case PRIMITIVETYPE_Point3d:
            {
            return ECJsonUtilities::Point3dToJson(jsonValue, ecsqlValue.GetPoint3d());
            }

            case PRIMITIVETYPE_String:
            {
            jsonValue =  ecsqlValue.GetText();
            return SUCCESS;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = ecsqlValue.GetGeometry();
            if (geom == nullptr)
                return ERROR;

            if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::IModelJs)
                {
                return ECJsonUtilities::IGeometryToIModelJson(jsonValue, *geom);
                }
            else
                {
                Json::Value tmp;
                auto stat =  ECJsonUtilities::IGeometryToJson(tmp, *geom);
                jsonValue.From(tmp);
                return stat;
                }
            }
            default:
                BeAssert(false && "Unknown type");
                return ERROR;
        }
    }


//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSystemPropertyInfo const& AdapterHelper::DetermineTopLevelSystemPropertyInfo(SchemaManager const& schemaManager, ECSqlColumnInfoCR colInfo, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    if (colInfo.IsGeneratedProperty() && !colInfo.IsSystemProperty())
        return ECSqlSystemPropertyInfo::NoSystemProperty();
    if (colInfo.IsGeneratedProperty() && formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::IModelJs)
        {
        T_Utf8StringVector accessStringV;
        Utf8String displayLabel = colInfo.GetProperty()->GetDisplayLabel();
        BeStringUtilities::Split(displayLabel.ToLower().c_str(), ".", accessStringV);
        auto &leafEntry = accessStringV.back();
        if (leafEntry == ("id"))
            return ECSqlSystemPropertyInfo::NavigationId();
        else if (leafEntry == ("relecclassid"))
            return ECSqlSystemPropertyInfo::NavigationRelECClassId();
        else if (leafEntry == ("ecinstanceid"))
            return ECSqlSystemPropertyInfo::ECInstanceId();
        else if (leafEntry == ("ecclassid"))
            return ECSqlSystemPropertyInfo::ECClassId();
        else if (leafEntry == ("sourceecinstanceid"))
            return ECSqlSystemPropertyInfo::SourceECInstanceId();
        else if (leafEntry == ("sourceecclassid"))
            return ECSqlSystemPropertyInfo::SourceECClassId();
        else if (leafEntry == ("targetecinstanceid"))
            return ECSqlSystemPropertyInfo::TargetECInstanceId();
        else if (leafEntry == ("targetecclassid"))
            return ECSqlSystemPropertyInfo::TargetECClassId();
        else
            return ECSqlSystemPropertyInfo::NoSystemProperty();
        }
    BeAssert(colInfo.GetProperty() != nullptr && "Must be checked before");
    ECSqlSystemPropertyInfo const& sysPropInfo = schemaManager.Main().GetSystemSchemaHelper().GetSystemPropertyInfo(*colInfo.GetProperty());
    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class || sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Relationship || sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Navigation)
        return sysPropInfo;

    return ECSqlSystemPropertyInfo::NoSystemProperty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void JsonECSqlSelectAdapter::FormatOptions::SetRowFormat(RowFormat fmt)
    {
    m_rowFormat = fmt;
    if (fmt == RowFormat::IModelJs)
        {
        m_int64Format = ECN::ECJsonInt64Format::AsHexadecimalString;
        m_memberNameCasing = MemberNameCasing::LowerFirstChar;
        m_blobMode = BlobMode::Base64String;
        m_base64Header = "encoding=base64;";
        }
    else
        {
        m_base64Header.clear();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String AdapterHelper::MemberNameFromSelectClauseItemIModelJs(ECDbSystemSchemaHelper const& systemSchemaHelper, ECSqlColumnInfo const& colInfo, ECSqlSystemPropertyInfo const& sysPropInfo, JsonECSqlSelectAdapter::MemberNameCasing casing)
    {
    // This is based on E:\BSW\temp\imodeljs\core\common\src\ECSqlTypes.ts
    T_Utf8StringVector accessStringV;
    if (colInfo.IsGeneratedProperty())
        BeStringUtilities::Split(colInfo.GetProperty()->GetDisplayLabel().c_str(), ".", accessStringV);
    else
        {
        for (auto const* it : colInfo.GetPropertyPath())
            accessStringV.push_back(it->GetProperty()->GetName().c_str());
        }

    if (accessStringV.size() == 1)
        {
        if (colInfo.IsGeneratedProperty())
            {
            Utf8String name(colInfo.GetProperty()->GetDisplayLabel());
            BeAssert(!name.empty());
            FormatMemberName(name, casing);
            return name;
            }

        return MemberNameFromSelectClauseItemCustom(systemSchemaHelper, colInfo, sysPropInfo, casing);
        }
    else
        {
        FormatMemberName(accessStringV.front(), casing);
        Utf8String tmp = accessStringV.front() + ".";
        for (int i = 1; i < accessStringV.size() - 1; ++i)
            tmp += accessStringV[i] + ".";

        auto &leafEntry = accessStringV.back();
        if (leafEntry == "Id")
            tmp += ECJsonSystemNames::Id();
        else if (leafEntry == "RelECClassId")
            tmp += ECJsonSystemNames::Navigation::RelClassName();
        else if (leafEntry == "X")
            tmp += ECJsonSystemNames::Point::X();
        else if (leafEntry == "Y")
            tmp += ECJsonSystemNames::Point::Y();
        else if (leafEntry == "Z")
            tmp += ECJsonSystemNames::Point::Z();
        else
            tmp += leafEntry;

        return tmp;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String AdapterHelper::MemberNameFromSelectClauseItemCustom(ECDbSystemSchemaHelper const& systemSchemaHelper, ECSqlColumnInfo const& colInfo, ECSqlSystemPropertyInfo const& sysPropInfo, JsonECSqlSelectAdapter::MemberNameCasing casing)
    {
    //if property is generated, the display label contains the select clause item as is.
    //The property name in contrast would have encoded special characters of the select clause item.
    //Ex: SELECT MyProp + 4 FROM Foo -> the member name in JSON must be "MyProp + 4"
    if (colInfo.IsGeneratedProperty() && !colInfo.IsSystemProperty())
        {
        Utf8String name(colInfo.GetProperty()->GetDisplayLabel());
        if (name.empty())
            name.assign(colInfo.GetProperty()->GetName());

        FormatMemberName(name, casing);
        return name;
        }
    if (!sysPropInfo.IsSystemProperty())
        {
        Utf8String name(colInfo.GetPropertyPath().ToString());
        BeAssert(!name.empty());
        FormatMemberName(name, casing);
        return name;
        }

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class)
        {
        switch (sysPropInfo.GetClass())
            {
            case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                return ECJsonSystemNames::Id();
            case ECSqlSystemPropertyInfo::Class::ECClassId:
                return ECJsonSystemNames::ClassName();
            default:
                BeAssert(false);
                return Utf8String();
            }
        }

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Navigation)
        {
        switch (sysPropInfo.GetNavigation())
            {
            case ECSqlSystemPropertyInfo::Navigation::Id:
                return ECJsonSystemNames::Navigation::Id();
            case ECSqlSystemPropertyInfo::Navigation::RelECClassId:
                return ECJsonSystemNames::Navigation::RelClassName();
            default:
                BeAssert(false);
                return Utf8String();
            }
        }
    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Relationship)
        {
        switch (sysPropInfo.GetRelationship())
            {
            case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                return ECJsonSystemNames::SourceId();
            case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                return ECJsonSystemNames::SourceClassName();
            case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                return ECJsonSystemNames::TargetId();
            case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                return ECJsonSystemNames::TargetClassName();
            default:
                BeAssert(false);
                return Utf8String();
            }
        }

    BeAssert(false && "Other system properties should not show up in ECSQL select clause");
    return Utf8String();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool AdapterHelper::NeedsMemberNameReformatting(JsonECSqlSelectAdapter::MemberNameCasing casingMode)
    {
    switch (casingMode)
        {
            case JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal:
                return false;

            case JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar:
                return true;

            default:
                BeAssert(false);
                return false;
        }
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void AdapterHelper::FormatMemberName(Utf8String& memberName, JsonECSqlSelectAdapter::MemberNameCasing casingMode)
    {
    switch (casingMode)
        {
            case JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal:
                return;

            case JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar:
                ECJsonUtilities::LowerFirstChar(memberName);
                return;

            default:
                BeAssert(false);
                return;
        }
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

