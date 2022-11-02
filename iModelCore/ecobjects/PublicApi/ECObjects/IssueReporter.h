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
    IssueCategory(const char* stringId): m_stringId(stringId) {}
    operator const char*() const { return m_stringId; }

    static IssueCategory BusinessProperties; // should be used when an issue is related to ECFramework;
    };

//=======================================================================================
// See https://bsw-wiki.bentley.com/bin/view.pl/Main/Badgers-CategoriesTypes-Guideline for parameter guides.
// @bsistruct
//+===============+===============+===============+===============+===============+======
struct ECOBJECTS_EXPORT IssueType
    {
    const char* m_stringId;
    IssueType(const char* stringId): m_stringId(stringId) {}
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
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct IIssueListener
    {
private:
    //! Fired by ECObjects whenever an issue occurred.
    //! @param[in] message Issue message
    ECOBJECTS_EXPORT virtual void _OnIssueReported(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message) const = 0;

public:
    IIssueListener() {}
    virtual ~IIssueListener() {}

#if !defined (DOCUMENTATION_GENERATOR)
    //! Report an issue to clients
    //! @param[in] message Issue message
    void ReportIssue(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message) const { _OnIssueReported(severity, category, type, message); }
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
    virtual void _Report(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message) const = 0;
    virtual NativeLogging::CategoryLogger GetLogger() const = 0;
public:

    template<typename ...FmtArgs>
    void ReportV(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message, FmtArgs&& ...fmtArgs) const
        {
        Utf8String formattedMessage;
        formattedMessage.Sprintf(message, std::forward<FmtArgs>(fmtArgs)...);
        Report(severity, category, type, formattedMessage.c_str());
        }

    void Report(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message) const
        {
        auto LOG = GetLogger();
        if (severity == IssueSeverity::Info)
            LOG.info(message);
        else if (severity == IssueSeverity::Warning || severity == IssueSeverity::CriticalWarning)
            LOG.warning(message);
        else if (severity == IssueSeverity::Error || severity == IssueSeverity::Fatal)
            LOG.error(message);
        _Report(severity, category, type, message);
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
    ECOBJECTS_EXPORT void _Report(IssueSeverity severity, IssueCategory category, IssueType type, Utf8CP message) const override;
    NativeLogging::CategoryLogger GetLogger() const override { return NativeLogging::CategoryLogger("ECObjectsNative"); }

public:
    IssueReporter() = default;
    IssueReporter(IssueReporter const&) = delete;
    IssueReporter& operator=(IssueReporter const&) = delete;

    ECOBJECTS_EXPORT BentleyStatus AddListener(IIssueListener const&);
    ECOBJECTS_EXPORT void RemoveListener();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
