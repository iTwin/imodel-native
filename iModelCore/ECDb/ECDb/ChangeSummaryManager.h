/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummaryManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//_BENTLEY_INTERNAL_ONLY_
#include "ChangeSummaryExtractor.h"
#include "ECDbSqlFunctions.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSCHEMA_ECDbChangeSummaries "ECDbChangeSummaries"
#define ECSCHEMA_ALIAS_ECDbChangeSummaries "change"

#define ECDBCHANGE_CLASS_ChangeSummary "ChangeSummary"
#define ECDBCHANGE_CLASS_InstanceChange "InstanceChange"
#define ECDBCHANGE_CLASS_PropertyValueChange "PropertyValueChange"

//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//=======================================================================================
struct ChangeSummaryManager final : NonCopyableClass
    {
    private:
        ECDbCR m_ecdb;
        ChangeSummaryExtractor m_extractor;
        mutable std::unique_ptr<ChangedValueSqlFunction> m_changedValueSqlFunction;

    public:
        explicit ChangeSummaryManager(ECDbCR ecdb) : m_ecdb(ecdb), m_extractor(ecdb) {}

        BentleyStatus SetupChangeSummaryCache() const;

        ChangeSummaryExtractor const& GetExtractor() const { return m_extractor; }

        void RegisterSqlFunctions() const;
        void UnregisterSqlFunction() const;

        void ClearCache();

        static Nullable<ChangeOpCode> ToChangeOpCode(DbOpcode);
        static Nullable<ChangeOpCode> ToChangeOpCode(int val);
        static Nullable<DbOpcode> ToDbOpCode(ChangeOpCode);
        static Nullable<ChangedValueState> ToChangedValueState(int val);
        static Nullable<ChangedValueState> ToChangedValueState(Utf8CP strVal);
        static Nullable<ChangeOpCode> DetermineOpCodeFromChangedValueState(ChangedValueState);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE

