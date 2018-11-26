/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Issues.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

#include <Bentley/BeNumerical.h>
#include <DgnPlatform/DgnPlatformLib.h>

#define LOG_ISSUES

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProgressMeter& Converter::GetProgressMeter() const
    {
    static NopProgressMeter s_nopMeter;
    auto meter = T_HOST.GetProgressMeter();
    return meter? *meter: s_nopMeter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtDPoint3d(DPoint3d const& pt)
    {
    return Utf8PrintfString("{%0.17lg,%0.17lg,%0.17lg}", pt.x,pt.y,pt.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtDouble(double value)
    {
    return Utf8PrintfString("%0.17lg", value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtDoubles(double const* values, size_t count)
    {
    Utf8String str("{");
    for (size_t i=0; i<count; ++i)
        {
        if (i != 0)
            str.append(", ");
        str.append(FmtDouble(values[i]));
        }
    str.append("}");
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtTransform(Transform const& trans)
    {
    return Utf8PrintfString("{%s,%s,%s}", 
                FmtDoubles(trans.form3d[0], 4).c_str(),
                FmtDoubles(trans.form3d[1], 4).c_str(),
                FmtDoubles(trans.form3d[2], 4).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtFileBaseName(DgnV8Api::DgnFile const& ff)
    {
    Utf8String fn(BeFileName::GetFileNameWithoutExtension(ff.GetFileName().c_str()));
    size_t idot = fn.find(".");
    if (idot != Utf8String::npos)
        fn.erase(idot);
    return fn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtModel(DgnV8ModelCR fm) 
    {
    Utf8String fname(FmtFileBaseName(*fm.GetDgnFileP()));
    Utf8String mname(fm.GetModelNameCP());
    return Utf8PrintfString("<%s, %s (%d)>", fname.c_str(), mname.c_str(), fm.GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtModelRef(DgnModelRefCR r) 
    {
    auto a = r.AsDgnAttachmentCP();
    if (nullptr != a)
        return FmtAttachment(*a);
    auto m = r.GetDgnModelP();
    if (nullptr != m)
        return FmtModel(*m);
    return Utf8String(r.GetModelNameCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtModel(DgnModelCR m) 
    {
    return Utf8PrintfString("%s (%lld)>", m.GetName().c_str(), m.GetModelId().GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtAttachment(DgnAttachmentCR v8Attachment) 
    {
    return Utf8PrintfString("[%ls, %ls (%llu)]", v8Attachment.GetAttachFileName().c_str(), v8Attachment.GetModelNameCP(), v8Attachment.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtElement(DgnV8Api::ElementHandle const& eh)
    {
    Bentley::WString desc;
    eh.GetHandler().GetDescription(eh, desc, 256);
    return Utf8PrintfString("%s (%lld)", Bentley::Utf8String(desc).c_str(), eh.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::IssueReporter::FmtElement(DgnElementCR el)
    {
    auto ecclass = el.GetElementClass();
    Utf8String ecclassName(ecclass? ecclass->GetName(): "?");
    return Utf8PrintfString("%s (%lld)", ecclassName.c_str(), el.GetElementId().IsValid() ? el.GetElementId().GetValue() : 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2012
//---------------------------------------------------------------------------------------
BentleyStatus Converter::IssueReporter::OpenReportFile()
    {
    if (nullptr != m_reportFile)
        return SUCCESS;

    if (m_triedOpenReport)
        return ERROR;

    m_triedOpenReport = true;
    m_reportFile = _wfsopen(m_reportFileName.GetName(), L"w+", _SH_DENYRW);

    return (nullptr == m_reportFile) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::IssueReporter::CloseReport()
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
void Converter::IssueReporter::Report(IssueSeverity severity, IssueCategory::StringId category, Utf8CP details, Utf8CP context)
    {
    Utf8String detailsStr(details ? details : "");
    Utf8String contextStr(context ? context : "");

    #ifdef LOG_ISSUES
    if (NativeLogging::LoggingConfig::IsProviderActive())
        {
        auto sev = (severity == Converter::IssueSeverity::Fatal)? LOG_FATAL: 
                   (severity == Converter::IssueSeverity::Error)? LOG_ERROR:
                   (severity == Converter::IssueSeverity::Warning)? LOG_WARNING:
                                                                LOG_INFO;
        if (LOG.isSeverityEnabled(sev))
            {
            Utf8CP contextStrDelim = Utf8String::IsNullOrEmpty(context) ? "" : " - ";
            LOG.messagev(sev, "%s - %s%s%s", category.m_str, detailsStr.c_str(), contextStrDelim, contextStr.c_str());
            }
        }
    #endif

    if (SUCCESS == OpenReportFile())
        fprintf(m_reportFile, "%s,%s,%s,%s\n", ToString(severity), category.m_str,
                escapeStringForCSV(detailsStr.c_str()).c_str(), escapeStringForCSV(contextStr.c_str()).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
Utf8CP Converter::IssueReporter::ToString(IssueSeverity sev)
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
void Converter::IssueReporter::ECDbIssueListener::_OnIssueReported(Utf8CP message) const
    {
    m_issueReporter.Report(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Message(), message);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::_ReportIssue(IssueSeverity severity, IssueCategory::StringId category, Utf8CP message, Utf8CP context) const
    {
    m_issueReporter.Report(severity, category, message, context);
    m_hadError |= ((int)severity <= (int)IssueSeverity::Error);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportIssue(IssueSeverity severity, IssueCategory::StringId category, Issue::StringId issue, Utf8CP details, Utf8CP context) 
    {
    Utf8String message;
    message.Sprintf(Issue::GetString(issue).c_str(), details);
    _ReportIssue(severity, category, message.c_str(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportIssueV(IssueSeverity severity, IssueCategory::StringId category, Issue::StringId issue, Utf8CP context, ...) 
    {
    va_list args;
    va_start(args, context);

    Utf8String message;
    message.VSprintf(Issue::GetString(issue).c_str(), args);
    _ReportIssue(severity, category, message.c_str(), context);
    va_end(args);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportError(IssueCategory::StringId category, Issue::StringId issue, Utf8CP details) 
    {
    ReportIssue(IssueSeverity::Error, category, issue, details);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportDgnV8FileOpenError(DgnV8Api::DgnFileStatus fstatus, WCharCP fn)
    {
    IssueCategory::StringId cat = IssueCategory::DiskIO();
    Issue::StringId issue = Issue::V8FileError();
    Utf8String details;
    switch (fstatus)
        {
        case DgnV8Api::DGNOPEN_STATUS_IsEncrypted:
            cat = IssueCategory::DigitalRights();
            issue = Issue::DigitalRightsAccessDenied();
            details = Utf8String(fn);
            break;

        case DgnV8Api::DGNFILE_ERROR_RightNotGranted:
        case DgnV8Api::DGNOPEN_STATUS_InsecureEnvironment:
            cat = IssueCategory::DigitalRights();
            issue = Issue::DigitalRightsNoExportRight();
            details = Utf8String(fn);
            break;

        case DgnV8Api::DGNOPEN_STATUS_FileNotFound:
            issue = Issue::FileNotFound();    
            details = Utf8String(fn);
            break;
        default: 
            details = Utf8PrintfString("%x", (int)fstatus); 
        }
    
    ReportError(cat, issue, details.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportSyncInfoIssue(IssueSeverity severity, IssueCategory::StringId category, Issue::StringId issue, Utf8CP details)
    {
    BeSQLite::DbResult lastError;
    Utf8String lastErrorDesc;
    GetSyncInfo().GetLastError(lastError, lastErrorDesc);
    BeFileName syncInfoFileName = SyncInfo::GetDbFileName(GetDgnDb());
    Utf8String desc;
    if (!lastErrorDesc.empty() || 0 != *details)
        desc.Sprintf("[%s] - %s - %s", Utf8String(syncInfoFileName).c_str(), lastErrorDesc.c_str(), details);
    else
        desc = Utf8String(syncInfoFileName);
    ReportIssue(severity, category, issue, desc.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::AddSteps(int32_t n) const
    {
    GetProgressMeter().AddSteps(n);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::AddTasks(int32_t n) const
    {
    GetProgressMeter().AddTasks(n);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::SetTaskName(ProgressMessage::StringId stringNum, ...) const
    {
    Utf8String fmt = ProgressMessage::GetString(stringNum);
    if (fmt.length() == 0)
        return;

    va_list args;
    va_start(args,stringNum);

    Utf8String value;
    value.VSprintf(fmt.c_str(), args);
    va_end(args);

    GetProgressMeter().SetCurrentTaskName(value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::SetStepName(ProgressMessage::StringId stringNum, ...) const
    {
    Utf8String fmt = ProgressMessage::GetString(stringNum);
    if (fmt.length() == 0)
        return;

    va_list args;
    va_start(args,stringNum);

    Utf8String value;
    value.VSprintf(fmt.c_str(), args);
    va_end(args);

    GetProgressMeter().SetCurrentStepName(value.c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
NativeLogging::ILogger& ConverterLogging::GetLogger(Namespace ns)
    {
    int idx = (int)ns;
    static NativeLogging::ILogger* s_loggers[(int)Namespace::MaxLoggers];

    if (s_loggers[idx] == nullptr)
        {
        static char const* s_loggerNs[] = {"DgnV8Converter", "DgnV8Converter.Level", "DgnV8Converter.Level.Mask", "DgnV8Converter.Model", "DgnV8Converter.Performance"};
        s_loggers[idx] = LoggingManager::GetLogger(s_loggerNs[idx]);
        }
    return *s_loggers[idx];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConverterLogging::IsSeverityEnabled(Namespace ns, NativeLogging::SEVERITY sev)
    {
    return GetLogger(ns).isSeverityEnabled(sev);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  10/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ConverterLogging::LogPerformance(StopWatch& stopWatch, Utf8CP description, ...)
    {
    stopWatch.Stop();
    const NativeLogging::SEVERITY severity = NativeLogging::LOG_INFO;
    NativeLogging::ILogger& logger = GetLogger(ConverterLogging::Namespace::Performance);
    if (logger.isSeverityEnabled(severity))
        {
        va_list args;
        va_start(args, description);
        Utf8String formattedDescription;
        formattedDescription.VSprintf(description, args);
        va_end(args);

        logger.messagev(severity, "%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::OnFatalError(IssueCategory::StringId cat, Issue::StringId issue, ...) const
    {
    GetProgressMeter().Hide();

    _OnFatalError();

    va_list args;
    va_start(args, issue);

    Utf8String message;
    message.VSprintf(Issue::GetString(issue).c_str(), args);
    va_end(args);

    _ReportIssue(IssueSeverity::Fatal, cat, message.c_str(), IssueCategory::GetString(cat).c_str());
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportProgress() const
    {
    if (DgnProgressMeter::ABORT_Yes == GetProgressMeter().ShowProgress())
        OnFatalError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
static WString shortenFileName (WStringCR fn)
    {
    if (fn.size() < 100)
        return fn;
    auto s = fn.substr (fn.size()-100, 100);
    s[0] = s[1] = '.';
    return s;
    }

//=======================================================================================
// Channels messages and progress from V8 functions to the dgndbsync progress meter.
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct BridgeFromV8ProgressMeter : DgnV8Api::IDgnProgressMeter
    {
    DEFINE_BENTLEY_NEW_DELETE_OPERATORS

    Converter& m_converter;
    DgnProgressMeter& m_meter;
    WString m_taskName;

    virtual void _AddTasks (ULong numTasksToAdd) override {m_meter.AddTasks((UInt32)numTasksToAdd);}
    virtual void _Hide() override {;}
    virtual bool _WasAborted() override {return false;}
    virtual void _UpdateTaskProgress() override {m_meter.ShowProgress();}
    virtual void _SetCurrentTaskName (Bentley::WCharCP newName, Bentley::WStringP previousName) override
        {
        if (previousName != NULL)
            *previousName = m_taskName.c_str();
        m_taskName.AssignOrClear (newName);
        }
    virtual void _OnTaskComplete() override {;}
    virtual void _SetCurrentTaskDescription (WCharCP filePathStr, WCharCP modelName) override
        {
        WString msg (m_taskName);
        msg.append (L" [");
        if (filePathStr)
            {
            msg.append (shortenFileName (filePathStr));
            }

        if (modelName)
            {
            if (filePathStr)
                msg.append (L" \\ ");
            msg.append (modelName);
            }
        msg.append (L"]");
        m_converter.SetTaskName (Converter::ProgressMessage::TASK_V8_PROGRESSS(), Utf8String(msg).c_str());
        }

    BridgeFromV8ProgressMeter (Converter& c) : m_converter(c), m_meter(c.GetProgressMeter()) {}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::SetV8ProgressMeter()
    {
    if (NULL == m_v8meter)
        m_v8meter = new BridgeFromV8ProgressMeter (*this);

    DgnV8Api::DgnPlatformLib::QueryHost()->SetProgressMeter (m_v8meter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ClearV8ProgressMeter()
    {
    if (NULL == m_v8meter)
        return;

    DgnV8Api::DgnPlatformLib::QueryHost()->SetProgressMeter (NULL);
    delete (BridgeFromV8ProgressMeter*)m_v8meter;
    m_v8meter = NULL;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/1
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportFailedModelConversion(ResolvedModelMapping const& v8mm)
    {
    ReportIssue(IssueSeverity::Error, IssueCategory::Unknown(), Issue::FailedToConvertModel(), nullptr, IssueReporter::FmtModel(v8mm.GetV8Model()).c_str());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/1
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ReportFailedThumbnails()
    {
    ReportIssue(IssueSeverity::Error, IssueCategory::Unknown(), Issue::FailedToConvertThumbnails(), nullptr);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE