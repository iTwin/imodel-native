/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Samples/ListProp/ListProperty.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "ListProperty.h"

static const wchar_t    s_spinner[] = L" /-\\|";
static const size_t     s_spinnerSize = _countof(s_spinner)-1;
static WCharCP          s_configFileName = L"DwgImporter.logging.config.xml";

BEGIN_DGNDBSYNC_DWG_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool    ListProperty::ValidatePropertyFormat (WStringR newString, WStringR currString, WStringR previousString) const
    {
    // An additional string that may need to be inserted before currString
    newString.clear ();

    if (WString::npos == currString.find(L':'))
        {
        // Current string has no property separator ":" - don't guess, just use previous string as is:
        if (!previousString.empty())
            newString = previousString;

        // move on to next string
        previousString = currString;
        return  false;
        }
    else if (currString.at(0) == L':')
        {
        // Current string starts by ":", combine it with previous string.
        if (!previousString.empty())
            {
            previousString += currString;
            return  false;
            }
        }
    else
        {
        // Current string appears like a well formated string by itself - safe to use previousString
        if (!previousString.empty())
            {
            newString = previousString;
            previousString.clear ();
            return  false;
            }
        }

    // Always empty the string we are tracking with if we have a valid string to add:
    previousString.clear ();

    // Go ahead and attach validated string to the element
    return  true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::ConvertMessage (T_Utf8StringVectorR utf8List, WStringR currMessage, WStringR prevMessage) const
    {
    currMessage.Trim ();
    if (currMessage.empty())
        return  BSIERROR;

    // Make format of "property name :/= <property value>".
    WString     messageToInsert;
    bool        saveCurrMessage = ValidatePropertyFormat (messageToInsert, currMessage, prevMessage);

    // Insert a new string before currString per above validation.
    Utf8String  propString;
    if (!messageToInsert.empty())
        {
        propString.Assign (messageToInsert.c_str());
        utf8List.push_back (propString);
        }

    // Add current string per above validation.
    if (saveCurrMessage)
        {
        propString.Assign (currMessage.c_str());
        utf8List.push_back (propString);
        }

    return  BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::ConvertMessageCollection (T_Utf8StringVectorR utf8List, T_WStringVectorR wcharList) const
    {
    /*-----------------------------------------------------------------------------------
    The strings have harvested from the DWG toolkit host's callback are just a collection
    of text strings.  These are the text string you'd see in AutoCAD when you use the LIST
    command on the same entity.  They are not real properties.
    
    To help make our Adhoc string properties look readable, here we try to parse the string 
    dump into a collection of strings each of which is in a format of:

        "property name : property value"

    Note that this by no means a foolproof string processor and exceptions printed out by
    the toolkit can happen, but is enough to serve for the purpose of a sample code.
    -----------------------------------------------------------------------------------*/
    WString     lastMessage;

    utf8List.clear ();

    for (auto& message : wcharList)
        {
        if (message.empty())
            continue;

        // Break down the text body into multiple lines of texts that are separately by a LINEFEED:
        size_t  lineFeedAt = WString::npos;
        while (WString::npos != (lineFeedAt = message.find(0x0A)))
            {
            if (lineFeedAt+1 == message.length())
                {
                lineFeedAt = WString::npos;
                break;
                }

            WString     subString(message.c_str(), lineFeedAt);
            ConvertMessage (utf8List, subString, lastMessage);
            message.erase (0, lineFeedAt + 1);
            }

        // Add last line of text:
        if (WString::npos == lineFeedAt)
            ConvertMessage (utf8List, message, lastMessage);
        }

    // Add last string that happens to be "property name : <no property value>"
    if (!lastMessage.empty())
        {
        WString     emptyMessage;
        ConvertMessage (utf8List, lastMessage, emptyMessage);
        }

    return  utf8List.empty() ? BSIERROR : BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::ListEntityProperties (DgnElementR element, DwgDbEntityCR entity)
    {
    BentleyStatus       status = BSISUCCESS;
    T_WStringVector     messageList;

    // Request DwgImporter host to collect message dump sent from the toolkit(RealDWG):
    T_Super::GetMessageCenter().StartListMessageCollection (&messageList);

    try
        {
        // Start ACAD command LIST on this entity and let the DwgImporter host fill up our messageList:
        entity.List ();
        }
    catch (...)
        {
        status = BSIERROR;
        LOG.errorv ("Failed LIST command on entity %ls[ID=%lld]", entity.GetDxfName().c_str(), entity.GetObjectId().ToUInt64());
        }

    // Done message dump
    T_Super::GetMessageCenter().StopListMessageCollection ();
    if (BSISUCCESS != status)
        return  status;

    // Convert the raw AutoCAD messages into something presentable as string properies:
    T_Utf8StringVector  stringProperties;
    if (BSISUCCESS != ConvertMessageCollection(stringProperties, messageList))
        return  BSIERROR;

    // Add each and every string as an Adhoc text property of the DgnElement
    size_t  digits = static_cast<size_t> (floor(log10(stringProperties.size())) + 1);
    size_t  count = 1;
    for (auto& string : stringProperties)
        {
        // Use property name "LPxx"
        Utf8PrintfString    uniqueName("LP%0*d", digits, count++);
        // Set the text value to the element's Adhoc property
        AdHocJsonValue      adhocProp;
        adhocProp.SetValueText ("Text", string.c_str());
        element.SetUserProperties (uniqueName.c_str(), adhocProp);
        }

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListProperty::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // Create a DgnElement from DWG entity by default, and get the results back:
    DgnElementP     dgnElement = nullptr;
    BentleyStatus   status = T_Super::_ImportEntity (results, inputs);
    if (BSISUCCESS != status || nullptr == (dgnElement = results.GetImportedElement()))
        {
        LOG.errorv ("Failed creating DgnElement from DWG entity %lld", inputs.GetEntity().GetObjectId().ToUInt64());
        return  status;
        }

    // Don't want to LIST standard ACAD entities:
    DwgString   className = inputs.GetEntity().GetClassName ();
    if (className.StartsWithI(L"AcDb"))
        return  status;

    LOG.tracev ("DgnElement %s(ID=%lld) has been created from a custom object, trying LIST command...", dgnElement->GetDisplayLabel().c_str(), dgnElement->GetElementId());

    // Create Adhoc properties from the LIST "properties" of a custom object:
    status = ListEntityProperties (*dgnElement, inputs.GetEntity());

    if (BSISUCCESS != status)
        LOG.error ("Failed adding LIST properties to DgnElement as Adhoc properties!");

    return  status;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void            ListPropertySample::GetImportConfiguration (BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0)
    {
    WString programDir = BeFileName::GetDirectoryName (argv0);

    if (!configurationPath.empty())
        {
        instanceFilePath.SetName(configurationPath);
        }
    else
        {
        instanceFilePath.SetName(programDir.c_str());
        instanceFilePath.AppendToPath(L"ConvertConfig.xml");
        }

    WString programBasename = BeFileName::GetFileNameWithoutExtension (argv0);

    LOG.tracev ("%ls creating DgnDb using configuration file <%ls>", programBasename.c_str(), instanceFilePath.GetName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListPropertySample::GetEnv(BeFileName& fn, WCharCP envname)
    {
    wchar_t filepath[MAX_PATH];
    if ((0 == GetEnvironmentVariableW(envname, filepath, MAX_PATH)))
        return BSIERROR;

    fn.SetName(filepath);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListPropertySample::GetLogConfigurationFilename(BeFileName& configFile, WCharCP programName)
    {
    WString programBasename = BeFileName::GetFileNameWithoutExtension(programName);

    if (BSISUCCESS == GetEnv(configFile, L"BENTLEY_DWGIMPORTER_LOGGING_CONFIG"))
        {
        if (configFile.DoesPathExist())
            {
            LOG.tracev ("%ls configuring logging with %s (Set by BENTLEY_DWGIMPORTER_LOGGING_CONFIG environment variable.)", programBasename.c_str(), configFile.GetName());
            return BSISUCCESS;
            }
        }

    configFile = BeFileName(BeFileName::DevAndDir, programName);
    configFile.AppendToPath(s_configFileName);
    configFile.BeGetFullPathName();

    if (BeFileName::DoesPathExist(configFile))
        {
        LOG.tracev ("%ls configuring logging using %ls. Override it by setting BENTLEY_DWGIMPORTER_LOGGING_CONFIG in environment.", programBasename.c_str(), configFile.GetName());
        return BSISUCCESS;
        }

    return BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void            ListPropertySample::InitLogging (WCharCP programName)
    {
    BeFileName configFile;

    if (BSISUCCESS == GetLogConfigurationFilename(configFile, programName))
        {
        NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, configFile);
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
        }
    else
        {
        std::cout << "Logging.config.xml not found. Activating default logging using console provider" << std::endl;
        NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::CONSOLE_LOGGING_PROVIDER);
        NativeLogging::LoggingConfig::SetSeverity(L"Performance", NativeLogging::LOG_TRACE);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListPropertySample::ImportDwgFile ()
    {
    // Instantiate our class
    ListProperty*  importer = new ListProperty (m_options);
    if (nullptr == importer)
        {
        LOG.errorv ("Out of memory prior to processing file %ls.", m_inputFileName.GetName());
        return  BSIERROR;
        }

    // Try open the inpurt DWG file:
    auto status = importer->OpenDwgFile (m_inputFileName);
    if (BSISUCCESS != status)
        {
        LOG.errorv ("Error opening DWG file %ls.", m_inputFileName.GetName());
        return BSIERROR;
        }

    // create a new or update existing output DgnDb:
    status = importer->CreateNewDgnDb (m_outputName);
    if (BSISUCCESS != status)
        {
        LOG.errorv (L"Error creating output file %ls.", m_outputName.GetName());
        return BSIERROR;
        }

    // ready to import DWG to DgnDb:
    status = importer->Process ();

    delete importer;

    // terminate the toolkit after DwgDbDatabase is released(i.e. above ~ListProperty call):
    DwgImporter::TerminateDwgHost ();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ListPropertySample::Initialize (WCharCP programName, WCharCP dwgFileName, WCharCP dgndbFolderName)
    {
    m_inputFileName.SetName (dwgFileName);
    m_outputName.SetName (dgndbFolderName);
    if (m_inputFileName.empty() || m_outputName.empty())
        {
        std::cout << "Empty input/output file name" << std::endl;
        return  BSIERROR;
        }

    m_inputFileName.BeGetFullPathName();
    
    m_outputName.BeGetFullPathName();
    if (!m_outputName.DoesPathExist())
        {
        if (m_outputName.GetExtension().empty())
            m_outputName.AppendSeparator();

        BeFileName  outputDir = m_outputName.GetDirectoryName();
        if (!outputDir.DoesPathExist() && (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(outputDir.c_str())))
            {
            std::wcout << L"Cannot create output directory" << outputDir.c_str() << std::endl;
            return BSIERROR;
            }
        }

    if (m_outputName.IsDirectory())
        {
        m_outputName.AppendToPath(m_inputFileName.GetFileNameWithoutExtension().c_str());
        m_outputName.AppendExtension(UNCOMPRESSED_DGNDB_EXT);
        }
    else
        {
        m_outputName.OverrideNameParts(L"." UNCOMPRESSED_DGNDB_EXT);
        }

    InitLogging (programName);

    // FOR THE CONSOLE PUBLISHER ONLY! "Gui" publishers won't have any console output and won't need this.
    // C++ programs start-up with the "C" locale in effect by default, and the "C" locale does not support conversions of any characters outside
    // the "basic character set". ... The call to setlocale() says "I want to use the user's default narrow string encoding". This encoding is
    // based on the Posix-locale for Posix environments. In Windows, this encoding is the ACP, which is based on the system-locale.
    // However, the success of this code is dependent on two things:
    //      1) The narrow encoding must support the wide character being converted.
    //      2) The font/gui must support the rendering of that character.
    // In Windows, #2 is often solved by setting cmd.exe's font to Lucida Console."
    // (http://cboard.cprogramming.com/cplusplus-programming/145590-non-english-characters-cout-2.html)
    setlocale (LC_CTYPE, "");

    // Instantiate the DgnPlatform host for the DwgImporter
    DwgImporter::InitializeDgnHost (*this);

    m_options.SetProgressMeter (&m_progressMeter);
    _set_error_mode (_OUT_TO_MSGBOX);

    // Resolve import config file.
    BeFileName configFile;
    GetImportConfiguration (configFile, m_options.GetConfigFile(), programName);
    m_options.SetConfigFile (configFile);

    // Set dir prefix to be dropped from filenames when coming up with unique but portable filenames in syncinfo
    m_options.SetInputRootDir (m_inputFileName.GetDirectoryName());

    BeFileName reportFileName (m_outputName);
    reportFileName.append (L"-issues");
    reportFileName.BeDeleteFile ();
    m_options.SetReportFile (reportFileName);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
L10N::SqlangFiles ListPropertySample::_SupplySqlangFiles () 
    {
    BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang/DwgImporter_en-US.sqlang.db3");
    BeAssert(sqlangFile.DoesPathExist());

    return L10N::SqlangFiles(sqlangFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool PrintfProgressMeter::HasDescription () const
    {
    return m_taskName.find(':') != Utf8String::npos;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnProgressMeter::Abort PrintfProgressMeter::_ShowProgress ()
    {
    if (m_aborted)
        return ABORT_Yes;

    auto now = BeTimeUtilities::QuerySecondsCounter();

    if ((now - m_timeOfLastSpinnerUpdate) < 0.25) // don't do printf's more than a few times per second -- too slow and not useful
        return ABORT_No;

    m_timeOfLastSpinnerUpdate = now;

    m_spinCount++;

    bool justShowSpinner = false;

    if ((now - m_timeOfLastUpdate) < 0.5)
        justShowSpinner = true;         // don't push out full messages more than a couple times per second -- too slow and not useful
    else
        justShowSpinner = (FmtMessage() == m_lastMessage);

    if (justShowSpinner)
        {
        printf("[%c]\r", s_spinner[m_spinCount%s_spinnerSize]);
        return ABORT_No;
        }
    
    ForceNextUpdateToDisplay();
    UpdateDisplay();
    return ABORT_No;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PrintfProgressMeter::_Hide ()
    {
    Utf8PrintfString msg("    %-123.123s %-16.16s", "", "");
    printf("%s\r", msg.c_str());
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void PrintfProgressMeter::UpdateDisplay0 (Utf8StringCR msgLeft)
    {
    m_lastMessage = msgLeft;

    // Display the number of tasks remaining. Not all major tasks have a task count.
    Utf8String tbd;
    if (m_fileCount || m_stepsRemaining || m_tasksRemaining)
        tbd = Utf8PrintfString(":%d/%d/%d", m_fileCount, m_stepsRemaining, m_tasksRemaining);

    // Display the spinner and the task.
    Utf8PrintfString msg("[%c] %-123.123s %-16.16s", s_spinner[m_spinCount%s_spinnerSize], msgLeft.c_str(), tbd.c_str());
    printf("%s\r", msg.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
Utf8String PrintfProgressMeter::FmtMessage () const
    {
    Utf8String msg(m_stepName);
    msg.append(": ");
    msg.append(m_taskName);
    return msg;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void    PrintfProgressMeter::UpdateDisplay ()
    {
    auto now = BeTimeUtilities::QuerySecondsCounter ();

    if ((now - m_timeOfLastUpdate) < 1.0)
        return;

    m_timeOfLastUpdate = now;

    UpdateDisplay0(FmtMessage());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void    PrintfProgressMeter::_SetCurrentTaskName   (Utf8CP newName)
    {
    if (newName && m_taskName == newName)
        return;

    m_taskName = newName? newName: "";
    m_spinCount=0;
    ForceNextUpdateToDisplay();
    UpdateDisplay();
    T_Super::_SetCurrentTaskName(newName); // decrements task count
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void    PrintfProgressMeter::_SetCurrentStepName (Utf8CP stepName)
    {
    if (NULL == stepName)
        {
        m_stepName.clear();
        return;
        }
    if (m_stepName.Equals(stepName))
        return;

    m_stepName = stepName;
    m_taskName.clear();
    m_spinCount=0;
    ForceNextUpdateToDisplay();
    UpdateDisplay();
    T_Super::_SetCurrentStepName(stepName); // decrements step count
    }

END_DGNDBSYNC_DWG_NAMESPACE


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int wmain (int argc, WCharP argv[])
    {
    /*-----------------------------------------------------------------------------------
    Expected command line arguments:

    ListProperty <input DWG full file name> <output DgnDb folder name>
    -----------------------------------------------------------------------------------*/
    if (argc < 3)
        return  1;

    ListPropertySample     sampleImporter;
    if (BSISUCCESS != sampleImporter.Initialize(argv[0], argv[1], argv[2]))
        return 2;

    // Begin importing DWG file into DgnDb
    BentleyStatus   status = sampleImporter.ImportDwgFile ();

    return (int)status;
    }
