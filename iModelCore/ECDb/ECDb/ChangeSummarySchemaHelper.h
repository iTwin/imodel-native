/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ChangeSummarySchemaHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSCHEMA_ECDbChangeSummaries "ECDbChangeSummaries"
#define ECSCHEMA_ALIAS_ECDbChangeSummaries "change"

#define ECDBCHANGE_CLASS_ChangeSummary "ChangeSummary"
#define ECDBCHANGE_CLASS_InstanceChange "InstanceChange"
#define ECDBCHANGE_CLASS_PropertyValueChange "PropertyValueChange"


//=======================================================================================
// @bsiclass                                            Krischan.Eberle      11/2017
//+===============+===============+===============+===============+===============+======
struct ChangeSummaryHelper final
    {
    private:
        ChangeSummaryHelper() = delete;
        ~ChangeSummaryHelper() = delete;

    public:

        static Nullable<ChangeOpCode> ToChangeOpCode(DbOpcode opCode)
            {
            switch (opCode)
                {
                    case DbOpcode::Delete:
                        return ChangeOpCode::Delete;
                    case DbOpcode::Insert:
                        return ChangeOpCode::Insert;
                    case DbOpcode::Update:
                        return ChangeOpCode::Update;
                    default:
                        BeAssert(false && "DbOpcode enum was changed. This code has to be adjusted.");
                        return Nullable<ChangeOpCode>();
                }
            }

        static Nullable<ChangeOpCode> ToChangeOpCode(int val)
            {
            if (val == Enum::ToInt(ChangeOpCode::Insert))
                return ChangeOpCode::Insert;

            if (val == Enum::ToInt(ChangeOpCode::Update))
                return ChangeOpCode::Update;

            if (val == Enum::ToInt(ChangeOpCode::Delete))
                return ChangeOpCode::Delete;

            return Nullable<ChangeOpCode>();
            }

        static Nullable<DbOpcode> ToDbOpCode(ChangeOpCode op)
            {
            switch (op)
                {
                    case ChangeOpCode::Delete:
                        return DbOpcode::Delete;
                    case ChangeOpCode::Insert:
                        return DbOpcode::Insert;
                    case ChangeOpCode::Update:
                        return DbOpcode::Update;
                    default:
                        BeAssert(false && "ChangeOpCode enum was changed. This code has to be adjusted.");
                        return Nullable<DbOpcode>();
                }
            }

        static Nullable<ChangedValueState> ToChangedValueState(int val)
            {
            if (val == Enum::ToInt(ChangedValueState::AfterInsert))
                return ChangedValueState::AfterInsert;

            if (val == Enum::ToInt(ChangedValueState::BeforeUpdate))
                return ChangedValueState::BeforeUpdate;

            if (val == Enum::ToInt(ChangedValueState::AfterUpdate))
                return ChangedValueState::AfterUpdate;

            if (val == Enum::ToInt(ChangedValueState::BeforeDelete))
                return ChangedValueState::BeforeDelete;

            return Nullable<ChangedValueState>();
            }

        static Nullable<ChangedValueState> ToChangedValueState(Utf8CP strVal)
            {
            if (BeStringUtilities::StricmpAscii(ENUM_TOSTRING(ChangedValueState::AfterInsert), strVal) == 0)
                return ChangedValueState::AfterInsert;

            if (BeStringUtilities::StricmpAscii(ENUM_TOSTRING(ChangedValueState::BeforeUpdate), strVal) == 0)
                return ChangedValueState::BeforeUpdate;

            if (BeStringUtilities::StricmpAscii(ENUM_TOSTRING(ChangedValueState::AfterUpdate), strVal) == 0)
                return ChangedValueState::AfterUpdate;

            if (BeStringUtilities::StricmpAscii(ENUM_TOSTRING(ChangedValueState::BeforeDelete), strVal) == 0)
                return ChangedValueState::BeforeDelete;

            return Nullable<ChangedValueState>();
            }

        static Nullable<ChangeOpCode> DetermineOpCodeFromChangedValueState(ChangedValueState state)
            {
            if (state == ChangedValueState::AfterInsert)
                return ChangeOpCode::Insert;

            if (state == ChangedValueState::BeforeUpdate || state == ChangedValueState::AfterUpdate)
                return ChangeOpCode::Update;

            if (state == ChangedValueState::BeforeDelete)
                return ChangeOpCode::Delete;

            BeAssert(false && "ChangedValueState enum or ChangeOpCode enum was changed. This code has to be adjusted.");
            return Nullable<ChangeOpCode>();
            }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE