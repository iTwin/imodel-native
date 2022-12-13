/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** PropertyNameExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(PropertyPath const& propPath) : ValueExp(Type::PropertyName), m_propertyPath(propPath), m_classRefExp(nullptr), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::ECSql),m_property(nullptr)
    {}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(PropertyPath const& propPath, RangeClassRefExp const& classRefExp, ECN::ECPropertyCR property) 
    : ValueExp(Type::PropertyName), m_propertyPath(propPath), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::ECSql),m_property(&property){

    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(ECSqlParseContext const& ctx, RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp)
    : ValueExp(Type::PropertyName), m_className(classRefExp.GetAlias()), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::SubQuery),m_property(nullptr) {
    Utf8String alias = derivedPropExp.GetAliasRecursively();
    if (!alias.empty()) {
        m_propertyPath.Push(alias);
    }else if (derivedPropExp.GetExpression()->GetType() == Exp::Type::PropertyName) {
        m_propertyPath = derivedPropExp.GetExpression()->GetAs<PropertyNameExp>().GetPropertyPath();
    } else {
        m_propertyPath.Push(derivedPropExp.GetName());
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
    if (m_propertyPath.Size() == 1)  {
        return Exp::IsAsteriskToken(m_propertyPath[0].GetName());
    }
    return false;
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp::PropertyNameExp(ECSqlParseContext const& ctx, Utf8StringCR propertyName, RangeClassRefExp const& classRefExp, ClassMap const& classMap)
    : ValueExp(Type::PropertyName), m_className(classRefExp.GetAlias()), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty()), m_sourceType(SourceType::ClassRef),m_property(nullptr)
    {
    m_propertyPath.Push(propertyName);
    if (m_propertyPath.Resolve(classMap) != SUCCESS)
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
    if (!IsPropertyRef() && !m_propertyPath.IsResolved()) {
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
        SetTypeInfo(ECSqlTypeInfo(GetPropertyMap()));
    }

    if (IsPropertyRef())
        return FinalizeParseStatus::Completed;

    //determine whether the exp refers to a system property
    if (IsPropertyRef())
        m_sysPropInfo = &ECSqlSystemPropertyInfo::NoSystemProperty();
    else
        m_sysPropInfo = &ctx.Schemas().Main().GetSystemSchemaHelper().GetSystemPropertyInfo(GetPropertyMap().GetProperty());

    return FinalizeParseStatus::Completed;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus PropertyNameExp::ResolveUnionOrderByArg(ECSqlParseContext& ctx)
    {
    BeAssert(ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::UnionOrderBy);
    ECSqlParseContext::UnionOrderByArg const* arg = static_cast<ECSqlParseContext::UnionOrderByArg const*>(ctx.CurrentArg());

    Utf8StringCR firstPropPathEntry = m_propertyPath[0].GetName();
    Utf8CP secondPropPathEntry = m_propertyPath.Size() > 1 ? m_propertyPath[1].GetName().c_str() : nullptr;
    for (SingleSelectStatementExp const* selectExp : arg->GetUnionClauses())
        for (Exp const* dpExp : selectExp->GetSelection()->GetChildren())
            {
            DerivedPropertyExp const& derivedPropertyExp = dpExp->GetAs<DerivedPropertyExp>();
            if (derivedPropertyExp.GetExpression()->GetType() == Exp::Type::PropertyName)
                {
                PropertyNameExp const& propertyNameExp = derivedPropertyExp.GetExpression()->GetAs<PropertyNameExp>();
                BeAssert(propertyNameExp.IsComplete());
                PropertyMap const& propertyMap = propertyNameExp.GetPropertyMap();
                if (secondPropPathEntry == nullptr)
                    {
                    if (propertyMap.GetName().EqualsI(firstPropPathEntry))
                        {
                        SetPropertyRef(derivedPropertyExp);
                        return SUCCESS;
                        }
                    }
                else
                    {
                    if (m_propertyPath.ToString(false, false).EqualsI(propertyNameExp.GetPropertyPath().ToString(false, false)))
                        {
                        SetPropertyRef(derivedPropertyExp);
                        return SUCCESS;
                        }

                    if (m_propertyPath.ToString(false, false).EqualsI(GetPropertyPath().ToString(false, false)))
                        {
                        SetPropertyRef(derivedPropertyExp);
                        return SUCCESS;
                        }

                    if (m_propertyPath.ToString(false, false).EqualsI(propertyMap.GetAccessString()))
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
    PropertyMatchResult result = GetClassRefExp()->FindProperty(ctx, m_propertyPath, options);
    if (result.isValid()) {
        if (result.GetPropertyMap() != nullptr) {
                m_propertyPath = result.ResolvedPath();
                BeAssert(m_propertyPath.IsResolved() && "Programmer Error: Unable to resolve path");
        } else if (result.IsDerivedProperty()) { 
            SetPropertyRef(*result.GetDerivedProperty());
            auto effectivePath = result.ResolvedPath();
            if (!GetPropertyRef()->IsComputedExp()) {
                if ( !GetPropertyRef()->TryResolvePath(effectivePath)) {
                    BeAssert(false && "Programmer Error: Unable to resolve path");
                }
            } else {
                m_propertyPath = effectivePath;
            }
        } else {
            m_propertyPath = result.ResolvedPath();
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
    BeAssert(!m_propertyPath.IsEmpty());
    
    std::vector<PropertyMatchResult> matchProps;
    auto hasBetterSolution = [](std::vector<PropertyMatchResult> const &matchs, PropertyMatchResult const& newSolution) {
        for (auto const& match : matchs) {
            if(match.Options().GetRangeInfo().GetScope() == newSolution.Options().GetRangeInfo().GetScope()) {
                if (match.Confidence() > newSolution.Confidence())
                    return true;
            }
        }
        return false;
    };

    auto countLocalRefs = [](std::vector<PropertyMatchResult> const &matchs) {
        int local = 0;
        for (auto const& match : matchs) {
            if (match.Options().GetRangeInfo().IsLocal())
                ++local;
        }
        return local;
    };

    auto eraseLowerConfidenceIfAny = [](std::vector<PropertyMatchResult>  &matchs, PropertyMatchResult& rc) {
        auto it = std::find_if(matchs.begin(), matchs.end(), [&](const PropertyMatchResult &el) {
            return el.Options().GetRangeInfo().GetScope() == rc.Options().GetRangeInfo().GetScope() && el.Confidence() < rc.Confidence();
        });
        if (it != matchs.end()){
            matchs.erase(it);
        }
    };

    auto eraseLongerResolvedPathIfAny = [](std::vector<PropertyMatchResult>  &matchs, PropertyMatchResult& rc) {
        auto it = std::find_if(matchs.begin(), matchs.end(), [&](const PropertyMatchResult &el) {
            return el.ResolvedPath().Size() > rc.ResolvedPath().Size();
        });
        if (it != matchs.end()){
            matchs.erase(it);
        }
    };

    for (RangeClassInfo const &rangeClassInfo : arg.GetRangeClassInfos()) {
        PropertyMatchOptions options;
        options.SetRangeInfo(rangeClassInfo);
        PropertyMatchResult rc = rangeClassInfo.GetExp().FindProperty(ctx, m_propertyPath, options);
        if (rc.isValid()) {
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
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "No property or enumeration found for expression '%s'.", m_propertyPath.ToString().c_str());
        return ERROR;
    }

    const auto local = countLocalRefs(matchProps);
    const auto inherited = matchProps.size() - local;
    if (!(local == 1 || inherited == 1)) {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Expression '%s' in ECSQL statement is ambiguous.", m_propertyPath.ToString().c_str());
        return ERROR;
    }

    PropertyMatchResult const &match = matchProps.front();
    SetClassRefExp(match.Options().GetRangeInfo().GetExp());
    if (match.IsPropertyMap()) {
        // resolved already
        m_propertyPath = match.ResolvedPath();
        BeAssert(m_propertyPath.IsResolved()); // this should be already resolved

    } else if (match.IsDerivedProperty()) { 
        SetPropertyRef(*match.GetDerivedProperty());
        // Property path is not resolved
        m_propertyPath = match.ResolvedPath();
        if (!GetPropertyRef()->IsComputedExp() && GetPropertyRef()->TryGetVirtualProperty() == nullptr) {
            if ( !GetPropertyRef()->TryResolvePath(m_propertyPath)) {
                BeAssert(false && "Programmer Error: Unable to resolve path");
            }
        }
    } else {
        // property def table view 
        m_propertyPath = match.ResolvedPath();
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
PropertyMap const& PropertyNameExp::GetPropertyMap() const
    {
    BeAssert(GetClassRefExp() != nullptr);
    PropertyMap const* propertyMap = nullptr;
    switch (GetClassRefExp()->GetType())
        {
        case Exp::Type::ClassName:
            {
            ClassNameExp const& classNameExp = GetClassRefExp()->GetAs<ClassNameExp>();
            propertyMap = classNameExp.GetInfo().GetMap().GetPropertyMaps().Find(GetPropertyPath().ToString(false).c_str());
            break;
            }

        case Exp::Type::SubqueryRef:
            {
            PropertyNameExp::PropertyRef const* propertyRef = GetPropertyRef();
            BeAssert(propertyRef != nullptr);
            propertyMap = propertyRef->TryGetPropertyMap(GetPropertyPath());
            if (propertyMap == nullptr) {
                BeAssert(propertyMap != nullptr && "Exp of a derived prop exp referenced from a sub query ref is expected to always be a prop name exp");
            }
            break;
            }

        default:
                BeAssert(false && "Unhandled ClassRefExp subtype. This code needs to be adjusted.");
                break;
        }

    BeAssert(propertyMap != nullptr && "PropertyNameExp's PropertyMap should never be nullptr.");
    return *propertyMap;
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
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyNameExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (!m_className.empty())
        ctx.AppendToECSql(m_className).AppendToECSql(".");

    ctx.AppendToECSql(m_propertyPath.ToString(true, false));
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
    return this->LinkedTo().GetColumnAlias().Equals(m_owner->GetPropertyPath()[0].GetName());
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
    if (next.GetExpression()->GetType() != Exp::Type::PropertyName)
        return nullptr;

    PropertyNameExp const &exp = next.GetExpression()->GetAs<PropertyNameExp>();
    if (exp.IsPropertyRef())
        return exp.GetPropertyRef()->TryGetPropertyMap();


    PropertyMap const &propertyMap = exp.GetPropertyMap();
    PropertyPath const& path = testPath;
    if (path.Size() == 1)
        return &propertyMap;

    Utf8String accessStringPrefix = next.GetColumnAlias().empty() ? path.ToString() : propertyMap.GetAccessString() + "." + path.Skip(1).ToString();
    if (!next.GetColumnAlias().empty())
        {
       // BeAssert(next.GetColumnAlias() == path.Take(1).ToString());
        }
    
    if (propertyMap.GetAccessString().EqualsIAscii(accessStringPrefix.c_str())) {
        m_cachedPropertyMap = &propertyMap;
    } else {
        SearchPropertyMapVisitor visitor;
        propertyMap.AcceptVisitor(visitor);
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
    return TryGetPropertyMap(m_owner->GetPropertyPath());
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
