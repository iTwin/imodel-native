/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
//ECIssueListener
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECDbIssue ECIssueListener::GetIssue() const
    {
    if (!m_issue.has_value())
        return m_issue;

    ECDbIssue copy(m_issue);
    //reset cached issue before returning
    m_issue = ECDbIssue();
    return copy;
    }

//************************************************************************************
//ECDbTestLogger
//************************************************************************************
NativeLogging::CategoryLogger ECDbTestLogger::Get()  {
    return NativeLogging::CategoryLogger("ECDbTests");
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ToString(JsonECSqlSelectAdapter::FormatOptions const& options)
    {
    Utf8String str("JsonECSqlSelectAdapter::FormatOptions(");
    switch (options.GetMemberCasingMode())
        {
            case JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal:
                str.append("MemberNameCasing::KeepOriginal");
                break;
            case JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar:
                str.append("MemberNameCasing::LowerFirstChar");
                break;

            default:
                return Utf8String("Unhandled JsonECSqlSelectAdapter::MemberNameCasing. Adjust ECDb ATP ToString method");
        }

    str.append(",");
    switch (options.GetInt64Format())
        {
            case ECJsonInt64Format::AsNumber:
                str.append(ENUM_TOSTRING(ECJsonInt64Format::AsNumber));
                break;
            case ECJsonInt64Format::AsDecimalString:
                str.append(ENUM_TOSTRING(ECJsonInt64Format::AsDecimalString));
                break;
            case ECJsonInt64Format::AsHexadecimalString:
                str.append(ENUM_TOSTRING(ECJsonInt64Format::AsHexadecimalString));
                break;
            default:
                return Utf8String("Unhandled ECJsonInt64Format. Adjust ECDb ATP ToString method");
        }

    str.append(")");
    return str;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ToString(JsonUpdater::Options const& options)
    {
    Utf8String str("JsonUpdater::Options(");
    switch (options.GetSystemPropertiesOption())
        {
            case JsonUpdater::SystemPropertiesOption::Fail:
                str.append("SystemPropertiesOption::Fail");
                break;
            case JsonUpdater::SystemPropertiesOption::Ignore:
                str.append("SystemPropertiesOption::Ignore");
                break;

            default:
                return Utf8String("Unhandled ReadonlyPropertiesOption. Adjust ECDb ATP ToString method");
        }

    str.append(", ");

    switch (options.GetReadonlyPropertiesOption())
        {
            case JsonUpdater::ReadonlyPropertiesOption::Fail:
                str.append("ReadonlyPropertiesOption::Fail");
                break;
            case JsonUpdater::ReadonlyPropertiesOption::Ignore:
                str.append("ReadonlyPropertiesOption::Ignore");
                break;

            case JsonUpdater::ReadonlyPropertiesOption::Update:
                str.append("ReadonlyPropertiesOption::Update");
                break;

            default:
                return Utf8String("Unhandled ReadonlyPropertiesOption. Adjust ECDb ATP ToString method");
        }

    str.append(", ECSQLOPTIONS: ").append(options.GetECSqlOptions());
    return str;
    }

END_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
//GTest PrintTo customizations
//************************************************************************************

BEGIN_BENTLEY_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BentleyStatus stat, std::ostream* os)
    {
    switch (stat)
        {
            case SUCCESS:
                *os << "SUCCESS";
                break;

            case ERROR:
                *os << "ERROR";
                break;

            default:
                *os << "Unhandled BentleyStatus. Adjust the PrintTo method";
                break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeInt64Id id, std::ostream* os) {  *os << id.GetValueUnchecked();  }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DateTime const& dt, std::ostream* os) { *os << dt.ToString().c_str(); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DateTime::Info const& dtInfo, std::ostream* os)
    {
    if (!dtInfo.IsValid())
        {
        *os << "<invalid DateTime::Info>";
        return;
        }

    switch (dtInfo.GetComponent())
        {
        case DateTime::Component::Date:
            *os << "Date";
            return;

        case DateTime::Component::DateAndTime:
        {
        *os << "TimeStamp [";
        switch (dtInfo.GetKind())
            {
            case DateTime::Kind::Local:
                *os << "Local";
                break;
            case DateTime::Kind::Unspecified:
                *os << "Unspecified";
                break;
            case DateTime::Kind::Utc:
                *os << "Utc";
                break;
            default:
                *os << "Unhandled DateTime::Kind. Adjust the PrintTo method";
                break;

            }
        *os << "]";
        return;
        }

        case DateTime::Component::TimeOfDay:
            *os << "TimeOfDay";
            return;

        default:
            *os << "Unhandled DateTime::Component. Adjust the PrintTo method";
            break;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(std::vector<Utf8CP> const& vec, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (Utf8CP str : vec)
        {
        if (!isFirstItem)
            *os << ",";

        *os << str;
        isFirstItem = false;
        }
    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(std::vector<Utf8String> const& vec, std::ostream* os)
    {
    *os << "{";
    bool isFirstItem = true;
    for (Utf8StringCR str : vec)
        {
        if (!isFirstItem)
            *os << ",";

        *os << str.c_str();
        isFirstItem = false;
        }
    *os << "}";
    }
END_BENTLEY_NAMESPACE

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECClassId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECValue const& val, std::ostream* os) {  *os << val.ToString().c_str(); }

END_BENTLEY_ECOBJECT_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DbResult r, std::ostream* os) { *os << Db::InterpretDbResult(r); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeBriefcaseBasedId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ProfileState state, std::ostream* os)
    {
    if (state.IsError())
        {
        *os << "ProfileState(Error)";
        return;
        }

    *os << "ProfileState(";
    switch (state.GetAge())
        {
            case ProfileState::Age::Older:
                *os << "Age::Older";
                break;
            case ProfileState::Age::UpToDate:
                *os << "Age::UpToDate";
                break;
            case ProfileState::Age::Newer:
                *os << "Age::Newer";
                break;
            default:
                *os << "Invalid ProfileState::Age";
                return;
        }

    *os << ",";
    switch (state.GetCanOpen())
        {
            case ProfileState::CanOpen::No:
                *os << "CanOpen::No";
                break;
            case ProfileState::CanOpen::Readonly:
                *os << "CanOpen::Readonly";
                break;
            case ProfileState::CanOpen::Readwrite:
                *os << "CanOpen::Readwrite";
                break;
            default:
                *os << "Invalid ProfileState::CanOpen";
                return;
        }

    if (state.GetAge() == ProfileState::Age::Older)
        *os << ",Upgradable: " << state.IsUpgradable() ? "yes" : "no";

    *os << ")";
    }

END_BENTLEY_SQLITE_NAMESPACE

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECInstanceId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECInstanceKey const& key, std::ostream* os)
    {
    *os << "{ECInstanceId:";
    PrintTo(key.GetInstanceId(), os);
    *os << ",ECClassId:";
    PrintTo(key.GetClassId(), os);
    *os << "}";
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECSqlStatus stat, std::ostream* os)
    {
    if (stat.IsSuccess())
        {
        *os << "ECSqlStatus::Success";
        return;
        }

    if (stat.IsSQLiteError())
        {
        *os << "ECSqlStatus::SQLiteError " << stat.GetSQLiteError();
        return;
        }

    switch (stat.Get())
        {
            case ECSqlStatus::Status::Error:
                *os << "ECSqlStatus::Error";
                return;

            case ECSqlStatus::Status::InvalidECSql:
                *os << "ECSqlStatus::InvalidECSql";
                return;

            default:
                *os << "Unhandled ECSqlStatus. Adjust the PrintTo method";
                break;
        }
    }
void PrintTo(SchemaImportResult rc, std::ostream* os)
    {
    switch ((SchemaImportResult::Status)rc)
        {
            case SchemaImportResult::ERROR:
                *os << "SchemaImportResult::ERROR";
                return;

            case SchemaImportResult::ERROR_DATA_TRANSFORM_REQUIRED:
                *os << "SchemaImportResult::ERROR_IMODEL_LOCK_FAILED";
                return;

            case SchemaImportResult::ERROR_READONLY:
                *os << "SchemaImportResult::ERROR_READONLY";
                return;

            case SchemaImportResult::OK:
                *os << "SchemaImportResult::OK";
                return;

            default:
                *os << "Unhandled SchemaImportResult::Status. Adjust the PrintTo method";
                break;
        }
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
