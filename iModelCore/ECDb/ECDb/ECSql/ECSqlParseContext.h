/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlParseContext.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ClassRefExp.h"
#include "ECSqlStatusContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ParameterExp;

//=======================================================================================
//! @bsiclass                                                Affan.Khan      03/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlParseContext
    {
    typedef bmap<ECN::ECClassId,ECN::ECClassCP> ClassListById;

private:
    ECDbCR m_ecdb;
    std::vector<void const*> m_finalizeParseArgs;
    bmap<Utf8String, std::shared_ptr<ClassNameExp::Info>> m_classNameExpInfoList;
    int m_currentECSqlParameterIndex;
    bvector<ParameterExp*> m_parameterExpList;
    bmap<Utf8CP, int, CompareUtf8> m_ecsqlParameterNameToIndexMapping;
    IClassMap::View m_classMapViewMode;
    ECSqlStatusContext& m_status;
    int m_aliasCount;
public:
    //! @param[in] classMapViewMode indicates what class map view to use for a given class in the ECSQL to parse.
    //!            ClassMap views differ for ECClasses that are domain classes and structs at the same time.
    ECSqlParseContext (ECDbCR ecdb, IClassMap::View classMapViewMode, ECSqlStatusContext& status)
        : m_ecdb (ecdb), m_currentECSqlParameterIndex (0), m_classMapViewMode (classMapViewMode), m_status (status), m_aliasCount (0)
        {}

    ECSqlStatus FinalizeParsing(Exp& rootExp);

    void PushFinalizeParseArg (void const* const arg);
    void const* const GetFinalizeParseArg () const;
    void PopFinalizeParseArg ();

    ECSqlStatus TryResolveClass (std::shared_ptr<ClassNameExp::Info>& classMetaInfo, Utf8StringCR schemaNameOrPrefix, Utf8StringCR className); 

    int TrackECSqlParameter (ParameterExp& parameterExp);

    bool IsSuccess () const { return m_status.IsSuccess ();} 
    ECSqlStatus GetStatus () const { return m_status.GetStatus ();}
    ECSqlStatus SetError (ECSqlStatus status, Utf8CP fmt, ...);
    ECSqlStatusContext& GetStatusContext () const {return m_status;}
    void GetSubclasses(ClassListById& classes, ECN::ECClassCR ecClass);
    void GetConstraintClasses (ClassListById& classes, ECN::ECRelationshipConstraintCR constraintEnd, bool* containAnyClass);
    bool IsEndClassOfRelationship (ECN::ECClassCR searchClass, ECN::ECRelationshipEnd searchEnd, ECN::ECRelationshipClassCR relationshipClass);

    ECDbCR GetECDb() const;
    ECDbSchemaManagerCR Schemas() const;

    Utf8String GenerateAlias();
    };



END_BENTLEY_SQLITE_EC_NAMESPACE
