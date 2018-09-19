/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PropertyNameExp.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************************** PropertyNameExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp(PropertyPath const& propPath) : ValueExp(Type::PropertyName), m_propertyPath(propPath), m_classRefExp(nullptr), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty())
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp(ECSqlParseContext const& ctx, RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp)
    : ValueExp(Type::PropertyName), m_className(classRefExp.GetAlias()), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty())
    {
    Utf8String propName = derivedPropExp.GetName();
    //WIP: Affan, why do we have to remove the square brackets?
    propName.ReplaceAll("[", "");
    propName.ReplaceAll("]", "");
    m_propertyPath.Push(propName);
    SetPropertyRef(derivedPropExp);
    }
    
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp(ECSqlParseContext const& ctx, Utf8StringCR propertyName, RangeClassRefExp const& classRefExp, ClassMap const& classMap)
    : ValueExp(Type::PropertyName), m_className(classRefExp.GetAlias()), m_classRefExp(&classRefExp), m_sysPropInfo(&ECSqlSystemPropertyInfo::NoSystemProperty())
    {
    m_propertyPath.Push(propertyName);
    if (m_propertyPath.Resolve(classMap) != SUCCESS)
        BeAssert(false && "Must always resolve correctly");
    }
    
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus PropertyNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        if (ResolveColumnRef(ctx) != SUCCESS)
            return FinalizeParseStatus::Error;

        return FinalizeParseStatus::NotCompleted;
        }

    //After children have been finalized
    if (!IsPropertyRef() && !m_propertyPath.IsResolved())
        {
        BeAssert(false && "PropertyNameExp property path is expected to be resolved at end of parsing.");
        return FinalizeParseStatus::Error;
        }

    if (IsPropertyRef())
        {
        DerivedPropertyExp const& derivedProperty = GetPropertyRefP()->LinkedTo();
        if (derivedProperty.GetNestedAlias().empty())
            const_cast<DerivedPropertyExp&>(derivedProperty).SetNestedAlias(ctx.GenerateAlias().c_str());

        if (SUCCESS != const_cast<DerivedPropertyExp&>(derivedProperty).FinalizeParsing(ctx))
            return FinalizeParseStatus::Error;

        SetTypeInfo(derivedProperty.GetExpression()->GetTypeInfo());
        }
    else
        SetTypeInfo(ECSqlTypeInfo(GetPropertyMap()));

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
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
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
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyNameExp::ResolveColumnRef(Utf8StringR error, RangeClassRefExp const& classRefExp, PropertyPath& propPath)
    {
    switch (classRefExp.GetType())
        {
            case Exp::Type::SubqueryRef:
            {
            SubqueryRefExp const& subQueryRef = classRefExp.GetAs<SubqueryRefExp>();
            DerivedPropertyExp const* derivedPropertyRef = subQueryRef.GetSubquery()->FindProperty(propPath[0].GetName().c_str());
            if (derivedPropertyRef == nullptr)
                {
                //"Should not be nullptr as the classRefExp was found in the first place by checking that the property exists"
                BeAssert(false);
                return ERROR;
                }

            SetPropertyRef(*derivedPropertyRef);
            //1. Select statement must have a ECClass define it. 
            //2. ECClass must have classMap info for this to work
            //3. In context of query SELECT NewFoo.Prop1, NewFoo.Prop2 FROM (SELECT A Prop1, B + C Prop2 FROM Foo WHERE A = 12)  NewFoo;
            //4. SELECT _a, [_b + _c]  FROM (SELECT _a "_a", _b + _c  "_b + _c" FROM _foo where _a = 12) NewFoo
            //5. SELECT _a, [_b + _c] FROM newFoo;
            //6. Scalar query.
            return SUCCESS;
            }
            case Exp::Type::ClassName:
            {
            ClassNameExp const& classNameExp = classRefExp.GetAs<ClassNameExp>();
            ClassMap const& classMap = classNameExp.GetInfo().GetMap();
            return propPath.Resolve(classMap, &error);
            }

            default:
                BeAssert(false && "New subclass of RangeClassRefExp was added, but is not yet processed in ValueExp::ResolveColumnRef");
                return ERROR;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyNameExp::ResolveColumnRef(ECSqlParseContext& ctx)
    {
    if (GetClassRefExp() != nullptr)
        return SUCCESS;

    BeAssert(ctx.CurrentArg() != nullptr && "PropertyNameExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
    if (ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::UnionOrderBy)
        return ResolveUnionOrderByArg(ctx);

    if (ctx.CurrentArg()->GetType() != ECSqlParseContext::ParseArg::Type::RangeClass)
        {
        BeAssert(ctx.CurrentArg()->GetType() == ECSqlParseContext::ParseArg::Type::RangeClass);
        return ERROR;
        }

    ECSqlParseContext::RangeClassArg const& arg = static_cast<ECSqlParseContext::RangeClassArg const&>(*ctx.CurrentArg());
    BeAssert(!m_propertyPath.IsEmpty());

    PropertyPath::Location const& firstPropPathEntry = m_propertyPath[0];

    std::vector<std::pair<RangeClassRefExp const*, size_t>> classMatches;
    std::vector<RangeClassRefExp const*> propNameClassRefExpMatches;
    for (RangeClassInfo const& rangeClassInfo : arg.GetRangeClassInfos())
        {
        if (rangeClassInfo.IsInherited())
            {
            if (!classMatches.empty() || !propNameClassRefExpMatches.empty())
                break;
            }

        RangeClassRefExp const& classRefExp = rangeClassInfo.GetExp();

        //assume first entry is property name
        if (classRefExp.ContainsProperty(firstPropPathEntry.GetName()))
            propNameClassRefExpMatches.push_back(&classRefExp);

        //assume first entry is schema name/alias or class name/alias
        //(Array index can only occur with prop name, but not with schema or class names. 
        //So array indexer is not a class match)
        if (m_propertyPath.Size() > 1 && !firstPropPathEntry.HasArrayIndex())
            {
            int classIndex = -1;
            Utf8StringCR alias = classRefExp.GetAlias();
            if (!alias.empty())
                {
                //if alias is present, only compare to alias and skip comparing with class name
                if (firstPropPathEntry.GetName().EqualsIAscii(alias))
                    classIndex = 0;
                }
            else
                {
                if (classRefExp.GetType() == Exp::Type::ClassName)
                    {
                    ClassNameExp const& classNameExp = classRefExp.GetAs<ClassNameExp>();
                    ECN::ECClassCR ecClass = classNameExp.GetInfo().GetMap().GetClass();

                    if (ecClass.GetName().EqualsIAscii(firstPropPathEntry.GetName()))
                        classIndex = 0;
                    else if (m_propertyPath.Size() > 2)
                        {
                        PropertyPath::Location const& secondPropPathEntry = m_propertyPath[1];
                        if (!secondPropPathEntry.HasArrayIndex())
                            {
                            ECN::ECSchemaCR schema = ecClass.GetSchema();
                            if (schema.GetName().EqualsIAscii(firstPropPathEntry.GetName()) || schema.GetAlias().EqualsIAscii(firstPropPathEntry.GetName()) &&
                                ecClass.GetName().EqualsIAscii(secondPropPathEntry.GetName()))
                                {
                                classIndex = 1;
                                }
                            }
                        }
                    }
                }

            if (classIndex >= 0)
                {
                BeAssert(classIndex < (int) m_propertyPath.Size());
                if (classRefExp.ContainsProperty(m_propertyPath[(size_t) (classIndex + 1)].GetName()))
                    classMatches.push_back(std::make_pair<RangeClassRefExp const*, size_t>(&classRefExp, (size_t) classIndex));
                }
            }

        }

    if (classMatches.empty() && propNameClassRefExpMatches.empty())
        {
        ctx.Issues().ReportV("No property or enumeration found for expression '%s'.", m_propertyPath.ToString().c_str());
        return ERROR;
        }

    if (classMatches.size() > 1)
        {
        ctx.Issues().ReportV("Expression '%s' in ECSQL statement is ambiguous.", m_propertyPath.ToString().c_str());
        return ERROR;
        }

    BeAssert(classMatches.size() <= 1);
    RangeClassRefExp const* classRefExp = nullptr;
    Utf8String error;
    bool verifyPropNameClassRefs = !propNameClassRefExpMatches.empty();

    if (classMatches.size() == 1)
        {
        classRefExp = classMatches[0].first;
        const size_t classIndex = classMatches[0].second;
        Utf8StringCR classAlias = m_propertyPath[classIndex].GetName();

        PropertyPath effectivePropPath(m_propertyPath);

        //Remove the schema and class alias from the property path
        for (size_t i = 0; i <= classIndex; i++)
            {
            effectivePropPath.Remove(0);
            }

        if (SUCCESS == ResolveColumnRef(error, *classRefExp, effectivePropPath))
            {
            //Must assign classAlias first because classAlias points into m_propertyPath which gets modified in the second line
            m_className = classAlias;
            m_propertyPath = effectivePropPath;
            verifyPropNameClassRefs = false; //do not verify prop names as class alias resolved correctly
            }
        }


    //Alias has priority over prop name. So only verify prop name if class alias failed or no class alias matches exist
    if (verifyPropNameClassRefs)
        {
        if (propNameClassRefExpMatches.size() > 1)
            {
            ctx.Issues().ReportV("Expression '%s' in ECSQL statement is ambiguous. Class alias might be missing.", m_propertyPath.ToString().c_str());
            return ERROR;
            }

        classRefExp = propNameClassRefExpMatches[0];
        error.clear();
        ResolveColumnRef(error, *classRefExp, m_propertyPath);
        }

    if (!error.empty())
        {
        ctx.Issues().Report(error.c_str());
        return ERROR;
        }

    SetClassRefExp(*classRefExp);
    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyNameExp::SetClassRefExp(RangeClassRefExp const& resolvedClassRef) { m_classRefExp = &resolvedClassRef; }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyNameExp::SetPropertyRef(DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp)
    {
    m_propertyRef = std::unique_ptr<PropertyRef>(new PropertyRef(derivedPropertyExpInSubqueryRefExp)); 
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
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
            DerivedPropertyExp const& referencedDerivedPropertyRef = propertyRef->LinkedTo();
            if (referencedDerivedPropertyRef.GetExpression()->GetType() != Exp::Type::PropertyName)
                {
                BeAssert(referencedDerivedPropertyRef.GetExpression()->GetType() == Exp::Type::PropertyName && "Exp of a derived prop exp referenced from a sub query ref is expected to always be a prop name exp");
                }
            propertyMap = &referencedDerivedPropertyRef.GetExpression()->GetAs<PropertyNameExp>().GetPropertyMap();
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
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PropertyNameExp::IsLhsAssignmentOperandExpression() const
    {
    if (FindParent(Exp::Type::Insert))
        return GetParent()->GetType() == Exp::Type::PropertyNameList;

    if (FindParent(Exp::Type::Update))
        return GetParent()->GetType() == Exp::Type::Assignment;

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyNameExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    if (!m_className.empty())
        ctx.AppendToECSql(m_className).AppendToECSql(".");

    ctx.AppendToECSql(m_propertyPath.ToString(true, false));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PropertyNameExp::_ToString() const
    {
    Utf8String str("PropertyName [");
    str.append(ToECSql()).append("]");
    return str;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
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
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyNameExp::PropertyRef::ToNativeSql(NativeSqlBuilder::List const& snippets) const
    {
    m_nativeSqlSnippets.clear();
    m_wasToNativeSqlCalled = false;
    Utf8String alias = m_linkedTo.GetColumnAlias();
    if (alias.empty())
        alias = m_linkedTo.GetNestedAlias();

    if (!alias.empty())
        {
        m_nativeSqlSnippets = snippets;
        if (m_nativeSqlSnippets.size() == 1)
            {
            m_nativeSqlSnippets.front().Clear();
            m_nativeSqlSnippets.front().AppendEscaped(alias.c_str());
            m_wasToNativeSqlCalled = true;
            }
        else
            {
            int idx = 0;
            Utf8String postfix;
            for (NativeSqlBuilder& snippet : m_nativeSqlSnippets)
                {
                postfix.clear();
                postfix.Sprintf("%s_%d", alias.c_str(), idx++);
                snippet.Clear();
                snippet.AppendEscaped(postfix.c_str());
                }

            m_wasToNativeSqlCalled = true;
            }
        }

    return m_wasToNativeSqlCalled ? SUCCESS : ERROR;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
