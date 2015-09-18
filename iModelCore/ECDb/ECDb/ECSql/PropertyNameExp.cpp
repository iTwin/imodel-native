/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PropertyNameExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

#include "PropertyNameExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using namespace std;

//****************************** PropertyNameExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (PropertyPath&& propPath) : ValueExp (), m_propertyPath (std::move (propPath)), m_isSystemProperty (false), m_classRefExp (nullptr)
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (Utf8CP propertyName) : ValueExp (), m_isSystemProperty (false), m_classRefExp (nullptr)
    {
    m_propertyPath.Push (propertyName);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp) 
: ValueExp (), m_isSystemProperty (false), m_classAlias (classRefExp.GetAlias ()), m_classRefExp (&classRefExp)
    {
    m_propertyPath.Push (derivedPropExp.GetName ().c_str());
    SetPropertyRef (derivedPropExp);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (Utf8CP propertyName, RangeClassRefExp const& classRefExp, IClassMap const& classMap) 
: ValueExp (), m_isSystemProperty (false), m_classAlias (classRefExp.GetAlias ()), m_classRefExp (&classRefExp)
    {
    m_propertyPath.Push (propertyName);
    if (m_propertyPath.Resolve (classMap) != SUCCESS)
        BeAssert (false && "Must always resolve correctly");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus PropertyNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        if (ResolveColumnRef (ctx) != SUCCESS)
            return FinalizeParseStatus::Error;
        else
            return FinalizeParseStatus::NotCompleted;
        }

    //After children have been finalized
    if (!IsPropertyRef() && !m_propertyPath.IsResolved ())
        {
        BeAssert (false && "PropertyNameExp property path is expected to be resolved at end of parsing.");
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "PropertyNameExp property path is expected to be resolved at end of parsing.");
        return FinalizeParseStatus::Error;
        }

    if (IsPropertyRef())
        {
        auto& derivedProperty = GetPropertyRefP ()->LinkedTo();
        if (derivedProperty.GetNestedAlias().empty())
            {            
            const_cast<DerivedPropertyExp&>(derivedProperty).SetNestedAlias (ctx.GenerateAlias ().c_str ());
            }

      
        auto stat = const_cast<DerivedPropertyExp&>(derivedProperty).FinalizeParsing (ctx);
        if (stat != SUCCESS)
            return FinalizeParseStatus::Error;

        SetTypeInfo (derivedProperty.GetExpression()->GetTypeInfo ());
        }
    else
        SetTypeInfo (ECSqlTypeInfo (GetPropertyMap ()));

    if (IsPropertyRef ())
        return FinalizeParseStatus::Completed;

    //determine whether the exp refers to a system property
    ECSqlSystemProperty systemPropKind;
    m_isSystemProperty = ECDbSystemSchemaHelper::TryGetSystemPropertyKind (systemPropKind, GetPropertyMap ().GetProperty ());
    if (m_isSystemProperty)
        m_systemProperty = systemPropKind;

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus PropertyNameExp::ResolveColumnRef(ECSqlParseContext& ctx)
    {
    if (GetClassRefExp() != nullptr)
        return SUCCESS;

    void const* finalizeParseArgs = ctx.GetFinalizeParseArg();
    BeAssert(finalizeParseArgs != nullptr && "PropertyNameExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
    RangeClassRefList const& rangeClassRefList = *static_cast<RangeClassRefList const*> (finalizeParseArgs);

    BeAssert(!m_propertyPath.IsEmpty());

    //work with copy of property path
    Utf8CP firstPropPathEntry = m_propertyPath[0].GetPropertyName();
    Utf8CP secondPropPathEntry = m_propertyPath.Size() > 1 ? m_propertyPath[1].GetPropertyName() : nullptr;

    bvector<RangeClassRefExp const*> aliasClassRefExpMatches;
    bvector<RangeClassRefExp const*> propNameClassRefExpMatches;
    int classAliasMatches = 0;
    for (RangeClassRefExp const* rangeClassRef : rangeClassRefList)
        {
        //assume first entry is prop name
        if (rangeClassRef->ContainProperty(firstPropPathEntry))
            propNameClassRefExpMatches.push_back(rangeClassRef);

        //assume first entry is class alias
        if (secondPropPathEntry != nullptr)
            {
            //alias can be duplicate. Its fine as long as property can be detected uniquely
            if (rangeClassRef->GetId().Equals(firstPropPathEntry))
                {
                classAliasMatches++;
                if (rangeClassRef->ContainProperty(secondPropPathEntry))
                    aliasClassRefExpMatches.push_back(rangeClassRef);

                }
            }
        }

    if (aliasClassRefExpMatches.empty() && propNameClassRefExpMatches.empty())
        {
        if (classAliasMatches > 0)
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECProperty '%s' not found in ECClass with alias '%s'.", secondPropPathEntry, firstPropPathEntry);
        else
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECProperty expression '%s' does not match with any of the ECClasses in the ECSQL statement.", m_propertyPath.ToString().c_str());

        return ERROR;
        }

    if (aliasClassRefExpMatches.size() > 1)
        {
        if (classAliasMatches > 1)
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECProperty expression '%s' in ECSQL statement is ambiguous. Duplicate definition of class alias.", m_propertyPath.ToString().c_str());
        else
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Duplicate ECProperty '%s' in ECClass with alias '%s'.", secondPropPathEntry, firstPropPathEntry);

        return ERROR;
        }

    BeAssert(aliasClassRefExpMatches.size() <= 1);

    RangeClassRefExp const* classRefExp = nullptr;
    Utf8CP classAlias = nullptr;
    Utf8String error;
    bool verifyPropNameClassRefs = !propNameClassRefExpMatches.empty();

    if (aliasClassRefExpMatches.size() == 1)
        {
        classRefExp = aliasClassRefExpMatches[0];
        classAlias = firstPropPathEntry;
        //Remove the alias from the property path
        PropertyPath effectivePropPath(m_propertyPath);
        effectivePropPath.Remove(0);

        if (SUCCESS == ResolveColumnRef(error, *classRefExp, effectivePropPath))
            {
            //Must assign classAlias first because classAlias points into m_propertyPath which gets modified in the second line
            m_classAlias = classAlias;
            m_propertyPath = effectivePropPath;
            verifyPropNameClassRefs = false; //do not verify prop names as class alias resolved correctly
            }
        }


    //Alias has priority over prop name. So only very prop name if class alias failed or no class alias matches exist
    if (verifyPropNameClassRefs)
        {
        if (propNameClassRefExpMatches.size() > 1)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECProperty expression '%s' in ECSQL statement is ambiguous. Class alias might be missing.", m_propertyPath.ToString().c_str());
            return ERROR;
            }

        classRefExp = propNameClassRefExpMatches[0];
        error.clear();
        ResolveColumnRef(error, *classRefExp, m_propertyPath);
        }

    if (!error.empty())
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, error.c_str());
        return ERROR;
        }

    SetClassRefExp(*classRefExp);
    return SUCCESS;
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
                SubqueryRefExp const& subQueryRef = static_cast<SubqueryRefExp const&>(classRefExp);
                DerivedPropertyExp const* derivedPropertyRef = subQueryRef.GetSubquery()->FindProperty(propPath[0].GetPropertyName());
                BeAssert(derivedPropertyRef != nullptr && "Should not be nullptr as the classRefExp was found in the first place by checking that the property exists");

                if (derivedPropertyRef != nullptr)
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
                ClassNameExp const& classNameExp = static_cast<ClassNameExp const&>(classRefExp);
                IClassMap const& classMap = classNameExp.GetInfo().GetMap();
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
void PropertyNameExp::SetClassRefExp (RangeClassRefExp const& resolvedClassRef)
    {
    m_classRefExp = &resolvedClassRef;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyNameExp::SetPropertyRef(DerivedPropertyExp const& derivedPropertyExpInSubqueryRefExp)
    {
    m_propertyRef = std::unique_ptr<PropertyRef>(new PropertyRef(derivedPropertyExpInSubqueryRefExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8CP PropertyNameExp::GetPropertyName() const
    {
    return m_propertyPath[0].GetPropertyName();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyMapCR PropertyNameExp::GetPropertyMap() const
    {    
    BeAssert (GetClassRefExp () != nullptr);
    if (GetClassRefExp ()->GetType () == Exp::Type::ClassName)
        {
        auto classNameExp = static_cast<ClassNameExp const*>(GetClassRefExp ());
        auto propertyMap = classNameExp->GetInfo ().GetMap ().GetPropertyMap (GetPropertyPath ().ToString (false).c_str ());
        BeAssert (propertyMap != nullptr && "PropertyNameExp's PropertyMap should never be nullptr.");
        return *propertyMap;
        }

    if (GetClassRefExp ()->GetType () == Exp::Type::SubqueryRef)
        {
        auto propertyRef = GetPropertyRef ();
        BeAssert (propertyRef != nullptr);
        auto const& derivedPropertyRef = propertyRef->LinkedTo ();
        if (derivedPropertyRef.GetExpression ()->GetType () == Exp::Type::PropertyName)
            {
            return static_cast<PropertyNameExp const*>(derivedPropertyRef.GetExpression ())->GetPropertyMap ();
            }
        }

    BeAssert (false && "Case not handled");
    return *((PropertyMapCP)(nullptr));
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    07/2014
//+---------------+---------------+---------------+---------------+---------------+--------
bool PropertyNameExp::TryGetSystemProperty (ECSqlSystemProperty& systemProperty) const
    {
    if (!IsSystemProperty ())
        return false;

    systemProperty = m_systemProperty;
    return true;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyNameExp::_DoToECSql(Utf8StringR ecsql) const
    {
    if (!m_classAlias.empty ())
        ecsql.append (m_classAlias).append (".");

    ecsql.append (m_propertyPath.ToString ());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String PropertyNameExp::_ToString() const 
    {
    Utf8String str ("PropertyName [");
    str.append (ToECSql ()).append ("]");
    return str;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
DerivedPropertyExp const& PropertyNameExp::PropertyRef::GetEndPointDerivedProperty () const
    {
    if (LinkedTo ().GetExpression ()->GetType () == Exp::Type::PropertyName)
        {
        auto next = static_cast<PropertyNameExp const*>(LinkedTo ().GetExpression ());
        if (next->IsPropertyRef ())
            return next->GetPropertyRef ()->GetEndPointDerivedProperty ();
        }

    return LinkedTo ();
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp const* PropertyNameExp::PropertyRef::GetEndPointPropertyNameIfAny () const
    {
    auto const& cur = GetEndPointDerivedProperty ();
    if (cur.GetExpression ()->GetType () == Exp::Type::PropertyName)
        return static_cast<PropertyNameExp const*>(cur.GetExpression ());

    return nullptr;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool PropertyNameExp::PropertyRef::Prepare(NativeSqlBuilder::List const& snippets)
    {
    m_nativeSqlSnippets.clear();
    m_isPrepared = false;
    auto alias = m_linkedTo.GetColumnAlias();
    if (alias.empty())
        alias = m_linkedTo.GetNestedAlias();

    if (!alias.empty())
        {
        m_nativeSqlSnippets = snippets;
        if (m_nativeSqlSnippets.size() == 1LL)
            {
            m_nativeSqlSnippets.front().Reset();
            m_nativeSqlSnippets.front().AppendEscaped(alias.c_str());
            m_isPrepared = true;
            }
        else
            {
            int idx = 0;
            Utf8String postfix;
            for (auto& snippet : m_nativeSqlSnippets)
                {
                postfix.clear();
                postfix.Sprintf("%s_%d", alias.c_str(), idx++);
                snippet.Reset();
                snippet.AppendEscaped(postfix.c_str());
                }

            m_isPrepared = true;
            }
        }

    return m_isPrepared;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
