/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/InstanceWriter.h>
#include <GeomSerialization/GeomLibsJsonSerialization.h>
#include <GeomSerialization/GeomLibsSerialization.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
namespace Internal {
    //******************************CachedStatement**************************************
    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    void CachedStatement::BuildPropertyIndexMap(bool addUseJsNameMap) {
        m_propertyIndexMap.clear();
        for (auto it = m_propertyBinders.begin(); it != m_propertyBinders.end(); ++it) {
            auto& prop = it->GetPropertyMap();
            m_propertyIndexMap[prop.GetName()] = it;

            if (!addUseJsNameMap)
                continue;

            if (prop.GetType() == PropertyMap::Type::ECInstanceId) {
                m_propertyIndexMap[ECJsonSystemNames::Id()] = it;
            } else if (prop.GetType() == PropertyMap::Type::ECClassId) {
                m_propertyIndexMap[ECJsonSystemNames::ClassName()] = it;
                m_propertyIndexMap[ECJsonSystemNames::ClassFullName()] = it;
            } else if (prop.GetType() == PropertyMap::Type::ConstraintECClassId) {
                if (prop.GetName().EqualsIAscii(ECDBSYS_PROP_SourceECClassId)) {
                    m_propertyIndexMap[ECJsonSystemNames::SourceClassName()] = it;
                } else if (prop.GetName().EqualsIAscii(ECDBSYS_PROP_TargetECClassId)) {
                    m_propertyIndexMap[ECJsonSystemNames::TargetClassName()] = it;
                }
            } else if(prop.GetType() == PropertyMap::Type::ConstraintECInstanceId) {
                if (prop.GetName().Equals(ECDBSYS_PROP_SourceECInstanceId)) {
                    m_propertyIndexMap[ECJsonSystemNames::SourceId()] = it;
                } else if (prop.GetName().Equals(ECDBSYS_PROP_TargetECInstanceId)) {
                    m_propertyIndexMap[ECJsonSystemNames::TargetId()] = it;
                }
            } else {
                Utf8String str = prop.GetName();
                str[0] = (Utf8Char) tolower(str[0]);
                m_propertyIndexMap[str] = it;
            }
        }
    }
    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    Utf8String CachedStatement::GetCurrentTimeStampProperty() const {
        BeAssert(m_classMap != nullptr);
        auto ca = m_classMap->GetClass().GetCustomAttributeLocal("CoreCustomAttributes", "ClassHasCurrentTimeStampProperty");
        if (ca == nullptr)
            return "";

        ECValue v;
        if (ECObjectsStatus::Success != ca->GetValue(v, "PropertyName")){
            return "";
        }
        return v.GetUtf8CP();
    }
    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    const CachedBinder* CachedStatement::FindBinder(Utf8StringCR name) const {
        if (!m_propertyIndexMap.empty()) {
            auto it = m_propertyIndexMap.find(name);
            if (it != m_propertyIndexMap.end()) {
                return &(*(it->second));
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
    //******************************StatementMruCache**************************************
    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    ECSqlStatus StatementMruCache::PrepareInsert(CachedStatement& cachedStmt) {
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

        auto rc = cachedStmt.GetStatement().Prepare(m_ecdb, ecsql.c_str());
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
    ECSqlStatus StatementMruCache::PrepareUpdate(CachedStatement& cachedStmt) {
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

        auto rc = cachedStmt.GetStatement().Prepare(m_ecdb, ecsql.c_str());
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
    ECSqlStatus StatementMruCache::PrepareDelete(CachedStatement& cachedStmt) {
        Utf8String ecsql;
        ecsql.append("DELETE FROM ").append(cachedStmt.GetClass().GetECSqlName()).append(" WHERE [ECInstanceId] = ?");

        auto rc = cachedStmt.GetStatement().Prepare(m_ecdb, ecsql.c_str());
        if (!rc.IsSuccess()) {
            return rc;
        }
        cachedStmt.m_instanceIdIndex = 1;
        return rc;
    }

    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    std::unique_ptr<CachedStatement> StatementMruCache::Prepare(CacheKey key) {
        auto cls = m_ecdb.Schemas().GetClass(key.GetClassId());
        if (cls == nullptr) {
            return nullptr;
        }
        auto classMap = m_ecdb.Schemas().Main().GetClassMap(*cls);
        if (cls == nullptr) {
            return nullptr;
        }

        auto cachedStmt = std::make_unique<CachedStatement>(*classMap);
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
    CachedStatement* StatementMruCache::TryGet(CacheKey key) {
        auto it = m_cache.find(key);
        if (it == m_cache.end()) {

            auto cachedStmt = Prepare(key);

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
    bool StatementMruCache::WithInsert(ECClassId classId, std::function<void(CachedStatement&)> fn) {
        BeMutexHolder _(m_mutex);
        auto cachedStmt = TryGet(CacheKey(classId, WriterOp::Insert));
        if (cachedStmt == nullptr) {
            return false;
        }
        fn(*cachedStmt);
        return true;
    }

    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    bool StatementMruCache::WithUpdate(ECClassId classId, std::function<void(CachedStatement&)> fn) {
        BeMutexHolder _(m_mutex);
        auto cachedStmt = TryGet(CacheKey(classId, WriterOp::Update));
        if (cachedStmt == nullptr) {
            return false;
        }
        fn(*cachedStmt);
        return true;
    }
    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    bool StatementMruCache::WithDelete(ECClassId classId, std::function<void(CachedStatement&)> fn) {
        BeMutexHolder _(m_mutex);
        auto cachedStmt = TryGet(CacheKey(classId, WriterOp::Delete));
        if (cachedStmt == nullptr) {
            return false;
        }
        fn(*cachedStmt);
        return true;
    }
    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-
    void StatementMruCache::Reset() {
        BeMutexHolder _(m_mutex);
        m_cache.clear();
        m_mru.clear();
    }
    //******************************InstanceWriter::Impl********************************
    //----------------------------------------------------------------------------------
    // @bsimethod
    //+---------------+---------------+---------------+---------------+---------------+-

}
//******************************InstanceWriter::Impl********************************
//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus InstanceWriter::Impl::BindPrimitive(Context& ctx, PrimitiveType type, IECSqlBinder& binder, BeJsConst val, Utf8CP propertyName, Utf8StringCR extendType) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (type == ECN::PRIMITIVETYPE_String) {
        if (!val.isString()) {
            ctx.SetError("Expected string for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        if (ExtendedTypeHelper::GetExtendedType(extendType) == ExtendedTypeHelper::ExtendedType::BeGuid) {
            BeGuid guid;
            if (!guid.FromString(val.asCString())) {
                ctx.SetError("Failed to parse guid from string: %s", val.asCString());
                return ECSqlStatus(BE_SQLITE_ERROR);
            }
            return binder.BindGuid(guid, IECSqlBinder::MakeCopy::Yes);
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
        if (!val.isString()) {
            ctx.SetError("Expected string/json for property %s, got %s", propertyName, val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        bvector<IGeometryPtr> geom;
        if (!IModelJson::TryIModelJsonStringToGeometry(val.asCString(), geom)) {
            ctx.SetError("Failed to parse geometry from json string: %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        BeAssert(geom.size() == 1);
        return binder.BindGeometry(*geom[0]);
    } else if (type == ECN::PRIMITIVETYPE_Point2d) {
        if (!val.isObject()) {
            ctx.SetError("Expected object for Point2d property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }

        auto x = ctx.IsUseJsName() ? val[ECN::ECJsonSystemNames::Point::X()] : val[ECDBSYS_PROP_PointX];
        auto y = ctx.IsUseJsName() ? val[ECN::ECJsonSystemNames::Point::Y()] : val[ECDBSYS_PROP_PointY];

        if (!x.isNumeric() || !y.isNumeric()) {
            ctx.SetError("Expected object with x and y numeric members for Point2d property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        auto p2d = DPoint2d::From(x.asDouble(), y.asDouble());
        return binder.BindPoint2d(p2d);
    } else if (type == ECN::PRIMITIVETYPE_Point3d) {
        if (!val.isObject()) {
            ctx.SetError("Expected object for Point3d property, got %s", val.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        auto x = ctx.IsUseJsName() ? val[ECN::ECJsonSystemNames::Point::X()] : val[ECDBSYS_PROP_PointX];
        auto y = ctx.IsUseJsName() ? val[ECN::ECJsonSystemNames::Point::Y()] : val[ECDBSYS_PROP_PointY];
        auto z = ctx.IsUseJsName() ? val[ECN::ECJsonSystemNames::Point::Z()] : val[ECDBSYS_PROP_PointZ];

        if (!x.isNumeric() || !y.isNumeric() || !z.isNumeric()) {
            ctx.SetError("Expected object with x, y and z numeric members for Point3d property, got %s", val.Stringify().c_str());
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
ECSqlStatus InstanceWriter::Impl::BindPrimitiveProperty(Context& ctx, PrimitiveECPropertyCR prop, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }
    return BindPrimitive(ctx, prop.GetType(), binder, val, prop.GetName().c_str(), prop.GetExtendedTypeName());
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus InstanceWriter::Impl::BindRootProperty(Context& ctx, PropertyMap const& propMap, IECSqlBinder& binder, BeJsConst val) {
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
ECSqlStatus InstanceWriter::Impl::BindDataProperty(Context& ctx, ECPropertyCR propMap, IECSqlBinder& binder, BeJsConst val) {
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
ECSqlStatus InstanceWriter::Impl::BindSystemProperty(Context& ctx, SystemPropertyMap const& prop, IECSqlBinder& binder, BeJsConst val) {

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
ECSqlStatus InstanceWriter::Impl::BindStructProperty(Context& ctx, StructECPropertyCR prop, IECSqlBinder& binder, BeJsConst val) {
    auto& structType = prop.GetType();
    return BindStruct(ctx, structType, binder, val);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus InstanceWriter::Impl::BindStruct(Context& ctx, ECStructClassCR structClass, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    ECSqlStatus rc = ECSqlStatus::Success;
    val.ForEachProperty([&](auto prop, auto val) {
        auto structProp = structClass.GetPropertyP(prop);
        if (structProp == nullptr) {
            if (ctx.NotifyUnknownJsProperty(prop, val) == Abortable::Abort) {
                return true;
            }
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
ECSqlStatus InstanceWriter::Impl::BindPrimitiveArrayProperty(Context& ctx, PrimitiveArrayECProperty const& prop, IECSqlBinder& binder, BeJsConst val) {
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
ECSqlStatus InstanceWriter::Impl::BindStructArrayProperty(Context& ctx, StructArrayECPropertyCR const& prop, IECSqlBinder& binder, BeJsConst val) {
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
            ctx.SetError("Expected object for struct array property, got %s", val.Stringify().c_str());
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

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
ECSqlStatus InstanceWriter::Impl::BindNavigationProperty(Context& ctx, NavigationECPropertyCR const& prop, IECSqlBinder& binder, BeJsConst val) {
    if (val.isNull()) {
        return binder.BindNull();
    }

    if (!val.isObject()) {
        ctx.SetError("Expected object for navigation property, got %s", val.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);
    }

    auto id = ctx.IsUseJsName() ? val[ECJsonSystemNames::Navigation::Id()] : val[ECDBSYS_PROP_NavPropId];
    auto relClassId = ctx.IsUseJsName() ? val[ECJsonSystemNames::Navigation::RelClassName()] : val[ECDBSYS_PROP_NavPropRelECClassId];

    if (!id.isNumeric() && !id.isString()) {
        ctx.SetError("Expected id for navigation property, got %s", id.Stringify().c_str());
        return ECSqlStatus(BE_SQLITE_ERROR);
    }

    ECClassId classId;
    if (!relClassId.isNull()) {
        if (!relClassId.isNumeric() && !relClassId.isString()) {
            ctx.SetError("Expected relClassId for navigation property, got %s", relClassId.Stringify().c_str());
            return ECSqlStatus(BE_SQLITE_ERROR);
        }
        if (ctx.IsUseJsName()) {
            auto classP = ctx.FindClass(relClassId.asCString());
            if (classP == nullptr) {
                ctx.SetError("Failed to find class with name: %s", relClassId.asCString());
                return ECSqlStatus(BE_SQLITE_ERROR);
            }
            classId = classP->GetId();
        } else {
            classId = relClassId.GetId64<ECClassId>();
        }
    }
    return binder.BindNavigation(id.GetId64<ECInstanceId>(), classId);
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool InstanceWriter::Impl::TryGetECClassId(Context& ctx, BeJsConst inst, ECClassId& classId) {
    if (ctx.IsUseJsName()) {
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
        classId = inst[ECDBSYS_PROP_ECClassId].GetId64<ECClassId>();
    }
    return true;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
bool InstanceWriter::Impl::TryGetECInstanceId(Context& ctx, BeJsConst inst, ECInstanceId& id) {
    if (ctx.IsUseJsName()) {
        auto name = inst[ECJsonSystemNames::Id()];
        if (!name.isString()) {
            ctx.SetError("Expected string for id, got %s", name.Stringify().c_str());
            return false;
        }
        id = name.GetId64<ECInstanceId>();
    } else {
        auto idJs = inst[ECDBSYS_PROP_ECInstanceId];
        if (!idJs.isNumeric() && !idJs.isString()) {
            ctx.SetError("Expected id for ECInstanceId property, got %s", idJs.Stringify().c_str());
            return false;
        }
        id = idJs.GetId64<ECInstanceId>();
    }
    return true;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Impl::Insert(BeJsConst inst, InstanceWriter::InsertOptions const& options) {
    ECInstanceKey key;
    return Insert(inst, options, key);
};

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Impl::Insert(BeJsConst inst, InstanceWriter::InsertOptions const& options, ECInstanceKey& out) {
    m_error.clear();
    if (!inst.isObject()) {
        return BE_SQLITE_ERROR;
    }

    Context ctx = Context(m_cache.GetECDb(), options, m_error);
    ECClassId classId;
    if (!TryGetECClassId(ctx, inst, classId)) {
        return BE_SQLITE_ERROR;
    }

    DbResult rc = BE_SQLITE_OK;
    if (!m_cache.WithInsert(classId, [&](Internal::CachedStatement& stmt) {
            inst.ForEachProperty([&](auto prop, auto val) {
                auto binder = stmt.FindBinder(prop);
                if (binder == nullptr) {
                    if (ctx.NotifyUnknownJsProperty(prop, val) == Abortable::Abort) {
                        return true;
                    }
                    return false; // continue
                }
                if (binder->GetPropertyMap().GetType() == PropertyMap::Type::ECInstanceId) {
                    return false; // skip ECInstanceId property, we will bind it later
                }
                return !BindRootProperty(ctx, binder->GetPropertyMap(), binder->GetBinder(), val).IsSuccess();
            });

            if (stmt.GetInstanceIdParameterIndex() < 0) {
                BeAssert(false && "Failed to find ECInstanceId parameter in insert statement");
                ctx.SetError("Failed to find ECInstanceId parameter in insert statement");
                rc = BE_SQLITE_ERROR;
            }

            auto& binder = stmt.GetStatement().GetBinder(stmt.GetInstanceIdParameterIndex());
            if (options.GetInstanceIdMode() == InstanceWriter::InsertOptions::InstanceIdMode::Auto) {
                binder.BindNull();
            } else if (options.GetInstanceIdMode() == InstanceWriter::InsertOptions::InstanceIdMode::FromJs) {
                ECInstanceId id;
                if (!TryGetECInstanceId(ctx, inst, id)) {
                    rc = BE_SQLITE_ERROR;
                    return;
                }
                binder.BindId(id);
            } else {
                binder.BindId(options.GetInstanceId());
            }

            rc = stmt.GetStatement().Step(out);
            if (rc != BE_SQLITE_DONE) {
                m_error = m_cache.GetECDb().GetLastError();
            }
        })) {
        // failed to prepare statement
        rc = BE_SQLITE_ERROR;
    }
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Impl::Update(BeJsConst inst, InstanceWriter::UpdateOptions const& options) {
    m_error.clear();
    if (!inst.isObject()) {
        return BE_SQLITE_ERROR;
    }

    Context ctx = Context(m_cache.GetECDb(), options, m_error);
    ECInstanceId id;
    ECClassId classId;
    if (!TryGetECInstanceId(ctx, inst, id)) {
        return BE_SQLITE_ERROR;
    }
    if (!TryGetECClassId(ctx, inst, classId)) {
        return BE_SQLITE_ERROR;
    }

    DbResult rc = BE_SQLITE_OK;
    if (!m_cache.WithUpdate(classId, [&](Internal::CachedStatement& stmt) {
            inst.ForEachProperty([&](auto prop, auto val) {
                auto binder = stmt.FindBinder(prop);
                if (binder == nullptr) {
                    if (ctx.NotifyUnknownJsProperty(prop, val) == Abortable::Abort) {
                        return true;
                    }
                    return false; // continue
                }
                return !BindRootProperty(ctx, binder->GetPropertyMap(), binder->GetBinder(), val).IsSuccess();
            });
            auto& binder = stmt.GetStatement().GetBinder((int)stmt.GetBinders().size());
            binder.BindId(id);
            rc = stmt.GetStatement().Step();
            if (rc != BE_SQLITE_DONE) {
                m_error = m_cache.GetECDb().GetLastError();
            }
        })) {
        // failed to prepare statement
        rc = BE_SQLITE_ERROR;
    }
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
DbResult InstanceWriter::Impl::Delete(BeJsConst inst, InstanceWriter::DeleteOptions const& options) {

    m_error.clear();
    if (!inst.isObject()) {
        return BE_SQLITE_ERROR;
    }
    Context ctx = Context(m_cache.GetECDb(), options, m_error);
    ECInstanceId id;
    ECClassId classId;
    if (!TryGetECInstanceId(ctx, inst, id)) {
        return BE_SQLITE_ERROR;
    }
    if (!TryGetECClassId(ctx, inst, classId)) {
        return BE_SQLITE_ERROR;
    }
    DbResult rc = BE_SQLITE_OK;
    if (!m_cache.WithDelete(classId, [&](Internal::CachedStatement& stmt) {
            auto& binder = stmt.GetStatement().GetBinder(1);
            binder.BindId(id);
            rc = stmt.GetStatement().Step();
            if (rc != BE_SQLITE_DONE) {
                m_error = m_cache.GetECDb().GetLastError();
            }
        })) {
        // failed to prepare statement
        rc = BE_SQLITE_ERROR;
    }
    return rc;
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
void InstanceWriter::Impl::Reset() {
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
DbResult InstanceWriter::Insert(BeJsConst inst, InsertOptions const& options,  ECInstanceKey& key) {
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
void InstanceWriter::Reset() {
    m_pImpl->Reset();
}

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
Utf8StringCR InstanceWriter::GetLastError() const {
    return m_pImpl->GetLastError();
}

END_BENTLEY_SQLITE_EC_NAMESPACE