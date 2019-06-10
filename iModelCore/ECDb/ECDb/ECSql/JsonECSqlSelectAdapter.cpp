/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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
// @bsiclass                                               Krischan.Eberle      11/2018
//+===============+===============+===============+===============+===============+======
struct JsonRef final
    {
public:
    enum class Type
        {
        JsonCpp,
        RapidJson
        };
private:
    Json::Value* m_jsonCpp = nullptr;
    rapidjson::Value* m_rapidJson = nullptr;
    rapidjson::MemoryPoolAllocator<>* m_allocator = nullptr;

public:
    JsonRef() {}
    explicit JsonRef(JsonValueR json): m_jsonCpp(&json) {}
    JsonRef(RapidJsonValueR json, rapidjson::MemoryPoolAllocator<>& allocator) : m_rapidJson(&json), m_allocator(&allocator) {}

    bool IsValid() const { return m_jsonCpp != nullptr || m_rapidJson != nullptr;  }
    Type GetType() const { return IsRapidJson() ? Type::RapidJson : Type::JsonCpp; }
    bool IsRapidJson() const { BeAssert(IsValid()); return m_rapidJson != nullptr; }

    JsonValueR JsonCpp() const { BeAssert(!IsRapidJson()); return *m_jsonCpp; }
    RapidJsonValueR RapidJson() const { BeAssert(IsRapidJson()); return *m_rapidJson; }
    rapidjson::MemoryPoolAllocator<>& Allocator() const { BeAssert(IsRapidJson()); return *m_allocator; }

    bool IsNull() const { return IsRapidJson() ? RapidJson().IsNull() : JsonCpp().isNull(); }
    bool IsObject() const { return IsRapidJson() ? RapidJson().IsObject() : JsonCpp().isObject(); }
    void SetObject()
        { 
        if (IsRapidJson())
            RapidJson().SetObject();
        else
            JsonCpp() = Json::Value(Json::objectValue);
        }

    void SetArray()
        {
        if (IsRapidJson())
            RapidJson().SetArray();
        else
            JsonCpp() = Json::Value(Json::arrayValue);
        }

    // If a member with that name already exists, it will be returned.
    // Otherwise a new member will be created and returned.
    // @param makeCopy if true, a copy of @p memberName will be added to the JSON. If false, a reference to @p memberName 
    // will used in the JSON -> caller must make sure the lifetime of @p memberName is longer than the JSON.
    JsonRef GetOrAddMember(Utf8CP memberName, bool makeCopy)
        {
        if (IsRapidJson())
            {
            // reuse an already existing member
            auto it = RapidJson().FindMember(memberName);
            if (it != RapidJson().MemberEnd())
                return JsonRef(it->value, Allocator());

            rapidjson::Value memberNameVal = makeCopy ? rapidjson::Value(memberName, Allocator()) : rapidjson::Value(rapidjson::StringRef(memberName));
            RapidJson().AddMember(memberNameVal.Move(), rapidjson::Value().Move(), Allocator());
            return JsonRef(RapidJson()[memberName], Allocator());
            }

        if (makeCopy)
            return JsonRef(JsonCpp()[memberName]);

        return JsonRef(JsonCpp()[Json::StaticString(memberName)]);
        }

    JsonRef Pushback()
        {
        if (IsRapidJson())
            {
            RapidJson().PushBack(rapidjson::Value().Move(), Allocator());
            return JsonRef(RapidJson()[RapidJson().Size() - 1], Allocator());
            }

        return JsonRef(JsonCpp().append(Json::Value()));
        }

    BentleyStatus FromId(BeInt64Id id) { return IsRapidJson() ? ECJsonUtilities::IdToJson(RapidJson(), id, Allocator()) : ECJsonUtilities::IdToJson(JsonCpp(), id); }

    void FromClass(ECN::ECClassCR ecClass)
        {
        if (IsRapidJson())
            ECJsonUtilities::ClassToJson(RapidJson(), ecClass, Allocator());
        else
            ECJsonUtilities::ClassNameToJson(JsonCpp(), ecClass);
        }

    };

