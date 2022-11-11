/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//****************************** TableValuedFunctionExp *********************************
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Exp::FinalizeParseStatus TableValuedFunctionExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
    if (FinalizeParseMode::BeforeFinalizingChildren == mode) {
        const auto& vsm = ctx.GetECDb().Schemas().Main().GetVirtualSchemaManager();
        const auto classValuedFunc = GetFunctionExp()->GetFunctionName();
        const auto tableViewClassP = vsm.GetClass(m_schemaName, classValuedFunc);
        if (tableViewClassP == nullptr) {
            ctx.Issues().ReportV(
                IssueSeverity::Error, 
                IssueCategory::BusinessProperties, 
                IssueType::ECDbIssue,
                "TableValuedFunction %s.%s() has no ECClass describing its output.", m_schemaName.c_str(), classValuedFunc.c_str());
            return Exp::FinalizeParseStatus::Error;
        }
        m_virtualEntityClass = tableViewClassP->GetEntityClassCP();
        return Exp::FinalizeParseStatus::Completed;
    }
    return Exp::FinalizeParseStatus::NotCompleted;
}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR TableValuedFunctionExp::_GetId() const {
    if (GetAlias().empty())
        return GetFunctionExp()->GetFunctionName();

    return GetAlias();
}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TableValuedFunctionExp::_ExpandSelectAsterisk(
    std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const&) const {
    BeAssert(m_virtualEntityClass != nullptr);
    auto alias = GetAlias();
    for(auto& prop : m_virtualEntityClass->GetProperties()) {
        PropertyPath path;
        path.Push(prop->GetName());
        path.SetPropertyDef(0, *prop);

        auto exp = std::make_unique<PropertyNameExp>(path,  *this, *prop);
        expandedSelectClauseItemList.push_back(
            std::make_unique<DerivedPropertyExp>(std::move(exp), nullptr)); 
    }
}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PropertyMatchResult TableValuedFunctionExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const& propertyPath, const PropertyMatchOptions& options) const {
    BeAssert(m_virtualEntityClass != nullptr);
    if (propertyPath.Size() == 1) {
       auto property = m_virtualEntityClass->GetPropertyP(propertyPath.First().GetName());
        if (property == nullptr) {
            return PropertyMatchResult::NotFound();
        }
        auto resolvedPath = propertyPath;
        resolvedPath.SetPropertyDef(0, *property);
        return PropertyMatchResult(options, propertyPath, resolvedPath, *property, 1);
    }
    if (propertyPath.Size() == 2) {
        auto property = m_virtualEntityClass->GetPropertyP(propertyPath.Last().GetName());
        if (property == nullptr) {
            return PropertyMatchResult::NotFound();
        }
        if (!GetAlias().EqualsIAscii(propertyPath.First().GetName())) {
            PropertyMatchResult::NotFound();
        }
        auto resolvedPath = propertyPath.Skip(1);
        resolvedPath.SetPropertyDef(0, *property);
        return PropertyMatchResult(options, propertyPath, resolvedPath, *property, 1);
    }  
    return PropertyMatchResult::NotFound();
}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TableValuedFunctionExp::_ToECSql(ECSqlRenderContext&) const{

}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String TableValuedFunctionExp::_ToString () const {
    return "";
}
/*---------------------------------------------------------------------------------------
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TableValuedFunctionExp::TableValuedFunctionExp (Utf8StringCR schemaName, std::unique_ptr<MemberFunctionCallExp> func, PolymorphicInfo polymorphic) 
    : RangeClassRefExp (Exp::Type::TableValuedFunction, polymorphic), m_schemaName(schemaName){
        AddChild(std::move(func));
}
//****************************** ClassNameExp *****************************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus ClassNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        {
        if (m_info == nullptr)
            {
            BeAssert(false);
            return FinalizeParseStatus::Error;
            }

        return FinalizeParseStatus::Completed;
        }

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassNameExp::_ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const& ctx) const
    {
    if (m_info == nullptr)
        {
        BeAssert(false);
        return;
        }

    ClassMap const& classMap = m_info->GetMap();
    for (PropertyMap const* propertyMap : classMap.GetPropertyMaps())
        {
        std::unique_ptr<PropertyNameExp> exp = std::make_unique<PropertyNameExp>(ctx, propertyMap->GetAccessString(), *this, classMap);
        expandedSelectClauseItemList.push_back(std::make_unique<DerivedPropertyExp>(std::move(exp), nullptr));
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyMatchResult ClassNameExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const& propertyPath, const PropertyMatchOptions& options) const {
    if (m_info == nullptr) {
        BeAssert(false);
        return PropertyMatchResult::NotFound();
        }

    BeAssert(!propertyPath.IsEmpty());
    if (propertyPath.IsEmpty())
        return PropertyMatchResult::NotFound();

    ECN::ECClassCR ecClass = GetInfo().GetMap().GetClass();
    auto const &firstPathComp = propertyPath.First();
    const bool classHasAlias = !GetAlias().empty();
    const bool matchClassAlias = classHasAlias && firstPathComp.GetName().EqualsIAscii(GetAlias());

    // RULE 2 - property path has alteast two accessor
    if (propertyPath.Size() == 2) {
        // RULE 2.1 - first accessor matches alias. Alias can only exist for classes and not schemas
        // RULE 2.2 - first access matches class name
        const bool matchClass = firstPathComp.GetName().EqualsIAscii(ecClass.GetName());
        if ( matchClassAlias || matchClass) {
            const bool classAliasWasIgnored = classHasAlias && !matchClassAlias && matchClass;
            PropertyPath effectivePath = propertyPath.Skip(1);
            PropertyMap const* propertyMap = GetInfo().GetMap().GetPropertyMaps().Find(effectivePath.ToString().c_str());
            if (propertyMap) {
                effectivePath.Resolve(GetInfo().GetMap());
                return PropertyMatchResult(options, propertyPath, effectivePath, *propertyMap, classAliasWasIgnored? -1 : 0 );
            }
        }
    }

    // RULE 3 - property path has alteast three access so it could be schema.class.property
    int propertyIndex = -1; // where in property path the property path start
    if (propertyPath.Size() > 2) {
        auto const &secondPathComp = propertyPath[1];
        bool classAliasWasIgnored = false;
        // RULE 3.1 See if first component is a schema or schema alias
        const bool matchSchema = matchClassAlias ||
                                    firstPathComp.GetName().EqualsIAscii(ecClass.GetSchema().GetName()) || 
                                    firstPathComp.GetName().EqualsIAscii(ecClass.GetSchema().GetAlias());
        if (matchSchema) {
            if (matchClassAlias) {
                propertyIndex = 1;
                classAliasWasIgnored = classHasAlias;
            } else {
                const bool matchClass =  secondPathComp.GetName().EqualsIAscii(ecClass.GetName());
                if (matchClass) {
                    propertyIndex = 2;
                    classAliasWasIgnored = classHasAlias;
                }
            }
        } else {
            // RULE 3.1 see if first component is alias or class name
            const bool matchAlias = classHasAlias && firstPathComp.GetName().EqualsIAscii(GetAlias());
            const bool matchClass = firstPathComp.GetName().EqualsIAscii(ecClass.GetName());
            // 0 - Class | Alias
            if (matchAlias || matchClass) {
                propertyIndex = 1;
                classAliasWasIgnored = classHasAlias && !matchAlias && matchClass;
            }
        }

        if (propertyIndex >= 0 ) {
            // we have a match and we also have effect propertyPath
            PropertyPath effectivePath = propertyPath.Skip(propertyIndex);
            auto propertyMap = GetInfo().GetMap().GetPropertyMaps().Find(effectivePath.ToString().c_str());
            if (propertyMap != nullptr) {
                effectivePath.Resolve(GetInfo().GetMap());
                return PropertyMatchResult(options, propertyPath, effectivePath, *propertyMap, classAliasWasIgnored? -1 : 0);
            }
        }
    }
    // RULE 1 - try acces string as is 
    auto propertyMap = GetInfo().GetMap().GetPropertyMaps().Find(propertyPath.ToString().c_str());
    if (propertyMap != nullptr) {
        PropertyPath effectivePath = propertyPath;
        effectivePath.Resolve(GetInfo().GetMap());
        return PropertyMatchResult(options, propertyPath, effectivePath, *propertyMap, 0);
    }

    // not found return empty result
    return PropertyMatchResult::NotFound();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ClassNameExp::ClassNameExp(Utf8StringCR className, Utf8StringCR schemaAlias, Utf8CP tableSpace, std::shared_ptr<Info> info, PolymorphicInfo polymorphic, std::unique_ptr<MemberFunctionCallExp> memberFuntionCall, bool disqualifyPrimaryJoin)
    : RangeClassRefExp(Type::ClassName, polymorphic), m_className(className), m_schemaAlias(schemaAlias), m_tableSpace(tableSpace), m_info(info),m_disqualifyPrimaryJoin(disqualifyPrimaryJoin)
    {
    if (memberFuntionCall)
        AddChild(std::move(memberFuntionCall));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ClassNameExp::_GetId() const
    {
    if (GetAlias().empty())
        return m_className;

    return GetAlias();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassNameExp::GetFullName() const
    {
    Utf8String fullName;
    if (!m_tableSpace.empty())
        fullName.append("[").append(m_tableSpace).append("].");

    if (!m_schemaAlias.empty())
        fullName.append("[").append(m_schemaAlias).append("].");

    fullName.append("[").append(m_className).append("]");

    return fullName;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ClassNameExp::_ToString() const
    {
    Utf8String str("ClassName [TableSpace: ");
    str.append(m_tableSpace.empty() ? "-" : m_tableSpace).append(", Schema alias: ").append(m_schemaAlias);
    str.append(", Class: ").append(m_className).append(", Alias: ").append(GetAlias()).append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ClassNameExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    const auto polymorphicInfo = GetPolymorphicInfo().ToECSql();
    if (!polymorphicInfo.empty())
        ctx.AppendToECSql(polymorphicInfo).AppendToECSql(" ");

    ctx.AppendToECSql(GetFullName());
    if (!GetAlias().empty())
        ctx.AppendToECSql(" ").AppendToECSql(GetAlias());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool PolymorphicInfo::TryParseToken(Type& type, Utf8StringCR str) 
    {
        if (str.EqualsIAscii("ONLY")) {
            type = Type::Only;
            return true;
        }
        if (str.EqualsIAscii("ALL")) {
            type = Type::All;
            return true;
        }
        return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PolymorphicInfo::ToECSql() const 
    {
    Utf8String ecsql;
    if (m_disqualify)
        {
        ecsql.append("+");
        }
    if (m_type == Type::Only)
        {
        ecsql.append("ONLY");
        }
    if (m_disqualify && m_type == Type::All)
        {
        ecsql.append("ALL");
        }
    return ecsql;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PolymorphicInfo PolymorphicInfo::Only()
    {
    static PolymorphicInfo ct(Type::Only, false);
    return ct;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PolymorphicInfo PolymorphicInfo::All()
    {
    static PolymorphicInfo ct(Type::All, false);
    return ct;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
