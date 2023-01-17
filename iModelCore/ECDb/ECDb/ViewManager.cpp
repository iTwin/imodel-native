/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSql/NativeSqlBuilder.h"
#include <functional>
#include <re2/re2.h>

#define VIEW_SCHEMA_Name "ClassView"
#define VIEW_CLASS_PersistedView "PersistedView"
#define VIEW_CLASS_TransientView "TransientView"
#define VIEW_PROP_Namespace "ecViews"
#define VIEW_PROP_DataVersion "dataVersion"
#define VIEW_PROP_ErrorCode "errorCode"
#define VIEW_PROP_ErrorMsg "errorMessage"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
const Utf8String ViewDef::CreateInsertECSql() const {
    Utf8String insertSql;
    auto rc = ViewDef::CreateInsertECSql(insertSql, m_classDef, m_propertyMaps);
    return rc == BE_SQLITE_OK ? insertSql : "";
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
const Utf8String ViewDef::CreateTransientECSql() const {
    Utf8String transientECSql;
    auto rc = ViewDef::CreateTransientECSql(transientECSql, m_classDef, m_query);
    return rc == BE_SQLITE_OK ? transientECSql : "";
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
const Utf8String ViewDef::CreateTruncateECSql() const {
    Utf8String truncateSql;
    auto rc = ViewDef::CreateTruncateECSql(truncateSql, m_classDef);
    return rc == BE_SQLITE_OK ? truncateSql : "";
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::TryGetPropertyMaps(std::vector<Utf8String>& propMaps, ECN::IECInstancePtr& inst) {
    const auto kPropertyMapsName = "PropertyMaps";
    if (inst.IsNull()) {
        return false;
    }
    ECValue v;
    if (ECObjectsStatus::Success != inst->GetValue(v, kPropertyMapsName)) {
        return false;
    }
    if (!v.IsArray() || v.GetArrayInfo().IsStructArray()) {
        return false;
    }
    const auto length = v.GetArrayInfo().GetCount();
    if (length == 0) {
        return false;
    }
    for(uint32_t i = 0 ; i < length; ++i) {
        if (ECObjectsStatus::Success != inst->GetValue(v, kPropertyMapsName, i)) {
            return false;
        }
        if (!v.IsString()) {
            return false;
        }
        Utf8String propName = v.GetUtf8CP();
        propName.Trim();
        propMaps.push_back(propName);
    }
    return true;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::TryGetPersistenceMethod(PersistenceMethod& method, ECN::IECInstancePtr& inst) {
    const auto kPersistenceMethod = "PersistenceMethod";
    if (inst.IsNull()) {
        return false;
    }
    ECValue v;
    if (ECObjectsStatus::Success != inst->GetValue(v, kPersistenceMethod)) {
        return false;
    }
    if (!v.IsString()) {
        return false;
    }
    Utf8String methodStr = v.GetUtf8CP();
    methodStr.Trim();
    if (methodStr.empty()) {
        return false;
    }
    if (methodStr.EqualsIAscii("Permanent")) {
        method = PersistenceMethod::Permanent;
        return true;
    } else if (methodStr.EqualsIAscii("Temporary")) {
        method = PersistenceMethod::Temporary;
        return true;
    } else {
        BeAssert(false && "unsupported type");
        return false;
    }
    return false;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::TryGetRefreshMethod(RefreshMethod& method, ECN::IECInstancePtr& inst) {
    const auto kRefreshMethod = "RefreshMethod";
    if (inst.IsNull()) {
        return false;
    }
    ECValue v;
    if (ECObjectsStatus::Success != inst->GetValue(v, kRefreshMethod)) {
        return false;
    }
    if (!v.IsString()) {
        return false;
    }

    Utf8String methodStr = v.GetUtf8CP();
    methodStr.Trim();
    if (methodStr.empty()) {
        return false;
    }
    if (methodStr.EqualsIAscii("Recompute")) {
        method = RefreshMethod::Recompute;
        return true;
    } else {
        BeAssert(false && "unsupported type");
        return false;
    }
    return false;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::TryGetQuery(Utf8StringR query, ECN::IECInstancePtr& inst) {
    const auto kQueryName = "Query";
    if (inst.IsNull()) {
        return false;
    }
    ECValue v;
    if (ECObjectsStatus::Success != inst->GetValue(v, kQueryName)) {
        return false;
    }
    if (!v.IsString()) {
        return false;
    }
    query = v.GetUtf8CP();
    query.Trim();
    return !query.empty();
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::HasPersistedViewDef(ECN::ECClassCR classDef) {
    return classDef.GetCustomAttributeLocal(VIEW_SCHEMA_Name, VIEW_CLASS_PersistedView).IsValid();
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::HasTransientViewDef(ECN::ECClassCR classDef) {
    return classDef.GetCustomAttributeLocal(VIEW_SCHEMA_Name, VIEW_CLASS_TransientView).IsValid() ;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::HasViewDef(ECN::ECClassCR classDef) {
    return ViewDef::HasPersistedViewDef(classDef) || ViewDef::HasTransientViewDef(classDef);
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
ViewDef::ValidationResult ViewDef::Validate(ECDbCR ecdb, ECN::ECClassCR classDef, ValidationMethod method, bool reportIssues ) {
    Issues issues(ecdb.GetImpl().Issues(), reportIssues);

    auto persistedViewDef = classDef.GetCustomAttributeLocal(VIEW_SCHEMA_Name, VIEW_CLASS_PersistedView);
    auto transientViewDef = classDef.GetCustomAttributeLocal(VIEW_SCHEMA_Name, VIEW_CLASS_TransientView);

    // Rule: WARN check if custom attribute for view def exist on class definition
    if (persistedViewDef.IsNull() && transientViewDef.IsNull()) {
        return ValidationResult::NotView;
    }
    // Rule: ERROR Both custom attribute must not exist at same time on same class.
    if (persistedViewDef.IsValid() && transientViewDef.IsValid()) {
          issues.Warn(
            "%s class can only have on off  %s.%s or %s.%s view definition custom attribute at a given time.",
            classDef.GetECSqlName().c_str(),
            VIEW_SCHEMA_Name,
            VIEW_CLASS_PersistedView,
            VIEW_SCHEMA_Name,
            VIEW_CLASS_TransientView);
        return ValidationResult::NotView;
    }

    auto viewDef = persistedViewDef.IsValid() ? persistedViewDef : transientViewDef;
    //! Rule: ERROR make sure class definition is EntityClass or RelationshipClass
    if (!classDef.IsEntityClass() && !classDef.IsRelationshipClass()) {
        issues.Error(
            "Invalid view class '%s'. View definition can only be applied to EntityClass or RelationshipClass.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }
    if (persistedViewDef.IsValid()) {
        if (classDef.GetClassModifier() != ECClassModifier::Sealed) {
            issues.Error(
                "Invalid view class '%s'. 'PersistedView' must be applied to a 'Sealed' class.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
    }
    if (transientViewDef.IsValid()) {
        if (classDef.GetClassModifier() != ECClassModifier::Abstract) {
            issues.Error(
                "Invalid view class '%s'. 'TransientView' must be applied to a 'Abstract' class.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
        if (classDef.HasBaseClasses()) {
            issues.Error(
                "Invalid view class '%s'. 'TransientView' cannot be derived from another class",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
    }
    // Read values from custom attribute
    std::vector<Utf8String> propertyMap;
    PersistenceMethod persistenceMethod = PersistenceMethod::None;
    RefreshMethod refreshMethod = RefreshMethod::None;
    Utf8String query;

    if (persistedViewDef.IsValid()) {
        // Rule 3: ERROR make sure persistence type attribute and have supported value.
        if (!ViewDef::TryGetPersistenceMethod(persistenceMethod, viewDef)) {
            issues.Error(
                "Invalid view class '%s'. 'PersistenceMethod' is required attribute for PersistedView custom attribute.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        // Rule 4: Refresh method is optional and default to w.r.t persistence type
        if (!ViewDef::TryGetRefreshMethod(refreshMethod, viewDef)) {
            issues.Error(
                "Invalid view class '%s'. 'RefreshMethod' is required attribute for PersistedView custom attribute.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        // Rule 7: ERROR property map is require for now but can be optional in future
        if (!ViewDef::TryGetPropertyMaps(propertyMap, viewDef)) {
            issues.Error(
                "Invalid view class '%s'. 'PropertyMap' is required attribute for PersistedView custom attribute.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        // Rule x: Verify if all none-empty entries in property map are unique.Otherwise we be writing to same property twice.
        std::set<Utf8String, CompareIUtf8Ascii> uniqueMapProp;
        for (const auto& prop : propertyMap) {
            if (prop.empty()) {
                continue;
            }
            if (uniqueMapProp.end() == uniqueMapProp.find(prop)) {
                uniqueMapProp.insert(prop);
            } else {
                issues.Error(
                    "Invalid view class '%s'. '%s property define more then once in 'propertyMap' attribute array.",
                    classDef.GetECSqlName().c_str(), prop.c_str());
                return ValidationResult::Error;
            }
        }

        // Rule 8: ERROR validate property map any issue is treated as error
        const auto  validatePropMapRc= ViewDef::ValidatePropMaps(classDef, propertyMap, issues);
        if (validatePropMapRc != ValidationResult::Success && validatePropMapRc != ValidationResult::Warning){
            return validatePropMapRc;
        }
    }

    // Rule 6: ERROR query is require attribute
    if (!ViewDef::TryGetQuery(query, viewDef)) {
        issues.Error(
            "Invalid view class '%s'. 'Query' is required attribute for PersistedView/TransientView defintion custom attribute.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }

    // Rule 9. ERROR view class def must always be leaf classes.
    //         This rule must be enforced on all schema import
    if (!classDef.GetDerivedClasses().empty()) {
        issues.Error(
            "Invalid view class '%s'. View class definition cannot have derived classes.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }
    const auto classRefs = ViewDef::GetClassReferencedInECSql(ecdb, query);
    for (auto classP : classRefs) {
        if (classP->GetECSqlName() == classDef.GetECSqlName()) {
            issues.Error(
                "Invalid view class '%s'. View query contain self reference to view class which is not allowed.",
                classDef.GetFullName());
            return ValidationResult::Error;
        }
    }
    // Following avoid dependency tree for view by not allow one view to be used in another one.
    for (auto classP : classRefs) {
        if (ViewDef::HasViewDef(*classP)) {
            issues.Error(
                "Invalid view class '%s'. View query contain reference to another view class '%s' which is not supported right now.",
                classDef.GetFullName(),
                classP->GetFullName());

            return ValidationResult::Error;
        }
    }
    // Verify persistence requirement
    if (ValidationMethod::Full == method) {
        return ViewDef::ValidateFull(ecdb, classDef, query, persistenceMethod, refreshMethod, propertyMap, issues);
    }
    return ValidationResult::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
ViewDef::ValidationResult ViewDef::ValidatePropMaps(
    ECN::ECClassCR classDef,
    std::vector<Utf8String> const& propMaps,
    Issues const& issues) {
    // Verify if properties all
    std::set<Utf8String, CompareIUtf8Ascii>  notAllowedProps {
        ECDBSYS_PROP_ECClassId,
    };
    std::set<Utf8String, CompareIUtf8Ascii> validProps {
        ECDBSYS_PROP_ECInstanceId,
        // ECDBSYS_PROP_ECClassId,  This cannot be written.
        ECDBSYS_PROP_SourceECInstanceId,
        ECDBSYS_PROP_SourceECClassId,
        ECDBSYS_PROP_TargetECInstanceId,
        ECDBSYS_PROP_TargetECClassId,
    };
    for(auto prop: classDef.GetProperties()) {
        validProps.insert(prop->GetName());
    }
    auto isValidProp= [&](Utf8StringCR prop)->ValidationResult {
        if (notAllowedProps.end() != notAllowedProps.find(prop)) {
            issues.Error(
                "Invalid view class '%s'. Cannot write to property '%'. Remove it from view definition property map.",
                classDef.GetECSqlName().c_str(), prop.c_str());
            return ValidationResult::Error;
        }
        if (validProps.end() == validProps.find(prop)) {
            issues.Error(
                "Invalid view class '%s'. '%s property not found in class.",
                classDef.GetECSqlName().c_str(), prop.c_str());
            return ValidationResult::Error;
        }
        return ValidationResult::Success;
    };

    auto isValidAccessString = [&](Utf8StringCR prop)->ValidationResult {
        if (prop.empty()) {
            return ValidationResult::Success;
        }
        const auto n = prop.find(".");
        if (n == Utf8String::npos) {
            return isValidProp(prop);
        } else {
            // "prop1.p"  at this point we only check if prop1 exist on not.
            const auto firstProp = prop.substr(0, n - 1);
            return isValidProp(prop);
        }
    };
    for (auto& prop : propMaps) {
        if (prop.empty()) {
            continue;
        }
        const auto rc = isValidAccessString(prop);
        if (rc != ValidationResult::Success) {
            return rc;
        }
    }
    return ValidationResult::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
ViewDef::ValidationResult ViewDef::ValidateFull(
    ECDbCR ecdb,
    ECN::ECClassCR classDef,
    Utf8StringCR query,
    PersistenceMethod persistenceMethod,
    RefreshMethod refreshMethod,
    std::vector<Utf8String> const& propertyMaps,
    Issues const& issues) {

    // fulling require that schema import and mapping already done.
    // This should be called at the end of schema import or at runtime.
    const auto targetClassDef = ecdb.Schemas().Main().GetClass(classDef.GetSchema().GetName(), classDef.GetName());
    if (targetClassDef == nullptr) {
        issues.Error(
            "Invalid view class '%s'. Failed to find class.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }
    const auto targetClassMap = ecdb.Schemas().Main().GetClassMap(*targetClassDef);
    if (targetClassDef == nullptr) {
        issues.Error(
            "Invalid view class '%s'. Failed to find class map.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }
    if (!ecdb.Schemas().GetDerivedClasses(*targetClassDef).empty()) {
        issues.Error(
            "Invalid view class '%s'. View class definition cannot have derived classes.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }
    const auto tableType = targetClassMap->GetPrimaryTable().GetType();
    // Rule 8: Class must be map to a table which is not virtual or existing
    if (persistenceMethod == PersistenceMethod::Permanent) {
        if (tableType == DbTable::Type::Virtual || tableType == DbTable::Type::Existing) {
            issues.Error(
                "Invalid view class '%s'. View class must be mapped to non-virtual or existing table.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
    // Rule 9: Class must be map to a temp table
    } else if (persistenceMethod == PersistenceMethod::Temporary) {
        if (tableType != DbTable::Type::Temp) {
            issues.Error(
                "Invalid view class '%s'. Class must be mapped to a temp table.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
    // Rule 10: ERROR Class must be map to a virtual table.
    }  else if (persistenceMethod == PersistenceMethod::None) {
        if (tableType != DbTable::Type::Virtual) {
            issues.Error(
                "Invalid view class '%s'. TransientView class must be mapped to a virtual table.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
    } else {
        issues.Error(
            "Invalid view class '%s'. PersistenceMethod is not supported.",
            classDef.GetECSqlName().c_str());
        BeAssert(false && "unsupported persistence type");
        return ValidationResult::Error;
    }

    const auto isPersisted = persistenceMethod == PersistenceMethod::Permanent || persistenceMethod == PersistenceMethod::Temporary;
    if (isPersisted) {
        // Rule 12: Make sure all access string defined in property map are valid w.r.t class map definition
        for(const auto& prop : propertyMaps) {
            if (prop.empty()) {
                continue;
            }
            const auto propMap = targetClassMap->GetPropertyMaps().Find(prop.c_str());
            if (propMap == nullptr) {
                issues.Error(
                    "Invalid view class '%s'. Could not find property map access string '%s' in class map.",
                    classDef.GetECSqlName().c_str()),
                    prop.c_str();
                return ValidationResult::Error;
            }
        }
    }
    // Rule 11: ERROR make sure query can be prepared.
    ECSqlStatement queryStmt;
    if (ECSqlStatus::Success != queryStmt.Prepare(ecdb, query.c_str())) {
        issues.Error(
            "Invalid view class '%s'. Failed to prepare query.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }
    if (!queryStmt.IsReadonly()) {
        issues.Error(
            "Invalid view class '%s'. View query need to be select statement and should not change db in any.",
            classDef.GetECSqlName().c_str());
        return ValidationResult::Error;
    }

    if (isPersisted) {
        const auto validateMapRc = ViewDef::ValidateECSqlStatementMap(queryStmt, *targetClassMap, propertyMaps, issues);
        if (validateMapRc != ValidationResult::Success)
            return validateMapRc;

        // Rule 12: ERROR make sure insert query can be prepared.
        ECSqlStatement insertStmt;
        Utf8String insertECSql;
        if (ViewDef::CreateInsertECSql(insertECSql, *targetClassDef, propertyMaps) != BE_SQLITE_OK) {
            issues.Error(
                "Invalid view class '%s'. unable to prepare insert query base on property maps.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        ecdb.GetImpl().GetViewManager().SetWritePolicy(*targetClassDef, ViewManager::WritePolicy::Writable);
        if (ECSqlStatus::Success != insertStmt.Prepare(ecdb, insertECSql.c_str())) {
            ecdb.GetImpl().GetViewManager().SetWritePolicy(*targetClassDef, ViewManager::WritePolicy::Readonly);
            issues.Error(
                "Invalid view class '%s'. Failed to prepare insert query.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        // Rule 13: ERROR make sure delete query can be prepared.
        ECSqlStatement deleteStmt;
        Utf8String deleteECSql;
        if (ViewDef::CreateTruncateECSql(deleteECSql, *targetClassDef) != BE_SQLITE_OK) {
            issues.Error(
                "Invalid view class '%s'. unable to prepare delete statement for view",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        if (ECSqlStatus::Success != deleteStmt.Prepare(ecdb, deleteECSql.c_str())) {
            ecdb.GetImpl().GetViewManager().SetWritePolicy(*targetClassDef, ViewManager::WritePolicy::Readonly);
            issues.Error(
                "Invalid view class '%s'. Failed to prepare delete query.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
        ecdb.GetImpl().GetViewManager().SetWritePolicy(*targetClassDef, ViewManager::WritePolicy::Readonly);
    } else {

        // Rule: Transient view must have ECInstanceId and should not contain ECClassId
        ECN::ECPropertyCP instanceIdProp = nullptr;
        ECN::ECPropertyCP classIdProp = nullptr;
        for (auto i=0; i < queryStmt.GetColumnCount(); ++i) {
            const auto prop = queryStmt.GetColumnInfo(i).GetProperty();
            if (prop->GetIsPrimitive()) {
                auto prim = prop->GetAsPrimitiveProperty();
                if (instanceIdProp == nullptr && prim->GetName().EqualsIAscii(ECDBSYS_PROP_ECInstanceId)) {
                    if (prim->GetType() != PrimitiveType::PRIMITIVETYPE_Long && prim->GetType()) {
                        issues.Error(
                            "Invalid view class '%s'. Query return ECInstanceId but its type is not long",
                            classDef.GetECSqlName().c_str());
                        return ValidationResult::Error;
                    }
                    instanceIdProp = prop;
                    continue;
                }
                if (classIdProp == nullptr && prim->GetName().EqualsIAscii(ECDBSYS_PROP_ECClassId)) {
                    classIdProp = prop;
                    continue;
                }
            }
        }

        // Rule: Must have ECInstanceId property
        if (instanceIdProp == nullptr) {
            issues.Error(
                "Invalid view class '%s'. Transient view must select and return ECInstanceId property.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        // Rule: Must not have ECClassId property
        if (classIdProp != nullptr) {
            issues.Error(
                "Invalid view class '%s'. Transient view must not select and return ECClassId property.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

        // Rule: ERROR transient view must have same name as properties and type.
        auto propertyCount = queryStmt.GetColumnCount();
        if (propertyCount != (int)targetClassMap->GetPropertyMaps().Size() - 1) {  // remove ec-class property
            issues.Error(
                "Invalid view class '%s'. Transient view must have same number of properties returned my query as class have.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }
        auto typeToString = [](ECN::PrimitiveType type) -> Utf8CP {
            switch (type) {
                case PrimitiveType::PRIMITIVETYPE_Binary: return "Binary";
                case PrimitiveType::PRIMITIVETYPE_Boolean: return "Boolean";
                case PrimitiveType::PRIMITIVETYPE_DateTime: return "DateTime";
                case PrimitiveType::PRIMITIVETYPE_Double: return "Double";
                case PrimitiveType::PRIMITIVETYPE_IGeometry: return "IGeometry";
                case PrimitiveType::PRIMITIVETYPE_Integer: return "Integer";
                case PrimitiveType::PRIMITIVETYPE_Long: return "Long";
                case PrimitiveType::PRIMITIVETYPE_Point2d: return "Point2d";
                case PrimitiveType::PRIMITIVETYPE_Point3d: return "Point3d";
                case PrimitiveType::PRIMITIVETYPE_String: return "String";
            };
            return "unsupported type";
        };
        for (auto i=0; i < queryStmt.GetColumnCount(); ++i) {
            auto& queryColumnInfo = queryStmt.GetColumnInfo(i);
            auto classPropMapP = targetClassMap->GetPropertyMaps().Find(queryColumnInfo.GetProperty()->GetName().c_str());
            if (classPropMapP == nullptr) {
                issues.Error(
                    "Invalid view class '%s'. class does not have property with name '%s' as defined in query at index %d",
                    classDef.GetECSqlName().c_str(),
                    queryColumnInfo.GetProperty()->GetName().c_str(),
                    i);
                return ValidationResult::Error;
            }
            auto queryPropP = queryColumnInfo.GetProperty();
            auto&  classProp = classPropMapP->GetProperty();
            if (classPropMapP == nullptr)  {
                issues.Error(
                    "Invalid view class '%s'. class does not have property with name '%s' as defined in query at index %d",
                    classDef.GetECSqlName().c_str(),
                    queryColumnInfo.GetProperty()->GetName().c_str(),
                    i);
                return ValidationResult::Error;
            }
            // validate types

            const auto& queryDataType = queryColumnInfo.GetDataType();
            if (queryDataType.IsPrimitive() ) {
                if (!classProp.GetIsPrimitive()) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' is of primitive type but class property with same name is not primitive.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str());
                    return ValidationResult::Error;
                }
                const auto primitiveType = classProp.GetAsPrimitiveProperty()->GetType();
                if (queryDataType.GetPrimitiveType() != primitiveType) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' has type '%s' which is not same as '%s' type of class property with same name.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str(),
                        typeToString(queryDataType.GetPrimitiveType()),
                        typeToString(primitiveType));
                    return ValidationResult::Error;
                }
            } else if (queryDataType.IsStruct()) {
                if (!classProp.GetIsStruct()) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' is of struct type but class property with same name is not struct.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str());
                    return ValidationResult::Error;
                }
                const auto classStructTypeName = classProp.GetAsStructProperty()->GetTypeFullName();
                const auto queryStructTypeName = queryPropP->GetAsStructProperty()->GetTypeFullName();
                if (queryStructTypeName != classStructTypeName) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' has type '%s' which is not same as '%s' type of class property with same name.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str(),
                        queryStructTypeName.c_str(),
                        classStructTypeName.c_str());
                    return ValidationResult::Error;
                }
            } else if (queryDataType.IsPrimitiveArray()) {
                if (!classProp.GetIsPrimitiveArray()) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' is of primitive array type but class property with same name is not primitive array.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str());
                    return ValidationResult::Error;
                }
                const auto classPrimitiveArrayType = classProp.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
                const auto queryPrimitiveArrayType = queryPropP->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
                if (queryPrimitiveArrayType != classPrimitiveArrayType) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' is of type '%s[]' which is not same as '%s[]' type of class property with same name.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str(),
                        typeToString(queryPrimitiveArrayType),
                        typeToString(classPrimitiveArrayType));
                    return ValidationResult::Error;
                }
            } else if (queryDataType.IsStructArray()) {
                if (!classProp.GetIsStructArray()) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' is of struct array type but class property with same name is not struct array.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str());
                    return ValidationResult::Error;
                }
                const auto classStructArrayTypeName = classProp.GetAsStructArrayProperty()->GetTypeFullName();
                const auto queryStructArrayTypeName = queryPropP->GetAsStructArrayProperty()->GetTypeFullName();
                if (queryStructArrayTypeName != classStructArrayTypeName) {
                    issues.Error(
                        "Invalid view class '%s'. Query property '%s' is of type '%s[]' which is not same as '%s[]' type of class property with same name.",
                        classDef.GetECSqlName().c_str(),
                        queryColumnInfo.GetProperty()->GetName().c_str(),
                        queryStructArrayTypeName.c_str(),
                        classStructArrayTypeName.c_str());
                    return ValidationResult::Error;
                }
            } else {
                BeAssert("unsupported type");
                return ValidationResult::Error;
            }
        }
        // Rule 11: ERROR make sure wrap ecsql can be prepared
        Utf8String wrapQueryECSql;
        ViewDef::CreateTransientECSql(wrapQueryECSql, *targetClassDef, query);

        ECSqlStatement wrapStmt;
        if (ECSqlStatus::Success != wrapStmt.Prepare(ecdb, wrapQueryECSql.c_str())) {
            issues.Error(
                "Invalid view class '%s'. Failed to prepare transient wrapper query.",
                classDef.GetECSqlName().c_str());
            return ValidationResult::Error;
        }

    }

    return ValidationResult::Success;
}
//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
DbResult ViewDef::CreateTransientECSql(Utf8StringR wrapECSql, ECN::ECClassCR classDef, Utf8StringCR userQuery) {
    NativeSqlBuilder selectBuilder;
    selectBuilder.AppendFormatted("SELECT ECInstanceId, %s ECClassId", classDef.GetId().ToHexStr().c_str());

    for (auto prop : classDef.GetProperties()) {
        selectBuilder.AppendComma();
        selectBuilder.AppendEscaped(prop->GetName().c_str());
    }
    selectBuilder.AppendFormatted(" FROM (%s)", userQuery.c_str());
    wrapECSql = selectBuilder.GetSql();
    return BE_SQLITE_OK;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
DbResult ViewDef::CreateInsertECSql(Utf8StringR insertSql, ECN::ECClassCR classDef, std::vector<Utf8String> const& map) {
    NativeSqlBuilder insertBuilder;
    NativeSqlBuilder paramBuilder;
    insertBuilder.AppendFormatted("insert into %s (", classDef.GetECSqlName().c_str());
    paramBuilder.Append(" values (");
    int idx = 0;
    for (auto& prop : map) {
        if(prop.empty()){
            continue;
        }
        ++idx;
        if (idx > 1) {
            insertBuilder.AppendComma();
            paramBuilder.AppendComma();
        }
        insertBuilder.Append(prop);
        paramBuilder.Append("?");
    }
    if (idx == 0) {
        return BE_SQLITE_ERROR;
    }
    paramBuilder.Append(")");
    insertBuilder.Append(")").Append(paramBuilder);
    insertSql = insertBuilder.GetSql();
    return BE_SQLITE_OK;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
DbResult ViewDef::CreateTruncateECSql(Utf8StringR deleteSql, ECN::ECClassCR classDef) {
    NativeSqlBuilder deleteBuilder;
    deleteBuilder.AppendFormatted("delete from %s", classDef.GetECSqlName().c_str());
    deleteSql = deleteBuilder.GetSql();
    return BE_SQLITE_OK;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
ViewDef::ValidationResult ViewDef::ValidateECSqlStatementMap(ECSqlStatement& queryStmt, ClassMapCR targetMap, std::vector<Utf8String> const& propMaps, Issues const& issues) {
    std::map<int, PropertyMap const*> paraMap;
    auto const& classDef = targetMap.GetClass();
    int k = 0;
    for (int i=0; i< (int)propMaps.size(); ++i) {
        if (propMaps[i].empty()) {
            continue;
        }
        paraMap[++k] = targetMap.GetPropertyMaps().Find(propMaps[i].c_str());
    }
    /*
    if (k <=  queryStmt.GetColumnCount()) {
        issues.Error(
            "Invalid view class '%s'. View query has less property selected then required by class via property map." ,
            classDef.GetECSqlName().c_str()
        );
        return ValidationResult::Error;
    } */
    auto isCompilablePrimitiveType = []( ECN::PrimitiveType sourceType, ECN::PrimitiveType targetType)  {
        switch(sourceType) {
            case ECN::PRIMITIVETYPE_Binary:
                return sourceType == targetType;
            case ECN::PRIMITIVETYPE_Boolean:
                return sourceType == targetType
                    || targetType ==ECN::PRIMITIVETYPE_Double
                    || targetType ==ECN::PRIMITIVETYPE_Long
                    || targetType ==ECN::PRIMITIVETYPE_Integer;
            case ECN::PRIMITIVETYPE_DateTime:
                return sourceType == targetType
                    || targetType ==ECN::PRIMITIVETYPE_Double;
            case ECN::PRIMITIVETYPE_Double:
                return sourceType == targetType;
            case ECN::PRIMITIVETYPE_IGeometry:
                return sourceType == targetType
                    || targetType ==  ECN::PRIMITIVETYPE_Binary;
            case ECN::PRIMITIVETYPE_Integer:
                return sourceType == targetType
                    || targetType ==ECN::PRIMITIVETYPE_Double
                    || targetType ==ECN::PRIMITIVETYPE_Long;
            case ECN::PRIMITIVETYPE_Long:
                return sourceType == targetType
                    || targetType ==ECN::PRIMITIVETYPE_Double;
            case ECN::PRIMITIVETYPE_Point2d:
                return sourceType == targetType;
            case ECN::PRIMITIVETYPE_Point3d:
                return sourceType == targetType;
            case ECN::PRIMITIVETYPE_String:
                return sourceType == targetType;
        };
        BeAssert(false && "unsupported primitive type");
        return false;
    };
    for (auto& kp: paraMap) {
        const auto idx = kp.first;
        const auto propMap = kp.second;
        const auto columnInfo = queryStmt.GetColumnInfo(idx - 1);
        if (!columnInfo.IsValid()) {
            return ValidationResult::Error;
        }
        const auto& sourceType = columnInfo.GetDataType();
        const auto& targetProp = propMap->GetProperty();
        if (sourceType.IsPrimitive()) {
            if (!targetProp.GetIsPrimitive()) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            const auto primitiveProp = targetProp.GetAsPrimitiveProperty();
            if (!isCompilablePrimitiveType(sourceType.GetPrimitiveType(), primitiveProp->GetType())) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            continue; // we are good.
        }
        if (sourceType.IsStruct()) {
            if (!targetProp.GetIsStruct()) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            const auto sourceStructType = columnInfo.GetStructType();
            if (sourceStructType == nullptr) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            const auto& targetStructType = targetProp.GetAsStructProperty()->GetType();
            if (sourceStructType->GetECSqlName() == targetStructType.GetECSqlName()) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            continue; // we are good.
        }
        if (sourceType.IsNavigation()) {
            if (!targetProp.GetIsNavigation()) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            if (columnInfo.GetProperty() == nullptr) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            const auto targetRel = targetProp.GetAsNavigationProperty()->GetRelationshipClass();
            const auto sourceRel = columnInfo.GetProperty()->GetAsNavigationProperty()->GetRelationshipClass();
            if (sourceRel->GetECSqlName() == targetRel->GetECSqlName()) {
                issues.Error(
                    "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                    classDef.GetECSqlName().c_str(),
                    propMap->GetAccessString().c_str(),
                    idx);
                return ValidationResult::Error;
            }
            continue; // we are good.
        }
        if (sourceType.IsArray()) {
            if (sourceType.IsPrimitiveArray()) {
                if (!targetProp.GetIsPrimitiveArray()) {
                    issues.Error(
                        "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                        classDef.GetECSqlName().c_str(),
                        propMap->GetAccessString().c_str(),
                        idx);
                    return ValidationResult::Error;
                }
                if (sourceType.GetPrimitiveType() != targetProp.GetAsPrimitiveArrayProperty()->GetType()){
                    issues.Error(
                        "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                        classDef.GetECSqlName().c_str(),
                        propMap->GetAccessString().c_str(),
                        idx);
                    return ValidationResult::Error;
                }
                continue; // we are good.
            }
            if (sourceType.IsStructArray()) {
                if (sourceType.IsStructArray()) {
                    if (!targetProp.GetIsStructArray()) {
                        issues.Error(
                            "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                            classDef.GetECSqlName().c_str(),
                            propMap->GetAccessString().c_str(),
                            idx);
                        return ValidationResult::Error;
                    }
                    if (columnInfo.GetProperty() == nullptr) {
                        issues.Error(
                            "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                            classDef.GetECSqlName().c_str(),
                            propMap->GetAccessString().c_str(),
                            idx);
                        return ValidationResult::Error;
                    }
                    const auto sourceStructArrayProp= columnInfo.GetProperty()->GetAsStructArrayProperty();
                    const auto targetStructArrayProp= targetProp.GetAsStructArrayProperty();
                    BeAssert(sourceStructArrayProp != nullptr);
                    BeAssert(targetStructArrayProp != nullptr);
                    if (targetStructArrayProp == nullptr || sourceStructArrayProp == nullptr) {
                        issues.Error(
                            "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                            classDef.GetECSqlName().c_str(),
                            propMap->GetAccessString().c_str(),
                            idx);
                        return ValidationResult::Error;
                    }
                    if (targetStructArrayProp->GetStructElementType().GetECSqlName() != sourceStructArrayProp->GetStructElementType().GetECSqlName()){
                        issues.Error(
                            "Invalid view class '%s'. View property '%s' is incompatible with query property at index %" PRId32 "." ,
                            classDef.GetECSqlName().c_str(),
                            propMap->GetAccessString().c_str(),
                            idx);
                        return ValidationResult::Error;
                    }
                    continue; // we are good.
                }
            }
        }
    }
    return ValidationResult::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
ViewDef::SharedPtr ViewDef::Create(ViewManager& mgr, ECN::ECClassCR classDef, bool reportIssues) {
    auto& ecdb = mgr.GetECDb();
    if (ViewDef::Validate(ecdb, classDef, ValidationMethod::Full, reportIssues) != ValidationResult::Success) {
        return nullptr;
    }

    std::vector<Utf8String> propertyMaps;
    PersistenceMethod persistenceMethod = PersistenceMethod::None;
    RefreshMethod refreshMethod = RefreshMethod::None;
    Utf8String query;

    auto persistedViewDef = classDef.GetCustomAttributeLocal(VIEW_SCHEMA_Name, VIEW_CLASS_PersistedView);
    auto transientViewDef = classDef.GetCustomAttributeLocal(VIEW_SCHEMA_Name, VIEW_CLASS_TransientView);

    if (persistedViewDef.IsNull() && transientViewDef.IsNull()){
        return nullptr;
    }

    auto viewDef = persistedViewDef.IsValid() ? persistedViewDef : transientViewDef;
    if (persistedViewDef.IsValid()) {
        if (!ViewDef::TryGetPersistenceMethod(persistenceMethod, viewDef))
            return nullptr;

        if (!ViewDef::TryGetRefreshMethod(refreshMethod, viewDef))
            return nullptr;

        if (!ViewDef::TryGetPropertyMaps(propertyMaps, viewDef))  {
            return nullptr;
        }
    }

    if (!ViewDef::TryGetQuery(query, viewDef)) {
        return nullptr;
    }

    return std::make_shared<ViewDef>(mgr, classDef, query, persistenceMethod, refreshMethod, propertyMaps);
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ViewDef::TruncateView() {
    if (m_refreshMethod != RefreshMethod::Recompute) {
        return BE_SQLITE_ERROR;
    }

    Utf8String deleteECSql;
    if (ViewDef::CreateTruncateECSql(deleteECSql, m_classDef) != BE_SQLITE_OK) {
        return BE_SQLITE_ERROR;
    }

    ECSqlStatement deleteStmt;
    if (ECSqlStatus::Success != deleteStmt.Prepare(m_mgr.GetECDb(), deleteECSql.c_str())) {
        return BE_SQLITE_ERROR;
    }

    const auto rc = deleteStmt.Step();
    if (rc != BE_SQLITE_DONE)
        return rc;

    return BE_SQLITE_DONE;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ViewDef::ViewProps& ViewDef::GetViewProps() const {
    if (m_viewProps == nullptr) {
        m_viewProps = std::make_unique<ViewProps>();
        ViewDef::TryGetViewProps(*m_viewProps, m_mgr.GetECDb(), *this);
    }
    return *m_viewProps;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::NeedRefresh() const {
    if (!SupportRefresh()) {
        return false;
    }
    const auto dbDataVer = m_mgr.GetECDb().GetDataVersion();
    const auto& viewProps = GetViewProps();
    return (viewProps.m_dataVersion < dbDataVer || viewProps.m_errorCode != BE_SQLITE_OK);
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult ViewDef::RefreshData(bool forceRefresh) {
    if (!SupportRefresh()) {
        return BE_SQLITE_ERROR;
    }

    // Check if we need to refresh view;
    if (!(NeedRefresh() && !forceRefresh)) {
        LOG.infov("Class view '%s' does not require update", GetClass().GetFullName());
        return BE_SQLITE_OK;
    }

    PERFLOG_START("ECDb", "ViewDef::RefreshData");
    auto makeReadonlyAndReturn = [&] (DbResult rc) {
        m_mgr.SetWritePolicy(GetClass(), ViewManager::WritePolicy::Readonly);
        return rc;
    };

    m_mgr.SetWritePolicy(GetClass(), ViewManager::WritePolicy::Writable);
    // Delete all rows
    if (m_refreshMethod == RefreshMethod::Recompute) {
        const auto rc = TruncateView();
        if (rc != BE_SQLITE_DONE) {
            return makeReadonlyAndReturn(rc);
        }
    }

    // prepare query row
    ECSqlStatement queryStmt;
    if (ECSqlStatus::Success != queryStmt.Prepare(m_mgr.GetECDb(), m_query.c_str())) {
        return makeReadonlyAndReturn(BE_SQLITE_ERROR);
    }

    // prepare insert rows
    Utf8String insertECSql;
    if (ViewDef::CreateInsertECSql(insertECSql, m_classDef, m_propertyMaps) != BE_SQLITE_OK) {
        return makeReadonlyAndReturn(BE_SQLITE_ERROR);
    }

    ECSqlStatement insertStmt;
    if (ECSqlStatus::Success != insertStmt.Prepare(m_mgr.GetECDb(), insertECSql.c_str())) {
        return makeReadonlyAndReturn(BE_SQLITE_ERROR);
    }

    m_mgr.SetWritePolicy(GetClass(), ViewManager::WritePolicy::Readonly);
    // row copier should copy from query output to insert
    RowCopier copier(queryStmt, insertStmt);
    int64_t rowCount = 0;
    auto rc = copier.Update(rowCount);
    auto& viewProps = GetViewProps();
    if (rc != BE_SQLITE_OK) {
        TruncateView();
        viewProps.m_errorCode = rc;
        viewProps.m_errorMessage =  BeSQLiteLib::GetLogError(rc);
    }

    viewProps.m_dataVersion = m_mgr.GetECDb().GetDataVersion() + 1;
    ViewDef::SaveViewProps(m_mgr.GetECDbR(), *this, viewProps);
    PERFLOG_FINISH("ECDb", "ViewDef::RefreshData");
    return rc;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::CanQuery() const {
    if(IsTransient()){
        return true;
    }    
    return GetViewProps().m_errorCode == BE_SQLITE_OK && GetViewProps().m_dataVersion != 0ull;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus RowCopier::BindPrimitive(IECSqlValue const& source, IECSqlBinder& target, ECN::PrimitiveType primitiveType){
    if (source.IsNull()) {
        return ECSqlStatus::Success;
    }
    switch(primitiveType) {
        case ECN::PRIMITIVETYPE_IGeometry:
        case ECN::PRIMITIVETYPE_Binary: {
            int length;
            const auto v = source.GetBlob(&length);
            return target.BindBlob(v, length, m_makeCopy);
        }
        case ECN::PRIMITIVETYPE_Boolean:
            return target.BindBoolean(source.GetBoolean());
        case ECN::PRIMITIVETYPE_Double:
            return target.BindDouble(source.GetDouble());
        case ECN::PRIMITIVETYPE_Integer:
            return target.BindInt(source.GetInt());
        case ECN::PRIMITIVETYPE_Long:
            return target.BindInt64(source.GetInt64());
        case ECN::PRIMITIVETYPE_Point2d:
            return target.BindPoint2d(source.GetPoint2d());
        case ECN::PRIMITIVETYPE_Point3d:
            return target.BindPoint3d(source.GetPoint3d());
        case ECN::PRIMITIVETYPE_String:
            return target.BindText(source.GetText(), m_makeCopy);
        case ECN::PRIMITIVETYPE_DateTime:
            return target.BindDateTime(source.GetDateTime());
    };
    return ECSqlStatus::Error;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus RowCopier::BindStruct(IECSqlValue const& source, IECSqlBinder& target){
    if (source.IsNull()) {
        return ECSqlStatus::Success;
    }
    for(auto& v : source.GetStructIterable()) {
        const auto propId = v.GetColumnInfo().GetPropertyPath().At(0).GetProperty()->GetId();
        const auto rc = BindValue(v, target[propId]);
        if (rc != ECSqlStatus::Success){
            return rc;
        }
    }
    return ECSqlStatus::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus RowCopier::BindStructArray(IECSqlValue const& source, IECSqlBinder& target){
    if (source.IsNull()) {
        return ECSqlStatus::Success;
    }
    for(auto& v : source.GetArrayIterable()) {
        const auto rc = BindStruct(v, target.AddArrayElement());
        if (rc != ECSqlStatus::Success){
            return rc;
        }
    }
    return ECSqlStatus::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus RowCopier::BindPrimitiveArray(IECSqlValue const& source, IECSqlBinder& target){
    if (source.IsNull()) {
        return ECSqlStatus::Success;
    }
    const auto type = source.GetColumnInfo().GetDataType().GetPrimitiveType();
    for(auto& v : source.GetArrayIterable()) {
        auto rc = BindPrimitive(v, target.AddArrayElement(), type);
        if (rc != ECSqlStatus::Success){
            return rc;
        }
    }
    return ECSqlStatus::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus RowCopier::BindValue(IECSqlValue const& source, IECSqlBinder& target){
    if (source.IsNull()) {
        return ECSqlStatus::Success;
    }

    const auto dataType = source.GetColumnInfo().GetDataType();
    if (dataType.IsPrimitive()) {
        return BindPrimitive(source, target, dataType.GetPrimitiveType());
    }
    if (dataType.IsStruct()) {
        return BindStruct(source, target);
    }
    if (dataType.IsStructArray()) {
        return BindStructArray(source, target);
    }
    if (dataType.IsPrimitiveArray()) {
        return BindPrimitiveArray(source, target);
    }
    return ECSqlStatus::Error;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus RowCopier::BindValues() {
    for(int i=0; i< (int)m_reader.GetColumnCount(); ++i){
        const auto rc = BindValue(m_reader.GetValue(i), m_writer.GetBinder(i + 1));
        if (rc != ECSqlStatus::Success)
            return rc;
    }
    return ECSqlStatus::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::SupportRefresh() const {
    return (m_persistenceMethod == PersistenceMethod::Permanent ||
            m_persistenceMethod == PersistenceMethod::Temporary) && (m_refreshMethod == RefreshMethod::Recompute) ;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
DbResult RowCopier::Update(int64_t& rowCount) {
    BeMutexHolder holder(m_reader.GetECDb()->GetImpl().GetMutex());
    rowCount = 0;
    DbResult rc = m_reader.Step();
    if (rc != BE_SQLITE_ROW && rc != BE_SQLITE_DONE) {
        return rc;
    }
    while(rc == BE_SQLITE_ROW) {
        m_writer.Reset();
        m_writer.ClearBindings();
        const auto bindRc = BindValues();
        if (bindRc != ECSqlStatus::Success) {
            if (bindRc.IsSQLiteError())
                return bindRc.GetSQLiteError();
            return BE_SQLITE_ERROR;
        }
        rc =  m_writer.Step();
        if (rc != BE_SQLITE_DONE) {
            return rc;
        }
        ++rowCount;
        rc = m_reader.Step();
    }
    return BE_SQLITE_OK;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ViewManager::LoadAndCacheViewDefs(bool forced) const {
    BeMutexHolder holder(m_ecdb.GetImpl().GetMutex());
    if (forced) {
        ClearCache();
    } else if (m_initialized) {
        return;
    }

    m_loading.store(true);
    PERFLOG_START("ECDb", "Loading view definitions");
    for (auto viewClass : ViewManager::GetViewClasses(m_ecdb)) {
        auto viewDef = ViewDef::Create(const_cast<ViewManager&>(*this), *viewClass, true /* report issues */);
        if (viewDef != nullptr) {
            LOG.infov("Loaded view definition for %s successfully.", viewClass->GetFullName());
            m_cachedViewDef[viewDef->GetClass().GetId()] = viewDef;
        } else {
            LOG.errorv("Failed to load view definition for class %s.", viewClass->GetFullName());
        }

    }
    m_initialized = true;
    m_loading.store(false);
    PERFLOG_FINISH("ECDb", "Loading view definitions");
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
void ViewManager::ClearCache() const {
     BeMutexHolder holder(m_ecdb.GetImpl().GetMutex());
     m_cachedViewDef.clear();
     m_writePolicy.clear();
     m_initialized = false;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ViewManager::ViewManager(ECDbR ecdb):m_ecdb(ecdb), m_initialized(false){}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ViewManager::ClassList ViewManager::GetViewClasses (bool onlyValidViewClasses) const {
    LoadAndCacheViewDefs(/* forced = */ false);
    ClassList classes;
    if (onlyValidViewClasses) {
        for (auto& kv : m_cachedViewDef) {
            if ( kv.second != nullptr) {
                classes.push_back(&kv.second->GetClass());
            }
        }
        return classes;
    }
    for (auto& kv : m_cachedViewDef) {
        auto classP = m_ecdb.Schemas().GetClass(kv.first);
        if (classP) {
            classes.push_back(classP);
        }
    }
    return classes;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewManager::ValidateViews(ClassList& failedViews, ClassList& validViews, bool reportIssues) const {
    for(auto viewClass : ViewManager::GetViewClasses(m_ecdb)) {
        if (ViewDef::Validate(m_ecdb, *viewClass, ViewDef::ValidationMethod::Full, reportIssues) == ViewDef::ValidationResult::Error) {
            failedViews.push_back(viewClass);
        } else {
            validViews.push_back(viewClass);
        }
    }
    return failedViews.empty();
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ViewDef::WeakPtr ViewManager::GetViewDef(ECN::ECClassCR viewClass) const {
    LoadAndCacheViewDefs(/* forced = */ false);
    auto it = m_cachedViewDef.find(viewClass.GetId());
    if (it != m_cachedViewDef.end()) {
        return ViewDef::WeakPtr(it->second);
    }
    return ViewDef::WeakPtr();
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
ViewManager::ClassList ViewManager::GetViewClasses(ECDbCR ecdb) {
    ClassList viewClasses;
    Statement stmt;
    const auto rc = stmt.Prepare(ecdb, R"(
        SELECT
            [ca].[ContainerId]
        FROM   [ec_class] [cs]
            JOIN [ec_schema] [sc] ON [cs].[schemaId] = [sc].[Id]
            JOIN [ec_CustomAttribute] [ca] ON [ca].[ClassId] = [cs].[Id]
        WHERE  [ca].[ContainerType] = ?1
                AND [sc].[Name] = ?2
                AND ([cs].[Name] = ?3 OR [cs].[Name] = ?4);
    )");
    if (rc != BE_SQLITE_OK) {
        BeAssert(false && "unable to prepare statement");
        return viewClasses;
    }
    stmt.BindInt(1, (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    stmt.BindText(2, VIEW_SCHEMA_Name, Statement::MakeCopy::No);
    stmt.BindText(3, VIEW_CLASS_PersistedView, Statement::MakeCopy::No);
    stmt.BindText(4, VIEW_CLASS_TransientView, Statement::MakeCopy::No);
    while(BE_SQLITE_ROW == stmt.Step()) {
        const auto classId = stmt.GetValueId<ECN::ECClassId>(0);
        const auto classP = ecdb.Schemas().GetClass(classId);
        if (classP != nullptr) {
            viewClasses.push_back(classP);
        }
    }
    return viewClasses;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::TryGetViewProps(ViewProps& props, ECDbCR ecdb, ViewDef const& viewDef) {
    props.m_dataVersion = 0ull;
    props.m_errorCode = BE_SQLITE_OK;
    props.m_errorMessage.clear();

    if (viewDef.IsTransient()) {
        return true;
    }

    auto spec = PropertySpec(
        viewDef.GetClass().GetFullName(),
        VIEW_PROP_Namespace,
        PropertySpec::Mode::Normal,
        PropertySpec::Compress::No);

    Utf8String viewPropJson;
    if (BE_SQLITE_ROW != ecdb.QueryProperty(viewPropJson, spec)) {
        return false;
    }

    BeJsDocument doc;
    doc.Parse(viewPropJson);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }

    if (!doc.hasMember(VIEW_PROP_DataVersion) || !doc.hasMember(VIEW_PROP_ErrorCode) ) {
        return false;
    }

    props.m_dataVersion = doc[VIEW_PROP_DataVersion].asUInt64();
    props.m_errorCode = Enum::FromInt<DbResult>(doc[VIEW_PROP_ErrorCode].asInt(Enum::ToInt<DbResult>(BE_SQLITE_OK)));
    if (props.m_errorCode != BE_SQLITE_OK) {
        props.m_errorMessage = doc[VIEW_PROP_ErrorMsg].asString();
    } else {
        props.m_errorMessage.clear();
    }

    return true;
}

//--------------------------------------------------------------------------------------
// @bsimethod
// static
//---------------+---------------+---------------+---------------+---------------+------
bool ViewDef::SaveViewProps(ECDbR ecdb, ViewDef const& viewDef,  ViewProps const & props) {
    if (viewDef.IsTransient()) {
        return true;
    }

    auto spec = PropertySpec(
        viewDef.GetClass().GetFullName(),
        VIEW_PROP_Namespace,
        PropertySpec::Mode::Normal,
        PropertySpec::Compress::No);

    BeJsDocument doc;
    doc.SetEmptyObject();
    doc[VIEW_PROP_DataVersion] =  BeInt64Id(props.m_dataVersion).ToHexStr();
    doc[VIEW_PROP_ErrorCode] = Enum::ToInt<DbResult>(props.m_errorCode);
    doc[VIEW_PROP_ErrorMsg] = props.m_errorMessage;

    const auto json = doc.Stringify();
    return ecdb.SaveProperty(spec, json, nullptr, 0) == BE_SQLITE_DONE;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewManager::SetWritePolicy(ECN::ECClassCR viewClass, WritePolicy policy) const {
    BeMutexHolder holder(m_ecdb.GetImpl().GetMutex());
    if (!ViewDef::HasPersistedViewDef(viewClass)) {
        return false;
    }
    m_writePolicy[viewClass.GetId()] = policy;
    return true;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewManager::GetWritePolicy(WritePolicy& policy, ECN::ECClassCR viewClass) const {
    BeMutexHolder holder(m_ecdb.GetImpl().GetMutex());
    if (!ViewDef::HasViewDef(viewClass)) {
        return false;
    }
    if (ViewDef::HasTransientViewDef(viewClass)) {
        policy = WritePolicy::Readonly;
        return true;
    }

    if (m_writePolicy.find(viewClass.GetId()) == m_writePolicy.end()) {
        m_writePolicy[viewClass.GetId()] = WritePolicy::Readonly;
    }
    policy = m_writePolicy[viewClass.GetId()];
    return true;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ViewManager::RefreshViews() const {
    PERFLOG_START("ECDB", "ViewManager::RefreshViews()");
    LoadAndCacheViewDefs(/* forced = */ false);
    int failedViewCount = 0;
    for(auto& kv : m_cachedViewDef) {
        if (kv.second == nullptr) {
            continue;
        }
        auto& viewDef = kv.second;
        if (!viewDef->SupportRefresh()) {
            continue;
        }
        if (viewDef->RefreshData() != BE_SQLITE_OK) {
            ++failedViewCount;
        }
    }
    PERFLOG_FINISH("ECDB", "ViewManager::RefreshViews()");
    return failedViewCount >0  ? ERROR :SUCCESS;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
bool ViewManager::HasTransientViews() const {
     BeMutexHolder holder(m_ecdb.GetImpl().GetMutex());
    LoadAndCacheViewDefs(false);
    for(auto& v: m_cachedViewDef){
        if (v.second->IsTransient()) {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
int ViewManager::SubstituteTransientViews(Utf8StringR out, Utf8StringCR originalQuery) const {
    BeMutexHolder holder(m_ecdb.GetImpl().GetMutex());
    if (!HasTransientViews()) {
        out = originalQuery;
        return 0;
    }

    re2::RE2::Options opt;
    opt.set_case_sensitive(false);
    opt.set_log_errors(false);


    re2::RE2 re(R"regex((?P<name>(?:\[?\s*)(?P<alias>[_\w\d]+)\s*\]?\.\[?\s*(?P<class>[_\w\d]+)\s*\]?))regex", opt);
    std::string fullName;
    std::string schemaName;
    std::string className;
    re2::StringPiece text = originalQuery;
    auto tBegin = &originalQuery[0];
    int nSubstitutionCount = 0;
    while (RE2::FindAndConsume(&text, re, &fullName, &schemaName, &className)) {
        const size_t tLength = (text.data() - tBegin -  fullName.length()+ 1);
        out.append(tBegin, tLength);
        tBegin = text.data();
        const auto classP = m_ecdb.Schemas().GetClass(schemaName, className, SchemaLookupMode::AutoDetect);
        if (classP != nullptr) {
            auto vc = m_cachedViewDef.find(classP->GetId());
            if (vc != m_cachedViewDef.end()) {
                auto viewDef = vc->second;
                if (viewDef->IsTransient()) {
                    out.append("(").append( viewDef->CreateTransientECSql()).append(")");
                    ++nSubstitutionCount;
                    continue;
                }
            }
        }
        out.append(fullName);
    }

   out.append(tBegin);
   return nSubstitutionCount;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
std::set<ECN::ECClassCP> ViewDef::GetClassReferencedInECSql(ECDbCR ecdb , Utf8StringCR query) {
    std::set<ECN::ECClassCP> classes;
    re2::RE2::Options opt;
    opt.set_case_sensitive(false);
    opt.set_log_errors(false);

    re2::RE2 re(R"regex((?P<name>(?:\[?\s*)(?P<alias>[_\w\d]+)\s*\]?\.\[?\s*(?P<class>[_\w\d]+)\s*\]?))regex", opt);
    std::string fullName;
    std::string schemaName;
    std::string className;
    re2::StringPiece text = query;
    while (RE2::FindAndConsume(&text, re, &fullName, &schemaName, &className)) {
        const auto classP = ecdb.Schemas().GetClass(schemaName, className, SchemaLookupMode::AutoDetect);
        if (classP != nullptr) {
            classes.insert(classP);
        }
    }
    return classes;
}
END_BENTLEY_SQLITE_EC_NAMESPACE
