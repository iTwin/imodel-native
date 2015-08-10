/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParseContext.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlParseContext.h"
#include "ValueExp.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     06/2015
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ECSqlParseContext::FinalizeParsing(Exp& rootExp)
    {
    ECSqlStatus stat = rootExp.FinalizeParsing(*this);
    if (stat != ECSqlStatus::Success)
        return stat;
        
    for (ParameterExp* parameterExp : m_parameterExpList)
        {
        if (!parameterExp->TryDetermineParameterExpType(*this, *parameterExp))
            parameterExp->SetDefaultTargetExpInfo();
        }

    return ECSqlStatus::Success;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ECSqlParseContext::PushFinalizeParseArg (void const* const arg)
    {
    m_finalizeParseArgs.push_back (arg);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void const* const ECSqlParseContext::GetFinalizeParseArg () const
    {
    if (m_finalizeParseArgs.empty ())
        return nullptr;

    return m_finalizeParseArgs [m_finalizeParseArgs.size () - 1];
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ECSqlParseContext::PopFinalizeParseArg ()
    {
    m_finalizeParseArgs.pop_back ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       04/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlStatus ECSqlParseContext::TryResolveClass (shared_ptr<ClassNameExp::Info>& classNameExpInfo, Utf8StringCR schemaNameOrPrefix, Utf8StringCR className) 
    {
    ECClassCP resolvedClass = m_ecdb.Schemas ().GetECClass (schemaNameOrPrefix.c_str (), className.c_str (), ResolveSchema::AutoDetect);

    if (resolvedClass == nullptr)
        {
        if (schemaNameOrPrefix.empty ())
            return SetError (ECSqlStatus::InvalidECSql, "ECClass '%s' does not exist. Try using fully qualified class name: <schema name>.<class name>.", className.c_str ());
        else
            return SetError (ECSqlStatus::InvalidECSql, "ECClass '%s.%s' does not exist.", schemaNameOrPrefix.c_str (), className.c_str ());
        }

    auto key = resolvedClass->GetSchema().GetName()  + ":" + resolvedClass->GetName();
    auto search = m_classNameExpInfoList.find(key) ;
    if (search != m_classNameExpInfoList.end())
        {
        classNameExpInfo = search->second;
        return ECSqlStatus::Success;
        }

    auto map = m_ecdb.GetECDbImplR().GetECDbMap ().GetClassMap (*resolvedClass);
    if (map == nullptr)
        return ECSqlStatus::ProgrammerError;

    auto policy = ECDbPolicyManager::GetClassPolicy (*map, IsValidInECSqlPolicyAssertion::Get ());
    if (!policy.IsSupported ())
        return SetError (ECSqlStatus::InvalidECSql, "Invalid ECClass '%s': %s", className.c_str (), policy.GetNotSupportedMessage ());

    IClassMap const& classMapView = map->GetView (m_classMapViewMode);
    classNameExpInfo = ClassNameExp::Info::Create (classMapView);
    m_classNameExpInfoList[key] = classNameExpInfo;

    return ECSqlStatus::Success;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                03/2014
//+---------------+---------------+---------------+---------------+---------------+------
int ECSqlParseContext::TrackECSqlParameter (ParameterExp& parameterExp)
    {
    m_parameterExpList.push_back(&parameterExp);

    const bool isNamedParameter = parameterExp.IsNamedParameter ();
    Utf8CP paramName = isNamedParameter ? parameterExp.GetParameterName () : nullptr;

    if (isNamedParameter)
        {
        auto it = m_ecsqlParameterNameToIndexMapping.find (paramName);
        if (it != m_ecsqlParameterNameToIndexMapping.end ())
            return it->second;
        }

    m_currentECSqlParameterIndex++;
    if (isNamedParameter)
        m_ecsqlParameterNameToIndexMapping[paramName] = m_currentECSqlParameterIndex;

    return m_currentECSqlParameterIndex;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlParseContext::GetSubclasses(ClassListById& classes, ECClassCR ecClass)
    {    
    for (auto derivedClass : Schemas ().GetDerivedECClasses (const_cast<ECClassR>(ecClass)))
        {
        if (classes.find(derivedClass->GetId()) == classes.end())
            {
            classes [derivedClass->GetId()] = derivedClass;
            GetSubclasses( classes, *derivedClass);
            }
        }
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlParseContext::GetConstraintClasses (ClassListById& classes, ECRelationshipConstraintCR constraintEnd, bool* containAnyClass)
    {
    if (containAnyClass)
        *containAnyClass = false;
    for(auto ecClass : constraintEnd.GetClasses())
        {
        if (containAnyClass && !(*containAnyClass) && ecClass->GetName () == "AnyClass" && ecClass->GetSchema ().GetName () == "Bentley_Standard_Classes")
            *containAnyClass = true;

        if (classes.find(ecClass->GetId()) == classes.end())
            {
            classes [ecClass->GetId()] = ecClass;
            if (constraintEnd.GetIsPolymorphic())
                GetSubclasses(classes, *ecClass);
            }
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSqlParseContext::IsEndClassOfRelationship (ECClassCR searchClass, ECRelationshipEnd searchEnd, ECRelationshipClassCR relationshipClass)
    {
    ECRelationshipConstraintCR constraintEnd = 
        (searchEnd == ECRelationshipEnd::ECRelationshipEnd_Source) ? relationshipClass.GetSource() : relationshipClass.GetTarget();

    bmap<ECClassId,ECClassCP> classes;
    bool containAnyClass;
    GetConstraintClasses(classes, constraintEnd, &containAnyClass);  
    if (containAnyClass)
        return true;

    return classes.find(searchClass.GetId()) != classes.end() ;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       08/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlParseContext::GenerateAlias()
    {
    Utf8String alias;
    alias.Sprintf("K%d", m_aliasCount++);
    return alias;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ECDbSchemaManagerCR ECSqlParseContext::Schemas () const
    {
    return GetECDb ().Schemas ();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     02/2014
//+---------------+---------------+---------------+---------------+---------------+--------
ECDbCR ECSqlParseContext::GetECDb () const
    {
    return m_ecdb;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                     09/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ECSqlStatus ECSqlParseContext::SetError (ECSqlStatus status, Utf8CP fmt, ...)
    {
    va_list args;
    va_start(args, fmt);
    m_status.SetErrorV (status, fmt, args);
    va_end(args);

    return status;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
