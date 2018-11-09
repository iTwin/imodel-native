/*--------------------------------------------------------------------------------------+
|
|     $Source: SyncManager/SyncManager.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "SyncManagerInternal.h"
#include <Bentley\BeFile.h>
#include <Bentley\BeDirectoryIterator.h>
#include <Logging\bentleylogging.h>
#include "windows.h"

// Time interval to monitor and notify converter progress in milliseconds (the progress listeners are notified in this interval)
#define PROGRESS_NOTIFICATION_INTERVAL 2500

// Extension of the semaphore file used to indicate complete conversions
#define SEMAPHORE_EXTENSION L"completed"

#define OPTION_PREFIX   "prefix"
#define OPTION_NAME     "name"
#define OPTION_SEPARATOR "separator"
#define OPTION_VALUE    "value"

#define LOG (*LoggingManager::GetLogger("DgnV8Converter"))
USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_DGNDBSYNC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
void IConverter::InitOptions(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const
    {
    m_options = Json::Value(Json::arrayValue);
    _InitOptions(inputPathname, outputDirectory, outputExtension, isUpdate);

    if (!m_rspPathname.empty())
        {
        BeAssert(m_rspPathname.DoesPathExist());
        Utf8String rspPathName(m_rspPathname.c_str());
        AddOption("@", rspPathName.c_str(), nullptr, nullptr);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
void IConverter::AddOption(Utf8CP prefix, Utf8CP name, Utf8CP separator, Utf8CP value) const
    {
    Json::Value option = Json::objectValue;

    if (!Utf8String::IsNullOrEmpty(prefix))
        option[OPTION_PREFIX] = prefix;

    BeAssert(!Utf8String::IsNullOrEmpty(name));
    option[OPTION_NAME] = name;

    if (!Utf8String::IsNullOrEmpty(value))
        {
        BeAssert(!Utf8String::IsNullOrEmpty(separator) && "Need to specify a separator if specifying a value");
        option[OPTION_SEPARATOR] = separator;
        option[OPTION_VALUE] = value;
        }

    m_options.append(option);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
bool IConverter::SupportsInputExtension(WCharCP inputExtension) const
    {
    return (std::find_if(m_inExtensions.begin(), m_inExtensions.end(), [&inputExtension] (WStringCR entry)
        {
        return (0 == ::wcsicmp(entry.c_str(), inputExtension));
        }) != m_inExtensions.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
bool IConverter::SupportsOutputExtension(WCharCP outputExtension) const
    {
    return (std::find_if(m_outExtensions.begin(), m_outExtensions.end(), [&outputExtension] (WStringCR entry)
        {
        return (0 == ::wcsicmp(entry.c_str(), outputExtension));
        }) != m_outExtensions.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
bool IConverter::Validate() const
    {
    if (m_exePathname.empty())
        {
        BeAssert(false && "Path to the converter exe needs to be specified");
        return false;
        }

    if (!m_exePathname.DoesPathExist())
        {
        BeAssert(false && "Converter was not found in the specified path");
        return false;
        }

    if (m_inExtensions.empty() || m_outExtensions.empty())
        {
        BeAssert(false && "Converter does not have a valid input and/or output extension");
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
Utf8String IConverter::FormatOptionValue(Utf8CP optionValue)
    {
    Utf8String optionValueStr = optionValue;
    
    optionValueStr.Trim();
    if (optionValueStr.empty())
        return optionValueStr;

    // Remove trailing "\" - these cause quoting to go awry.
    optionValueStr.Trim("\\"); // Removes leading '\' too, but good enough. 

    // Check for spaces
    if (optionValueStr.find_first_of("\t\n ") == Utf8String::npos)
        return optionValueStr;

    Utf8PrintfString ret("\"%s\"", optionValueStr.c_str());
    return ret;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
Utf8String IConverter::BuildCommandLine(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const
    {
    InitOptions(inputPathname, outputDirectory, outputExtension, isUpdate);

    Utf8PrintfString cmdLine("\"%s\"", m_exePathname.GetNameUtf8());

    uint32_t length = m_options.size();
    for (uint32_t ii = 0; ii < length; ii++)
        {
        JsonValueCR entry = m_options[ii];
        cmdLine.append(" ");

        if (entry.isMember(OPTION_PREFIX))
            cmdLine.append(entry[OPTION_PREFIX].asString());

        BeAssert(entry.isMember(OPTION_NAME) && "Name is mandatory for every option");
        cmdLine.append(entry[OPTION_NAME].asString());

        if (entry.isMember(OPTION_VALUE))
            {
            Utf8String valueFormatted = FormatOptionValue(entry[OPTION_VALUE].asCString());
            if (!valueFormatted.empty())
                {
                BeAssert(entry.isMember(OPTION_SEPARATOR));

                cmdLine.append(entry[OPTION_SEPARATOR].asCString());
                cmdLine.append(valueFormatted);
                }
            }
        }

    return cmdLine;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
BeFileName IConverter::BuildOutputPathname(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension)
    {
    BeFileName outputPathname = outputDirectory;
    outputPathname.AppendToPath(inputPathname.GetFileNameWithoutExtension().c_str());
    outputPathname.AppendExtension(outputExtension);
    return outputPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
BeFileName IConverter::BuildSemaphorePathname(BeFileNameCR outputPathname)
    {
    BeFileName pathname = outputPathname;
    pathname.AppendExtension(SEMAPHORE_EXTENSION);
    return pathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
BeFileName IConverter::BuildLogPathname(BeFileNameCR inputPathname, BeFileNameCR outputDirectory) const
    {
    BeFileName logPathname = outputDirectory;
    logPathname.AppendToPath(inputPathname.GetFileNameWithoutExtension().c_str());
    logPathname.AppendExtension(m_logExtension.c_str());
    return logPathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
BentleyStatus IConverter::WriteJsonToFile(BeFileNameCR pathname, JsonValueCR jsonValue)
    {
    Utf8String strValue = Json::FastWriter().write(jsonValue);

    FILE* file = fopen(pathname.GetNameUtf8().c_str(), "w");
    if (file == NULL)
        {
        BeAssert(false);
        return ERROR;
        }
    fprintf(file, "%s", strValue.c_str());
    fclose(file);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
BentleyStatus IConverter::ReadJsonFromFile(JsonValueR jsonValue, BeFileNameCR pathname)
    {
    BeFile file;
    if (BeFileStatus::Success != file.Open(pathname, BeFileAccess::Read))
        {
        BeAssert(false);
        return ERROR;
        }

    uint64_t rawSize;
    if (BeFileStatus::Success != file.GetSize(rawSize) || rawSize > UINT32_MAX)
        {
        BeAssert(false);
        return ERROR;
        }

    uint32_t sizeToRead = (uint32_t) rawSize;
    BeAssert(sizeToRead > 0);

    uint32_t sizeRead;
    ScopedArray<Byte> scopedBuffer(sizeToRead);
    Byte* buffer = scopedBuffer.GetData();
    if (BeFileStatus::Success != file.Read(buffer, &sizeRead, sizeToRead) || sizeRead != sizeToRead)
        {
        BeAssert(false);
        return ERROR;
        }

    file.Close();
    
    Utf8String strValue = (Utf8CP) buffer;
    return Json::Reader::Parse(strValue, jsonValue) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
void IConverter::MarkConversionComplete(BeFileNameCR inputPathname, BeFileNameCR outputPathname)
    {
    // Marks a conversion as completed with a semaphore file
    time_t mtime;
    BeFileNameStatus status = inputPathname.GetFileTime(nullptr, nullptr, &mtime);
    BeAssert(status == BeFileNameStatus::Success);
    
    Json::Value value = Json::objectValue;
    value["SourcePathname"] = inputPathname.GetNameUtf8();
    value["SourceLastModified"] = BeJsonUtilities::StringValueFromInt64((int64_t) mtime);
    
    BeFileName semaphorePathname = BuildSemaphorePathname(outputPathname);
    WriteJsonToFile(semaphorePathname, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
bool IConverter::CheckConversionComplete(BeFileNameCR inputPathname, BeFileNameCR outputPathname)
    {
    // Check output file
    if (!outputPathname.DoesPathExist())
        return false;

    // Checks semaphore file
    BeFileName semaphorePathname = BuildSemaphorePathname(outputPathname);
    if (!semaphorePathname.DoesPathExist())
        return false;

    Json::Value value;
    BentleyStatus status = ReadJsonFromFile(value, semaphorePathname);
    if (status != SUCCESS)
        return false;

    // Check if the output was created from a different source path (with the same file name)
    if (0 != ::strcmp(value["SourcePathname"].asCString(), inputPathname.GetNameUtf8().c_str()))
        return false;

    // Check if the source file has been modified since it was last published
    int64_t lastGenerated = BeJsonUtilities::Int64FromValue(value["SourceLastModified"], 0);

    time_t mtime;
    BeFileNameStatus fileStatus = inputPathname.GetFileTime(nullptr, nullptr, &mtime);
    BeAssert(fileStatus == BeFileNameStatus::Success);
    int64_t lastModified = (int64_t) mtime;

    return (lastGenerated == lastModified);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
SyncStatus IConverter::LaunchProcess(Utf8StringCR cmdLine, IProgressListenerP progressListener)
    {
    STARTUPINFO info;
    ZeroMemory(&info, sizeof(info));
    info.cb = sizeof(info);

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));

    if (FALSE == CreateProcessA(NULL, (LPSTR) cmdLine.c_str(), NULL, NULL, FALSE, CREATE_NO_WINDOW, /*(void*)env.c_str()*/ NULL, NULL, &info, &processInfo))
        {
        BeAssert(false && "Error launching converter");
        return SyncStatus::Error;
        }

    SyncStatus status = SyncStatus::Error;
    while (true)
        {
        DWORD ret = WaitForSingleObject(processInfo.hProcess, PROGRESS_NOTIFICATION_INTERVAL);
        if (ret == WAIT_OBJECT_0)
            {
            status = SyncStatus::Completed;
            break;
            }

        if (ret != WAIT_TIMEOUT)
            {
            status = SyncStatus::Error;
            break;
            }

        if (progressListener && IProgressListener::Abort::Yes == progressListener->UpdateProgress())
            {
            TerminateProcess(processInfo.hProcess, EXIT_FAILURE);
            status = SyncStatus::Aborted;
            break;
            }
        }

    DWORD exitCode;
    if (!GetExitCodeProcess(processInfo.hProcess, &exitCode))
        status = SyncStatus::Error;

    status = (exitCode == 0) ? SyncStatus::Completed : SyncStatus::Error;

    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
void IConverter::CleanOutput(BeFileNameCR outputDirectory, WCharCP outputFileNameNoExt)
    {
    bvector<WString> deleteExtensions;
    deleteExtensions.insert(deleteExtensions.end(), m_interimExtensions.begin(), m_interimExtensions.end());
    deleteExtensions.push_back(m_logExtension.c_str());    
    for (WString const& ext : m_outExtensions)
        {
        deleteExtensions.push_back(ext.c_str());
        WPrintfString semaphoreExt(L"%ls.%ls", ext.c_str(), SEMAPHORE_EXTENSION);
        deleteExtensions.push_back(semaphoreExt.c_str());
        }
 
    BeFileName pathname = outputDirectory;
    pathname.AppendToPath(outputFileNameNoExt);
    for (WString const& ext : deleteExtensions)
        {
        BeFileName outputPathname = pathname;
        outputPathname.AppendExtension(ext.c_str());

        if (outputPathname.DoesPathExist())
            {
            BeFileNameStatus beStatus = outputPathname.BeDeleteFile();
            BeAssert(beStatus == BeFileNameStatus::Success);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    07/2016
//---------------------------------------------------------------------------------------
Utf8CP IConverter::SyncStatusToString(SyncStatus status)
    {
    switch (status)
        {
        case SyncStatus::Completed:
            return "Completed";
        case SyncStatus::CompletedWithWarnings:
            return "CompletedWithWarnings";
        case SyncStatus::InvalidInput:
            return "InvalidInput";
        case SyncStatus::ConverterNotFound:
            return "ConverterNotFound";
        case SyncStatus::Aborted:
            return "Aborted";
        case SyncStatus::Error:
            return "Error";
        }
    return "Unknown Error";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
SyncStatus IConverter::Synchronize(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, IProgressListenerP progressListener)
    {
    StopWatch timer(true);

    BeFileName outputPathname = BuildOutputPathname(inputPathname, outputDirectory, outputExtension);

    bool isUpdate = false;
    if (CheckConversionComplete(inputPathname, outputPathname))
        {
        isUpdate = SupportsUpdates();
        if (!isUpdate)
            return SyncStatus::Completed;
        }
    else
        CleanOutput(outputDirectory, outputPathname.GetFileNameWithoutExtension().c_str());

    Utf8String  cmdLine = BuildCommandLine(inputPathname, outputDirectory, outputExtension, isUpdate);

    LOG.infov("Launching command to convert: %s", cmdLine.c_str());

    SyncStatus status = LaunchProcess(cmdLine, progressListener);

    if ((status == SyncStatus::Completed || status == SyncStatus::CompletedWithWarnings) && !outputPathname.DoesPathExist())
        status = SyncStatus::Error;

    if (!isUpdate && status == SyncStatus::Completed)
        MarkConversionComplete(inputPathname, outputPathname);

    LOG.infov("Finished command to convert with status: %s", SyncStatusToString(status));
    LOG.infov("Time for converting from %ls to %ls: %.0f seconds", inputPathname.GetExtension().c_str(), outputPathname.GetExtension().c_str(), timer.GetCurrentSeconds());

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
DgnDwgConverterBase::DgnDwgConverterBase(BeFileNameCR exePathname, WCharCP inputExtension) : IConverter(exePathname)
    {
    m_inExtensions.push_back(inputExtension);
    
    m_outExtensions.push_back(L"ibim");
    m_outExtensions.push_back(L"imodel");

    m_logExtension = L"ibim-issues";
    
    m_interimExtensions.push_back(L"ibim.syncinfo");
    m_interimExtensions.push_back(L"ibim.syncinfo-journal");
    m_interimExtensions.push_back(L"ibim-journal");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
void DgnDwgConverterBase::_InitOptions(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const
    {
    Utf8CP prefix = "--";
    Utf8CP separator = "=";

    AddOption(prefix, "no-assert-dialogs", nullptr, nullptr);
    AddOption(prefix, "input", separator, inputPathname.GetNameUtf8().c_str());
    AddOption(prefix, "output", separator, outputDirectory.GetNameUtf8().c_str());
    if (0 == ::wcsicmp(outputExtension, L"imodel"))
        AddOption(prefix, "compress", nullptr, nullptr);
    if (isUpdate)
        AddOption(prefix, "update", nullptr, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
IConverterPtr DgnConverter::Create(BeFileNameCR exePathname)
    {
    IConverterPtr converter = new DgnConverter(exePathname);
    if (!converter->Validate())
        return nullptr;

    return converter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
IConverterPtr DwgConverter::Create(BeFileNameCR exePathname)
    {
    IConverterPtr converter = new DwgConverter(exePathname);
    if (!converter->Validate())
        return nullptr;

    return converter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
RvmVueConverterBase::RvmVueConverterBase(BeFileNameCR exePathname, WCharCP inputExtension) : IConverter(exePathname)
    {
    m_inExtensions.push_back(inputExtension);
    m_outExtensions.push_back(L"dgn");
    m_logExtension = L"log";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
void RvmVueConverterBase::_InitOptions(BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension, bool isUpdate) const
    {
    BeAssert(m_inExtensions.size() == 1);
    bool isVue = (0 == ::wcsicmp(m_inExtensions[0].c_str(), L"vue"));

    // PxJob64.exe exportrvm - rvm:"<input .rvm file name>"[-dgn:"<output .dgn or i.dgn file name>"][-log:"<output log file name>"]
    // SPxJob64.exe exportvue -vue:"<input .vue file name>" [-dgn:"<output .dgn or i.dgn file name>"] [-log:"<output log file name>"]
    if (isVue)
        AddOption(nullptr, "exportvue", nullptr, nullptr);
    else
        AddOption(nullptr, "exportrvm", nullptr, nullptr);

    Utf8CP prefix = "-";
    Utf8CP separator = ":";

    AddOption(prefix, isVue ? "vue" : "rvm", separator, inputPathname.GetNameUtf8().c_str());

    BeFileName outputPathname = BuildOutputPathname(inputPathname, outputDirectory, outputExtension);
    AddOption(prefix, "dgn", separator, outputPathname.GetNameUtf8().c_str());

    BeFileName logPathname = BuildLogPathname(inputPathname, outputDirectory);
    AddOption(prefix, "log", separator, logPathname.GetNameUtf8().c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
IConverterPtr RvmConverter::Create(BeFileNameCR exePathname)
    {
    IConverterPtr converter = new RvmConverter(exePathname);
    if (!converter->Validate())
        return nullptr;

    return converter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
IConverterPtr VueConverter::Create(BeFileNameCR exePathname)
    {
    IConverterPtr converter = new VueConverter(exePathname);
    if (!converter->Validate())
        return nullptr;

    return converter;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
BentleyStatus SyncManager::RegisterConverter(IConverterR converter)
    {
    if (!converter.Validate())
        return ERROR;

    m_converters.push_back(&converter);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
IConverterP SyncManager::GetRegisteredConverter(WCharCP inExtension, WCharCP outExtension)
    {
    if (!inExtension || !outExtension)
        {
        BeAssert(false && "Invalid input to GetRegisteredConverter");
        return nullptr;
        }

    for (IConverterPtr& converter : m_converters)
        {
        if (inExtension && !converter->SupportsInputExtension(inExtension))
            continue;

        if (outExtension && !converter->SupportsOutputExtension(outExtension))
            continue;

        return converter.get();
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
// static
BeFileName SyncManager::BuildPathname(BeFileNameCR directory, WCharCP fileNameNoExt, WCharCP extension)
    {
    BeFileName pathname = directory;
    pathname.AppendToPath(fileNameNoExt);
    pathname.AppendExtension(extension);
    return pathname;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2016
//---------------------------------------------------------------------------------------
SyncStatus SyncManager::Synchronize(bvector<Utf8String>& errors, BeFileNameCR inputPathname, BeFileNameCR outputDirectory, WCharCP outputExtension)
    {
    // Error checks
    WString inputExtension = inputPathname.GetExtension();
    if (WString::IsNullOrEmpty(outputExtension) || inputExtension.empty())
        {
        BeAssert(false && "Input and output extensions cannot be empty");
        return SyncStatus::InvalidInput;
        }
    
    SyncStatus status;

    // Try 1 stage conversion
    IConverterP converter1 = GetRegisteredConverter(inputExtension.c_str(), outputExtension);
    if (converter1)
        return converter1->Synchronize(inputPathname, outputDirectory, outputExtension, m_progressListener);
     
    // Try 2 stage conversion with a dgn file in-between
    converter1 = GetRegisteredConverter(inputExtension.c_str(), L"dgn");
    IConverterP converter2 = GetRegisteredConverter(L"dgn", outputExtension);
    if (!converter1 || !converter2)
        {
        LOG.infov("Could not find a registered converter for the specified input and output extensions");
        BeAssert(false);
        return SyncStatus::ConverterNotFound;
        }

    status = converter1->Synchronize(inputPathname, outputDirectory, L"dgn", m_progressListener);
    if (status != SyncStatus::Completed && status != SyncStatus::CompletedWithWarnings)
        return status;

    BeFileName interimPathname = IConverter::BuildOutputPathname(inputPathname, outputDirectory, L"dgn");
    status = converter2->Synchronize(interimPathname, outputDirectory, outputExtension, m_progressListener);
    return status;
    }

END_DGNDBSYNC_NAMESPACE
