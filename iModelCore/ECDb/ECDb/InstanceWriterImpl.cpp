/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/InstanceWriter.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <GeomSerialization/GeomLibsSerialization.h>
// #include <Napi/napi.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define FOREACH_CONTINUE false;
#define FOREACH_ABORT true;

using Impl = InstanceWriter::Impl;
using MruStatementCache = Impl::MruStatementCache;
using CachedWriteStatement = MruStatementCache::CachedWriteStatement;
using BindContext = Impl::BindContext;
using CachedBinder = MruStatementCache::CachedBinder;

//******************************BindContext**************************************
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void BindContext::SetError(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    m_error.VSprintf(fmt, args);
    va_end(args);
}

void BindContext::PrependError(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    Utf8String additionalError;
    additionalError.VSprintf(fmt, args);
    va_end(args);

    if (!m_error.empty())
        additionalError.append(" ");

    m_error.insert(0, additionalError);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus BindContext::NotifyUserProperty(Utf8CP name, BeJsConst val, CachedWriteStatement& stmt) const {
    auto handler = m_options.GetUserPropertyHandler();
    ECSqlStatus rc;
    if (handler != nullptr) {
        auto status =  handler(name, val, [&](Utf8CP name) -> std::optional<PropertyBinder> {
            auto prop = stmt.FindBinder(name);
            if (prop != nullptr) {
                return PropertyBinder(prop->GetProperty(), prop->GetBinder());
            }
            return std::nullopt; }, rc);
        if (status == PropertyHandlerResult::Handled) { return rc; }
    }
    return ECSqlStatus::Success;
}
//******************************CachedWriteStatement**************************************
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void CachedWriteStatement::BuildPropertyIndexMap(bool addUseJsNameMap) {
    m_propertyIndexMap.clear();
    for (auto it = m_propertyBinders.begin(); it != m_propertyBinders.end(); ++it) {
        auto& prop = it->GetPropertyMap();
        m_propertyIndexMap[prop.GetName()] = &(*it);

        if (!addUseJsNameMap)
            continue;

        if (prop.GetType() == PropertyMap::Type::ECInstanceId) {
            m_propertyIndexMap[ECJsonSystemNames::Id()] = &(*it);
        } else if (prop.GetType() == PropertyMap::Type::ECClassId) {
            m_propertyIndexMap[ECJsonSystemNames::ClassName()] = &(*it);
            m_propertyIndexMap[ECJsonSystemNames::ClassFullName()] = &(*it);
        } else if (prop.GetType() == PropertyMap::Type::ConstraintECClassId) {
            if (prop.GetName().EqualsIAscii(ECDBSYS_PROP_SourceECClassId)) {
                m_propertyIndexMap[ECJsonSystemNames::SourceClassName()] = &(*it);
            } else if (prop.GetName().EqualsIAscii(ECDBSYS_PROP_TargetECClassId)) {
                m_propertyIndexMap[ECJsonSystemNames::TargetClassName()] = &(*it);
            }
        } else if (prop.GetType() == PropertyMap::Type::ConstraintECInstanceId) {
            if (prop.GetName().Equals(ECDBSYS_PROP_SourceECInstanceId)) {
                m_propertyIndexMap[ECJsonSystemNames::SourceId()] = &(*it);
            } else if (prop.GetName().Equals(ECDBSYS_PROP_TargetECInstanceId)) {
                m_propertyIndexMap[ECJsonSystemNames::TargetId()] = &(*it);
            }
        } else {
            Utf8String str = prop.GetName();
            str[0] = (Utf8Char)tolower(str[0]);
            m_propertyIndexMap[str] = &(*it);
        }
    }
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
Utf8String CachedWriteStatement::GetCurrentTimeStampProperty() const {
    BeAssert(m_classMap != nullptr);
    auto ca = m_classMap->GetClass().GetCustomAttribute("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
    if (ca == nullptr)
        return "";

    ECValue v;
    if (ECObjectsStatus::Success != ca->GetValue(v, "PropertyName")) {
        return "";
    }
    return v.GetUtf8CP();
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
const CachedBinder* CachedWriteStatement::FindBinder(Utf8StringCR name) const {
    if (!m_propertyIndexMap.empty()) {
        auto it = m_propertyIndexMap.find(name);
        if (it != m_propertyIndexMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    for (auto& binder : m_propertyBinders) {
        if (binder.GetProperty().GetName().EqualsIAscii(name)) {
            return &binder;
        }
    }
    return nullptr;
}
//******************************MruStatementCache**************************************
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus MruStatementCache::PrepareInsert(CachedWriteStatement& cachedStmt) {
    Utf8String ecsql;
    std::vector<PropertyMap const*> props;

    auto& classMap = cachedStmt.GetClassMap();
    ecsql.append("INSERT INTO ").append(cachedStmt.GetClass().GetECSqlName()).append(" (");
    bool isFirst = true;

    const auto timestampPropName = cachedStmt.GetCurrentTimeStampProperty();
    auto& maps = classMap.GetPropertyMaps();
    for (auto it = maps.begin(); it != maps.end(); ++it) {
        auto prop = *it;
        if (prop->GetType() == PropertyMap::Type::ECClassId) {
            continue;
        }

        if (timestampPropName.EqualsIAscii(prop->GetName())) {
            continue;
        }

        if (!isFirst) {
            ecsql.append(", ");
        } else {
            isFirst = false;
        }
        ecsql.append("[").append(prop->GetName()).append("]");
        props.push_back(prop);
        if (prop->GetType() == PropertyMap::Type::ECInstanceId) {
            cachedStmt.m_instanceIdIndex = (int)props.size();
        }
    }

    ecsql.append(") VALUES (");

    for (auto it = props.begin(); it != props.end(); ++it) {
        if (it != props.begin()) {
            ecsql.append(", ");
        }
        ecsql.append("?");
    }
    ecsql.append(")");

    auto crudWriteToken = m_ecdb.GetImpl().GetSettingsManager().GetCrudWriteToken();
    auto rc = cachedStmt.GetStatement().Prepare(m_ecdb, ecsql.c_str(), crudWriteToken);
    if (!rc.IsSuccess()) {
        return rc;
    }

    int i = 1;
    for (auto prop : props) {
        cachedStmt.GetBinders().emplace_back(*prop, cachedStmt.GetStatement().GetBinder(i++));
    }
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus MruStatementCache::PrepareUpdate(CachedWriteStatement& cachedStmt) {
    Utf8String ecsql;
    std::vector<PropertyMap const*> props;

    const auto timestampPropName = cachedStmt.GetCurrentTimeStampProperty();
    auto& classMap = cachedStmt.GetClassMap();
    ecsql.append("UPDATE ").append(cachedStmt.GetClass().GetECSqlName()).append(" SET ");
    bool isFirst = true;
    for (auto prop : classMap.GetPropertyMaps()) {
        if (prop->GetType() == PropertyMap::Type::ECClassId || prop->GetType() == PropertyMap::Type::ECInstanceId) {
            continue;
        }
        if (prop->GetProperty().GetIsReadOnly()) {
            continue;
        }
        if (timestampPropName.EqualsIAscii(prop->GetName())) {
            continue;
        }
        if (!isFirst) {
            ecsql.append(", ");
        } else {
            isFirst = false;
        }
        ecsql.append("[").append(prop->GetName()).append("] = ?");
        props.push_back(prop);
    }

    ecsql.append(" WHERE [ECInstanceId] = ?");

    auto crudWriteToken = m_ecdb.GetImpl().GetSettingsManager().GetCrudWriteToken();
    auto rc = cachedStmt.GetStatement().Prepare(m_ecdb, ecsql.c_str(), crudWriteToken);
    if (!rc.IsSuccess()) {
        return rc;
    }

    int i = 1;
    for (auto prop : props) {
        cachedStmt.GetBinders().emplace_back(*prop, cachedStmt.GetStatement().GetBinder(i++));
    }

    cachedStmt.m_instanceIdIndex = i;
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus MruStatementCache::PrepareDelete(CachedWriteStatement& cachedStmt) {
    Utf8String ecsql;
    ecsql.append("DELETE FROM ").append(cachedStmt.GetClass().GetECSqlName()).append(" WHERE [ECInstanceId] = ?");

    auto crudWriteToken = m_ecdb.GetImpl().GetSettingsManager().GetCrudWriteToken();
    auto rc = cachedStmt.GetStatement().Prepare(m_ecdb, ecsql.c_str(), crudWriteToken);
    if (!rc.IsSuccess()) {
        return rc;
    }
    cachedStmt.m_instanceIdIndex = 1;
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
std::unique_ptr<CachedWriteStatement> MruStatementCache::Prepare(CacheKey key) {
    auto cls = m_ecdb.Schemas().GetClass(key.GetClassId());
    if (cls == nullptr) {
        return nullptr;
    }
    auto classMap = m_ecdb.Schemas().Main().GetClassMap(*cls);
    if (classMap == nullptr) {
        return nullptr;
    }

    auto cachedStmt = std::make_unique<CachedWriteStatement>(*classMap);
    auto rc = ECSqlStatus::Success;
    if (key.GetOp() == WriterOp::Insert) {
        rc = PrepareInsert(*cachedStmt);
    } else if (key.GetOp() == WriterOp::Update) {
        rc = PrepareUpdate(*cachedStmt);
    } else if (key.GetOp() == WriterOp::Delete) {
        rc = PrepareDelete(*cachedStmt);
    };

    if (!rc.IsSuccess()) {
        return nullptr;
    }
    cachedStmt->BuildPropertyIndexMap(m_addSupportForJsName);
    return cachedStmt;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
CachedWriteStatement* MruStatementCache::TryGet(CacheKey key) {
    auto it = m_cache.find(key);
    if (it == m_cache.end()) {

        auto cachedStmt = Prepare(key);
        if (cachedStmt == nullptr)
            return nullptr;

        it = m_cache.insert({key, std::move(cachedStmt)}).first;
        m_mru.push_back(key);
    } else {
        m_mru.erase(std::remove(m_mru.begin(), m_mru.end(), key), m_mru.end());
        m_mru.push_back(key);
    }

    while (m_cache.size() > (size_t)m_maxCache) {
        m_cache.erase(m_mru.front());
        m_mru.erase(m_mru.begin());
    }

    it->second->GetStatement().ClearBindings();
    it->second->GetStatement().Reset();
    return it->second.get();
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult MruStatementCache::WithInsert(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn) {
    BeMutexHolder _(m_mutex);
    auto cachedStmt = TryGet(CacheKey(classId, WriterOp::Insert));
    if (cachedStmt == nullptr) {
        LOG.errorv("Failed to prepare insert statement for class: %s", classId.ToHexStr().c_str());
        return BE_SQLITE_ERROR;
    }
    return fn(*cachedStmt);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult MruStatementCache::WithUpdate(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn) {
    BeMutexHolder _(m_mutex);
    auto cachedStmt = TryGet(CacheKey(classId, WriterOp::Update));
    if (cachedStmt == nullptr) {
        LOG.errorv("Failed to prepare update statement for class: %s", classId.ToHexStr().c_str());
        return BE_SQLITE_ERROR;
    }
    return fn(*cachedStmt);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult MruStatementCache::WithDelete(ECClassId classId, std::function<DbResult(CachedWriteStatement&)> fn) {
    BeMutexHolder _(m_mutex);
    auto cachedStmt = TryGet(CacheKey(classId, WriterOp::Delete));
    if (cachedStmt == nullptr) {
        LOG.errorv("Failed to prepare delete statement for class: %s", classId.ToHexStr().c_str());
        return BE_SQLITE_ERROR;
    }
    return fn(*cachedStmt);
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void MruStatementCache::Reset() {
    BeMutexHolder _(m_mutex);
    m_cache.clear();
    m_mru.clear();
}

//******************************Impl********************************
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindPrimitive(BindContext& ctx, PrimitiveType type, IECSqlBinder& binder, BeJsConst val, Utf8CP propertyName, Utf8StringCR extendType) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (type == ECN::PRIMITIVETYPE_String) {
        if (!val.isString()) {
            ctx.SetError("Expected string for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }

        return binder.BindText(val.asCString(), IECSqlBinder::MakeCopy::Yes);
    } else if (type == ECN::PRIMITIVETYPE_Boolean) {
        if (!val.isBool()) {
            ctx.SetError("Expected boolean for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindBoolean(val.asBool());
    } else if (type == ECN::PRIMITIVETYPE_Integer) {
        if (!val.isNumeric()) {
            ctx.SetError("Expected integer for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindInt(val.asInt());
    } else if (type == ECN::PRIMITIVETYPE_Double) {
        if (!val.isNumeric()) {
            ctx.SetError("Expected integer for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindDouble(val.asDouble());
    } else if (type == ECN::PRIMITIVETYPE_Long) {
        if (!val.isNumeric()) {
            ctx.SetError("Expected long for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindInt64(val.asInt64());
    } else if (type == ECN::PRIMITIVETYPE_Binary) {

        if (ExtendedTypeHelper::GetExtendedType(extendType) == ExtendedTypeHelper::ExtendedType::BeGuid && val.isString() && !val.isBinary()) {
            BeGuid guid;
            if (guid.FromString(val.asCString()) != SUCCESS) {
                ctx.SetError("Failed to parse guid from string: %s", val.asCString());
                return ECSqlStatus(BE_SQLITE_ERROR);
            }
            return binder.BindGuid(guid, IECSqlBinder::MakeCopy::Yes);
        }

        if (ExtendedTypeHelper::GetExtendedType(extendType) == ExtendedTypeHelper::ExtendedType::GeometryStream && val.isString() && !val.isBinary()) {
            return ctx.GetECDb().GetImpl().WithSnappyWriter<ECSqlStatus>([&](SnappyToBlob& writer) {
                ByteStream bs;
                val.GetBinary(bs);
                if (SUCCESS != GeomBlobHeader::Compress(bs, writer, bs)) {
                    ctx.SetError("Failed to compress geomstream property (%s)", propertyName);
                    return ECSqlStatus(BE_SQLITE_ERROR);
                }
                return binder.BindBlob(bs.GetData(), bs.GetSize(), IECSqlBinder::MakeCopy::Yes);
            });
        }

        if (!val.isBinary()) {
            ctx.SetError("Expected binary/base64 for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }

        ByteStream bs;
        val.GetBinary(bs);
        return binder.BindBlob(bs.GetData(), bs.GetSize(), IECSqlBinder::MakeCopy::Yes);
    } else if (type == ECN::PRIMITIVETYPE_DateTime) {
        if (!val.isString()) {
            ctx.SetError("Expected string/datetime for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        auto dt = DateTime::FromString(val.asCString());
        if (!dt.IsValid()) {
            ctx.SetError("Failed to parse datetime from string: %s", val.asCString());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindDateTime(dt);
    } else if (type == ECN::PRIMITIVETYPE_IGeometry) {
        if (val.isString()) {
            bvector<IGeometryPtr> geom;
            if (!IModelJson::TryIModelJsonStringToGeometry(val.asCString(), geom)) {
                ctx.SetError("Failed to parse geometry from json string: %s", val.Stringify().c_str());
                return ECSqlStatus(BE_SQLITE_ERROR);
            }
            BeAssert(geom.size() == 1);
            return binder.BindGeometry(*geom[0]);
        }
        if (val.isObject()) {
            bvector<IGeometryPtr> geom;
            if (!IModelJson::TryIModelJsonValueToGeometry(val, geom)) {
                ctx.SetError("Failed to parse geometry from json object: %s", val.Stringify().c_str());
                return ECSqlStatus(BE_SQLITE_ERROR);
            }
            BeAssert(geom.size() == 1);
            return binder.BindGeometry(*geom[0]);
        }
        ctx.SetError("Expected string/json for property %s, got %s", propertyName, val.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);

    } else if (type == ECN::PRIMITIVETYPE_Point2d) {
        if (!val.isObject()) {
            ctx.SetError("Expected instance to be of type object for Point2d property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }

        auto x = ctx.UseJsNames() ? val[ECN::ECJsonSystemNames::Point::X()] : val[ECDBSYS_PROP_PointX];
        auto y = ctx.UseJsNames() ? val[ECN::ECJsonSystemNames::Point::Y()] : val[ECDBSYS_PROP_PointY];

        if (!x.isNumeric() || !y.isNumeric()) {
            ctx.SetError("Expected instance to be of type object with x and y numeric members for Point2d property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        auto p2d = DPoint2d::From(x.asDouble(), y.asDouble());
        return binder.BindPoint2d(p2d);
    } else if (type == ECN::PRIMITIVETYPE_Point3d) {
        if (!val.isObject()) {
            ctx.SetError("Expected instance to be of type object for Point3d property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        auto x = ctx.UseJsNames() ? val[ECN::ECJsonSystemNames::Point::X()] : val[ECDBSYS_PROP_PointX];
        auto y = ctx.UseJsNames() ? val[ECN::ECJsonSystemNames::Point::Y()] : val[ECDBSYS_PROP_PointY];
        auto z = ctx.UseJsNames() ? val[ECN::ECJsonSystemNames::Point::Z()] : val[ECDBSYS_PROP_PointZ];

        if (!x.isNumeric() || !y.isNumeric() || !z.isNumeric()) {
            ctx.SetError("Expected instance to be of type object with x, y and z numeric members for Point3d property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        auto p3d = DPoint3d::From(x.asDouble(), y.asDouble(), z.asDouble());
        return binder.BindPoint3d(p3d);
    }

    BeAssert(false && "Unsupported property type");
    return ECSqlStatus(BE_SQLITE_ERROR);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindPrimitiveProperty(BindContext& ctx, PrimitiveECPropertyCR prop, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }
    return BindPrimitive(ctx, prop.GetType(), binder, val, prop.GetName().c_str(), prop.GetExtendedTypeName());
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindRootProperty(BindContext& ctx, PropertyMap const& propMap, IECSqlBinder& binder, BeJsConst val) {
    if (auto handler = ctx.GetOptions().GetCustomBindHandler()) {
        ECSqlStatus rc = ECSqlStatus::Success;
        if (handler(propMap.GetProperty(), binder, ctx.GetInstance(), val, rc) == PropertyHandlerResult::Handled) {
            return rc;
        }
    }

    if (val.isNull()) {
        return binder.BindNull();
    }

    if (propMap.IsData())
        return BindDataProperty(ctx, propMap.GetProperty(), binder, val);

    if (propMap.IsSystem())
        return BindSystemProperty(ctx, propMap.GetAs<SystemPropertyMap>(), binder, val);

    BeAssert(false && "Unknown property type");
    return ECSqlStatus(BE_SQLITE_ERROR);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindDataProperty(BindContext& ctx, ECPropertyCR propMap, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (auto prop = propMap.GetAsPrimitiveProperty()) {
        return BindPrimitiveProperty(ctx, *prop, binder, val);
    } else if (auto prop = propMap.GetAsStructProperty()) {
        return BindStructProperty(ctx, *prop, binder, val);
    } else if (auto prop = propMap.GetAsStructArrayProperty()) {
        return BindStructArrayProperty(ctx, *prop, binder, val);
    } else if (auto prop = propMap.GetAsPrimitiveArrayProperty()) {
        return BindPrimitiveArrayProperty(ctx, *prop, binder, val);
    } else if (auto prop = propMap.GetAsNavigationProperty()) {
        return BindNavigationProperty(ctx, *prop, binder, val);
    }
    BeAssert(false && "Unknown property type");
    return ECSqlStatus(BE_SQLITE_ERROR);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindSystemProperty(BindContext& ctx, SystemPropertyMap const& prop, IECSqlBinder& binder, BeJsConst val) {

    switch (prop.GetType()) {
    case PropertyMap::Type::ECInstanceId:

        return ECSqlStatus::Success;

    case PropertyMap::Type::ConstraintECInstanceId: {
        if (!val.isNumeric() && !val.isString()) {
            ctx.SetError("Expected id for ECInstanceId property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindId(val.GetId64<ECInstanceId>());
    }
    case PropertyMap::Type::ECClassId:
    case PropertyMap::Type::ConstraintECClassId: {
        if (!val.isNumeric() && !val.isString()) {
            ctx.SetError("Expected id for ECClassId property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindId(val.GetId64<ECClassId>());
    }
    };
    BeAssert(false && "Unknown property type");
    return ECSqlStatus(BE_SQLITE_ERROR);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindStructProperty(BindContext& ctx, StructECPropertyCR prop, IECSqlBinder& binder, BeJsConst val) {
    auto& structType = prop.GetType();
    return BindStruct(ctx, structType, binder, val);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindStruct(BindContext& ctx, ECStructClassCR structClass, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (!val.isObject()) {
        ctx.SetError("Expected instance to be of type object for struct property, got %s", val.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);
    }

    ECSqlStatus rc = ECSqlStatus::Success;
    val.ForEachProperty([&](auto prop, auto val) {
        auto structProp = structClass.GetPropertyP(prop);
        if (structProp == nullptr) {
            return false;
        }
        rc = BindDataProperty(ctx, *structProp, binder[structProp->GetId()], val);
        if (!rc.IsSuccess()) {
            return true;
        }
        return false;
    });
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindPrimitiveArrayProperty(BindContext& ctx, PrimitiveArrayECProperty const& prop, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (!val.isArray()) {
        ctx.SetError("Expected array for primitive array property, got %s", val.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);
    }

    ECSqlStatus rc = ECSqlStatus::Success;
    const auto type = prop.GetType();
    val.ForEachArrayMember([&](auto index, auto val) {
        auto& elBinder = binder.AddArrayElement();
        rc = BindPrimitive(ctx, type, elBinder, val, prop.GetName().c_str(), prop.GetExtendedTypeName());
        if (!rc.IsSuccess()) {
            return true;
        }
        return false;
    });
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindStructArrayProperty(BindContext& ctx, StructArrayECPropertyCR prop, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (!val.isArray()) {
        ctx.SetError("Expected array for struct array property, got %s", val.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);
    }

    ECSqlStatus rc = ECSqlStatus::Success;
    auto& structType = prop.GetStructElementType();
    val.ForEachArrayMember([&](auto index, auto val) {
        if (val.isNull()) {
            return false;
        }
        if (!val.isObject()) {
            ctx.SetError("Expected instance to be of type object for struct array property, got %s", val.Stringify().c_str());
            return true;
        }

        auto& elBinder = binder.AddArrayElement();
        rc = BindStruct(ctx, structType, elBinder, val);
        if (!rc.IsSuccess()) {
            return true;
        }
        return false;
    });

    return rc;
}

namespace {
    bool IsHexadecimalOrDecimal(Utf8StringCR str) {
        if (std::all_of(str.begin(), str.end(), ::isdigit))
            return true;

        if (str.size() < 3 || str[0] != '0' || (str[1] != 'x' && str[1] != 'X'))
            return false;

        return std::all_of(str.begin() + 2, str.end(), [](char c) {
            return std::isxdigit(static_cast<unsigned char>(c));
        });
    }
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus Impl::BindNavigationProperty(BindContext& ctx, NavigationECPropertyCR prop, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (val.isString() || val.isNumeric()) {
        if (!IsHexadecimalOrDecimal(val.asString())) {
            ctx.SetError("Value supplied is not a valid decimal or hexadecimal value for the ECInstanceId/id field %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        return binder.BindNavigation(val.GetId64<ECInstanceId>());
    }

    if (!val.isObject()) {
        ctx.SetError("Expected instance to be of type object for navigation property, got %s", val.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);
    }

    auto id = ctx.UseJsNames() ? val[ECJsonSystemNames::Navigation::Id()] : val[ECDBSYS_PROP_NavPropId];
    auto relClassId = ctx.UseJsNames() ? val[ECJsonSystemNames::Navigation::RelClassName()] : val[ECDBSYS_PROP_NavPropRelECClassId];

    if (!id.isNumeric() && !id.isString()) {
        ctx.SetError("Expected id for navigation property, got %s", id.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);
    }

    ECClassId classId;
    if (!relClassId.isNull()) {
        if (!relClassId.isNumeric() && !relClassId.isString()) {
            ctx.SetError("Expected relClassId/relClassName for navigation property, got %s", relClassId.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        if (ctx.UseJsNames()) {
            auto classP = ctx.FindClass(relClassId.asCString());
            if (classP == nullptr) {
                ctx.SetError("Failed to find class with name: %s", relClassId.asCString());
                return ECSqlStatus(BE_SQLITE_ERROR);
            }
            classId = classP->GetId();
        } else {
            if (relClassId.isString() && !IsHexadecimalOrDecimal(relClassId.asString())) {
                ctx.SetError("Value supplied is not a valid decimal or hexadecimal value for the RelECClassId field %s", relClassId.Stringify().c_str());
                return ECSqlStatus(BE_SQLITE_ERROR);
            }
            classId = relClassId.GetId64<ECClassId>();
        }
    }
    return binder.BindNavigation(id.GetId64<ECInstanceId>(), classId);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool Impl::TryGetECClassId(BindContext& ctx, BeJsConst inst, ECClassId& classId) {
    if (ctx.UseJsNames()) {
        // className has higher priority
        auto className = inst[ECJsonSystemNames::ClassName()];
        if (!className.isNull()) {
            if (!className.isString()) {
                ctx.SetError("Expected string for class name, got %s", className.Stringify().c_str());
                return false;
            }

            auto classP = ctx.FindClass(className.asString());
            if (classP == nullptr) {
                ctx.SetError("Failed to find class with name: %s", className.asCString());
                return false;
            }
            classId = classP->GetId();
        }

        // classFullName has lower priority
        auto classFullName = inst[ECJsonSystemNames::ClassFullName()];
        if (!classFullName.isNull()) {
            if (!classFullName.isString()) {
                ctx.SetError("Expected string for class name, got %s", classFullName.Stringify().c_str());
                return false;
            }

            auto classP = ctx.FindClass(classFullName.asString());
            if (classP == nullptr) {
                ctx.SetError("Failed to find class with name: %s", classFullName.asCString());
                return false;
            }
            classId = classP->GetId();
        }
    } else {
        auto idJs = inst[ECDBSYS_PROP_ECClassId];
        if (!idJs.isNumeric() && !idJs.isString()) {
            ctx.SetError("Expected id for ECClassId property, got %s", idJs.Stringify().c_str());
            return false;
        }
        if (idJs.isString() && !IsHexadecimalOrDecimal(idJs.asString())) {
            ctx.SetError("Value supplied is not a valid decimal or hexadecimal value for the ECClassId field %s", idJs.Stringify().c_str());
            return false;
        }
        classId = inst[ECDBSYS_PROP_ECClassId].GetId64<ECClassId>();
    }
    return true;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool Impl::TryGetECInstanceId(BindContext& ctx, BeJsConst inst, ECInstanceId& id) {
    if (ctx.UseJsNames()) {
        auto name = inst[ECJsonSystemNames::Id()];
        if (!name.isString()) {
            ctx.SetError("Expected string for id, got %s", name.Stringify().c_str());
            return false;
        }
        if (!IsHexadecimalOrDecimal(name.asString())) {
            ctx.SetError("Value supplied is not a valid decimal value %s", name.Stringify().c_str());
            return false;
        }
        id = name.GetId64<ECInstanceId>();
    } else {
        auto idJs = inst[ECDBSYS_PROP_ECInstanceId];
        if (!idJs.isNumeric() && !idJs.isString()) {
            ctx.SetError("Expected id for ECInstanceId property, got %s", idJs.Stringify().c_str());
            return false;
        }
        if (idJs.isString() && !IsHexadecimalOrDecimal(idJs.asString())) {
            ctx.SetError("Value supplied is not a valid decimal or hexadecimal value %s", idJs.Stringify().c_str());
            return false;
        }
        id = idJs.GetId64<ECInstanceId>();
    }
    return true;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool Impl::TryGetId(ECInstanceId& instanceId, BeJsConst in, JsFormat jsFmt) const {
    if (jsFmt == JsFormat::JsName) {
        auto name = in[ECJsonSystemNames::Id()];
        if (!name.isString()) {
            return false;
        }
        if (name.isString() && !IsHexadecimalOrDecimal(name.asString())) {
            LOG.errorv("Value supplied is not a valid decimal or hexadecimal value %s", name.Stringify().c_str());
            return false;
        }
        instanceId = name.GetId64<ECInstanceId>();
    } else {
        auto idJs = in[ECDBSYS_PROP_ECInstanceId];
        if (!idJs.isNumeric() && !idJs.isString()) {
            return false;
        }
        if (idJs.isString() && !IsHexadecimalOrDecimal(idJs.asString())) {
            LOG.errorv("Value supplied is not a valid decimal or hexadecimal value %s", idJs.Stringify().c_str());
            return false;
        }
        instanceId = idJs.GetId64<ECInstanceId>();
    }
    return true;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool Impl::TryGetInstanceKey(ECInstanceKeyR key, BeJsConst in, JsFormat jsFmt) const {
    ECInstanceId instanceId;
    ECClassId classId;
    if (!TryGetId(instanceId, in, jsFmt)) {
        return false;
    }
    if (!TryGetClassId(classId, in, jsFmt)) {
        return false;
    }
    key = ECInstanceKey(classId, instanceId);
    return true;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool Impl::TryGetClassId(ECN::ECClassId& classId, BeJsConst in, JsFormat jsFmt) const {
    if (jsFmt == JsFormat::JsName) {
        // className has higher priority
        if (in.isStringMember(ECJsonSystemNames::ClassName())) {
            auto name = in[ECJsonSystemNames::ClassName()];
            auto classP = m_cache.GetECDb().Schemas().FindClass(name.asString());
            if (classP != nullptr) {
                classId = classP->GetId();
            }
            return classId.IsValid();
        }
        if (in.isStringMember(ECJsonSystemNames::ClassFullName())) {
            auto name = in[ECJsonSystemNames::ClassFullName()];
            auto classP = m_cache.GetECDb().Schemas().FindClass(name.asString());
            if (classP != nullptr) {
                classId = classP->GetId();
            }
            return classId.IsValid();
        }
        return false;
    }

    auto idJs = in[ECDBSYS_PROP_ECClassId];
    if (!idJs.isNumeric() && !idJs.isString()) {
        return false;
    }
    if (idJs.isString() && !IsHexadecimalOrDecimal(idJs.asString())) {
        LOG.errorv("Value supplied is not a valid decimal or hexadecimal value %s", idJs.Stringify().c_str());
        return false;
    }
    classId = in[ECDBSYS_PROP_ECClassId].GetId64<ECClassId>();

    return classId.IsValid();
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void Impl::ToJson(BeJsValue out, ECInstanceId instanceId, ECClassId classId, JsFormat jsFmt) const {
    if (!out.isObject()) {
        out.SetEmptyObject();
    }
    if (jsFmt == JsFormat::JsName) {
        out[ECJsonSystemNames::Id()] = instanceId.ToHexStr();
        auto classP = m_cache.GetECDb().Schemas().GetClass(classId);
        if (classP != nullptr) {
            out[ECJsonSystemNames::ClassFullName()] = classP->GetFullName();
        }
    } else {
        out[ECDBSYS_PROP_ECInstanceId] = instanceId.ToHexStr();
        out[ECDBSYS_PROP_ECClassId] = classId.ToHexStr();
    }
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void Impl::ToJson(BeJsValue out, ECInstanceKeyCR key, JsFormat jsFmt) const {
    ToJson(out, key.GetInstanceId(), key.GetClassId(), jsFmt);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult Impl::Insert(BeJsConst inst, InstanceWriter::InsertOptions const& options) {
    ECInstanceKey key;
    return Insert(inst, options, key);
};

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult Impl::Insert(BeJsConst inst, InstanceWriter::InsertOptions const& options, ECInstanceKey& out) {
    BindContext ctx = BindContext(*this, inst, options);
    if (!inst.isObject()) {
        ctx.SetError("Expected instance to be of type object");
        return BE_SQLITE_ERROR;
    }

    if (m_cache.GetECDb().IsReadonly()) {
        ctx.SetError("Connection is readonly");
        return BE_SQLITE_READONLY;
    }

    ECClassId classId;
    if (!TryGetECClassId(ctx, inst, classId)) {
        ctx.PrependError("Failed to get ECClassId/className/classFullName.");
        return BE_SQLITE_ERROR;
    }

    auto rc = m_cache.WithInsert(classId, [&](CachedWriteStatement& stmt) {
        ECSqlStatus bindStatus = ECSqlStatus::Success;
        inst.ForEachProperty([&](auto prop, auto val) {
            auto binder = stmt.FindBinder(prop);
            if (binder == nullptr) {
                bindStatus = ctx.NotifyUserProperty(prop, val, stmt);
                if (bindStatus != ECSqlStatus::Success) {
                    return FOREACH_ABORT;
                }
                return FOREACH_CONTINUE; // continue
            }
            if (binder->GetPropertyMap().GetType() == PropertyMap::Type::ECInstanceId) {
                return false; // skip ECInstanceId property, we will bind it later
            }
            if (auto canBind = ctx.GetOptions().GetCanBindHandler()) {
                if (!canBind(binder->GetProperty())) {
                    return FOREACH_CONTINUE;
                }
            }

            bindStatus = BindRootProperty(ctx, binder->GetPropertyMap(), binder->GetBinder(), val);
            if (!bindStatus.IsSuccess()) {
                return true;
            }
            return false;
        });

        if (!bindStatus.IsSuccess()) {
            ctx.PrependError("Failed to bind property.");
            return BE_SQLITE_ERROR;
        }
        if (stmt.GetInstanceIdParameterIndex() < 0) {
            BeAssert(false && "Failed to find ECInstanceId parameter in insert statement");
            ctx.SetError("Failed to find ECInstanceId parameter in insert statement");
            return BE_SQLITE_ERROR;
        }

        auto& binder = stmt.GetStatement().GetBinder(stmt.GetInstanceIdParameterIndex());
        if (options.GetInstanceIdMode() == InstanceWriter::InsertOptions::InstanceIdMode::Auto) {
            binder.BindNull();
        } else if (options.GetInstanceIdMode() == InstanceWriter::InsertOptions::InstanceIdMode::FromJs) {
            ECInstanceId id;
            if (!TryGetECInstanceId(ctx, inst, id)) {
                ctx.PrependError("Failed to get ECInstanceId/id.");
                return BE_SQLITE_ERROR;
            }
            binder.BindId(id);
        } else {
            binder.BindId(options.GetInstanceId());
        }

        auto rc = out.IsValid() ? stmt.GetStatement().Step() : stmt.GetStatement().Step(out);
        if (rc != BE_SQLITE_DONE) {
            ctx.SetError(m_cache.GetECDb().GetLastError().c_str());
            if (!ctx.HasError()) {
                ctx.SetError("Failed to insert instance");
            }
        }
        return rc;
    });
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult Impl::Update(BeJsConst inst, InstanceWriter::UpdateOptions const& options) {
    BindContext ctx = BindContext(*this, inst, options);
    if (!inst.isObject()) {
        ctx.SetError("Expected instance to be of type object");
        return BE_SQLITE_ERROR;
    }

    if (m_cache.GetECDb().IsReadonly()) {
        ctx.SetError("Connection is readonly");
        return BE_SQLITE_READONLY;
    }

    ECInstanceId id;
    ECClassId classId;
    if (!TryGetECInstanceId(ctx, inst, id)) {
        ctx.PrependError("Failed to get ECInstanceId/id.");
        return BE_SQLITE_ERROR;
    }

    if (!TryGetECClassId(ctx, inst, classId)) {
        ctx.PrependError("Failed to get ECClassId/className/classFullName.");
        return BE_SQLITE_ERROR;
    }

    auto rc = m_cache.WithUpdate(classId, [&](CachedWriteStatement& stmt) {
        ECSqlStatus bindStatus = ECSqlStatus::Success;
        // m_cache.GetECDb().GetInstanceRepository().Read(ECInstanceKey(classId, id), )
        if (options.GetUseIncrementalUpdate()) {
            // bind any properties missing in the provided instance by reading it from db
            // if these properties are not binded they are otherwise set to null.
            InstanceReader::Position pos(id, classId);
            m_cache.GetECDb().GetInstanceReader().Seek(pos, [&](const InstanceReader::IRowContext& row, auto _) {
                JsReadOptions param;
                param.SetUseJsNames(options.GetUseJsNames());
                param.SetAbbreviateBlobs(false);
                param.SetConvertClassIdsToClassNames(options.GetUseJsNames());
                param.SetSkipReadOnlyProperties(true);
                param.SetUseClassFullNameInsteadofClassName(options.GetUseJsNames());
                row.GetJson(param).ForEachProperty([&](auto prop, auto val) {
                    if (inst.hasMember(prop)) {
                        return FOREACH_CONTINUE
                    }

                    if (auto binder = stmt.FindBinder(prop)) {
                        bindStatus = BindRootProperty(ctx, binder->GetPropertyMap(), binder->GetBinder(), val);
                        if (!bindStatus.IsSuccess()) {
                            return FOREACH_ABORT;
                        }
                    }
                    return FOREACH_CONTINUE;
                });
            });
        }

        // bind properties from the provided instance
        if (bindStatus.IsSuccess()) {
            inst.ForEachProperty([&](auto prop, auto val) {
                auto binder = stmt.FindBinder(prop);
                if (binder == nullptr) {
                    bindStatus = ctx.NotifyUserProperty(prop, val, stmt);
                    if (bindStatus != ECSqlStatus::Success) {
                        return FOREACH_ABORT;
                    }
                    return FOREACH_CONTINUE; // continue
                }

                if (auto canBind = ctx.GetOptions().GetCanBindHandler()) {
                    if (!canBind(binder->GetProperty())) {
                        return FOREACH_CONTINUE;
                    }
                }

                bindStatus = BindRootProperty(ctx, binder->GetPropertyMap(), binder->GetBinder(), val);
                if (!bindStatus.IsSuccess()) {
                    return FOREACH_ABORT;
                }
                return FOREACH_CONTINUE;
            });
        }

        if (!bindStatus.IsSuccess()) {
            ctx.PrependError("Failed to bind property.");
            return BE_SQLITE_ERROR;
        }

        auto& binder = stmt.GetStatement().GetBinder(stmt.GetInstanceIdParameterIndex());
        binder.BindId(id);
        auto rc = stmt.GetStatement().Step();
        if (rc != BE_SQLITE_DONE) {
            ctx.SetError(m_cache.GetECDb().GetLastError().c_str());
            if (!ctx.HasError()) {
                ctx.SetError("Failed to update instance");
            }
        }
        m_cache.GetECDb().GetInstanceReader().InvalidateSeekPos(ECInstanceKey(classId, id));
        return rc;
    });
    return rc;
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult Impl::Delete(ECInstanceKeyCR key, InstanceWriter::DeleteOptions const& options) {
    BindContext ctx = BindContext(*this, BeJsDocument::Null(), options);
    if (m_cache.GetECDb().IsReadonly()) {
        ctx.SetError("Connection is readonly");
        return BE_SQLITE_READONLY;
    }
    auto rc = m_cache.WithDelete(key.GetClassId(), [&](CachedWriteStatement& stmt) {
        auto& binder = stmt.GetStatement().GetBinder(1);
        binder.BindId(key.GetInstanceId());
        auto rc = stmt.GetStatement().Step();
        if (rc != BE_SQLITE_DONE) {
            ctx.SetError(m_cache.GetECDb().GetLastError().c_str());
            if (!ctx.HasError()) {
                ctx.SetError("Failed to delete instance");
            }
        }
        return rc;
    });
    m_cache.GetECDb().GetInstanceReader().InvalidateSeekPos(key);
    return rc;
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult Impl::Delete(BeJsConst inst, InstanceWriter::DeleteOptions const& options) {
    BindContext ctx = BindContext(*this, inst, options);
    if (!inst.isObject()) {
        ctx.SetError("Expected instance to be of type object");
        return BE_SQLITE_ERROR;
    }

    if (m_cache.GetECDb().IsReadonly()) {
        ctx.SetError("Connection is readonly");
        return BE_SQLITE_READONLY;
    }

    ECInstanceKey key;
    if (!TryGetInstanceKey(key, inst, ctx.GetOptions().GetUseJsNames() ? JsFormat::JsName : JsFormat::Standard)) {
        ctx.SetError("Failed to get ECInstanceId/id and ECClassId/className/classFullName.");
        return BE_SQLITE_ERROR;
    }
    return Delete(key, options);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void Impl::Reset() {
    m_error.clear();
    m_cache.Reset();
}

//******************************InstanceWriter**************************************
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
InstanceWriter::InstanceWriter(ECDbCR ecdb, uint32_t cacheSize) : m_pImpl(new Impl(ecdb, cacheSize)) {}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
InstanceWriter::~InstanceWriter() { delete m_pImpl; }

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Insert(BeJsConst inst, InsertOptions const& options, ECInstanceKey& key) {
    return m_pImpl->Insert(inst, options, key);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Insert(BeJsConst inst, InsertOptions const& options) {
    return m_pImpl->Insert(inst, options);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Update(BeJsConst inst, UpdateOptions const& options) {
    return m_pImpl->Update(inst, options);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Delete(BeJsConst inst, DeleteOptions const& options) {
    return m_pImpl->Delete(inst, options);
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Delete(ECInstanceKeyCR key, DeleteOptions const& options) {
    return m_pImpl->Delete(key, options);
}
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void InstanceWriter::Reset() {
    m_pImpl->Reset();
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
Utf8StringCR InstanceWriter::GetLastError() const {
    return m_pImpl->GetLastError();
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
InstanceWriter::InsertOptions& InstanceWriter::InsertOptions::UseInstanceId(ECInstanceId id) {
    m_newInstanceId = id;
    m_instanceIdMode = InstanceIdMode::Manual;
    return *this;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
InstanceWriter::InsertOptions& InstanceWriter::InsertOptions::UseInstanceIdFromJs() {
    m_instanceIdMode = InstanceIdMode::FromJs;
    m_newInstanceId = ECInstanceId();
    return *this;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
InstanceWriter::InsertOptions& InstanceWriter::InsertOptions::UseAutoECInstanceId() {
    m_instanceIdMode = InstanceIdMode::FromJs;
    m_newInstanceId = ECInstanceId();
    return *this;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool InstanceWriter::TryGetId(ECInstanceId& instanceId, BeJsConst in, JsFormat jsFmt) const {
    return m_pImpl->TryGetId(instanceId, in, jsFmt);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool InstanceWriter::TryGetClassId(ECN::ECClassId& classId, BeJsConst in, JsFormat jsFmt) const {
    return m_pImpl->TryGetClassId(classId, in, jsFmt);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool InstanceWriter::TryGetInstanceKey(ECInstanceKeyR key, BeJsConst in, JsFormat jsFmt) const {
    return m_pImpl->TryGetInstanceKey(key, in, jsFmt);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void InstanceWriter::ToJson(BeJsValue out, ECInstanceId instanceId, ECClassId classId, JsFormat jsFmt) const {
    m_pImpl->ToJson(out, instanceId, classId, jsFmt);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void InstanceWriter::ToJson(BeJsValue out, ECInstanceKeyCR key, JsFormat jsFmt) const {
    m_pImpl->ToJson(out, key, jsFmt);
}

// struct FastPropertyCache {

//     private:
//         ECDbCR m_ecdb;
//         std::map<ECPropertyId, ECPropertyId> m_reverseOverridenPropertyMap;
//         DbResult LoadProperties(ECClassId classId) {
//             auto sql = R"sql(
//                 SELECT
//                     [base_prop].[Id] [base],
//                     [overridden_prop].[Id] [overridden]
//                 FROM   [ec_Property] [base_prop]
//                     JOIN [ec_cache_ClassHierarchy] [ch] ON [ch].[BaseClassId] = [base_prop].[ClassId]
//                     JOIN [ec_Property] [overridden_prop] ON [overridden_prop].[ClassId] = [ch].[ClassId]
//                             AND [overridden_prop].[Name] = [base_prop].[Name]
//                             AND [overridden_prop].[Id] != [base_prop].[Id]
//                 WHERE  [base_prop].[ClassId] = ?;
//             )sql";
//             auto stmt = m_ecdb.GetImpl().GetCachedSqliteStatement(sql);
//             if (!stmt.IsValid()) {
//                 return BE_SQLITE_ERROR;
//             }
//             stmt->BindId(1, classId);
//             while (stmt->Step() == BE_SQLITE_ROW) {
//                 auto base = stmt->GetValueId<ECClassId>(0);
//                 auto overridden = stmt->GetValueId<ECClassId>(1);
//                 m_reverseOverridenPropertyMap[overridden] = base;
//             }

//         }
//     public:

// }

END_BENTLEY_SQLITE_EC_NAMESPACE