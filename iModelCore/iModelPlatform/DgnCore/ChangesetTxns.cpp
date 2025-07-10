/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Bentley/SHA1.h>
#include <DgnPlatform/DgnChangeSummary.h>
#include <ECDb/ChangeIterator.h>
#include <folly/ProducerConsumerQueue.h>
#include <thread>

USING_NAMESPACE_BENTLEY_SQLITE

#define CURRENT_CS_END_TXN_ID "CurrentChangeSetEndTxnId"
#define PARENT_CS_ID "ParentChangeSetId"
#define PARENT_CHANGESET "parentChangeset"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#define THROW_JS_TYPE_EXCEPTION(str) BeNapi::ThrowJsTypeException(info.Env(), str);
#define THROW_JS_DGN_DB_EXCEPTION(env, str, status) BeNapi::ThrowJsException(env, str, (int)status, DgnDbStatusHelper::GetITwinError(status));

#define ARGUMENT_IS_PRESENT(i) (info.Length() > (i))
#define ARGUMENT_IS_NUMBER(i) (ARGUMENT_IS_PRESENT(i) && info[i].IsNumber())
#define ARGUMENT_IS_NOT_NUMBER(i) !ARGUMENT_IS_NUMBER(i)
#define REQUIRE_ARGUMENT_INTEGER(i, var)\
    if (ARGUMENT_IS_NOT_NUMBER(i)) {\
        THROW_JS_TYPE_EXCEPTION("Argument " #i " must be a number")\
    }\
    int32_t var = info[i].As<Napi::Number>().Int32Value();

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution LocalChangeSet::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) {
    const auto jsIModelDb = m_dgndb.GetJsIModelDb();
    if (nullptr == jsIModelDb) {
        return ChangeSet::ConflictResolution::Abort;
    }

    const auto jsDgnDb = jsIModelDb->Value();
    const auto env = jsDgnDb.Env();
    const auto onRebaseLocalTxnConflict = m_dgndb.GetJsTxns().Get("_onRebaseLocalTxnConflict").As<Napi::Function>();

    if (!onRebaseLocalTxnConflict.IsFunction()) {
        THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "_onRebaseLocalTxnConflict() does not exists", DgnDbStatus::BadArg);
    }
    auto arg = Napi::Object::New(env);
    arg.Set("cause", Napi::Number::New(env, (int)cause));
    arg.Set("opcode", Napi::Number::New(env, (int)iter.GetOpcode()));
    arg.Set("indirect", Napi::Boolean::New(env, iter.IsIndirect()));
    arg.Set("tableName", Napi::String::New(env, iter.GetTableName()));
    arg.Set("columnCount", Napi::Number::New(env, iter.GetColumnCount()));
    arg.Set("getForeignKeyConflicts", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> Napi::Value {
        return Napi::Number::New(env, iter.GetForeignKeyConflicts());
    }));

    auto txnInfo = Napi::Object::New(env);
    txnInfo.Set("id", Napi::String::New(env, m_id.GetValue() == 0 ? "0x0" : BeInt64Id(m_id.GetValue()).ToHexStr()));
    txnInfo.Set("descr", Napi::String::New(env, m_descr));
    txnInfo.Set("type" , Napi::String::New(env, m_type == TxnType::Data? "Data" : "Schema"));

    arg.Set("txn", txnInfo);
    arg.Set("getColumnNames", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> Napi::Value {
        auto array = Napi::Array::New(env);
        bvector<Utf8String> columns;
        if (m_dgndb.GetColumns(columns, iter.GetTableName().c_str())) {
            for(int i = 0; i < static_cast<int>(columns.size()); ++i)
                array.Set(i, Napi::String::New(env, columns[i].c_str()));
        }
        return array;
    }));
    arg.Set("getPrimaryKeyColumns", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> Napi::Value {
        auto array = Napi::Array::New(env);
        int k = -1;
        for (int i = 0; i < iter.GetPrimaryKeyColumnCount(); ++i){
            if (iter.IsPrimaryKeyColumn(i)) {
                array.Set(++k, Napi::Number::New(env, i));
            }
        }
        return array;
    }));
    arg.Set("getValueType", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
        REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
        REQUIRE_ARGUMENT_INTEGER(1, stage);
        if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
            return env.Undefined();

        auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
        if (!val.IsValid())
            return env.Undefined();

        return Napi::Number::New(env, (int)val.GetValueType());
    }));
    arg.Set("getValueBinary", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
        REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
        REQUIRE_ARGUMENT_INTEGER(1, stage);
        if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
            return env.Undefined();

        auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
        if (!val.IsValid())
            return env.Undefined();

        if (val.IsNull())
            return env.Null();

        auto nBytes = val.GetValueBytes();
        auto blob = Napi::Uint8Array::New(env, nBytes); // Napi::Buffer<uint8_t>::New(env, nBytes);
        memcpy(blob.Data(), val.GetValueBlob(), nBytes);
        return blob;
    }));
    arg.Set("getValueText", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
        REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
        REQUIRE_ARGUMENT_INTEGER(1, stage);
        if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
            return env.Undefined();

        auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
        if (!val.IsValid())
            return env.Undefined();

        if (val.IsNull())
            return env.Null();

        return Napi::String::New(env, val.GetValueText());
    }));
    arg.Set("getValueId", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
        REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
        REQUIRE_ARGUMENT_INTEGER(1, stage);
        if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
            return env.Undefined();

        auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
        if (!val.IsValid())
            return env.Undefined();

        if (val.IsNull())
            return env.Null();

        return Napi::String::New(env, val.GetValueUInt64() == 0 ? "0x0" : BeInt64Id(val.GetValueUInt64()).ToHexStr());
    }));
    arg.Set("isValueNull", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
        REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
        REQUIRE_ARGUMENT_INTEGER(1, stage);
        auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
        if (!val.IsValid())
            return env.Undefined();

        if (val.IsNull())
            return env.Null();

        return Napi::Boolean::New(env, val.IsNull());
    }));
    arg.Set("getValueInteger", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
        REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
        REQUIRE_ARGUMENT_INTEGER(1, stage);
        if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
            return env.Undefined();

        auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
        if (!val.IsValid())
            return env.Undefined();

        if (val.IsNull())
            return env.Null();

        return Napi::Number::New(env, static_cast<double>(val.GetValueInt64()));
    }));
    arg.Set("getValueDouble", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
        REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
        REQUIRE_ARGUMENT_INTEGER(1, stage);
        if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
            return env.Undefined();

        auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
        if (!val.IsValid())
            return env.Undefined();

        if (val.IsNull())
            return env.Null();

        return Napi::Number::New(env, val.GetValueDouble());
    }));
    arg.Set("dump", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> void {
        iter.Dump(m_dgndb, false, 1);
    }));
    arg.Set("setLastError", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> void {
        if (info.Length() != 1)
            THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "setLastError() Expect a string type arg", DgnDbStatus::BadArg);

        auto val = info[0];
        if (!val.IsString())
            THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "setLastError() Expect a string type arg", DgnDbStatus::BadArg);

        m_lastErrorMessage = val.As<Napi::String>().Utf8Value();
    }));

    const auto resJsVal = onRebaseLocalTxnConflict.Call(m_dgndb.GetJsTxns(), {arg});
    if (resJsVal.IsUndefined())
        THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "_onRebaseLocalTxnConflict must return a resolution", DgnDbStatus::BadArg);

    if (!resJsVal.IsNumber())
        THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "_onRebaseLocalTxnConflict did not return a number", DgnDbStatus::BadArg);

    const auto resolution = (ChangeSet::ConflictResolution)resJsVal.As<Napi::Number>().Int32Value();
    if (resolution != ChangeSet::ConflictResolution::Abort  && resolution != ChangeSet::ConflictResolution::Replace && resolution != ChangeSet::ConflictResolution::Skip )
        THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "_onRebaseLocalTxnConflict returned unsupported value for conflict resolution", DgnDbStatus::BadArg);

    return resolution;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ChangeSet::ConflictResolution ChangesetFileReader::_OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) {
    if (!m_dgndb){
        BeAssert(false && "DgnDb is not set");
        return ChangeSet::ConflictResolution::Abort;
    }

    const auto jsIModelDb = m_dgndb->GetJsIModelDb();
    if (nullptr != jsIModelDb) {
        const auto jsDgnDb = jsIModelDb->Value();
        const auto env = jsDgnDb.Env();
        const auto onChangesetConflictFunc = jsDgnDb.Get("onChangesetConflict").As<Napi::Function>();

        if (onChangesetConflictFunc.IsFunction()) {
            auto arg = Napi::Object::New(env);
            arg.Set("cause", Napi::Number::New(env, (int)cause));
            arg.Set("opcode", Napi::Number::New(env, (int)iter.GetOpcode()));
            arg.Set("indirect", Napi::Boolean::New(env, iter.IsIndirect()));
            arg.Set("tableName", Napi::String::New(env, iter.GetTableName()));
            arg.Set("columnCount", Napi::Number::New(env, iter.GetColumnCount()));
            arg.Set("changesetFile", Napi::String::New(env, GetFiles().front().GetNameUtf8()));
            arg.Set("getForeignKeyConflicts", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> Napi::Value {
                return Napi::Number::New(env, iter.GetForeignKeyConflicts());
            }));
            arg.Set("getColumnNames", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> Napi::Value {
                auto array = Napi::Array::New(env);
                bvector<Utf8String> columns;
                if (m_dgndb->GetColumns(columns, iter.GetTableName().c_str())) {
                    for(int i = 0; i < static_cast<int>(columns.size()); ++i)
                        array.Set(i, Napi::String::New(env, columns[i].c_str()));
                }
                return array;
            }));
            arg.Set("getPrimaryKeyColumns", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> Napi::Value {
                auto array = Napi::Array::New(env);
                int k = -1;
                for (int i = 0; i < iter.GetPrimaryKeyColumnCount(); ++i){
                    if (iter.IsPrimaryKeyColumn(i)) {
                        array.Set(++k, Napi::Number::New(env, i));
                    }
                }
                return array;
            }));
           arg.Set("getValueType", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
                REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
                REQUIRE_ARGUMENT_INTEGER(1, stage);
                if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
                    return env.Undefined();

                auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
                if (!val.IsValid())
                    return env.Undefined();

                return Napi::Number::New(env, (int)val.GetValueType());
            }));
            arg.Set("getValueBinary", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
                REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
                REQUIRE_ARGUMENT_INTEGER(1, stage);
                if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
                    return env.Undefined();

                auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
                if (!val.IsValid())
                    return env.Undefined();

                if (val.IsNull())
                    return env.Null();

                auto nBytes = val.GetValueBytes();
                auto blob = Napi::Uint8Array::New(env, nBytes);
                memcpy(blob.Data(), val.GetValueBlob(), nBytes);
                return blob;
            }));
            arg.Set("getValueText", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
                REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
                REQUIRE_ARGUMENT_INTEGER(1, stage);
                if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
                    return env.Undefined();

                auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
                if (!val.IsValid())
                    return env.Undefined();

                if (val.IsNull())
                    return env.Null();

                return Napi::String::New(env, val.GetValueText());
            }));
            arg.Set("getValueId", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
                REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
                REQUIRE_ARGUMENT_INTEGER(1, stage);
                if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
                    return env.Undefined();

                auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
                if (!val.IsValid())
                    return env.Undefined();

                if (val.IsNull())
                    return env.Null();

                return Napi::String::New(env, val.GetValueUInt64() == 0 ? "0x0" : BeInt64Id(val.GetValueUInt64()).ToHexStr());
            }));
            arg.Set("isValueNull", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
                REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
                REQUIRE_ARGUMENT_INTEGER(1, stage);
                auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
                if (!val.IsValid())
                    return env.Undefined();

                if (val.IsNull())
                    return env.Null();

                return Napi::Boolean::New(env, val.IsNull());
            }));
            arg.Set("getValueInteger", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
                REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
                REQUIRE_ARGUMENT_INTEGER(1, stage);
                if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
                    return env.Undefined();

                auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
                if (!val.IsValid())
                    return env.Undefined();

                if (val.IsNull())
                    return env.Null();

                return Napi::Number::New(env, static_cast<double>(val.GetValueInt64()));
            }));
            arg.Set("getValueDouble", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> Napi::Value {
                REQUIRE_ARGUMENT_INTEGER(0, columnIdx);
                REQUIRE_ARGUMENT_INTEGER(1, stage);
                if ((columnIdx < 0 && columnIdx >= iter.GetColumnCount()) || (stage !=0 && stage != 1))
                    return env.Undefined();

                auto val = iter.GetValue(columnIdx, (Changes::Change::Stage)stage);
                if (!val.IsValid())
                    return env.Undefined();

                if (val.IsNull())
                    return env.Null();

                return Napi::Number::New(env, val.GetValueDouble());
            }));
            arg.Set("dump", Napi::Function::New(env, [&](const Napi::CallbackInfo&) -> void {
                iter.Dump(*m_dgndb, false, 1);
            }));
            arg.Set("setLastError", Napi::Function::New(env, [&](const Napi::CallbackInfo& info) -> void {
                if (info.Length() != 1)
                    THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "setLastError() Expect a string type arg", DgnDbStatus::BadArg);

                auto val = info[0];
                if (!val.IsString())
                    THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "setLastError() Expect a string type arg", DgnDbStatus::BadArg);

                m_lastErrorMessage = val.As<Napi::String>().Utf8Value();
            }));

            const auto resolutionJsVal = onChangesetConflictFunc.Call(jsDgnDb, {arg});

            // if handler return undefined we revert to native handler.
            if (!resolutionJsVal.IsUndefined()) {
                if (!resolutionJsVal.IsNumber())
                    THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "onChangesetConflict did not return a number", DgnDbStatus::BadArg);

                const auto resolution = (ChangeSet::ConflictResolution)resolutionJsVal.As<Napi::Number>().Int32Value();
                if (resolution != ChangeSet::ConflictResolution::Abort  && resolution != ChangeSet::ConflictResolution::Replace && resolution != ChangeSet::ConflictResolution::Skip )
                    THROW_JS_DGN_DB_EXCEPTION(jsDgnDb.Env(), "onChangesetConflict returned unsupported value for conflict resolution", DgnDbStatus::BadArg);

                return resolution;
            }
        }
    }

    if (cause == ChangeSet::ConflictCause::Data && !iter.IsIndirect()) {
/*
        * From SQLite Docs CHANGESET_DATA as the second argument
        * when processing a DELETE or UPDATE change if a row with the required
        * PRIMARY KEY fields is present in the database, but one or more other
        * (non primary-key) fields modified by the update do not contain the
        * expected "before" values.
        *
        * The conflicting row, in this case, is the database row with the matching
        * primary key.
        *
        * Another reason this will be invoked is when SQLITE_CHANGESETAPPLY_FKNOACTION
        * is passed ApplyChangeset(). The flag will disable CASCADE action and treat
        * them as CASCADE NONE resulting in conflict handler been called.
        */
        if (!m_dgndb->Txns().HasPendingTxns()) {
            // This changeset is bad. However, it is already in the timeline. We must allow services such as
            // checkpoint-creation, change history, and other apps to apply any changeset that is in the timeline.
        LOG.warning("UPDATE/DELETE before value do not match with one in db or CASCADE action was triggered.");
        iter.Dump(*m_dgndb, false, 1);
    } else {
            if (iter.GetTableName().StartsWithIAscii("ec_")) {
                return ChangeSet::ConflictResolution::Skip;
            }
            if (iter.GetTableName().EqualsIAscii ("be_Prop")) {
                 Utf8String ns = iter.GetValue(0, Changes::Change::Stage::Old).GetValueText();
                 Utf8String name = iter.GetValue(1, Changes::Change::Stage::Old).GetValueText();
                if (ns.EqualsIAscii("ec_Db") && name.EqualsIAscii("localDbInfo")) {
                    return ChangeSet::ConflictResolution::Replace;
                }
            }

            m_lastErrorMessage = "UPDATE/DELETE before value do not match with one in db or CASCADE action was triggered.";
            LOG.fatal(m_lastErrorMessage.c_str());
            iter.Dump(*m_dgndb, false, 1);
            return ChangeSet::ConflictResolution::Abort;
        }
    }
    // Handle some special cases
    if (cause == ChangeSet::ConflictCause::Conflict) {
// From the SQLite docs: "CHANGESET_CONFLICT is passed as the second argument to the conflict handler while processing an INSERT change if the operation would result in duplicate primary key values."
        // This is always a fatal error - it can happen only if the app started with a briefcase that is behind the tip and then uses the same primary key values (e.g., ElementIds)
        // that have already been used by some other app using the SAME briefcase ID that recently pushed changes. That can happen only if the app makes changes without first pulling and acquiring locks.
        if (!m_dgndb->Txns().HasPendingTxns()) {
            // This changeset is bad. However, it is already in the timeline. We must allow services such as
            // checkpoint-creation, change history, and other apps to apply any changeset that is in the timeline.
        LOG.warning("PRIMARY KEY INSERT CONFLICT - resolved by replacing the existing row with the incoming row");
iter.Dump(*m_dgndb, false, 1);
        } else {
            if (iter.GetTableName().StartsWithIAscii("ec_")) {
                return ChangeSet::ConflictResolution::Skip;
            }
            m_lastErrorMessage = "PRIMARY KEY INSERT CONFLICT - rejecting this changeset";
            LOG.fatal(m_lastErrorMessage.c_str());
            iter.Dump(*m_dgndb, false, 1);
            return ChangeSet::ConflictResolution::Abort;
        }
    }

    if (cause == ChangeSet::ConflictCause::ForeignKey) {
// Note: No current or conflicting row information is provided if it's a FKey conflict
        // Since we abort on FKey conflicts, always try and provide details about the error
        int nConflicts = iter.GetForeignKeyConflicts();

        uint64_t notUsed;
        // Note: There is no performance implication of follow code as it happen toward end of
        // apply_changeset only once so we be querying value for 'DebugAllowFkViolations' only once.
        if (m_dgndb->QueryBriefcaseLocalValue(notUsed, "DebugAllowFkViolations") == BE_SQLITE_ROW) {
            LOG.errorv("Detected %d foreign key conflicts in changeset. Continuing merge as 'DebugAllowFkViolations' flag is set. Run 'PRAGMA foreign_key_check' to get list of violations.", nConflicts);
            return ChangeSet::ConflictResolution::Skip;
        } else {

            m_lastErrorMessage = Utf8PrintfString("Detected %d foreign key conflicts in ChangeSet. Aborting merge.", nConflicts);
            LOG.error(m_lastErrorMessage.c_str());
            return ChangeSet::ConflictResolution::Abort;
        }
    }

    if (cause == ChangeSet::ConflictCause::NotFound) {
/*
         * Note: If ConflictCause = NotFound, the primary key was not found, and returning ConflictResolution::Replace is
         * not an option at all - this will cause a BE_SQLITE_MISUSE error.
         */
        return ChangeSet::ConflictResolution::Skip;
    }

    if (ChangeSet::ConflictCause::Constraint == cause) {
        if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO)) {
            LOG.infov("------------------------------------------------------------------");
            LOG.infov("Conflict detected - Cause: %s", ChangeSet::InterpretConflictCause(cause, 1));
            iter.Dump(*m_dgndb, false, 1);
        }

        LOG.warning("Constraint conflict handled by rejecting incoming change. Constraint conflicts are NOT expected. These happen most often when two clients both insert elements with the same code. That indicates a bug in the client or the code server.");
        return ChangeSet::ConflictResolution::Skip;
    }

/*
     * If we don't have a control, we always accept the incoming revision in cases of conflicts:
     *
     * + In a briefcase with no local changes, the state of a row in the Db (i.e., the final state of a previous revision)
     *   may not exactly match the initial state of the incoming revision. This will cause a conflict.
     *      - The final state of the incoming (later) revision will always be setup exactly right to accommodate
     *        cases where dependency handlers won't be available (for instance on the server), and we have to rely on
     *        the revision to correctly set the final state of the row in the Db. Therefore it's best to resolve the
     *        conflict in favor of the incoming change.
     * + In a briefcase with local changes, the state of relevant dependent properties (due to propagated indirect changes)
     *   may not correspond with the initial state of these properties in an incoming revision. This will cause a conflict.
     *      - Resolving the conflict in favor of the incoming revision may cause some dependent properties to be set
     *        incorrectly, but the dependency handlers will run anyway and set this right. The new changes will be part of
     *        a subsequent revision generated from that briefcase.
     *
     * + Note that conflicts can NEVER happen between direct changes made locally and direct changes in the incoming revision.
     *      - Only one user can make a direct change at one time, and the next user has to pull those changes before getting a
     *        lock to the same element
     *
     * + Also see comments in TxnManager::MergeDataChanges()
     */
    if (LOG.isSeverityEnabled(NativeLogging::LOG_INFO)) {
        LOG.infov("------------------------------------------------------------------");
        LOG.infov("Conflict detected - Cause: %s", ChangeSet::InterpretConflictCause(cause, 1));
        iter.Dump(*m_dgndb, false, 1);
        LOG.infov("Conflicting resolved by replacing the existing entry with the change");
    }
    return ChangeSet::ConflictResolution::Replace;
}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct ChangesetIdGenerator : ChangeStream {
    SHA1 m_hash;

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    static int HexCharToInt(char input) {
        if (input >= '0' && input <= '9')
            return input - '0';
        if (input >= 'A' && input <= 'F')
            return input - 'A' + 10;
        if (input >= 'a' && input <= 'f')
            return input - 'a' + 10;

        BeAssert(false);
        return 0;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void AddStringToHash(Utf8StringCR hashString) {
        Byte hashValue[SHA1::HashBytes];
        if (hashString.empty()) {
            memset(hashValue, 0, SHA1::HashBytes);
        } else {
            BeAssert(hashString.length() == SHA1::HashBytes * 2);
            for (int ii = 0; ii < SHA1::HashBytes; ii++) {
                char hexChar1 = hashString.at(2 * ii);
                char hexChar2 = hashString.at(2 * ii + 1);
                hashValue[ii] = (Byte)(16 * HexCharToInt(hexChar1) + HexCharToInt(hexChar2));
            }
        }

        m_hash.Add(hashValue, SHA1::HashBytes);
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    DbResult _Append(Byte const* pData, int nData) override {
        m_hash.Add(pData, nData);
        return BE_SQLITE_OK;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    ChangeSet::ConflictResolution _OnConflict(ChangeSet::ConflictCause cause, Changes::Change iter) override {
        return ChangeSet::ConflictResolution::Abort;
    }

public:
    bool _IsEmpty() const override final { return false; }
    RefCountedPtr<Changes::Reader> _GetReader() const override { return nullptr; }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    static Utf8String GenerateId(Utf8StringCR parentRevId, BeFileNameCR changesetFile, DgnDbR dgndb) {
        return GenerateId(parentRevId, changesetFile, dgndb.m_private_iModelDbJs.Env());
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    static Utf8String GenerateId(Utf8StringCR parentRevId, BeFileNameCR changesetFile, Napi::Env env) {
        auto throwError = [&env](const char* message, ChangesetStatus status) {
            if(env== nullptr) {
                throw std::runtime_error(message);
            }
            BeNapi::ThrowJsException(env, message, (int)status);
        };

        if (parentRevId.length() != SHA1::HashBytes * 2 && !parentRevId.empty()) {
            throwError("Invalid parent changeset id. Expect empty or SHA1 hash", ChangesetStatus::BadVersionId);
        }

        if (!changesetFile.DoesPathExist()) {
            throwError("Invalid changeset file. File not not found.", ChangesetStatus::FileNotFound);
        }

        ChangesetIdGenerator idGen;
        idGen.AddStringToHash(parentRevId);

        ChangesetFileReader fs(changesetFile, nullptr);
        auto reader = fs.MakeReader();

        DbResult result;
        Utf8StringCR prefix = reader->GetPrefix(result);
        if (BE_SQLITE_OK != result) {
            if (result == BE_SQLITE_ERROR_InvalidChangeSetVersion) {
                throwError("Unsupported changeset persistence file version", ChangesetStatus::InvalidVersion);
            } else {
                throwError("Corrupted changeset file", ChangesetStatus::CorruptedChangeStream);
            }
        }

        if (!prefix.empty()) {
            idGen._Append((Byte const*)prefix.c_str(), (int)prefix.SizeInBytes());
        }

        result = idGen.ReadFrom(*reader);
        if (BE_SQLITE_OK != result)
            throwError("Corrupted changeset file", ChangesetStatus::CorruptedChangeStream);

        return idGen.m_hash.GetHashString();
    }
};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String ChangesetProps::ComputeChangesetId(Utf8StringCR parentRevId, BeFileNameCR changesetFile, Napi::Env env){
    return ChangesetIdGenerator::GenerateId(parentRevId, changesetFile, env);
}


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void ChangesetProps::Dump(DgnDbR dgndb) const {
// Don't log "sensitive" information in production builds.
#if !defined(NDEBUG)
    LOG.infov("Id : %s", m_id.c_str());
    LOG.infov("ParentId : %s", m_parentId.c_str());
    LOG.infov("DbGuid: %s", m_dbGuid.c_str());
    LOG.infov("User Name: %s", m_userName.c_str());
    LOG.infov("Summary: %s", m_summary.c_str());
    LOG.infov("File: %ls", m_fileName.GetNameUtf8().c_str());
    LOG.infov("DateTime: %s", m_dateTime.ToString().c_str());

    ChangesetFileReader reader(m_fileName, &dgndb);

    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = reader.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    UNUSED_VARIABLE(result);
    BeAssert(result == BE_SQLITE_OK);

    LOG.infov("Contains Schema Changes: %s", containsSchemaChanges ? "yes" : "no");
    LOG.infov("Contains Ddl Changes: %s", (ddlChanges.GetSize() > 0) ? "yes" : "no");
    if (ddlChanges.GetSize() > 0)
        ddlChanges.Dump("DDL: ");

    reader.Dump("ChangeSet:\n", dgndb, false, 0);
#endif
}

/**
 * validate the content of a changeset props to ensure it is from the right iModel and its checksum is valid
 */
void ChangesetProps::ValidateContent(DgnDbR dgndb) const {
    if (m_dbGuid != dgndb.GetDbGuid().ToString())
        dgndb.ThrowException("changeset did not originate from this iModel", (int) ChangesetStatus::WrongDgnDb);

    if (!m_fileName.DoesPathExist())
        dgndb.ThrowException("changeset file does not exist", (int) ChangesetStatus::FileNotFound);

    if (m_id != ChangesetIdGenerator::GenerateId(m_parentId, m_fileName, dgndb))
      dgndb.ThrowException("incorrect id for changeset", (int) ChangesetStatus::CorruptedChangeStream);
}

/**
 * determine whether the Changeset has schema changes.
 */
bool ChangesetProps::ContainsDdlChanges(DgnDbR dgndb) const {
    ChangesetFileReader changeStream(m_fileName, &dgndb);
    bool containsSchemaChanges;
    DdlChanges ddlChanges;
    DbResult result = changeStream.MakeReader()->GetSchemaChanges(containsSchemaChanges, ddlChanges);
    if (BE_SQLITE_OK != result)
        dgndb.ThrowException("error reading changeset data", result);

    return containsSchemaChanges;
}

/**
 * clear the iModel-specific briefcase local values for changesets. Called when an iModel's guid changes.
 */
void TxnManager::ClearSavedChangesetValues() {
    m_dgndb.DeleteBriefcaseLocalValue(PARENT_CHANGESET);
    m_dgndb.DeleteBriefcaseLocalValue(PARENT_CS_ID);

    // these are just cruft from old versions.
    m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);
    m_dgndb.DeleteBriefcaseLocalValue("ReversedChangeSetId");
}

/**
 * save the parent-changeset-id in the briefcase local values table. This enables us to tell the most recently applied changes for a briefcase.
 */
void TxnManager::SaveParentChangeset(Utf8StringCR revisionId, int32_t changesetIndex) {
    if (revisionId.length() != SHA1::HashBytes * 2)
        m_dgndb.ThrowException("invalid changesetId", (int)ChangesetStatus::BadVersionId);

    // Early versions of iModels didn't save the changesetIndex, which is unfortunate since that's what we usually need. You can
    // get it by a round-trip query to IModelHub, but it's often convenient to know it locally, so we now store it in the be_local table.

    // Its value can only be used if the id in the PARENT_CHANGESET matches PARENT_CS_ID, since older software may update the latter without
    // updating the former. Eventually, we should be able to remove this, since it's redundant with the value stored in PARENT_CHANGESET. We set it
    // here for backwards compatibility for old apps before changesetIndex was persisted, and so we can tell that PARENT_CHANGESET is valid.
    m_dgndb.SaveBriefcaseLocalValue(PARENT_CS_ID, revisionId);

    // A changesetIndex value greater than 0 means that we obtained the value from iModelHub successfully. Old apps didn't save this value
    // so we can't rely on it. This can only be used if the value of "id" matches the value in PARENT_CS_ID
    if (changesetIndex > 0) {
        BeJsDocument parentChangeset;
        parentChangeset["id"] = revisionId;
        parentChangeset["index"] = changesetIndex;
        auto jsonStr = parentChangeset.Stringify();
        m_dgndb.SaveBriefcaseLocalValue(PARENT_CHANGESET, jsonStr);
    } else {
        // the application hasn't been converted to supply changesetIndex. Remove any values saved by other apps, because it's now out of sync.
        m_dgndb.DeleteBriefcaseLocalValue(PARENT_CHANGESET);
    }
}

/**
 * get the parent-changeset-id from the briefcase local values table. This identifies the most recently applied changeset for a briefcase.
 */
Utf8String TxnManager::GetParentChangesetId() const {
    Utf8String revisionId;
    DbResult result = m_dgndb.QueryBriefcaseLocalValue(revisionId, PARENT_CS_ID);
    return (BE_SQLITE_ROW == result) ? revisionId : "";
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TxnManager::GetParentChangesetIndex(int32_t& index, Utf8StringR id) const {
    id = GetParentChangesetId();
    index = id.empty() ? 0 : -1; // not known

    // the PARENT_CHANGESET member may or may not exist. If it does, and only if its id member is equal to parentId, use it's index value
    Utf8String json;
    if (BE_SQLITE_ROW != m_dgndb.QueryBriefcaseLocalValue(json, PARENT_CHANGESET))
        return;

    BeJsDocument jsonObj(json);
    if (jsonObj.isStringMember("id") && jsonObj.isNumericMember("index") && id.Equals(jsonObj["id"].asString()))
        index = jsonObj["index"].GetInt();
}

/**
 * Write the changes from:
 *  1) the DdlChanges
 *  2) data changes
 *
 * into a file about to become a changeset file.
 */
void TxnManager::WriteChangesToFile(BeFileNameCR pathname, DdlChangesCR ddlChanges, ChangeGroupCR dataChangeGroup) {
    ChangesetFileWriter writer(pathname, dataChangeGroup.ContainsEcSchemaChanges(), ddlChanges, &m_dgndb);

    if (BE_SQLITE_OK !=  writer.Initialize())
        m_dgndb.ThrowException("unable to initialize change writer", (int) ChangesetStatus::FileWriteError);

    if (BE_SQLITE_OK != writer.FromChangeGroup(dataChangeGroup))
        m_dgndb.ThrowException("unable to save changes to file", (int) ChangesetStatus::FileWriteError);

    if (!pathname.DoesPathExist())
        m_dgndb.ThrowException("changeset file not created", (int) ChangesetStatus::FileWriteError);
}
/**
 * Create changeset from local changes
*/
std::unique_ptr<ChangeSet> TxnManager::CreateChangesetFromLocalChanges(bool includeInMemoryChanges){
    DbResult rc;
    includeInMemoryChanges = includeInMemoryChanges && HasDataChanges();
    if (!HasPendingTxns() && !includeInMemoryChanges) {
        return nullptr;
    }
 // Group all local txn
    auto endTxnId = GetCurrentTxnId();
    auto startTxnId = QueryNextTxnId(TxnId(0));
    DdlChanges ddlChanges;
    ChangeGroup dataChangeGroup;
    for (auto currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = QueryNextTxnId(currTxnId)) {
        if (TxnType::Data != GetTxnType(currTxnId))
            continue;

        ChangeSet sqlChangeSet;
        if (BE_SQLITE_OK != ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None))
            m_dgndb.ThrowException("unable to read data changes", (int) ChangesetStatus::CorruptedTxn);

        rc = sqlChangeSet.AddToChangeGroup(dataChangeGroup);
        if (BE_SQLITE_OK != rc)
            m_dgndb.ThrowException("add to changes failed", (int) rc);
    }

    // Include change that has not been persisted using Db::SaveChanges()
    if (includeInMemoryChanges && HasDataChanges()) {
        ChangeSet inMemChangeSet;
        rc = inMemChangeSet.FromChangeTrack(*this);
        if (BE_SQLITE_OK != rc)
            m_dgndb.ThrowException("fail to add in memory changes", (int) rc);

        inMemChangeSet.AddToChangeGroup(dataChangeGroup);
        if (BE_SQLITE_OK != rc)
            m_dgndb.ThrowException("add to changes failed", (int) rc);
    }

    // Create changeset from change group
    auto cs = std::make_unique<ChangeSet>();
    rc = cs->FromChangeGroup(dataChangeGroup);
    if (BE_SQLITE_OK != rc)
        m_dgndb.ThrowException("failed to create changeset from change group", (int) rc);


    return cs;
}

/**
 * open txn
*/
std::unique_ptr<ChangeSet> TxnManager::OpenLocalTxn(TxnManager::TxnId id){
    auto cs = std::make_unique<ChangeSet>();
    const auto rc = ReadDataChanges(*cs, id, TxnAction::None);
    if (BE_SQLITE_OK != rc) {
        m_dgndb.ThrowException(SqlPrintfString("failed to read txn (id:%s)", BeInt64Id(id.GetValue()).ToHexStr().c_str()).GetUtf8CP(), (int) rc);
    }
    return cs;
}

/**
 * Iterate over local txn and notify changed instance keys and change type.
 * rootClassFilter is optional but if provided iterator will only iterate over
 * changes if it to one of root class or any of its derived class.
 *
 * Note: For large set of changes this function may consume a lot of none contigious memory.
 */
void TxnManager::ForEachLocalChange(std::function<void(ECInstanceKey const&, DbOpcode)> cb, bvector<Utf8String> const& rootClassFilter, bool includeInMemoryChanges) {
    DbResult rc;

    includeInMemoryChanges = includeInMemoryChanges && HasDataChanges();
    if (!HasPendingTxns() && !includeInMemoryChanges) {
        return;
    }

    // Expand root class filter into all possible derived classes including root class
    bset<ECClassId> allowedClasses;
    if (!rootClassFilter.empty()) {
        bvector<Utf8String> classIdFilter;
        for(auto& qualifiedName : rootClassFilter) {
            if (auto classP = m_dgndb.Schemas().FindClass(qualifiedName)) {
                classIdFilter.push_back(classP->GetId().ToHexStr());
            } else {
                m_dgndb.ThrowException(SqlPrintfString("unknown class '%s' provided as filter", qualifiedName.c_str()).GetUtf8CP(), (int) BE_SQLITE_ERROR);
            }
        }
        if (!classIdFilter.empty()) {
            auto filterStmt = m_dgndb.GetCachedStatement(SqlPrintfString("SELECT [ClassId] FROM [ec_cache_ClassHierarchy] WHERE [BaseClassId] IN (%s) GROUP BY [ClassId]", BeStringUtilities::Join(classIdFilter, ",").c_str()).GetUtf8CP());
            while(filterStmt->Step() == BE_SQLITE_ROW) {
                allowedClasses.insert(filterStmt->GetValueId<ECClassId>(0));
            }
        }
    }

    // cache TableName/ExclusiveRootClassId map
    bmap<Utf8String, ECClassId> dataTables;
    auto tblStmt = m_dgndb.GetCachedStatement(R"x(
        SELECT [t].[Name],
               COALESCE ([t].[ExclusiveRootClassId], [p].[ExclusiveRootClassId]) [ExclusiveRootClassId]
        FROM   [ec_Table] [t]
            LEFT JOIN [ec_Table] [p] ON [t].[ParentTableId] = [p].[Id]
        WHERE  [t].[Type] IN (0, 1, 3) AND [t].[Name] NOT LIKE 'ecdbf_%')x");

    while(tblStmt->Step() == BE_SQLITE_ROW) {
        dataTables[tblStmt->GetValueText(0)] = tblStmt->GetValueId<ECClassId>(1);
    }

    auto cs = CreateChangesetFromLocalChanges(includeInMemoryChanges);

    // Optimization based on fact that in changeset change to same table is contigious
    struct {
        Utf8String zTab;
        int iPk = -1;
        int iClassId = -1;
        bool isECDataTable = false;
        ECClassId rootClassId;
        Statement classIdStmt;
    } current;

    // Set of key we already notified va callback
    bset<ECInstanceKey> instanceKeysAlreadyNotified;

    // Iterate over changeset and notify each change if it meet filter criteria
    for(auto& change : cs->GetChanges()) {
        Utf8CP zTab;
        int nCol;
        DbOpcode op;

        rc = change.GetOperation(&zTab, &nCol, &op, nullptr);
        if (BE_SQLITE_OK != rc) {
            m_dgndb.ThrowException("failed to read changeset", (int) rc);
        }

        // if new table then we need to figure out pk and classid column including root classid for table
        if (0 != strcmp(current.zTab.c_str(), zTab)) {
            current.zTab = zTab;
            auto it = dataTables.find(current.zTab);
            current.isECDataTable = it != dataTables.end();
            if (current.classIdStmt.IsPrepared()) {
                current.classIdStmt.Finalize();
            }
            if (current.isECDataTable) {
                auto stmt = m_dgndb.GetCachedStatement(R"x(
                    SELECT
                        (SELECT [cid] FROM PRAGMA_TABLE_INFO (?1) WHERE [pk] = 1),
                        (SELECT [cid] FROM PRAGMA_TABLE_INFO (?1) WHERE [name] = 'ECClassId')
                )x");

                stmt->BindText(1, zTab, Statement::MakeCopy::No);
                rc = stmt->Step();
                if (BE_SQLITE_ROW != rc)
                    m_dgndb.ThrowException("failed to read changeset", (int) rc);

                // Column index of ECInstanceId column in table.
                current.iPk = stmt->IsColumnNull(0) ? -1 : stmt->GetValueInt(0);

                // Column index of ECClassId column in table if it has one.
                current.iClassId = stmt->IsColumnNull(1) ? -1 : stmt->GetValueInt(1);

                // ExclusiveRootClassId for table which is used when ECClassId column does not exist in table
                current.rootClassId = it->second;

                // if iClassId is not pointing to a column id then do not attempt to prepare classid.
                if (current.iClassId != -1) {
                    rc = current.classIdStmt.Prepare(m_dgndb, SqlPrintfString("SELECT [ECClassId] FROM [%s] WHERE ROWID=?", zTab).GetUtf8CP());
                    if (BE_SQLITE_OK != rc)
                        m_dgndb.ThrowException("failed to read changeset", (int) rc);
                }
            } else {
                current.iPk = -1;
                current.iClassId = -1;
                current.rootClassId = ECClassId();
            }
        }
        if (!current.isECDataTable) {
            continue;
        }
        if (current.iPk < 0) {
            m_dgndb.ThrowException(SqlPrintfString("no integer primary key in '%s'", current.zTab.c_str()).GetUtf8CP(), (int) BE_SQLITE_ERROR);
        }

        ECInstanceId id;
        ECClassId classId = current.rootClassId;

        // read primary key or ECInstanceId
        DbValue valId = op == DbOpcode::Insert ? change.GetNewValue(current.iPk) : change.GetOldValue(current.iPk);
        if (!valId.IsValid())
            m_dgndb.ThrowException("failed to read local changes", (int) BE_SQLITE_ERROR);
        id = ECInstanceId(valId.GetValueUInt64());

        // It is possible ECClassId column may not exist in a table and in case we fall back to ExclusiveRootClassId for that table.
        if (current.iClassId != -1) {
            DbValue valClassId = op == DbOpcode::Insert ? change.GetNewValue(current.iClassId) : change.GetOldValue(current.iClassId);
            if (valClassId.IsValid()) {
                classId = ECClassId(valClassId.GetValueUInt64());
            } else {
                if (op == DbOpcode::Update) {
                    current.classIdStmt.Reset();
                    current.classIdStmt.ClearBindings();
                    current.classIdStmt.BindId(1, id);
                    if (current.classIdStmt.Step() == BE_SQLITE_ROW) {
                        classId = current.classIdStmt.GetValueId<ECClassId>(0);
                    }
                } else {
                    m_dgndb.ThrowException(SqlPrintfString("failed to read classid form table '%s' for row '%s'",
                        current.zTab.c_str(), id.ToHexStr().c_str()).GetUtf8CP(), (int) BE_SQLITE_ERROR);
                }
            }
        }

        // If root class filter was provided make sure current classid is part of
        // filter class or one of its derived class else skip to next change
        if (!allowedClasses.empty()) {
            if (allowedClasses.find(classId) == allowedClasses.end()) {
                continue;
            }
        }

        auto key = ECInstanceKey(classId, id);

        // Skip if we already seen this instance key e.g. first as bis_Element and later in bis_GeometricElement3d
        if (instanceKeysAlreadyNotified.find(key) != instanceKeysAlreadyNotified.end())
            continue;

        cb(key, op);
        instanceKeysAlreadyNotified.insert(key);
    }
}

/**
 * Create a new changeset file from all of the Txns in this briefcase. After this function returns, the
 * changeset is "in-progress" and its file must be uploaded to iModelHub. After that succeeds, the Txns included
 * in the changeset are deleted in the call to `FinishCreateChangeset`.
 *
 * The changeset file name is the "tempFileNameBase" of the briefcase with ".changeset" appended. For tests, this method
 * takes a "extension" argument that is appended to the filename before ".changeset" is added. That's so tests
 * can save more than one changeset while they work (without mocking iModelHub.)
 */
ChangesetPropsPtr TxnManager::StartCreateChangeset(Utf8CP extension) {
    if (m_changesetInProgress.IsValid())
        m_dgndb.ThrowException("a changeset is currently in progress", (int) ChangesetStatus::IsCreatingChangeset);

    if (!IsTracking())
        m_dgndb.ThrowException("change tracking not enabled", (int) ChangesetStatus::ChangeTrackingNotEnabled);

    if (HasChanges())
        m_dgndb.ThrowException("local uncommitted changes present", (int) ChangesetStatus::HasUncommittedChanges);

    // make sure there's nothing undone before we start to create a changeset
    DeleteReversedTxns();

    if (!HasPendingTxns())
        m_dgndb.ThrowException("no changes are present", (int) ChangesetStatus::NoTransactions);

    TxnManager::TxnId endTxnId = GetCurrentTxnId();
    TxnId startTxnId = QueryNextTxnId(TxnId(0));

    DdlChanges ddlChangeGroup;
    ChangeGroup dataChangeGroup(m_dgndb);
    for (TxnId currTxnId = startTxnId; currTxnId < endTxnId; currTxnId = QueryNextTxnId(currTxnId)) {
        auto txnType = GetTxnType(currTxnId);
        if (txnType == TxnType::EcSchema) // if we have EcSchema changes, set the flag on the change group
            dataChangeGroup.SetContainsEcSchemaChanges();

        if (txnType == TxnType::Ddl) {
            DdlChanges ddlChange;
            if (ZIP_SUCCESS != ReadChanges(ddlChange, currTxnId))
                m_dgndb.ThrowException("unable to read schema changes", (int) ChangesetStatus::CorruptedTxn);

            for(auto& ddl : ddlChange.GetDDLs())
                ddlChangeGroup.AddDDL(ddl.c_str());

        } else {
            ChangeSet sqlChangeSet;
            if (BE_SQLITE_OK != ReadDataChanges(sqlChangeSet, currTxnId, TxnAction::None))
                m_dgndb.ThrowException("unable to read data changes", (int) ChangesetStatus::CorruptedTxn);

            DbResult result = sqlChangeSet.AddToChangeGroup(dataChangeGroup);
            if (BE_SQLITE_OK != result)
                m_dgndb.ThrowException("add to changes failed", (int) result);
        }
    }

    BeFileName changesetFileName((m_dgndb.GetTempFileBaseName() + (extension ? extension : "") +  ".changeset").c_str());
    WriteChangesToFile(changesetFileName, ddlChangeGroup, dataChangeGroup);

    auto parentRevId = GetParentChangesetId();
    auto revId = ChangesetIdGenerator::GenerateId(parentRevId, changesetFileName, m_dgndb);
    auto dbGuid = m_dgndb.GetDbGuid().ToString();

    auto changesetType = ChangesetProps::ChangesetType::Regular;
    if (dataChangeGroup.ContainsEcSchemaChanges()) {
        changesetType = m_dgndb.Schemas().GetSchemaSync().IsEnabled() ? ChangesetProps::ChangesetType::SchemaSync : ChangesetProps::ChangesetType::Schema;
    }

    m_changesetInProgress = new ChangesetProps(revId, -1, parentRevId, dbGuid, changesetFileName, changesetType);
    m_changesetInProgress->m_endTxnId = endTxnId;

    // clean this cruft up from older versions.
    m_dgndb.DeleteBriefcaseLocalValue(CURRENT_CS_END_TXN_ID);

    auto rc = m_dgndb.SaveChanges();
    if (BE_SQLITE_OK != rc)
        m_dgndb.ThrowException("cannot save changes to start changeset", rc);

    // We've just saved all the local Txns together into a changeset file that can now be pushed to iModelHub. Only after that succeeds we can delete
    // the Txns from this changeset. In the meantime, we can allow further changes to this briefcase that will be included in the *next* changeset.
    // However, we cannot allow the changes we've just included in the changeset to be undone. Therefore, we start a new "session" which
    // makes the current Txns unreachable for undo/redo.
    StartNewSession();
    BeAssert(!IsUndoPossible());
    BeAssert(!IsRedoPossible());

    return m_changesetInProgress;
}

/**
 * This method is called after an in-progress changeset file, created in this session, was successfully uploaded to iModelHub. We can now delete
 * all of the Txns up to the last one included in the changeset.
 * Note: If the upload fails for any reason, or we crashed before it succeeds, a new one must be recreated from the current state of the briefcase.
 */
void TxnManager::FinishCreateChangeset(int32_t changesetIndex, bool keepFile) {
    if (!IsChangesetInProgress())
        m_dgndb.ThrowException("no changeset in progress", (int) ChangesetStatus::IsNotCreatingChangeset);

    TxnId endTxnId = m_changesetInProgress->m_endTxnId;
    if (!endTxnId.IsValid())
        m_dgndb.ThrowException("changeset in progress is not valid", (int) ChangesetStatus::IsNotCreatingChangeset);

    m_dgndb.Txns().DeleteFromStartTo(endTxnId);
    m_changesetInProgress->SetChangesetIndex(changesetIndex);

    SaveParentChangeset(m_changesetInProgress->GetChangesetId(), changesetIndex);

    auto rc = m_dgndb.SaveChanges();
    if (BE_SQLITE_OK != rc)
        m_dgndb.ThrowException("cannot save changes to complete changeset", rc);

    // if there were no *new* changes during the process of uploading the changeset (which is very likely), we can
    // restart the session id back to 0. Otherwise just leave it alone so undo/redo of those changes is still possible.
    if (!HasPendingTxns())
        Initialize();

    m_changesetInProgress->SetDateTime(DateTime::GetCurrentTimeUtc());
    StopCreateChangeset(keepFile);
}

/**
 * free the in-progress ChangesetProps, if present, and optionally delete its changeset file.
 */
void TxnManager::StopCreateChangeset(bool keepFile) {
    if (!keepFile && m_changesetInProgress.IsValid() && m_changesetInProgress->m_fileName.DoesPathExist())
        m_changesetInProgress->m_fileName.BeDeleteFile();

    m_changesetInProgress = nullptr;
}


//--------------------------------------------------------------------------------------
// @bsimethod
// UNUSED_CODE
//--------------------------------------------------------------------------------------
ChangesetStatus TxnManager::ProcessRevisions(bvector<ChangesetPropsCP> const &revisions, RevisionProcessOption processOptions) {
    ChangesetStatus status;
    switch (processOptions) {
    case RevisionProcessOption::Merge:
        PullMergeBegin();
        for (ChangesetPropsCP revision : revisions) {
            status = MergeChangeset(*revision, false);
            if (ChangesetStatus::Success != status) {
                PullMergeEnd();
                return status;
            }
        }
        break;
    case RevisionProcessOption::Reverse:
        PullMergeBegin();
        for (ChangesetPropsCP revision : revisions) {
            ReverseChangeset(*revision);
        }
        PullMergeEnd();
        break;
    default:
        BeAssert(false && "Invalid revision process option");
    }
    PullMergeEnd();
    return ChangesetStatus::Success;
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
