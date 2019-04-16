/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
struct DynamicSelectClauseECClass final
    {
    private:
        ECN::ECSchemaPtr m_schema = nullptr;
        ECN::ECEntityClassP m_class = nullptr;
        bmap<Utf8String, DerivedPropertyExp const*, CompareIUtf8Ascii> m_selectClauseNames;

        //not copyable
        DynamicSelectClauseECClass(DynamicSelectClauseECClass const&) = delete;
        DynamicSelectClauseECClass& operator=(DynamicSelectClauseECClass const&) = delete;

        ECSqlStatus Initialize();

        ECSqlStatus AddProperty(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext&, Utf8StringCR propName, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp);
        BentleyStatus AddSchemaReference(ECN::ECSchemaCR schemaToReference) const;
        ECN::ECEntityClassR GetClass() const { BeAssert(m_class != nullptr); return *m_class; }
        ECN::ECSchemaR GetSchema() const { BeAssert(m_schema != nullptr); return *m_schema; }
        BentleyStatus AssignCustomAttributes(ECSqlPrepareContext&, ECN::ECProperty& dtProp, ECSqlTypeInfo const& typeInfo) const;

    public:
        DynamicSelectClauseECClass() {}
        ECSqlStatus GeneratePropertyIfRequired(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext&, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
