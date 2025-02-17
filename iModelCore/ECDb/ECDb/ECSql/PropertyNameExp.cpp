/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//****************************** ExtractPropertyValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void ExtractPropertyValueExp::_ToECSql(ECSqlRenderContext& ctx) const {
        ctx.AppendToECSql(GetInstancePath().ToString().c_str());
        ctx.AppendToECSql("->");
        ctx.AppendToECSql(m_targetPath.ToString().c_str());
        if (m_isOptionalProp) {
            ctx.AppendToECSql("?");
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void ExtractPropertyValueExp::_ToJson(BeJsValue val, JsonFormat const&) const {
    //! ITWINJS_PARSE_TREE: ExtractPropertyValueExp
    val.SetEmptyObject();
    val["id"] = "PropertyNameExp";
    ECSqlRenderContext ctx;
    _ToECSql(ctx);
    val["path"] = ctx.GetECSql();
    if (m_isOptionalProp){
        val["isOptional"] = m_isOptionalProp;
    }
}
//****************************** ExtractInstanceValueExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void ExtractInstanceValueExp::_ToECSql(ECSqlRenderContext& ctx) const{
    ctx.AppendToECSql(GetInstancePath().ToString().c_str());
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void ExtractInstanceValueExp::_ToJson(BeJsValue val, JsonFormat const&) const {
    //! ITWINJS_PARSE_TREE: ExtractInstanceValueExp
    val.SetEmptyObject();
    val["id"] = "PropertyNameExp";
    val["path"] = GetInstancePath().ToString();
}
//****************************** PropertyNameExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(PropertyPath const& propPath) : ValueExp(Type::PropertyName),m_originalPropertyPath(propPath), m_resolvedPropertyPath(propPath), m_classRefExp(nullptr), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::ECSql),m_property(nullptr)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(PropertyPath const& propPath, RangeClassRefExp const& classRefExp, ECN::ECPropertyCR property)
    : ValueExp(Type::PropertyName), m_resolvedPropertyPath(propPath),m_originalPropertyPath(propPath), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::ECSql),m_property(&property){

    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(ECSqlParseContext const& ctx, RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp)
    : ValueExp(Type::PropertyName), m_className(classRefExp.GetAlias()), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::SubQuery),m_property(nullptr) {
    Utf8String alias = derivedPropExp.GetAliasRecursively();
    if (!alias.empty()) {
        m_resolvedPropertyPath.Push(alias);
    }else if (derivedPropExp.GetExpression()->GetType() == Exp::Type::PropertyName) {
        m_resolvedPropertyPath = derivedPropExp.GetExpression()->GetAs<PropertyNameExp>().GetResolvedPropertyPath();
    } else {
        m_resolvedPropertyPath.Push(derivedPropExp.GetName());
    }
    SetPropertyRef(derivedPropExp);
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ECN::ECPropertyCP PropertyNameExp::GetVirtualProperty() const {
    if (m_property)
        return m_property;

    if (IsPropertyRef()) {
        return GetPropertyRef()->TryGetVirtualProperty();
    }
    return nullptr;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::IsWildCard() const {
    if (m_resolvedPropertyPath.Size() == 1)  {
        return Exp::IsAsteriskToken(m_resolvedPropertyPath[0].GetName());
    }
    return false;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(ECSqlParseContext const& ctx, Utf8StringCR propertyName, RangeClassRefExp const& classRefExp, ClassMap const& classMap)
    : ValueExp(Type::PropertyName), m_className(classRefExp.GetAlias()), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::ClassRef),m_property(nullptr)
    {
    m_resolvedPropertyPath.Push(propertyName);
    if (m_resolvedPropertyPath.Resolve(classMap) != SUCCESS)
        {
        BeAssert(false && "Must always resolve correctly");
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus PropertyNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode) {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren) {
        if (ResolveColumnRef(ctx) != SUCCESS)
            return FinalizeParseStatus::Error;
        return FinalizeParseStatus::NotCompleted;
    }

    if (IsVirtualProperty(false)){
        SetTypeInfo(ECSqlTypeInfo(*GetVirtualProperty()));
        return FinalizeParseStatus::Completed;
    }

    //After children have been finalized
    if (!IsPropertyRef() && !m_resolvedPropertyPath.IsResolved()) {
        BeAssert(false && "PropertyNameExp property path is expected to be resolved at end of parsing.");
        return FinalizeParseStatus::Error;
    }

    if (IsPropertyRef()) {
        DerivedPropertyExp const& derivedProperty = GetPropertyRefP()->LinkedTo();
        if (derivedProperty.GetNestedAlias().empty())
            const_cast<DerivedPropertyExp&>(derivedProperty).SetNestedAlias(ctx.GenerateAlias().c_str());

        if (SUCCESS != const_cast<DerivedPropertyExp&>(derivedProperty).FinalizeParsing(ctx))
            return FinalizeParseStatus::Error;

        if (auto resolvedVirtualProp = GetPropertyRef()->TryGetVirtualProperty()) {
            SetTypeInfo(ECSqlTypeInfo(*resolvedVirtualProp));
        } else if (PropertyMap const *resolvedMap = GetPropertyRef()->TryGetPropertyMap()) {
            SetTypeInfo(ECSqlTypeInfo(*resolvedMap));
        } else {
            SetTypeInfo(derivedProperty.GetExpression()->GetTypeInfo());
        }
    } else {
        SetTypeInfo(ECSqlTypeInfo(*GetPropertyMap()));
    }

    if (IsPropertyRef())
        return FinalizeParseStatus::Completed;

    //determine whether the exp refers to a system property
    if (IsPropertyRef())
        m_sysPropInfo = &ECSqlSystemPropertyInfo::NoSystemProperty();
    else
        m_sysPropInfo = &ctx.Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(GetPropertyMap()->GetProperty());

    return FinalizeParseStatus::Completed;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus PropertyNameExp::ResolveUnionOrderByArg(ECSqlParseContext& ctx)
    {
    BeAssert(ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::UnionOrderBy);
    ECSqlParseContext::UnionOrderByArg const* arg = static_cast<ECSqlParseContext::UnionOrderByArg const*>(ctx.CurrentArg());

    Utf8StringCR firstPropPathEntry = m_resolvedPropertyPath[0].GetName();
    Utf8CP secondPropPathEntry = m_resolvedPropertyPath.Size() > 1 ? m_resolvedPropertyPath[1].GetName().c_str() : nullptr;
    for (SingleSelectStatementExp const* selectExp : arg->GetUnionClauses())
        for (Exp const* dpExp : selectExp->GetSelection()->GetChildren())
            {
            DerivedPropertyExp const& derivedPropertyExp = dpExp->GetAs<DerivedPropertyExp>();
            if (derivedPropertyExp.GetExpression()->GetType() == Exp::Type::PropertyName)
                {
                PropertyNameExp const& propertyNameExp = derivedPropertyExp.GetExpression()->GetAs<PropertyNameExp>();
                BeAssert(propertyNameExp.IsComplete());
                PropertyMap const* propertyMap = propertyNameExp.GetPropertyMap();
                if (secondPropPathEntry == nullptr)
                    {
                    if (propertyMap->GetName().EqualsI(firstPropPathEntry))
                        {
                        SetPropertyRef(derivedPropertyExp);
                        return SUCCESS;
                        }
                    }
                else
                    {
                    if (m_resolvedPropertyPath.ToString(false, false).EqualsI(propertyNameExp.GetResolvedPropertyPath().ToString(false, false)))
                        {
                        SetPropertyRef(derivedPropertyExp);
                        return SUCCESS;
                        }

                    if (m_resolvedPropertyPath.ToString(false, false).EqualsI(GetResolvedPropertyPath().ToString(false, false)))
                        {
                        SetPropertyRef(derivedPropertyExp);
                        return SUCCESS;
                        }

                    if (m_resolvedPropertyPath.ToString(false, false).EqualsI(propertyMap->GetAccessString()))
                        {
                        SetPropertyRef(derivedPropertyExp);
                        return SUCCESS;
                        }
                    }
                }


            if (derivedPropertyExp.GetColumnAlias().EqualsI(firstPropPathEntry))
                {
                SetPropertyRef(derivedPropertyExp);
                return SUCCESS;
                }

            }

    return ERROR;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus PropertyNameExp::ResolveLocalRef(ECSqlParseContext& ctx) {

    if (GetClassRefExp() == nullptr)
        return SUCCESS;

    // SELECT * FROM (subquery) column should not be resolved.
    if (GetSourceType() == SourceType::SubQuery)
        return SUCCESS;

    PropertyMatchOptions options;
    PropertyMatchResult result = GetClassRefExp()->FindProperty(ctx, m_resolvedPropertyPath, options);
    if (result.isValid()) {
        if (result.GetPropertyMap() != nullptr) {
                m_resolvedPropertyPath = result.ResolvedPath();
                BeAssert(m_resolvedPropertyPath.IsResolved() && "Programmer Error: Unable to resolve path");
        } else if (result.IsDerivedProperty()) {
            SetPropertyRef(*result.GetDerivedProperty());
            auto effectivePath = result.ResolvedPath();
            if (!GetPropertyRef()->IsComputedExp()) {
                if ( !GetPropertyRef()->TryResolvePath(effectivePath)) {
                    BeAssert(false && "Programmer Error: Unable to resolve path");
                }
            } else {
                m_resolvedPropertyPath = effectivePath;
            }
        } else {
            m_resolvedPropertyPath = result.ResolvedPath();
            SetVirtualProperty(*result.GetProperty());
        }
        return SUCCESS;
    }
    return ERROR;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus PropertyNameExp::ResolveColumnRef(ECSqlParseContext& ctx)
    {
    // This mean the PropertyNameExp was created by expanding WILDCARD so it both has ClassRef and DerviedProperty.
    if (GetClassRefExp() != nullptr)
        return ResolveLocalRef(ctx);

    if (!ctx.CurrentArg()) {
        return ERROR;
    }
    if (ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::UnionOrderBy)
        return ResolveUnionOrderByArg(ctx);

    if (ctx.CurrentArg()->GetType() != ECSqlParseContext::ParseArg::Type::RangeClass)
        {
        BeAssert(ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::RangeClass);
        return ERROR;
        }

    ECSqlParseContext::RangeClassArg const& arg = static_cast<ECSqlParseContext::RangeClassArg const&>(*ctx.CurrentArg());

    auto rangeClasses = [&]() {
        std::vector<RangeClassInfo> rangeClasses(arg.GetRangeClassInfos());
        auto parent = FindParent(Exp::Type::SingleSelect);
        while(parent != nullptr) {
            auto singleSelect = parent->GetAsCP<SingleSelectStatementExp>();
            if (singleSelect->IsRowConstructor()) {
                parent = nullptr;
                continue;
            }

            auto cursorRangeClasses = singleSelect->GetFrom()->FindRangeClassRefExpressions();
            for (auto const& rangeClass : cursorRangeClasses) {
                if (std::find_if(std::begin(rangeClasses), std::end(rangeClasses), [&rangeClass](RangeClassInfo& v) { return &v.GetExp() == &rangeClass.GetExp(); }) != std::end(rangeClasses))
                    continue;
                rangeClasses.push_back(RangeClassInfo(rangeClass, RangeClassInfo::Scope::Inherited));
            }
            parent = singleSelect->FindParent(Exp::Type::SingleSelect);
        }
        return rangeClasses;
    }();

    BeAssert(!m_resolvedPropertyPath.IsEmpty());

    std::vector<PropertyMatchResult> matchProps;
    auto hasBetterSolution = [](std::vector<PropertyMatchResult> const &matches, PropertyMatchResult const& newSolution) {
        for (auto const& match : matches) {
            if(match.Options().GetRangeInfo().GetScope() == newSolution.Options().GetRangeInfo().GetScope()) {
                if (match.Confidence() > newSolution.Confidence())
                    return true;
            }
        }
        return false;
    };

    auto countLocalRefs = [](std::vector<PropertyMatchResult> const &matches) {
        int local = 0;
        for (auto const& match : matches) {
            if (match.Options().GetRangeInfo().IsLocal())
                ++local;
        }
        return local;
    };

    auto eraseLowerConfidenceIfAny = [](std::vector<PropertyMatchResult>  &matches, PropertyMatchResult& rc) {
        auto it = std::find_if(matches.begin(), matches.end(), [&](const PropertyMatchResult &el) {
            return el.Options().GetRangeInfo().GetScope() == rc.Options().GetRangeInfo().GetScope() && el.Confidence() < rc.Confidence();
        });
        if (it != matches.end()){
            matches.erase(it);
        }
    };

    auto eraseLongerResolvedPathIfAny = [](std::vector<PropertyMatchResult>  &matches, PropertyMatchResult& rc) {
        auto it = std::find_if(matches.begin(), matches.end(), [&](const PropertyMatchResult &el) {
            return el.ResolvedPath().Size() > rc.ResolvedPath().Size();
        });
        if (it != matches.end()){
            matches.erase(it);
        }
    };

    for (RangeClassInfo const &rangeClassInfo : rangeClasses) {
        PropertyMatchOptions options;
        options.SetRangeInfo(rangeClassInfo);
        PropertyMatchResult rc = rangeClassInfo.GetExp().FindProperty(ctx, m_resolvedPropertyPath, options);
        if (rc.isValid()) {
            if (rc.IsDerivedProperty()) {
                // make sure non-cyclic
                auto exp = rc.GetDerivedProperty()->GetExpression();
                if (exp->GetType() == Exp::Type::PropertyName) {
                    auto& propName = exp->GetAs<PropertyNameExp>();
                    if (!propName.IsPropertyRef()) {
                        if (&propName == this)
                            continue;
                    }
                }
            }
            // see if current resolution is the shortest resolved path (prefer alias of property path)
            eraseLongerResolvedPathIfAny(matchProps, rc);
            eraseLowerConfidenceIfAny(matchProps, rc);
            if (!hasBetterSolution(matchProps, rc)) {
                matchProps.push_back(rc);

                // exit on first chance in case property is ambiguous
                const auto local = countLocalRefs(matchProps);
                const auto inherited = matchProps.size() - local;
                // stop its ambiguous
                if (local > 1 || inherited > 1)
                    break;
            }
        }
    }

    if (matchProps.empty()) {
        // Check if a column alias is being used within the select
        if (auto parentSelect = FindParent(Exp::Type::SingleSelect); parentSelect != nullptr)
            {
            for (const auto dpExp : parentSelect->GetAsCP<SingleSelectStatementExp>()->GetSelection()->GetChildren())
                {
                const auto& derivedPropertyExp = dpExp->GetAs<DerivedPropertyExp>();
                if (derivedPropertyExp.GetColumnAlias().EqualsI(GetPropertyName()))
                    {
                    SetPropertyRef(derivedPropertyExp);
                    return SUCCESS;
                    }
                }
            }
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0565,
            "No property or enumeration found for expression '%s'.", m_resolvedPropertyPath.ToString().c_str());
        return ERROR;
    }

    const auto local = countLocalRefs(matchProps);
    const auto inherited = matchProps.size() - local;
    if (!(local == 1 || inherited == 1)) {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, ECDbIssueId::ECDb_0566,
            "Expression '%s' in ECSQL statement is ambiguous.", m_resolvedPropertyPath.ToString().c_str());
        return ERROR;
    }

    PropertyMatchResult const &match = matchProps.front();
    SetClassRefExp(match.Options().GetRangeInfo().GetExp());
    if (match.IsPropertyMap()) {
        // resolved already
        m_resolvedPropertyPath = match.ResolvedPath();
        BeAssert(m_resolvedPropertyPath.IsResolved()); // this should be already resolved

    } else if (match.IsDerivedProperty()) {
        SetPropertyRef(*match.GetDerivedProperty());
        // Property path is not resolved
        m_resolvedPropertyPath = match.ResolvedPath();
        if (!GetPropertyRef()->IsComputedExp() && GetPropertyRef()->TryGetVirtualProperty() == nullptr) {
            if ( !GetPropertyRef()->TryResolvePath(m_resolvedPropertyPath)) {
                BeAssert(false && "Programmer Error: Unable to resolve path");
            }
        }
    } else {
        // property def table viewow
        m_resolvedPropertyPath = match.ResolvedPath();
        SetVirtualProperty(*match.GetProperty());
    }

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::HasUserDefinedAlias() const {

    // look up parent for alias
    if (this->GetParent() && this->GetParent()->GetType() == Exp::Type::DerivedProperty) {
        if (this->GetParent()->GetAs<DerivedPropertyExp>().HasAlias())
            return true;
    }

    if (!IsPropertyRef())
        return false;

    DerivedPropertyExp const *linkTo = &GetPropertyRef()->LinkedTo();
    do {
        if (linkTo->HasAlias())
            return true;

        if (linkTo->IsComputed())
            return true;

        auto& prop = linkTo->GetExpression()->GetAs<PropertyNameExp>();
        linkTo = prop.IsPropertyRef() ? &prop.GetPropertyRef()->LinkedTo() : nullptr;
    } while (linkTo != nullptr);

    return false;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyNameExp::SetClassRefExp(RangeClassRefExp const& resolvedClassRef) {
    m_classRefExp = &resolvedClassRef;
    m_className = resolvedClassRef.GetAlias();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyNameExp::SetPropertyRef(DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp)
    {
    m_propertyRef = std::unique_ptr<PropertyRef>(new PropertyRef(derivedPropertyExpInSubqueryRefExp, this));
    }

//------------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+---------
PropertyMap const* PropertyNameExp::GetPropertyMap() const
    {
    auto classRefExp = GetClassRefExp();
    if (classRefExp == nullptr)
        return nullptr;

    PropertyMap const* propertyMap = nullptr;
    switch (classRefExp->GetType())
        {
        case Exp::Type::ClassName:
            {
            ClassNameExp const& classNameExp = classRefExp->GetAs<ClassNameExp>();
            propertyMap = classNameExp.GetInfo().GetMap().GetPropertyMaps().Find(GetResolvedPropertyPath().ToString(false).c_str());
            break;
            }

        case Exp::Type::SubqueryRef:
            {  
            PropertyNameExp::PropertyRef const* propertyRef = GetPropertyRef();
            BeAssert(propertyRef != nullptr);
            propertyMap = propertyRef->TryGetPropertyMap(GetResolvedPropertyPath());
            if (propertyMap == nullptr) {
                BeAssert(propertyMap != nullptr && "Exp of a derived prop exp referenced from a sub query ref is expected to always be a prop name exp");
            }
            break;
            }
        case Exp::Type::CommonTableBlock:
            { 
            /*// This block is added because if the cte block has no columns we treat the select statement inside cte block just as a subquery of 
            outer cte select statement and we pass the classref as CommonTableBlockExp,
             the classref stays as CommonTableBlockExp if we "select *" in outer select statement
             Ex- with cte as (select * from meta.ECClassDef) select * from cte*/
            CommonTableBlockExp const& cteBlockExp = classRefExp->GetAs<CommonTableBlockExp>();  
            if(cteBlockExp.GetColumns().size() == 0)
                {
                PropertyNameExp::PropertyRef const* propertyRef = GetPropertyRef();
                BeAssert(propertyRef != nullptr);
                propertyMap = propertyRef->TryGetPropertyMap(GetResolvedPropertyPath());
                if (propertyMap == nullptr) 
                    {
                    BeAssert(propertyMap != nullptr && "Exp of a derived prop exp referenced from a common table block is expected to always be a prop name exp");
                    }
                }
            break;
            }
        case Exp::Type::CommonTableBlockName :
            {
            /*// This block is added because if the cte block has no columns we treat the select statement inside cte block just as a subquery of 
            outer cte select statement and we pass the classref as CommonTableBlockExp,
             the classref becomes as CommonTableBlockNameExp if we "select <column>" in outer select statement
             Ex- with cte as (select * from meta.ECClassDef) select ECInstanceId from cte*/ 
            CommonTableBlockNameExp const& cteBlockNameExp = classRefExp->GetAs<CommonTableBlockNameExp>();
            CommonTableBlockExp const* cteBlock = cteBlockNameExp.GetBlock();
            if(cteBlock != nullptr && cteBlock->GetColumns().size() == 0)
                {
                PropertyNameExp::PropertyRef const* propertyRef = GetPropertyRef();
                BeAssert(propertyRef != nullptr);
                propertyMap = propertyRef->TryGetPropertyMap(GetResolvedPropertyPath());
                if (propertyMap == nullptr) {
                    BeAssert(propertyMap != nullptr && "Exp of a derived prop exp referenced from a common table block name is expected to always be a prop name exp");
                }
                break;
                }
            return nullptr; // This block returns nullptr for proper alias referencing if the cte block has columns
            }
        default:
                BeAssert(false && "Unhandled ClassRefExp subtype. This code needs to be adjusted.");
                break;
        }

    BeAssert(propertyMap != nullptr && "PropertyNameExp's PropertyMap should never be nullptr.");
    return propertyMap;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::IsLhsAssignmentOperandExpression() const
    {
    if (FindParent(Exp::Type::Insert))
        return GetParent()->GetType() == Exp::Type::PropertyNameList;

    if (FindParent(Exp::Type::Update))
        return GetParent()->GetType() == Exp::Type::Assignment;

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyNameExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: PropertyNameExp
    val.SetEmptyObject();
    val["id"] = "PropertyNameExp";
    Utf8String path;
    path.append(m_originalPropertyPath.ToString(false, false, true));
    if (path.empty()) {
        path.append(m_resolvedPropertyPath.ToString(false, false, true));
    }
    val["path"] = path;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyNameExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (!m_className.empty()) {
        auto classRef = GetClassRefExp();
        if (classRef != nullptr && classRef->GetType() == Exp::Type::ClassName && classRef->GetAs<ClassNameExp>().HasMetaInfo())
            ctx.AppendToECSql(classRef->GetAs<ClassNameExp>().GetInfo().GetMap().GetClass().GetName()).AppendToECSql(".");
        else
            ctx.AppendToECSql(m_className).AppendToECSql(".");
    }
    ctx.AppendToECSql(m_resolvedPropertyPath.ToString(true, false, true));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String PropertyNameExp::_ToString() const
    {
    Utf8String str("PropertyName [");
    str.append(ToECSql()).append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::PropertyRef::ReferToAlias() const
    {
    return this->LinkedTo().GetColumnAlias().Equals(m_owner->GetResolvedPropertyPath()[0].GetName());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::PropertyRef::IsComputedExp() const
    {
    return this->GetEndPointDerivedProperty().GetExpression()->GetType() != Exp::Type::PropertyName;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::PropertyRef::IsSqlColumnNameExp() const
    {
    return this->GetEndPointDerivedProperty().GetExpression()->GetType() == Exp::Type::SqlColumnName;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::PropertyRef::TryResolvePath(PropertyPath &path) const
    {
    PropertyMap const *propertyMap = TryGetPropertyMap(path);
    if (propertyMap == nullptr)
        return false;

    PropertyMap::Path resolvePath = propertyMap->GetPath();
    int n = static_cast<int>(std::min(resolvePath.size(), path.Size()));
    if (n == 0)
        {
        BeAssert(false && "Programmer Error: path should have atleast one element");
        return false;
        }
    // reverse assign the property def. The alias may be in path and in prefix. We are only interested in last property only.
    for (int i = n - 1; i >= 0; --i)
        path.SetPropertyDef(i, resolvePath[i].GetProperty());

    path.SetClassMap(propertyMap->GetClassMap());
    return true;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ECN::ECPropertyCP PropertyNameExp::PropertyRef::TryGetVirtualProperty() const {
    DerivedPropertyExp const &next = LinkedTo();
    if (next.GetExpression()->GetType() != Exp::Type::PropertyName)
        return nullptr;

    PropertyNameExp const &exp = next.GetExpression()->GetAs<PropertyNameExp>();
    if (exp.IsPropertyRef())
        return exp.GetPropertyRef()->TryGetVirtualProperty();

    return exp.GetVirtualProperty();
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMap const * PropertyNameExp::PropertyRef::TryGetPropertyMap(PropertyPath const& testPath) const
    {
    if (m_cachedPropertyMap)
        return m_cachedPropertyMap;

    DerivedPropertyExp const &next = LinkedTo();
    if (next.GetExpression()->GetType() != Exp::Type::PropertyName && next.GetExpression()->GetType() != Exp::Type::NavValueCreationFunc)
        return nullptr;

    PropertyNameExp const &exp = next.GetExpression()->GetType() == Exp::Type::PropertyName ? next.GetExpression()->GetAs<PropertyNameExp>() : *next.GetExpression()->GetAs<NavValueCreationFuncExp>().GetPropertyNameExp();
    if (exp.IsPropertyRef())
        return exp.GetPropertyRef()->TryGetPropertyMap();


    PropertyMap const *propertyMap = exp.GetPropertyMap();
    PropertyPath const& path = testPath;
    if (path.Size() == 1)
        return propertyMap;

    Utf8String accessStringPrefix = next.GetColumnAlias().empty() ? path.ToString() : propertyMap->GetAccessString() + "." + path.Skip(1).ToString();
    if (!next.GetColumnAlias().empty())
        {
       // BeAssert(next.GetColumnAlias() == path.Take(1).ToString());
        }

    if (propertyMap->GetAccessString().EqualsIAscii(accessStringPrefix.c_str())) {
        m_cachedPropertyMap = propertyMap;
    } else {
        SearchPropertyMapVisitor visitor;
        propertyMap->AcceptVisitor(visitor);
        for(PropertyMap const* it : visitor.Results())
            {
            if (it->GetAccessString().EqualsIAscii(accessStringPrefix.c_str()))
                {
                m_cachedPropertyMap = it;
                break;
                }
            }
        }
    return m_cachedPropertyMap;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMap const * PropertyNameExp::PropertyRef::TryGetPropertyMap() const
    {
    return TryGetPropertyMap(m_owner->GetResolvedPropertyPath());
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const& PropertyNameExp::PropertyRef::GetEndPointDerivedProperty() const
    {
    if (LinkedTo().GetExpression()->GetType() == Exp::Type::PropertyName)
        {
        PropertyNameExp const& next = LinkedTo().GetExpression()->GetAs<PropertyNameExp>();
        if (next.IsPropertyRef())
            return next.GetPropertyRef()->GetEndPointDerivedProperty();
        }

    return LinkedTo();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool PropertyNameExp::PropertyRef::IsPure() const
    {
    if (!LinkedTo().GetColumnAlias().empty() || LinkedTo().GetExpression()->GetType() != Exp::Type::PropertyName)
        return false;

    PropertyNameExp const& next = LinkedTo().GetExpression()->GetAs<PropertyNameExp>();
    if (next.IsPropertyRef())
        return next.GetPropertyRef()->IsPure();

    return true;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyNameExp::PropertyRef::ToNativeSql(NativeSqlBuilder::List const& snippets, std::vector<bool> const& filter) const
    {
    if (m_wasToNativeSqlCalled)
        return SUCCESS;

    const bool noColumnSelected = std::find(filter.begin(), filter.end(), true) == filter.end();
    if (noColumnSelected)
        {
        BeAssert(false && "Programmer Error: Filter must have alteast one none false bit set.");
        return ERROR;
        }

    m_nativeSqlSnippets.clear();
    Utf8String alias = m_linkedTo.GetColumnAlias();
    if (alias.empty() || m_linkedTo.OriginateInASubQuery())
        alias = m_linkedTo.GetNestedAlias();

    if (!alias.empty())
        {
        NativeSqlBuilder::List localSnippets = snippets;
        if (localSnippets.size() == 1)
            {
            localSnippets.front().Clear();
            localSnippets.front().AppendEscaped(alias.c_str());
            }
        else
            {
            int idx = 0;
            Utf8String postfix;
            for (NativeSqlBuilder& snippet : localSnippets)
                {
                postfix.clear();
                postfix.Sprintf("%s_%d", alias.c_str(), idx++);
                snippet.Clear();
                snippet.AppendEscaped(postfix.c_str());
                }
            }

        m_wasToNativeSqlCalled = true;
        for (int i =0; i< filter.size(); ++i)
            {
            if (filter[i])
                m_nativeSqlSnippets.push_back(localSnippets[i]);
            }
        }


    return m_wasToNativeSqlCalled? SUCCESS : ERROR;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyNameExp::PropertyRef::ToNativeSql(NativeSqlBuilder::List const& snippets) const
    {
    std::vector<bool> selectAll;
    for (int i = 0; i < snippets.size(); ++i)
        selectAll.push_back(true);

    return ToNativeSql(snippets, selectAll);
    }

//=========================================================================================
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
InstanceValueExp::InstanceValueExp(Type type, PropertyPath instancePath): ValueExp(type), m_instancePath(instancePath) {
    BeAssert(InstanceValueExp::IsValidSourcePath(instancePath) && "InstancePath is not valid");
    auto alias = InstanceValueExp::GetInstanceAlias(instancePath);
    bool hasAlias = !Utf8String::IsNullOrEmpty(alias);
    PropertyPath classIdPath;
    if (hasAlias){
        classIdPath.Push(alias);
    }
    classIdPath.Push(ECDBSYS_PROP_ECClassId);
    m_classIdExpIdx = AddChild(std::make_unique<PropertyNameExp>(std::move(classIdPath)));

    PropertyPath instIdPath;
    if (hasAlias) {
        instIdPath.Push(alias);
    }
    instIdPath.Push(ECDBSYS_PROP_ECInstanceId);
    m_instIdExpIdx = AddChild(std::make_unique<PropertyNameExp>(std::move(instIdPath)));
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool InstanceValueExp::IsValidSourcePath(PropertyPath const& path) {
    if (path.Size() == 0 || path.Size() > 2) {
        return false;
    }
    return path.Last().GetName().Equals("$");
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP InstanceValueExp::GetInstanceAlias(PropertyPath const& path) {
    BeAssert(InstanceValueExp::IsValidSourcePath(path) && "SourcePath is not valid");
    return path.Size() == 2 ? path.First().GetName().c_str() : nullptr;
}
END_BENTLEY_SQLITE_EC_NAMESPACE