//=======================================================================================
//! Helper class for the JsonECSqlSelectAdapter
// @bsiclass                                                Ramanujam.Raman      10/2012
//+===============+===============+===============+===============+===============+======
struct AdapterHelper final
    {
    private:
        //static
        AdapterHelper() = delete;
        ~AdapterHelper() = delete;

        static BentleyStatus DetermineMemberNames(bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus DetermineUniqueMemberNames(bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&);

        static BentleyStatus SelectClauseItemToJson(JsonRef&, IECSqlValue const&, ECSqlSystemPropertyInfo const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus PropertyValueToJson(JsonRef&, IECSqlValue const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus NavigationToJson(JsonRef&, IECSqlValue const&, SchemaManager const&);
        static BentleyStatus StructToJson(JsonRef&, IECSqlValue const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus PrimitiveArrayToJson(JsonRef&, IECSqlValue const&, ECN::PrimitiveType, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus StructArrayToJson(JsonRef&, IECSqlValue const&, SchemaManager const&, JsonECSqlSelectAdapter::FormatOptions const&);
        static BentleyStatus PrimitiveToJson(JsonRef&, IECSqlValue const&, ECN::PrimitiveType, JsonECSqlSelectAdapter::FormatOptions const&);

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
        static BentleyStatus GetRow(JsonRef&, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, bool appendToRow, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&, bool copyMemberNames);
        static BentleyStatus GetRowInstance(JsonRef&, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>&, ECN::ECClassId, ECSqlStatement const&, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const&);
    };

//--------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRow(JsonValueR rowJson, bool appendToJson) const
    {
    JsonRef json(rowJson);
    return AdapterHelper::GetRow(json, m_cacheImpl->m_uniqueMemberNames, appendToJson, m_ecsqlStatement, m_ecsqlHash, m_formatOptions, m_copyMemberNames);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JsonECSqlSelectAdapter::GetRow(JsonValueR rowJson, ECSqlStatement const& stmt, bool appendToJson) const
    {
    JsonRef json(rowJson);
    return AdapterHelper::GetRow(json, m_cacheImpl->m_uniqueMemberNames, appendToJson, stmt, stmt.GetHashCode(), m_formatOptions, m_copyMemberNames);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                11/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRow(RapidJsonValueR rowJson, rapidjson::MemoryPoolAllocator<>& allocator, bool appendToJson) const
    {
    JsonRef json(rowJson, allocator);
    return AdapterHelper::GetRow(json, m_cacheImpl->m_uniqueMemberNames, appendToJson, m_ecsqlStatement, m_ecsqlHash, m_formatOptions, m_copyMemberNames);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRowInstance(JsonValueR rowJson, ECClassId classId) const
    {
    JsonRef json(rowJson);
    return AdapterHelper::GetRowInstance(json, m_cacheImpl->m_memberNames, classId, m_ecsqlStatement, m_ecsqlHash, m_formatOptions);
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                11/2018
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus JsonECSqlSelectAdapter::GetRowInstance(RapidJsonValueR rowJson, ECClassId classId, rapidjson::MemoryPoolAllocator<>& allocator) const
    {
    JsonRef json(rowJson, allocator);
    return AdapterHelper::GetRowInstance(json, m_cacheImpl->m_memberNames, classId, m_ecsqlStatement, m_ecsqlHash, m_formatOptions);
    }

//******************************************************************************************
// AdapterHelper
//******************************************************************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                 09/2017
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
// @bsimethod                                 Krischan.Eberle                 11/2018
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus AdapterHelper::GetRow(JsonRef& rowJson, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& uniqueMemberNames, bool appendToJson, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions, bool copyMemberNames)
    {
    if (SUCCESS != DetermineUniqueMemberNames(uniqueMemberNames, stmt, ecsqlHash, formatOptions))
        return ERROR;
   
    if (appendToJson)
        {
        if (rowJson.IsNull() || !rowJson.IsObject())
            return ERROR;
        }
    else
        rowJson.SetObject();

    const int count = stmt.GetColumnCount();
    BeAssert(count == (int) uniqueMemberNames.size());
    SchemaManager const& schemaManager = stmt.GetECDb()->Schemas();
    for (int columnIndex = 0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = stmt.GetValue(columnIndex);
        if (ecsqlValue.IsNull())
            continue;

        auto const& member = uniqueMemberNames[(size_t) columnIndex];
        Utf8StringCR memberName = member.first;
        JsonRef memberJson = rowJson.GetOrAddMember(memberName.c_str(), copyMemberNames);
        ECSqlSystemPropertyInfo const& sysPropInfo = member.second;
        if (SUCCESS != SelectClauseItemToJson(memberJson, ecsqlValue, sysPropInfo, schemaManager, formatOptions))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::GetRowInstance(JsonRef& rowJson, bvector<bpair<Utf8String, ECSqlSystemPropertyInfo>>& memberNames, ECClassId classId, ECSqlStatement const& stmt, uint64_t ecsqlHash, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    if (!classId.IsValid())
        return ERROR;

    if (SUCCESS != DetermineMemberNames(memberNames, stmt, ecsqlHash, formatOptions))
        return ERROR;

    rowJson.SetObject();

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
        JsonRef memberJson = rowJson.GetOrAddMember(memberName.c_str(), true);
        if (SUCCESS != SelectClauseItemToJson(memberJson, ecsqlValue, sysPropInfo, schemaManager, formatOptions))
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
// @bsimethod                                    Krischan.Eberle            11/2018
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
// @bsimethod                                    Krischan.Eberle            11/2018
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
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::SelectClauseItemToJson(JsonRef& json, IECSqlValue const& ecsqlValue, ECSqlSystemPropertyInfo const& sysPropInfo, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    BeAssert(!ecsqlValue.IsNull() && "Should have been caught before");

    if (!sysPropInfo.IsSystemProperty())
        return PropertyValueToJson(json, ecsqlValue, schemaManager, formatOptions);
    
    BeAssert(ecsqlValue.GetId<BeInt64Id>().IsValid());

    if (sysPropInfo.GetType() == ECSqlSystemPropertyInfo::Type::Class)
        {
        switch (sysPropInfo.GetClass())
            {
                case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                    return json.FromId(ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Class::ECClassId:
                {
                ECClassCP cls = schemaManager.GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
                if (cls == nullptr)
                    return ERROR;

                json.FromClass(*cls);
                return SUCCESS;
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
                    return json.FromId(ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Navigation::RelECClassId:
                {
                ECClassCP cls = schemaManager.GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
                if (cls == nullptr)
                    return ERROR;

                json.FromClass(*cls);
                return SUCCESS;
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
                    return json.FromId(ecsqlValue.GetId<BeInt64Id>());

                case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                {
                ECClassCP cls = schemaManager.GetClass(ecsqlValue.GetId<ECClassId>(), ecsqlValue.GetColumnInfo().GetRootClass().GetTableSpace().c_str());
                if (cls == nullptr)
                    return ERROR;

                json.FromClass(*cls);
                return SUCCESS;
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
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::PropertyValueToJson(JsonRef& jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    BeAssert(!ecsqlValue.IsNull() && "Should have been caught before to avoid that a null JSON value is created");

    ECN::ECPropertyCP prop = ecsqlValue.GetColumnInfo().GetProperty();
    BeAssert(prop != nullptr);

    if (prop->GetIsPrimitive())
        return PrimitiveToJson(jsonValue, ecsqlValue, prop->GetAsPrimitiveProperty()->GetType(), formatOptions);

    if (prop->GetIsStruct())
        return StructToJson(jsonValue, ecsqlValue, schemaManager, formatOptions);

    if (prop->GetIsNavigation())
        return NavigationToJson(jsonValue, ecsqlValue, schemaManager);

    if (prop->GetIsPrimitiveArray())
        return PrimitiveArrayToJson(jsonValue, ecsqlValue, prop->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType(), formatOptions);

    if (prop->GetIsStructArray())
        return StructArrayToJson(jsonValue, ecsqlValue, schemaManager, formatOptions);

    BeAssert(false && "Unhandled ECProperty type. Adjust the code for this new ECProperty type");
    return ERROR;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::PrimitiveArrayToJson(JsonRef& jsonValue, IECSqlValue const& ecsqlValue, PrimitiveType arrayElementType, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    jsonValue.SetArray();
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        JsonRef arrayElementJson = jsonValue.Pushback();
        if (SUCCESS != PrimitiveToJson(arrayElementJson, arrayElementValue, arrayElementType, formatOptions))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::StructArrayToJson(JsonRef& jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    jsonValue.SetArray();
    for (IECSqlValue const& arrayElementValue : ecsqlValue.GetArrayIterable())
        {
        if (arrayElementValue.IsNull())
            continue;

        JsonRef arrayElementJson = jsonValue.Pushback();
        if (SUCCESS != StructToJson(arrayElementJson, arrayElementValue, schemaManager, formatOptions))
            return ERROR;
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle               06/2017
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::NavigationToJson(JsonRef& jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager)
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

    jsonValue.SetObject();
    JsonRef navIdJson = jsonValue.GetOrAddMember(ECJsonSystemNames::Navigation::Id(), false);
    if (SUCCESS != navIdJson.FromId(navId))
        return ERROR;

    if (relClass == nullptr)
        return SUCCESS;

    JsonRef relClassNameJson = jsonValue.GetOrAddMember(ECJsonSystemNames::Navigation::RelClassName(), false);
    relClassNameJson.FromClass(*relClass);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::StructToJson(JsonRef& jsonValue, IECSqlValue const& ecsqlValue, SchemaManager const& schemaManager, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    jsonValue.SetObject();
    for (IECSqlValue const& structMemberValue : ecsqlValue.GetStructIterable())
        {
        if (structMemberValue.IsNull())
            continue;

        ECPropertyCP memberProp = structMemberValue.GetColumnInfo().GetProperty();
        JsonRef memberJson;
        if (NeedsMemberNameReformatting(formatOptions.GetMemberCasingMode()))
            {
            Utf8String memberPropName(memberProp->GetName());
            FormatMemberName(memberPropName, formatOptions.GetMemberCasingMode());
            memberJson = jsonValue.GetOrAddMember(memberPropName.c_str(), true);
            }
        else
            memberJson = jsonValue.GetOrAddMember(memberProp->GetName().c_str(), true);

        BeAssert(memberJson.IsValid());

        if (SUCCESS != PropertyValueToJson(memberJson, structMemberValue, schemaManager, formatOptions))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2012
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus AdapterHelper::PrimitiveToJson(JsonRef& jsonValue, IECSqlValue const& ecsqlValue, ECN::PrimitiveType primType, JsonECSqlSelectAdapter::FormatOptions const& formatOptions)
    {
    BeAssert(!ecsqlValue.IsNull());
    switch (primType)
        {
            case PRIMITIVETYPE_Binary:
            {
            int size = 0;
            Byte const* data = (Byte const*) ecsqlValue.GetBlob(&size);
            if (jsonValue.IsRapidJson())
                {
                if (formatOptions.GetBlobMode() == JsonECSqlSelectAdapter::BlobMode::Base64String)
                    return ECJsonUtilities::BinaryToJson(jsonValue.RapidJson(), data, (size_t) size, jsonValue.Allocator(), formatOptions.GetBase64Header().c_str());
                
                
                // render as int[]
                jsonValue.SetArray();
                for (int i = 0; i < size; i++)
                    jsonValue.RapidJson().PushBack(data[i], jsonValue.Allocator());
                return BentleyStatus::SUCCESS;
                }


            if (formatOptions.GetBlobMode() == JsonECSqlSelectAdapter::BlobMode::Base64String)
                return ECJsonUtilities::BinaryToJson(jsonValue.JsonCpp(), data, (size_t) size, formatOptions.GetBase64Header().c_str());

            // render as int[]
            jsonValue.SetArray();
            for (Json::ArrayIndex i = 0; i < (Json::ArrayIndex)size; i++)
                jsonValue.JsonCpp()[i] = data[i];
            return BentleyStatus::SUCCESS;
            }
            case PRIMITIVETYPE_Boolean:
            {
            const bool val = ecsqlValue.GetBoolean();
            if (jsonValue.IsRapidJson())
                jsonValue.RapidJson().SetBool(val);
            else
                jsonValue.JsonCpp() = val;

            return SUCCESS;
            }

            case PRIMITIVETYPE_DateTime:
            {
            DateTime dt = ecsqlValue.GetDateTime();
            if (jsonValue.IsRapidJson())
                ECJsonUtilities::DateTimeToJson(jsonValue.RapidJson(), dt, jsonValue.Allocator());
            else
                ECJsonUtilities::DateTimeToJson(jsonValue.JsonCpp(), dt);

            return SUCCESS;
            }
            case PRIMITIVETYPE_Double:
            {
            const double val = ecsqlValue.GetDouble();
            if (jsonValue.IsRapidJson())
                jsonValue.RapidJson().SetDouble(val);
            else
                jsonValue.JsonCpp() = val;

            return SUCCESS;
            }
            case PRIMITIVETYPE_Integer:
            {
            if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::Custom)
                {
                const int val = ecsqlValue.GetInt();
                if (jsonValue.IsRapidJson())
                    jsonValue.RapidJson().SetInt(val);
                else
                    jsonValue.JsonCpp() = val;
                }
            else
                {
                const double val = std::trunc(ecsqlValue.GetDouble());
                if (jsonValue.IsRapidJson())
                    jsonValue.RapidJson().SetDouble(val);
                else
                    jsonValue.JsonCpp() = val;
                }
            return SUCCESS;
            }
            case PRIMITIVETYPE_Long:
            {            
            const auto isId = ecsqlValue.GetColumnInfo().GetProperty()->GetAsPrimitiveProperty()->GetExtendedTypeName().EqualsIAscii("Id");
            if (formatOptions.GetRowFormat() == JsonECSqlSelectAdapter::RowFormat::Custom || isId )
                {
                const int64_t val = ecsqlValue.GetInt64();
                if (jsonValue.IsRapidJson())
                    ECJsonUtilities::Int64ToJson(jsonValue.RapidJson(), val, jsonValue.Allocator(), formatOptions.GetInt64Format());
                else
                    ECJsonUtilities::Int64ToJson(jsonValue.JsonCpp(), val, formatOptions.GetInt64Format());
                }
            else
                {
                const double val =std::trunc(ecsqlValue.GetDouble());
                if (jsonValue.IsRapidJson())
                    jsonValue.RapidJson().SetDouble(val);
                else
                    jsonValue.JsonCpp() = val;
                }
            return SUCCESS;
            }
            case PRIMITIVETYPE_Point2d:
            {                
            const DPoint2d val = ecsqlValue.GetPoint2d();
            if (jsonValue.IsRapidJson())
                return ECJsonUtilities::Point2dToJson(jsonValue.RapidJson(), val, jsonValue.Allocator());

            return ECJsonUtilities::Point2dToJson(jsonValue.JsonCpp(), val);
            }

            case PRIMITIVETYPE_Point3d:
            {
            const DPoint3d val = ecsqlValue.GetPoint3d();
            if (jsonValue.IsRapidJson())
                return ECJsonUtilities::Point3dToJson(jsonValue.RapidJson(), val, jsonValue.Allocator());

            return ECJsonUtilities::Point3dToJson(jsonValue.JsonCpp(), val);
            }

            case PRIMITIVETYPE_String:
            {
            Utf8CP val = ecsqlValue.GetText();
            if (jsonValue.IsRapidJson())
                jsonValue.RapidJson().SetString(val, jsonValue.Allocator()); // copy the string into the json as the original string gets released after the next Step().
            else
                jsonValue.JsonCpp() = val;

            return SUCCESS;
            }

            case PRIMITIVETYPE_IGeometry:
            {
            IGeometryPtr geom = ecsqlValue.GetGeometry();
            if (geom == nullptr)
                return ERROR;

            if (jsonValue.IsRapidJson())
                return ECJsonUtilities::IGeometryToJson(jsonValue.RapidJson(), *geom, jsonValue.Allocator());

            return ECJsonUtilities::IGeometryToJson(jsonValue.JsonCpp(), *geom);
            }
            default:
                BeAssert(false && "Unknown type");
                return ERROR;
        }
    }


//---------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static
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
// @bsimethod                                    Affan.Khan                 06/2019
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
// @bsimethod                                    Affan.Khan                 06/2019
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String AdapterHelper::MemberNameFromSelectClauseItemIModelJs(ECDbSystemSchemaHelper const& systemSchemaHelper, ECSqlColumnInfo const& colInfo, ECSqlSystemPropertyInfo const& sysPropInfo, JsonECSqlSelectAdapter::MemberNameCasing casing)
    {
    // This is based on E:\BSW\temp\imodeljs\core\common\src\ECSqlTypes.ts
    T_Utf8StringVector accessStringV;
    if (colInfo.IsGeneratedProperty())
        BeStringUtilities::Split(colInfo.GetProperty()->GetDisplayLabel().c_str(), ".", accessStringV);
    else
        {
        for (auto const& it : colInfo.GetPropertyPath())
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
// @bsimethod                                    Krischan.Eberle                 09/2017
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
// @bsimethod                                    Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
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
// @bsimethod                                    Krischan.Eberle                 09/2017
//+---------------+---------------+---------------+---------------+---------------+------
//static 
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

