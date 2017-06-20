/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Samples/XData/ImportXData.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "ImportXData.h"

static const wchar_t    s_spinner[] = L" /-\\|";
static const size_t     s_spinnerSize = _countof(s_spinner)-1;
static WCharCP          s_configFileName = L"DwgImporter.logging.config.xml";

//---------------------------------------------------------------------------------------
//  This is a helper macro that adds a unique Adhoc property in AdhocPropertiesBuilder(below):
//
//  1. Create an Adhoc property name, "XDxx", where xx is a 2-digit count of total entries
//  2. Get AdHocJsonPropertyValue by name, from the input DgnElement
//  3. Set the Adhoc value by type
//  4. Increase the property counter by 1 for next property
//
//---------------------------------------------------------------------------------------
#define ADDADHOCPROPERTY(_Type_, _Value_)                                               \
    Utf8PrintfString    uniqueName("XD%02d", m_count++);                                \
    AdHocJsonValue      adhocProp;                                                      \
    adhocProp.SetValue##_Type_ (#_Type_, _Value_);                                      \
    m_element.SetUserProperties (uniqueName.c_str(), adhocProp);


BEGIN_DGNDBSYNC_DWG_NAMESPACE

//=======================================================================================
// A helper class that adds an Adhoc property at a time to an input DgnElement. It tracks
// number of the properties added to ensure unique property names being added on the same 
// element.
//
// @bsiclass
//=======================================================================================
struct  AdhocPropertiesBuilder
    {
private:
    DgnElementR     m_element;
    uint32_t        m_count;

public:
    // Constructor
    AdhocPropertiesBuilder (DgnElementR el) : m_element(el) { m_count = 0; }

    // Add properties by type
    void AddInt (int32_t i)         { ADDADHOCPROPERTY(Int, i) }
    void AddInt64 (int64_t i)       { ADDADHOCPROPERTY(Int64, i) }
    void AddDouble (double d)       { ADDADHOCPROPERTY(Double, d) }
    void AddText (Utf8CP t)         { ADDADHOCPROPERTY(Text, t) }
    void AddPoint3d (DPoint3dCR p)  { ADDADHOCPROPERTY(Point3d, p) }
    };  // AdhocPropertiesBuilder

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXData::ConvertXData (DgnElementR element, DwgDbEntityCR entity)
    {
    // Get xdata from the entity - an empty regapp name extracts all xdata
    DwgString           allRegApps;
    DwgResBufIterator   xdata = entity.GetXData (allRegApps);

    BentleyStatus       status = BSISUCCESS;

    if (xdata.IsValid())
        {
        LOG.tracev ("Entity %ls has XDATA, adding Adhoc properties to DgnElement...", entity.GetDxfName().c_str());

        AdhocPropertiesBuilder  builder(element);

        // Iterate through all entries, prints them and add them to DgnElement as Adhoc properties:
        for (DwgResBufP curr = xdata->Start(); curr != xdata->End(); curr = curr->Next())
            {
            switch (curr->GetDataType())
                {
                case DwgResBuf::DataType::Integer8:
                    LOG.tracev ("XDATA Int8= %d", curr->GetInteger8());
                    builder.AddInt (curr->GetInteger8());
                    break;
                case DwgResBuf::DataType::Integer16:
                    LOG.tracev ("XDATA Int16= %d", curr->GetInteger16());
                    builder.AddInt (curr->GetInteger16());
                    break;
                case DwgResBuf::DataType::Integer32:
                    LOG.tracev ("XDATA Int32= %d", curr->GetInteger32());
                    builder.AddInt (curr->GetInteger32());
                    break;
                case DwgResBuf::DataType::Integer64:
                    LOG.tracev ("XDATA Int64= %I64d", curr->GetInteger64());
                    builder.AddInt64 (curr->GetInteger64());
                    break;
                case DwgResBuf::DataType::Double:
                    LOG.tracev ("XDATA Double= %g", curr->GetDouble());
                    builder.AddDouble (curr->GetDouble());
                    break;
                case DwgResBuf::DataType::Text:
                    LOG.tracev ("XDATA String= %ls", curr->GetString().c_str());
                    builder.AddText (DwgHelper::ToUtf8CP(curr->GetString()));
                    break;
                case DwgResBuf::DataType::BinaryChunk:
                    {
                    // Binary is not currently supported as an Adhoc property - just show size in a text
                    DwgBinaryData   data;
                    if (DwgDbStatus::Success == curr->GetBinaryData(data))
                        {
                        LOG.tracev ("XDATA Binary data size = %lld", data.GetSize());
                        builder.AddText (Utf8PrintfString("binary data in %lld bytes", data.GetSize()).c_str());
                        }
                    else
                        {
                        BeDataAssert(false && "failed extracting binary xdata!");
                        }
                    break;
                    }
                case DwgResBuf::DataType::Handle:
                    LOG.tracev ("XDATA Handle= %ls", curr->GetHandle().AsAscii().c_str());
                    builder.AddText (Utf8PrintfString("Handle=%ls", curr->GetHandle().AsAscii().c_str()).c_str());
                    break;
                case DwgResBuf::DataType::HardOwnershipId:
                case DwgResBuf::DataType::SoftOwnershipId:
                case DwgResBuf::DataType::HardPointerId:
                case DwgResBuf::DataType::SoftPointerId:
                    LOG.tracev ("XDATA ObjectId= %ls", curr->GetObjectId().ToAscii().c_str());
                    builder.AddText (Utf8PrintfString("ObjectID=%ls", curr->GetObjectId().ToAscii().c_str()).c_str());
                    break;
                case DwgResBuf::DataType::Point3d:
                    {
                    DPoint3d    point;
                    if (DwgDbStatus::Success == curr->GetPoint3d(point))
                        {
                        LOG.tracev ("XDATA Point3d= %g, %g, %g", point.x, point.y, point.z);
                        builder.AddPoint3d (point);
                        }
                    else
                        {
                        BeDataAssert (false && "failed extracting Point3d xdata!");
                        }
                    break;
                    }
                case DwgResBuf::DataType::None:
                case DwgResBuf::DataType::NotRecognized:
                default:
                    LOG.warning ("Unexpected XDATA type!!!");
                }
            }
        }

    return  status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXData::_ImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    // Create a DgnElement from DWG entity by default, and get the results back:
    DgnElementP     dgnElement = nullptr;
    BentleyStatus   status = T_Super::_ImportEntity (results, inputs);
    if (BSISUCCESS != status || nullptr == (dgnElement = results.GetImportedElement()))
        {
        LOG.errorv ("Failed creating DgnElement from DWG entity %lld", inputs.GetEntity().GetObjectId().ToUInt64());
        return  status;
        }

    LOG.tracev ("DgnElement %s(ID=%lld) has been created, checking XDATA...", dgnElement->GetDisplayLabel().c_str(), dgnElement->GetElementId());

    status = ConvertXData (*dgnElement, inputs.GetEntity());

    if (BSISUCCESS != status)
        LOG.error ("Failed adding xdata to DgnElement as Adhoc properties!");

    return  status;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void            ImportXDataSample::GetImportConfiguration (BeFileNameR instanceFilePath, BeFileNameCR configurationPath, WCharCP argv0)
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
BentleyStatus   ImportXDataSample::GetEnv(BeFileName& fn, WCharCP envname)
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
BentleyStatus   ImportXDataSample::GetLogConfigurationFilename(BeFileName& configFile, WCharCP programName)
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
void            ImportXDataSample::InitLogging (WCharCP programName)
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
BentleyStatus   ImportXDataSample::ImportDwgFile ()
    {
    // Instantiate our class
    ImportXData*  importer = new ImportXData (m_options);
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

    // terminate the toolkit after DwgDbDatabase is released(i.e. above ~ImportXData call):
    DwgImporter::TerminateDwgHost ();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus   ImportXDataSample::Initialize (WCharCP programName, WCharCP dwgFileName, WCharCP dgndbFolderName)
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
L10N::SqlangFiles ImportXDataSample::_SupplySqlangFiles () 
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

    ImportXData <input DWG full file name> <output DgnDb folder name>
    -----------------------------------------------------------------------------------*/
    if (argc < 3)
        return  1;

    ImportXDataSample     sampleImporter;
    if (BSISUCCESS != sampleImporter.Initialize(argv[0], argv[1], argv[2]))
        return 2;

    // Begin importing DWG file into DgnDb
    BentleyStatus   status = sampleImporter.ImportDwgFile ();

    return (int)status;
    }
