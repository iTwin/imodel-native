/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/DynamicSelectClauseECClass.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "SelectStatementExp.h"
#include "ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// Dynamically created ECClass that represents the select clause of an ECSQL statement. Each item in the select
// clause is an ECProperty of the class.
// @bsiclass                                                 Krischan.Eberle     10/2013
//+===============+===============+===============+===============+===============+======
struct DynamicSelectClauseECClass
    {
private:
    static Utf8CP const SCHEMANAME;
    static Utf8CP const CLASSNAME;

    ECN::ECSchemaPtr m_schema;
    ECN::ECEntityClassP m_class;
    bmap<Utf8String, DerivedPropertyExp const*> m_selectClauseNames;

    ECSqlStatus Initialize ();

    ECSqlStatus AddProperty(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext&, Utf8CP propName, DerivedPropertyExp const& selectClauseItemExp, ECDbCR);
    ECSqlStatus AddReferenceToStructSchema(ECN::ECSchemaCR structSchema) const;
    ECN::ECClassR GetClassR() const { return *m_class; }
    ECN::ECSchemaR GetSchemaR() const { return *m_schema; }
    ECSqlStatus SetBackReferenceToPropertyPath(ECPropertyR generatedProperty, DerivedPropertyExp const& selectClauseItemExp, ECDbCR ecdb);

public:
    DynamicSelectClauseECClass ();
    DynamicSelectClauseECClass (DynamicSelectClauseECClass const& rhs);
    DynamicSelectClauseECClass& operator= (DynamicSelectClauseECClass const& rhs);
    DynamicSelectClauseECClass (DynamicSelectClauseECClass&& rhs);
    DynamicSelectClauseECClass& operator= (DynamicSelectClauseECClass&& rhs);

    ECSqlStatus GeneratePropertyIfRequired(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext&, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp, ECDbCR);
    bool IsGeneratedProperty (ECN::ECPropertyCR selectClauseProperty) const;
    static BentleyStatus ParseBackReferenceToPropertyPath(PropertyPath& propertyPath, ECPropertyCR generatedProperty, ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
