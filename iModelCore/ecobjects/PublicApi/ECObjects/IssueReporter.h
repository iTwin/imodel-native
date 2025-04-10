/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>
#include <Bentley/BeThread.h>
#include <Bentley/Logging.h>
#include <utility>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
// @bsienum
//+===============+===============+===============+===============+===============+======
enum struct IssueSeverity
    {
    Fatal = 1,           //! task fails immediately
    Error = 2,           //! task will fail, but will try to continue working and discover more errors first
    Warning = 3,
    Info = 4,
    CriticalWarning = 5, //! task will not fail, but there is a high likelihood of unintended output
    };

//=======================================================================================
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct ECOBJECTS_EXPORT IssueCategory
    {
    const char* m_stringId;
    explicit IssueCategory(const char* stringId): m_stringId(stringId) {}
    operator const char*() const { return m_stringId; }

    static IssueCategory BusinessProperties; // should be used when an issue is related to ECFramework;
    static IssueCategory SchemaSync;
    };

//=======================================================================================
// See https://bsw-wiki.bentley.com/bin/view.pl/Main/Badgers-CategoriesTypes-Guideline for parameter guides.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct ECOBJECTS_EXPORT IssueType
    {
    const char* m_stringId;
    explicit IssueType(const char* stringId): m_stringId(stringId) {}
    operator const char*() const { return m_stringId; }

    static IssueType ECClass;
    static IssueType ECCustomAttribute;
    static IssueType ECInstance;
    static IssueType ECProperty;
    static IssueType ECRelationshipClass;
    static IssueType ECSchema;
    static IssueType ECSQL;
    static IssueType ImportFailure;
    static IssueType Units;
    static IssueType InvalidInputData;
    static IssueType Error;
    };

//=======================================================================================
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct ECOBJECTS_EXPORT IssueId
    {
    const char* m_issueId;
    explicit IssueId(const char* issueId): m_issueId(issueId) {}
    operator const char*() const { return m_issueId; }
    };

//=======================================================================================
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct ECOBJECTS_EXPORT ECIssueId
    {
    static IssueId EC_0001;
    static IssueId EC_0002;
    static IssueId EC_0003;
    static IssueId EC_0004;
    static IssueId EC_0005;
    static IssueId EC_0006;
    static IssueId EC_0007;
    static IssueId EC_0008;
    static IssueId EC_0009;
    static IssueId EC_0010;
    static IssueId EC_0011;
    static IssueId EC_0012;
    static IssueId EC_0013;
    static IssueId EC_0014;
    static IssueId EC_0015;
    static IssueId EC_0016;
    static IssueId EC_0017;
    static IssueId EC_0018;
    static IssueId EC_0019;
    static IssueId EC_0020;
    static IssueId EC_0021;
    static IssueId EC_0022;
    static IssueId EC_0023;
    static IssueId EC_0024;
    static IssueId EC_0025;
    static IssueId EC_0026;
    static IssueId EC_0027;
    static IssueId EC_0028;
    static IssueId EC_0029;
    static IssueId EC_0030;
    static IssueId EC_0031;
    static IssueId EC_0032;
    static IssueId EC_0033;
    static IssueId EC_0034;
    static IssueId EC_0035;
    static IssueId EC_0036;
    static IssueId EC_0037;
    static IssueId EC_0038;
    static IssueId EC_0039;
    static IssueId EC_0040;
    static IssueId EC_0041;
    static IssueId EC_0042;
    static IssueId EC_0043;
    static IssueId EC_0044;
    static IssueId EC_0045;
    static IssueId EC_0046;
    static IssueId EC_0047;
    static IssueId EC_0048;
    static IssueId EC_0049;
    static IssueId EC_0050;
    static IssueId EC_0051;
    static IssueId EC_0052;
    static IssueId EC_0053;
    static IssueId EC_0054;
    static IssueId EC_0055;
    static IssueId EC_0056;
    static IssueId EC_0057;
    static IssueId EC_0058;
    static IssueId EC_0059;
    static IssueId EC_0060;
    static IssueId EC_0061;
    static IssueId EC_0062;
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct IIssueListener
    {
private:
    //! Fired by ECObjects whenever an issue occurred.
    //! @param[in] message Issue message
    ECOBJECTS_EXPORT virtual void _OnIssueReported(IssueSeverity severity, IssueCategory category, IssueType type, IssueId id, Utf8CP message) const = 0;

public:
    IIssueListener() {}
    virtual ~IIssueListener() {}

#if !defined (DOCUMENTATION_GENERATOR)
    //! Report an issue to clients
    //! @param[in] message Issue message
    void ReportIssue(IssueSeverity severity, IssueCategory category, IssueType type, IssueId id, Utf8CP message) const { _OnIssueReported(severity, category, type, id, message); }
#endif
    };


/*---------------------------------------------------------------------------------**//**
* use an instance of this interface to report issues during ECObject API
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct IIssueReporter
    {
protected:
    IIssueReporter() {}
    virtual void _Report(IssueSeverity severity, IssueCategory category, IssueType type, IssueId id, Utf8CP message) const = 0;
    virtual NativeLogging::CategoryLogger GetLogger() const = 0;
public:

    template<typename ...FmtArgs>
    void ReportV(IssueSeverity severity, IssueCategory category, IssueType type, IssueId id, Utf8CP message, FmtArgs&& ...fmtArgs) const
        {
        Utf8String formattedMessage;
        formattedMessage.Sprintf(message, std::forward<FmtArgs>(fmtArgs)...);
        Report(severity, category, type, id, formattedMessage.c_str());
        }

    void Report(IssueSeverity severity, IssueCategory category, IssueType type, IssueId id, Utf8CP message) const
        {
        auto LOG = GetLogger();
        if (severity == IssueSeverity::Info)
            LOG.info(message);
        else if (severity == IssueSeverity::Warning || severity == IssueSeverity::CriticalWarning)
            LOG.warning(message);
        else if (severity == IssueSeverity::Error || severity == IssueSeverity::Fatal)
            LOG.error(message);
        _Report(severity, category, type, id, message);
        }

    virtual ~IIssueReporter() {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct IssueReporter : public IIssueReporter
    {
private:
    mutable BeMutex m_mutex;
    IIssueListener const* m_issueListener = nullptr;

protected:
    ECOBJECTS_EXPORT void _Report(IssueSeverity severity, IssueCategory category, IssueType type, IssueId id, Utf8CP message) const override;
    NativeLogging::CategoryLogger GetLogger() const override { return NativeLogging::CategoryLogger("ECObjectsNative"); }

public:
    IssueReporter() = default;
    IssueReporter(IssueReporter const&) = delete;
    IssueReporter& operator=(IssueReporter const&) = delete;

    ECOBJECTS_EXPORT BentleyStatus AddListener(IIssueListener const&);
    ECOBJECTS_EXPORT void RemoveListener();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
