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

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// Dynamically created ECClass that represents the select clause of an ECSQL statement. Each item in the select
// clause is an ECProperty of the class.
// @bsiclass                                                 Krischan.Eberle     10/2013
//+===============+===============+===============+===============+===============+======
struct DynamicSelectClauseECClass
    {
private:
    static WCharCP const SCHEMANAME;
    static WCharCP const CLASSNAME;

    ECN::ECSchemaPtr m_schema;
    ECN::ECClassP m_class;

    ECSqlStatus Initialize ();

    ECSqlStatus AddReferenceToStructSchema (ECN::ECSchemaCR structSchema) const;
    ECN::ECClassR GetClassR () const;
    ECN::ECSchemaR GetSchemaR () const;
    ECSqlStatus SetBackReferenceToPropertyPath(ECPropertyR generatedProperty, DerivedPropertyExp const& selectClauseItemExp, ECDbCR ecdb);


public:
    DynamicSelectClauseECClass ();
    DynamicSelectClauseECClass (DynamicSelectClauseECClass const& rhs);
    DynamicSelectClauseECClass& operator= (DynamicSelectClauseECClass const& rhs);
    DynamicSelectClauseECClass (DynamicSelectClauseECClass&& rhs);
    DynamicSelectClauseECClass& operator= (DynamicSelectClauseECClass&& rhs);

    ECSqlStatus AddProperty (ECN::ECPropertyCP& generatedProperty, DerivedPropertyExp const& selectClauseItemExp, ECDbCR ecdb);
    bool IsGeneratedProperty (ECN::ECPropertyCR selectClauseProperty) const;
    static BentleyStatus ParseBackReferenceToPropertyPath(PropertyPath& propertyPath, ECPropertyCR generatedProperty, ECDbCR edb);

    };

END_BENTLEY_SQLITE_EC_NAMESPACE
