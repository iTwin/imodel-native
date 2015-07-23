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

bool PropertyNameExp::PropertyRef::Prepare (NativeSqlBuilder::List const& snippets)
    {
    m_nativeSqlSnippets.clear ();
    m_isPrepared = false;
    auto alias = m_linkedTo.GetColumnAlias ();
    if (alias.empty ())
        alias = m_linkedTo.GetNestedAlias ();

    if (!alias.empty ())
        {
        m_nativeSqlSnippets = snippets;
        if (m_nativeSqlSnippets.size () == 1LL)
            {
            m_nativeSqlSnippets.front ().Reset ();
            m_nativeSqlSnippets.front ().AppendEscaped (alias.c_str ());
            m_isPrepared = true;
            }
        else
            {
            int idx = 0;
            Utf8String postfix;
            for (auto& snippet : m_nativeSqlSnippets)
                {
                postfix.clear ();
                postfix.Sprintf ("%s_%d", alias.c_str (), idx++);
                snippet.Reset ();
                snippet.AppendEscaped (postfix.c_str ());
                }

            m_isPrepared = true;
            }     
        }

    return m_isPrepared;
    }
//****************************** PropertyNameExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (PropertyPath&& propPath) 
    : ValueExp (), m_propertyPath (std::move (propPath)), m_isSystemProperty (false), m_classRefExp (nullptr)
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (Utf8CP propertyName) 
: ValueExp (), m_isSystemProperty (false), m_classRefExp (nullptr)
    {
    m_propertyPath.Push (propertyName);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (RangeClassRefExp const& classRefExp, DerivedPropertyExp const& derivedPropExp) 
: ValueExp (), m_isSystemProperty (false), m_classAlias (classRefExp.GetAlias ()), m_classRefExp (&classRefExp)
    {
    m_propertyPath.Push (derivedPropExp.GetName ());
    SetPropertyRef (derivedPropExp);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyNameExp::PropertyNameExp (Utf8CP propertyName, RangeClassRefExp const& classRefExp, IClassMap const& classMap) 
: ValueExp (), m_isSystemProperty (false), m_classAlias (classRefExp.GetAlias ()), m_classRefExp (&classRefExp)
    {
    m_propertyPath.Push (propertyName);
    if (m_propertyPath.Resolve (classMap) != PropertyPath::Status::Success)
        BeAssert (false && "Must always resolve correctly");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus PropertyNameExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        if (ResolveColumnRef (ctx) != ECSqlStatus::Success)
            return FinalizeParseStatus::Error;
        else
            return FinalizeParseStatus::NotCompleted;
        }

    //After children have been finalized
    if (!IsPropertyRef() && !m_propertyPath.IsResolved ())
        {
        BeAssert (false && "PropertyNameExp property path is expected to be resolved at end of parsing.");
        ctx.SetError (ECSqlStatus::ProgrammerError, "PropertyNameExp property path is expected to be resolved at end of parsing.");
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
        if (stat != ECSqlStatus::Success)
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

    auto policy = ECDbPolicyManager::GetPropertyPolicy (GetPropertyMap (), IsValidInECSqlPolicyAssertion::Get ());
    if (!policy.IsSupported ())
        {
        ctx.SetError (ECSqlStatus::InvalidECSql, "Expression '%s' refers to an ECProperty not supported by ECSQL: %s", ToECSql ().c_str (), policy.GetNotSupportedMessage ());
        return FinalizeParseStatus::Error;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus PropertyNameExp::ResolveColumnRef (ECSqlParseContext& ctx)
    {
    if (GetClassRefExp () != nullptr)
        return ECSqlStatus::Success;

    auto finalizeParseArgs = ctx.GetFinalizeParseArg ();
    BeAssert (finalizeParseArgs != nullptr && "PropertyNameExp::_FinalizeParsing: ECSqlParseContext::GetFinalizeParseArgs is expected to return a RangeClassRefList.");
    RangeClassRefList const& rangeClassRefList = *static_cast<RangeClassRefList const*> (finalizeParseArgs);

    auto const& propPath = GetPropertyPath ();

    auto firstEntryMightBeAlias = propPath.Size() > 1;

    
    auto const& firstPropPathEntry = GetPropertyPath().At(0).GetPropertyName();

    bvector<RangeClassRefExp const*> matchingClassRefExpList;
    //1. Assume that the first entry of the prop path is a property name and not a class alias and directly search it in classrefs
    for (auto rangeClassRef : rangeClassRefList)
        {
        if (rangeClassRef->ContainProperty(firstPropPathEntry))
            matchingClassRefExpList.push_back(rangeClassRef);
        }

    if (matchingClassRefExpList.size() > 1) 
        return ctx.SetError(ECSqlStatus::InvalidECSql, "ECProperty expression '%s' in ECSQL statement is ambiguous. Class alias might be missing.", propPath.ToString().c_str());

    RangeClassRefExp const* classRefExp = nullptr;
    if (matchingClassRefExpList.size() == 1)
        {
        classRefExp = matchingClassRefExpList[0];
        }
    else if (firstEntryMightBeAlias)    //2. Presume the first entry is an alias
        {
        BeAssert (matchingClassRefExpList.size () == 0);
        auto const& potentialAlias = firstPropPathEntry;
        auto const& propName = GetPropertyPath().At(1).GetPropertyName();
        int classAliasMatches = 0;
        for(auto rangeClassRef : rangeClassRefList)
            {
            //alias can be duplicate. Its fine as long as property can be detected uniquely
            if (rangeClassRef->GetId ().Equals (potentialAlias))
                {
                classAliasMatches++;
                if (rangeClassRef->ContainProperty(propName))
                    matchingClassRefExpList.push_back(rangeClassRef);

                }
            }

        const auto matchCount = matchingClassRefExpList.size ();
        if (matchCount == 1)
            {
            m_classAlias = potentialAlias;
            //Remove the alias from the property path
            m_propertyPath.Remove(0);
            classRefExp = matchingClassRefExpList[0];
            }
        else if (matchCount == 0)
            {
            if (classAliasMatches == 0)
                return ctx.SetError (ECSqlStatus::InvalidECSql, "ECProperty expression '%s' does not match with any of the ECClasses in the ECSQL statement.", propPath.ToString().c_str());
            else
                return ctx.SetError (ECSqlStatus::InvalidECSql, "ECProperty '%s' not found in ECClass with alias '%s'.", propName.c_str (), potentialAlias.c_str ());
            }
        else //matchCount > 1
            {
            if (classAliasMatches > 1)
                return ctx.SetError(ECSqlStatus::InvalidECSql, "ECProperty expression '%s' in ECSQL statement is ambiguous. Duplicate definition of class alias.", propPath.ToString().c_str());
            else
                return ctx.SetError (ECSqlStatus::InvalidECSql, "Duplicate ECProperty '%s' in ECClass with alias '%s'.", propName.c_str (), m_classAlias.c_str ());
            }
        }
    else
        return ctx.SetError (ECSqlStatus::InvalidECSql, "ECProperty '%s' not found in any of the ECClasses used in the ECSQL statement.", propPath.ToString(false).c_str());

    SetClassRefExp(*classRefExp);

    switch (classRefExp->GetType())
        {
        case Exp::Type::SubqueryRef:
            {
            //WIP_ECSQL: Why is prop path not resolved for this case?
            SubqueryRefExp const* subQueryRef = static_cast<SubqueryRefExp const*>(classRefExp);
            auto derivedPropertyRef = subQueryRef->GetSubquery()->FindProperty(propPath.At(0).GetPropertyName ());
            if (derivedPropertyRef)
                SetPropertyRef(*derivedPropertyRef);
            
            //1. Select statement must have a ECClass define it. 
            //2. ECClass must have classMap info for this to work
            //3. In context of query SELECT NewFoo.Prop1, NewFoo.Prop2 FROM (SELECT A Prop1, B + C Prop2 FROM Foo WHERE A = 12)  NewFoo;
            //4. SELECT _a, [_b + _c]  FROM (SELECT _a "_a", _b + _c  "_b + _c" FROM _foo where _a = 12) NewFoo
            //5. SELECT _a, [_b + _c] FROM newFoo;
            //6. Scalar query.
            break;

            }
        case Exp::Type::ClassName:
            {
            ClassNameExp const* classNameExp = static_cast<ClassNameExp const*>(classRefExp);
            auto& classMap = classNameExp->GetInfo().GetMap();
            auto status = m_propertyPath.Resolve(classMap);
            switch(status)
                {
                case PropertyPath::Status::Success:
                    break;
                case PropertyPath::Status::PropertyPathIsEmpty:
                    BeAssert (false && "Invalid property. Property path is empty.");
                    return ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid property %s. Property path is empty.", propPath.ToString().c_str());
                case PropertyPath::Status::FailedToResolvePropertyName:
                    return ctx.SetError(ECSqlStatus::InvalidECSql, "ECProperty '%s' does not exist", propPath.ToString().c_str());
                case PropertyPath::Status::InvalidPropertyPath:
                    BeAssert (false && "Invalid property path");
                    return ctx.SetError(ECSqlStatus::ProgrammerError, "Invalid property path %s", propPath.ToString().c_str());
                case PropertyPath::Status::ArrayIndexNotSupported:
                    BeAssert (false && "Array index are not currently supported");
                    return ctx.SetError (ECSqlStatus::NotYetSupported, "Array index are not currently supported %s", propPath.ToString ().c_str ());

                default:
                    BeAssert (false && "Internal error while resolving property");
                    return ctx.SetError(ECSqlStatus::ProgrammerError, "Internal error while resolving property %s", propPath.ToString().c_str());
                }
            break;
            }
        default:
            BeAssert (false && "New subclass of RangeClassRefExp was added, but is not yet processed in ValueExp::ResolveColumnRef");
            return ctx.SetError(ECSqlStatus::ProgrammerError, "New subclass of RangeClassRefExp ('%s') exists which is not yet processed in ValueExp::ResolveColumnRef.", classRefExp->ToECSql ().c_str ());
        }

    return ECSqlStatus::Success;
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
RangeClassRefExp const* PropertyNameExp::GetClassRefExp() const
    {
    return m_classRefExp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                    10/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP PropertyNameExp::GetClassAlias() const
    {
    return m_classAlias.c_str ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR PropertyNameExp::GetPropertyName() const
    {
    return m_propertyPath.Bottom().GetPropertyName();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
PropertyPath const& PropertyNameExp::GetPropertyPath() const
    {
    return m_propertyPath;
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
END_BENTLEY_SQLITE_EC_NAMESPACE
