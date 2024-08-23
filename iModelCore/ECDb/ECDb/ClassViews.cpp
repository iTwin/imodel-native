/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSql/NativeSqlBuilder.h"
#include <functional>


USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsimethod
//======================================================================================
bool ClassViews::IsViewClass(ECN::ECClassCR viewClass) {
    return viewClass.GetCustomAttributeLocal(ViewSchemaName, ViewClassName).IsValid();
}

//======================================================================================
// @bsimethod
//======================================================================================
bool ClassViews::TryGetQuery(Utf8StringR query, ECN::ECClassCR viewClass) {
    auto viewCA = viewClass.GetCustomAttributeLocal(ViewSchemaName, ViewClassName);
    if (!viewCA.IsValid())
        return false;

    ECN::ECValue v;
    if (ECObjectsStatus::Success != viewCA->GetValue(v, ViewClassQueryProp))
        return false;

    query = v.GetUtf8CP();
    query.Trim();
    if (query.empty())
        return false;

    return true;
}

//======================================================================================
// @bsimethod
//======================================================================================
bvector<ECN::ECClassCP> ClassViews::FindViewClasses(ECDbCR conn) {
    bvector<ECN::ECClassCP> viewClasses;
    Statement stmt;
    const auto rc = stmt.Prepare(conn, R"(
        SELECT
            [ca].[ContainerId]
        FROM   [ec_class] [cs]
            JOIN [ec_schema] [sc] ON [cs].[schemaId] = [sc].[Id]
            JOIN [ec_CustomAttribute] [ca] ON [ca].[ClassId] = [cs].[Id]
        WHERE  [ca].[ContainerType] = ?1
                AND [sc].[Name] = ?2
                AND ([cs].[Name] = ?3);
    )");

    if (rc != BE_SQLITE_OK) {
        BeAssert(false && "unable to prepare statement");
        return viewClasses;
    }

    stmt.BindInt(1, (int)SchemaPersistenceHelper::GeneralizedCustomAttributeContainerType::Class);
    stmt.BindText(2, ViewSchemaName, Statement::MakeCopy::No);
    stmt.BindText(3, ViewClassName, Statement::MakeCopy::No);
    while(BE_SQLITE_ROW == stmt.Step()) {
        const auto classId = stmt.GetValueId<ECN::ECClassId>(0);
        const auto classP = conn.Schemas().GetClass(classId);
        if (classP != nullptr) {
            viewClasses.push_back(classP);
        }
    }
    return viewClasses;
}

//======================================================================================
// @bsimethod
//======================================================================================
bool ClassViews::CheckViews(ECDbCR conn) {
    int invalidViews = 0;
    auto dbViews = FindViewClasses(conn);
    for (auto viewClass : FindViewClasses(conn)) {
        if (!IsValid(*viewClass, conn)) {
            ++invalidViews;
        }
    }
    if (invalidViews > 0) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0700, "Total of %d view classes were checked and %d were found to be invalid.", (int)dbViews.size(), invalidViews);
        return false;
    }
    return invalidViews == 0;
}
//======================================================================================
// @bsimethod
//======================================================================================
bool ClassViews::IsValid(ECN::ECClassCR viewClass, ECDbCR conn){
    auto viewClassName = viewClass.GetFullName();
    if (!viewClass.IsEntityClass() && !viewClass.IsRelationshipClass()) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0701, "Invalid view class '%s'. View definition can only be applied to EntityClass or RelationshipClass.", viewClassName);
        return false;
    }

    auto classMap = conn.Schemas().Main().GetClassMap(viewClass);
    if (classMap == nullptr) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0716,"Invalid view class '%s'. View class does not have a classmap", viewClassName);
        return false;
    }
    if (viewClass.IsRelationshipClass()) {
        auto relationshipClass = viewClass.GetRelationshipClassCP();
        if (!IsViewClass(*relationshipClass->GetTarget().GetAbstractConstraint()) || !IsViewClass(*relationshipClass->GetSource().GetAbstractConstraint())) {
            conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0722,"Invalid view class '%s'. If view is applied to the ECRelationshipClass, then source and target classes must be view classes as well.", viewClassName);
            return false;
        }
    }
    if (classMap->GetPrimaryTable().GetType() != DbTable::Type::Virtual) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0717,"Invalid view class '%s'. View class must be mapped to virtual table and not physical table.", viewClassName);
        return false;
    }
    if (viewClass.GetClassModifier() != ECClassModifier::Abstract) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0702, "Invalid view class '%s'. 'QueryView' customattribute must be applied to a 'Abstract' class.", viewClassName);
        return false;
    }
    if (viewClass.HasBaseClasses()) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0703, "Invalid view class '%s'. 'QueryView' cannot be derived from another class", viewClassName);
        return false;
    }
    if (!conn.Schemas().GetDerivedClasses(viewClass).empty()) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0704, "Invalid view class '%s'. View class definition cannot have derived classes.", viewClassName);
        return false;
    }
    Utf8String ecsql;
    if (!TryGetQuery(ecsql, viewClass)) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0705, "Invalid view class '%s'. No query specified in view", viewClassName);
        return false;
    }
    ECSqlStatement stmt;
    if (ECSqlStatus::Success != stmt.Prepare(conn, ecsql.c_str())){
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0706, "Invalid view class '%s'. Failed to prepare view query (%s)", viewClassName, ecsql.c_str());
        return false;
    }
    std::set<Utf8String, CompareIUtf8Ascii> viewClassProps;
    for (auto viewClassProp : viewClass.GetProperties()) {
        int queryPropIndex = -1;
        for (auto i = 0; i < stmt.GetColumnCount(); ++i) {
            if (viewClassProp->GetName().EqualsIAscii(stmt.GetColumnInfo(i).GetProperty()->GetName())) {
                queryPropIndex = i;
                break;
            }
        }
        if (queryPropIndex<0) {
            conn.GetImpl().Issues().ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECDbIssue, ECDbIssueId::ECDb_0707, "Invalid view class '%s'. View class has property '%s' which is not returned by view query.", viewClassName, viewClassProp->GetName().c_str());
            return false;
        }

        auto queryProp = stmt.GetColumnInfo(queryPropIndex).GetProperty();
        if (viewClassProp->GetTypeFullName() != queryProp->GetTypeFullName()) {
            conn.GetImpl().Issues().ReportV(
                    IssueSeverity::Error,
                    IssueCategory::BusinessProperties,
                    IssueType::ECDbIssue, ECDbIssueId::ECDb_0708, "Invalid view class '%s'. View class property '%s' type does not match the type returned by view query ('%s' <> '%s').",
                    viewClassName,
                    queryProp->GetName().c_str(),
                    viewClassProp->GetTypeFullName().c_str(),
                    queryProp->GetTypeFullName().c_str());
            return false;
        }
        viewClassProps.insert(viewClassProp->GetName());
    }

    bool hasECInstanceId = false;
    bool hasECClassId = false;
    // check if non-data columns are valid
    for (auto i = 0; i < stmt.GetColumnCount(); ++i) {
        const auto colInfo = stmt.GetColumnInfo(i);
        const auto sysProp = colInfo.GetProperty();
        if (viewClassProps.find(sysProp->GetName()) == viewClassProps.end()) {
            const auto isSys = colInfo.IsSystemProperty();
            auto colName =  sysProp->GetName(); //Selecting any value as ECInstanceId or ECClassId is allowed
            if(colName.EqualsI(ECDBSYS_PROP_ECInstanceId)) {
                if(!colInfo.GetDataType().IsPrimitive() ||
                    (colInfo.GetDataType().GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Long &&
                     colInfo.GetDataType().GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Integer)) {
                    conn.GetImpl().Issues().ReportV(
                            IssueSeverity::Error,
                            IssueCategory::BusinessProperties,
                            IssueType::ECDbIssue, ECDbIssueId::ECDb_0725, "Invalid view class '%s'. ECInstanceId must be a primitive integer or long.", viewClassName);
                    return false;
                }

                hasECInstanceId = true;
                continue;
            }

            if(colName.EqualsI(ECDBSYS_PROP_ECClassId)) {
                if(!colInfo.GetDataType().IsPrimitive() ||
                    (colInfo.GetDataType().GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Long &&
                     colInfo.GetDataType().GetPrimitiveType() != PrimitiveType::PRIMITIVETYPE_Integer)) {
                    conn.GetImpl().Issues().ReportV(
                            IssueSeverity::Error,
                            IssueCategory::BusinessProperties,
                            IssueType::ECDbIssue, ECDbIssueId::ECDb_0726, "Invalid view class '%s'. ECClassId must be a primitive integer or long.", viewClassName);
                    return false;
                }
                hasECClassId = true;
                continue;
            }

            if (isSys) { //returning a system property is allowed
                continue;
            }

            conn.GetImpl().Issues().ReportV(
                        IssueSeverity::Error,
                        IssueCategory::BusinessProperties,
                        IssueType::ECDbIssue, ECDbIssueId::ECDb_0709, "Invalid view class '%s'. View query returns property '%s' which not defined in view class or is a invalid system property.", viewClassName, sysProp->GetName().c_str());
            return false;
        }
    }

    if(!hasECInstanceId) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0727, "Invalid view class '%s'. View query must return ECInstanceId.", viewClassName);
        return false;
    }

    if(!hasECClassId) {
        conn.GetImpl().Issues().ReportV(
                IssueSeverity::Error,
                IssueCategory::BusinessProperties,
                IssueType::ECDbIssue, ECDbIssueId::ECDb_0728, "Invalid view class '%s'. View query must return ECClassId.", viewClassName);
        return false;
    }

    return true;
}

END_BENTLEY_SQLITE_EC_NAMESPACE
