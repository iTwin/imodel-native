/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/ProcessLogger.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

#include <Bentley/BeNumerical.h>
#include <DgnPlatform/DgnPlatformLib.h>

#define LOG_ISSUES

BEGIN_DWG_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
NativeLogging::ILogger&         DwgImportLogging::GetLogger (Namespace name)
    {
    int                             index = static_cast<int> (name);
    static NativeLogging::ILogger*  s_loggers[static_cast<int>(Namespace::MaxLoggers)];

    if (s_loggers[index] == nullptr)
        {
        static char const*  s_loggerNamespaces[] = 
            {
            "DwgImporter",
            "DwgImporter.Model",
            "DwgImporter.Layer",
            "DwgImporter.Linetype",
            "DwgImporter.Textstyle",
            "DwgImporter.Material",
            "DwgImporter.Entity",
            "DwgImporter.Dictionary",
            "DwgImporter.Performance"
            };
        s_loggers[index] = LoggingManager::GetLogger (s_loggerNamespaces[index]);
        }

    return *s_loggers[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DwgImportLogging::IsSeverityEnabled (Namespace name, NativeLogging::SEVERITY severity)
    {
    return DwgImportLogging::GetLogger(name).isSeverityEnabled(severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImportLogging::LogPerformance (StopWatch& stopWatch, Utf8CP description, ...)
    {
    stopWatch.Stop ();

    const NativeLogging::SEVERITY   severity = NativeLogging::LOG_INFO;
    NativeLogging::ILogger&         logger = DwgImportLogging::GetLogger (DwgImportLogging::Namespace::Performance);

    if (logger.isSeverityEnabled(severity))
        {
        va_list args;
        va_start(args, description);

        Utf8String      formattedDescription;
        formattedDescription.VSprintf(description, args);
        va_end(args);

        logger.messagev (severity, "%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtDPoint3d (DPoint3dCR pt)
    {
    return Utf8PrintfString ("{%0.17lg,%0.17lg,%0.17lg}", pt.x,pt.y,pt.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtDouble (double value)
    {
    return Utf8PrintfString ("%0.17lg", value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtDoubles (double const* values, size_t count)
    {
    Utf8String str ("{");
    for (size_t i=0; i<count; ++i)
        {
        if (i != 0)
            str.append (", ");
        str.append (FmtDouble(values[i]));
        }
    str.append ("}");
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtTransform (TransformCR trans)
    {
    return Utf8PrintfString("{%s,%s,%s}", 
                FmtDoubles(trans.form3d[0], 4).c_str(),
                FmtDoubles(trans.form3d[1], 4).c_str(),
                FmtDoubles(trans.form3d[2], 4).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtModel (DwgDbBlockTableRecordCR block)
    {
    Utf8String  fileName (block.GetDatabase()->GetFileName().c_str());
    Utf8String  blockName (block.GetName().c_str());
        
    return Utf8PrintfString ("<%s, %s (%I64d)>", fileName.c_str(), blockName.c_str(), block.GetObjectId().GetHandle().AsUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtXReference (DwgDbBlockTableRecordCR block)
    {
    Utf8String  path (block.GetPath().c_str());
    Utf8String  blockName (block.GetName().c_str());
        
    return Utf8PrintfString ("<%s, %ls (%I64d)>", path.c_str(), blockName.c_str(), block.GetObjectId().GetHandle().AsUInt64());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtElement (DgnElementCR el)
    {
    auto ecclass = el.GetElementClass();
    Utf8String ecclassName(ecclass? ecclass->GetName(): "?");
    return Utf8PrintfString("element %s (%lld)", ecclassName.c_str(), el.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgImporter::IssueReporter::FmtEntity (DwgDbEntityCR entity)
    {
    DwgDbObjectId   id = entity.GetObjectId ();
    return Utf8PrintfString("entity %ls [%lld]", id.GetDwgClassName().c_str(), id.GetHandle().AsUInt64());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2012
//---------------------------------------------------------------------------------------
BentleyStatus   DwgImporter::IssueReporter::OpenReportFile()
    {
    if (nullptr != m_reportFile)
        return SUCCESS;

    if (m_triedOpenReport)
        return BSIERROR;

    m_triedOpenReport = true;
    m_reportFile = _wfsopen(m_reportFileName.GetName(), L"w+", _SH_DENYRW);

    return (nullptr == m_reportFile) ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::IssueReporter::CloseReport()
    {
    if (m_reportFile)
        {
        fclose(m_reportFile);
        m_reportFile = 0;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2012
//---------------------------------------------------------------------------------------
static Utf8String escapeStringForCSV(Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return Utf8String();

    Utf8String processedStr = str;

    if ((nullptr == strchr(str, ',')) && (nullptr == strchr(str, '\"')))
        return processedStr;

    processedStr.ReplaceAll("\"", "\"\"");
    processedStr.insert(0, "\"");
    processedStr += "\"";

    return processedStr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::IssueReporter::Report(IssueSeverity severity, IssueCategory::StringId category, Utf8CP details, Utf8CP context)
    {
    Utf8String detailsStr(details ? details : "");
    Utf8String contextStr(context ? context : "");

#ifdef LOG_ISSUES
    if (NativeLogging::LoggingConfig::IsProviderActive())
        {
        auto sev = (severity == DwgImporter::IssueSeverity::Fatal)? LOG_FATAL: 
                   (severity == DwgImporter::IssueSeverity::Error)? LOG_ERROR:
                   (severity == DwgImporter::IssueSeverity::Warning)? LOG_WARNING:
                                                                LOG_INFO;
        if (LOG.isSeverityEnabled(sev))
            {
            Utf8CP contextStrDelim = Utf8String::IsNullOrEmpty(context) ? "" : " - ";
            LOG.messagev(sev, "%s - %s%s%s", category.m_str, detailsStr.c_str(), contextStrDelim, contextStr.c_str());
            }
        }
#endif

    if (SUCCESS == OpenReportFile())
        {
        fprintf (m_reportFile, "%s,%s,%s", ToString(severity), category.m_str, escapeStringForCSV(detailsStr.c_str()).c_str());
        if (!contextStr.empty())
            fprintf (m_reportFile, " - %s", escapeStringForCSV(contextStr.c_str()).c_str());
        fprintf (m_reportFile, "\n");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
Utf8CP      DwgImporter::IssueReporter::ToString(IssueSeverity sev)
    {
    switch (sev)
        {
            case IssueSeverity::Error:
                return "Error";

            case IssueSeverity::Fatal:
                return "Fatal";

            case IssueSeverity::Info:
                return "Info";

            case IssueSeverity::Warning:
                return "Warning";

            default:
                BeAssert(false && "Please update IssueReporter::ToString to new value of IssueSeverity.");
                return "";
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     09/2015
//---------------------------------------------------------------------------------------
void            DwgImporter::IssueReporter::ECDbIssueListener::_OnIssueReported(Utf8CP message) const
    {
    m_issueReporter.Report (DwgImporter::IssueSeverity::Error, IssueCategory::Sync(), Issue::Message(), message);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::_ReportIssue(IssueSeverity severity, IssueCategory::StringId category, Utf8CP message, Utf8CP contextIn) 
    {
    Utf8CP contextStr = contextIn;

    Utf8PrintfString testString("%s%s%s%s", IssueReporter::ToString(severity), category.m_str, nullptr == message ? "" : message, nullptr == contextIn ? "" : contextIn);
    if (m_reportedIssues.find(testString) != m_reportedIssues.end())
        return;
    m_reportedIssues.insert (testString);

    m_issueReporter.Report (severity, category, message, contextStr);
    if ((int)severity <= (int)IssueSeverity::Error)
        m_errorCount++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::ReportIssue(IssueSeverity severity, IssueCategory::StringId category, Issue::StringId issue, Utf8CP details, Utf8CP context) 
    {
    Utf8String message;
    message.Sprintf(Issue::GetString(issue).c_str(), details);
    _ReportIssue(severity, category, message.c_str(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::ReportIssueV(IssueSeverity severity, IssueCategory::StringId category, Issue::StringId issue, Utf8CP context, ...) 
    {
    va_list args;
    va_start (args, context);

    Utf8String message;
    message.VSprintf(Issue::GetString(issue).c_str(), args);
    _ReportIssue(severity, category, message.c_str(), context);
    va_end (args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::ReportError (IssueCategory::StringId category, Issue::StringId issue, Utf8CP details) 
    {
    ReportIssue (IssueSeverity::Error, category, issue, details);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::ReportSyncInfoIssue (IssueSeverity severity, IssueCategory::StringId category, Issue::StringId issue, Utf8CP details)
    {
    BeSQLite::DbResult lastError;
    Utf8String lastErrorDesc;
    GetSyncInfo().GetLastError (lastError, lastErrorDesc);
    BeFileName syncInfoFileName = DwgSyncInfo::GetDbFileName (GetDgnDb());
    Utf8String desc;
    if (!lastErrorDesc.empty() || 0 != *details)
        desc.Sprintf("[%s] - %s - %s", Utf8String(syncInfoFileName).c_str(), lastErrorDesc.c_str(), details);
    else
        desc = Utf8String(syncInfoFileName);
    ReportIssue (severity, category, issue, desc.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::AddTasks(int32_t n) 
    {
    GetProgressMeter().AddTasks (n);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::SetTaskName(ProgressMessage::StringId stringNum, ...)
    {
    Utf8String fmt = ProgressMessage::GetString (stringNum);
    if (fmt.length() == 0)
        return;

    va_list args;
    va_start(args,stringNum);

    Utf8String value;
    value.VSprintf (fmt.c_str(), args);
    va_end (args);

    GetProgressMeter().SetCurrentTaskName (value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::SetStepName (ProgressMessage::StringId stringNum, ...)
    {
    Utf8String fmt = ProgressMessage::GetString (stringNum);
    if (fmt.length() == 0)
        return;

    va_list args;
    va_start(args,stringNum);

    Utf8String value;
    value.VSprintf (fmt.c_str(), args);
    va_end (args);

    GetProgressMeter().SetCurrentStepName (value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::OnFatalError(IssueCategory::StringId cat, Issue::StringId issue, ...)
    {
    GetProgressMeter().Hide();

    _OnFatalError();

    va_list args;
    va_start (args, issue);

    Utf8String message;
    message.VSprintf(Issue::GetString(issue).c_str(), args);
    va_end (args);

    _ReportIssue(IssueSeverity::Fatal, cat, message.c_str(), IssueCategory::GetString(cat).c_str());
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgImporter::Progress ()
    {
    if (DgnProgressMeter::ABORT_Yes == this->GetProgressMeter().ShowProgress())
        this->OnFatalError ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProgressMeterR   DwgImporter::GetProgressMeter () const
    {
    // use DgnHost's progress meter
    static NopProgressMeter s_nopProgressMeter;

    auto hostProgressMeter = T_HOST.GetProgressMeter ();

    return  nullptr == hostProgressMeter ? s_nopProgressMeter : *hostProgressMeter;
    }
 
END_DWG_NAMESPACE