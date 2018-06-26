/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/BackDoor/ECDbTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/ECDb/ECDbTests.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

//************************************************************************************
//ECDbIssueListener
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle     09/2015
//---------------------------------------------------------------------------------------
ECDbIssue ECDbIssueListener::GetIssue() const
    {
    if (!m_issue.IsIssue())
        return m_issue;

    ECDbIssue copy(m_issue);
    //reset cached issue before returning
    m_issue = ECDbIssue();
    return copy;
    }

//************************************************************************************
//ECDbTestLogger
//************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static member initialization
BentleyApi::NativeLogging::ILogger* ECDbTestLogger::s_logger = nullptr;


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    03/2014
//---------------------------------------------------------------------------------------
//static
BentleyApi::NativeLogging::ILogger& ECDbTestLogger::Get()
    {
    if (s_logger == nullptr)
        s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger(L"ECDbTests");

    return *s_logger;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  09/17
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
// @bsimethod                                    Krischan.Eberle                  09/17
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
// @bsimethod                                    Krischan.Eberle                  06/17
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
// @bsimethod                                    Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeInt64Id id, std::ostream* os) {  *os << id.GetValueUnchecked();  }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DateTime const& dt, std::ostream* os) { *os << dt.ToString().c_str(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  07/17
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
// @bsimethod                                    Krischan.Eberle                  07/17
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
// @bsimethod                                    Krischan.Eberle                  05/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECClassId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECValue const& val, std::ostream* os) {  *os << val.ToString().c_str(); }

END_BENTLEY_ECOBJECT_NAMESPACE

BEGIN_BENTLEY_SQLITE_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(DbResult r, std::ostream* os) { *os << Db::InterpretDbResult(r); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(BeBriefcaseBasedId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/18
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
// @bsimethod                                    Krischan.Eberle                  06/17
//+---------------+---------------+---------------+---------------+---------------+------
void PrintTo(ECInstanceId id, std::ostream* os) { PrintTo((BeInt64Id) id, os); }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  06/17
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
// @bsimethod                                    Krischan.Eberle                  05/17
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

END_BENTLEY_SQLITE_EC_NAMESPACE
