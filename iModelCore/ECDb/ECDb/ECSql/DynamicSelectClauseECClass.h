/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "SelectStatementExp.h"
#include "ECSqlPrepareContext.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// Dynamically created ECClass that represents the select clause of an ECSQL statement. Each item in the select
// clause is an ECProperty of the class.
// @bsiclass
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

        ECSqlStatus AddProperty(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext&, Utf8StringCR propName, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp, bool isDynamic);
        BentleyStatus AddSchemaReference(ECN::ECSchemaCR schemaToReference) const;
        ECN::ECEntityClassR GetClass() const { BeAssert(m_class != nullptr); return *m_class; }
        ECN::ECSchemaR GetSchema() const { BeAssert(m_schema != nullptr); return *m_schema; }
        BentleyStatus AssignCustomAttributes(ECSqlPrepareContext&, ECN::ECProperty& dtProp, ECSqlTypeInfo const& typeInfo) const;

    public:
        DynamicSelectClauseECClass() {}
        ECSqlStatus GeneratePropertyIfRequired(ECN::ECPropertyCP& generatedProperty, ECSqlPrepareContext&, DerivedPropertyExp const& selectClauseItemExp, PropertyNameExp const* selectClauseItemPropNameExp, bool isDynamic);
        ECSqlStatus CheckForDuplicateName(Utf8StringCR propName, Utf8StringCR columnAlias, bool& isDuplicate, ECSqlPrepareContext& ctx); //Checks if a column of the given name is already selected
        void RegisterSelectClauseItem(Utf8StringCR propName, DerivedPropertyExp const& selectClauseItemExp);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
