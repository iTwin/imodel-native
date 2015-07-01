/*--------------------------------------------------------------------------------------+
|
|     $Source: IModelExtractor/IModelExtractor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Logging/bentleylogging.h>
#include <Bentley/BeFilename.h>
#include <BeSQLite/IModelDb.h>
#include <Windows.h>

#define APPLICATION_NAME    L"imodel"
#define PACKAGE_EXTENSION   L"imodel"
#define PROJECT_EXTENSION   L"idgndb"

#define LOG                     (*LoggingManager::GetLogger (APPLICATION_NAME))
#define EXTRACTOR_E(FMT,...)    fprintf (stderr, "Error: " FMT "\n", __VA_ARGS__); //LOG.errorv (FMT, __VA_ARGS__);
#define EXTRACTOR_I(FMT,...)    fprintf (stdout, FMT "\n", __VA_ARGS__); //LOG.infov (FMT, __VA_ARGS__);
#define EXTRACTOR_V(FMT,...)    LOG.tracev (FMT, __VA_ARGS__);

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_LOGGING

//=======================================================================================
// @bsiclass                                            Grigas.Petraitis     04/2013
//=======================================================================================
struct CommandLineParser
{
private:
    int         m_argc;
    WCharCP*    m_argv;
private:
    WCharCP GetArg (WCharCP arg, bool& isFlag);
protected:
    virtual void _ParseArgument (WCharCP arg, bool isFlag, bool isLast) { }
    virtual bool _IsValid () = 0;
public:
    CommandLineParser (int argc, WCharCP argv[]);
    BentleyStatus Parse ();
};

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
CommandLineParser::CommandLineParser (int argc, WCharCP argv[])
    : m_argc (argc), m_argv (argv)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
WCharCP CommandLineParser::GetArg (WCharCP arg, bool& isFlag)
    {
    if (0 == wcsncmp (L"--", arg, 2))
        {
        isFlag = true;
        return arg + 2;
        }

    if (0 == wcsncmp (L"/", arg, 1) || 0 == wcsncmp (L"-", arg, 1))
        {
        isFlag = true;
        return arg + 1;
        }

    isFlag = false;
    return arg;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
BentleyStatus CommandLineParser::Parse ()
    {
    if (m_argc < 2)
        return ERROR;

    for (int i = 1; i < m_argc; i++)
        {
        bool isFlag;
        WCharCP arg = GetArg (m_argv[i], isFlag);
        _ParseArgument (arg, isFlag, i == m_argc - 1);
        }
    return _IsValid () ? SUCCESS : BSIERROR;
    }

//=======================================================================================
// @bsiclass                                            Grigas.Petraitis     04/2013
//=======================================================================================
struct IModelExtractorCmdParser : public CommandLineParser
{
private:
    bool    m_showHelp;
    bool    m_verbose;
    bool    m_overwrite;
    WCharCP m_input;
    WCharCP m_output;
    bool    m_waitsForOutput;

protected:
    virtual void _ParseArgument (WCharCP arg, bool isFlag, bool isLast) override
        {
        if (isFlag)
            {
            if (0 == wcsncmp (L"h", arg, 1) || 0 == wcsncmp (L"help", arg, 4))
                m_showHelp = true;
            else if (0 == wcsncmp (L"v", arg, 1) || 0 == wcsncmp (L"verbose", arg, 7))
                m_verbose = true;
            else if (0 == wcsncmp (L"w", arg, 1) || 0 == wcsncmp (L"overWrite", arg, 9))
                m_overwrite = true;
            else if (0 == wcsncmp (L"o", arg, 1) || 0 == wcsncmp (L"output", arg, 6))
                m_waitsForOutput = true;
            }
        else
            {
            if (m_waitsForOutput)
                {
                m_output = arg;
                m_waitsForOutput = false;
                }
            else if (isLast)
                m_input = arg;
            }
        }

    virtual bool _IsValid () override
        {
        // if we're showing "help", other options aren't important
        if (m_showHelp)
            return true;

        // input file is required and must not be empty
        if (NULL == m_input || '\0' == *m_input)
            return false;
        // fail if there was "-x" option without the specified output path
        if (m_waitsForOutput)
            return false;

        // all other options are optional
        return true;
        }
public:
    IModelExtractorCmdParser (int argc, WCharCP argv[])
        : CommandLineParser (argc, argv), m_showHelp (false), m_verbose (false), m_overwrite (false),
        m_input (NULL), m_output (NULL), m_waitsForOutput (false)
        {
        }
    bool ShowHelp () { return m_showHelp; }
    bool Verbose () { return m_verbose; }
    bool Overwrite () { return m_overwrite; }
    WCharCP GetInput () { return m_input; }
    WCharCP GetOutput () { return m_output; }
};

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static void printUsage (WCharCP programName)
    {
    WString exeName = BeFileName::GetFileNameAndExtension (programName);
    EXTRACTOR_I ("\
Usage: %s [OPTIONS]... INPUT\n\
Extracts INPUT i-model to .idgndb. Optional arguments:\n\
    -o, --output OUTPUT     If the specified OUTPUT path is a file name, extract .idgndb to this path. If the specified path is a directory name, extract the .idgndb to that directory with base name of the INPUT file.\n\
    -w, --overWrite         Overwrite output file if it already exists.\n\
    -v, --verbose           Verbose output.\n\
    -h, --help              Display this help and exit.\
\n", Utf8String (exeName.c_str ()).c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static BentleyStatus initialize (bool verbose)
    {
    // set logger options
    LoggingConfig::SetOption (CONFIG_OPTION_DEFAULT_SEVERITY, LOG_TEXT_INFO);
    LoggingConfig::ActivateProvider (NativeLogging::CONSOLE_LOGGING_PROVIDER);
    LoggingConfig::SetSeverity (APPLICATION_NAME, verbose ? LOG_TRACE : LOG_INFO);

    // create temporary directory
    WChar tempPathW[MAX_PATH];
    ::GetTempPathW (_countof (tempPathW), tempPathW);

    BeFileName tempDir (tempPathW);
    tempDir.AppendToPath (L"Bentley");
    tempDir.AppendToPath (APPLICATION_NAME);

    BeFileNameStatus status = BeFileName::CreateNewDirectory (tempDir.GetName());
    if ((status != BeFileNameStatus::Success) && (status != BeFileNameStatus::AlreadyExists))
        {
        EXTRACTOR_E ("Error creating temporary directory <%s>", Utf8String (tempDir.GetName ()).c_str ());
        return BSIERROR;
        }
    EXTRACTOR_V ("Using temporary directory <%s>", Utf8String (tempDir.GetName ()).c_str ());

    // initialize BeSQLite
    BeSQLiteLib::Initialize (tempDir);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static BentleyStatus validateInputFilePath (WCharCP path)
    {
    // make sure the input file exists
    if (!BeFileName::DoesPathExist (path))
        {
        EXTRACTOR_E ("Input file <%s> does not exist", Utf8String (path).c_str ());
        return BSIERROR;
        }
    // make sure the input path points to a file
    if (BeFileName::IsDirectory (path))
        {
        EXTRACTOR_E ("Input path <%s> is a directory", Utf8String (path).c_str ());
        return BSIERROR;
        }
    EXTRACTOR_V ("Using input file <%s>", Utf8String (path).c_str ());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static BentleyStatus validateOutputFilePath (WCharCP path, bool overwrite)
    {
    // make sure the output directory exists
    WString outputDirectory = BeFileName::GetDirectoryName (path);
    if (outputDirectory.empty ())
        {
        EXTRACTOR_E ("Invalid output directory <%s>", Utf8String (outputDirectory.c_str ()).c_str ());
        return BSIERROR;
        }
    if (!BeFileName::DoesPathExist (outputDirectory.c_str ()))
        {
        EXTRACTOR_V ("Output directory <%s> does not exist. Creating...", Utf8String (outputDirectory.c_str ()).c_str ());
        if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory (outputDirectory.c_str ()))
            {
            EXTRACTOR_E ("Cannot create output directory <%s>", Utf8String (outputDirectory.c_str ()).c_str ());
            return BSIERROR;
            }
        EXTRACTOR_V ("Output directory created");
        }
    // make sure the output file does not exist
    if (BeFileName::DoesPathExist (path))
        {
        EXTRACTOR_V ("Output file <%s> already exists", Utf8String (path).c_str ());
        if (overwrite)
            {
            EXTRACTOR_V ("Deleting output file...");
            if (BeFileNameStatus::Success != BeFileName::BeDeleteFile (path))
                {
                EXTRACTOR_E ("Output file <%s> cannot be deleted", Utf8String (path).c_str ());
                return BSIERROR;
                }
            EXTRACTOR_V ("Output file deleted");
            }
        else 
            {
            EXTRACTOR_E ("Specify -o or --overWrite option to overwrite the output file <%s>", Utf8String (path).c_str ());
            return BSIERROR;
            }
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static DbResult checkPackageVersion (BeSQLite::Db& db, Db::OpenParams const& openParams)
    {
    EXTRACTOR_V ("Querying package schema version...");
    Utf8String versionString;
    if (BE_SQLITE_ROW != db.QueryProperty (versionString, PackageProperty::SchemaVersion ()))
        {
        EXTRACTOR_E ("Could not find package schema version property");
        return BE_SQLITE_ERROR_NoPropertyTable;
        }
    EXTRACTOR_V ("Actual package schema version: %s", versionString.c_str ());

    SchemaVersion actualPackageSchemaVersion (0,0,0,0);
    actualPackageSchemaVersion.FromJson (versionString.c_str ());
    SchemaVersion expectedPackageVersion (PACKAGE_CURRENT_VERSION_Major, PACKAGE_CURRENT_VERSION_Minor, PACKAGE_CURRENT_VERSION_Sub1, PACKAGE_CURRENT_VERSION_Sub2);
    SchemaVersion minimumAutoUpgradablePackageVersion (PACKAGE_SUPPORTED_VERSION_Major, PACKAGE_SUPPORTED_VERSION_Minor, PACKAGE_SUPPORTED_VERSION_Sub1, PACKAGE_SUPPORTED_VERSION_Sub2);

    EXTRACTOR_V ("Expected package schema version: %s", expectedPackageVersion.ToJson ().c_str ());
    EXTRACTOR_V ("Minimum auto-upgradable package schema version: %s", minimumAutoUpgradablePackageVersion.ToJson ().c_str ());

    bool needsUpgrade = false; //unused as this app does not implement any auto-upgrade
    const auto stat = BeSQLite::Db::CheckProfileVersion (needsUpgrade, expectedPackageVersion, actualPackageSchemaVersion, minimumAutoUpgradablePackageVersion, openParams.IsReadonly(), "Package");
    switch (stat) 
        {
        case BE_SQLITE_ERROR_ProfileTooOld:
            EXTRACTOR_E ("Package schema is too old");
            break;
        case BE_SQLITE_ERROR_ProfileTooNew:
            EXTRACTOR_E ("Package schema is too new");
            break;
        default:
            EXTRACTOR_V ("Package schema version is compatible.");
            break;
        }

    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static BentleyStatus getEmbeddedFileName (WString& embeddedFileName, BeSQLite::Db& db/*, WCharCP expectedName*/)
    {
    // construct expected embedded file's name
    Utf8String expectedName;
    if (!embeddedFileName.empty ())
        expectedName.Assign (embeddedFileName.c_str ());
    else
        expectedName.Sprintf ("%s.%s", Utf8String (BeFileName::GetFileNameWithoutExtension (WString (db.GetDbFileName (), BentleyCharEncoding::Utf8).c_str ()).c_str ()).c_str (), Utf8String (PROJECT_EXTENSION).c_str ());

    DbEmbeddedFileTable& embeddedFiles = db.EmbeddedFiles ();
    BeRepositoryBasedId id = embeddedFiles.QueryFile (expectedName.c_str ());
    if (id.IsValid ())
        {
        embeddedFileName.AssignUtf8 (expectedName.c_str ());
        return SUCCESS;
        }
    else
        {
        EXTRACTOR_V ("Expected file <%s> was not found in the package. Looking for a single embedded file...", expectedName.c_str ());
        DbEmbeddedFileTable::Iterator iterator = embeddedFiles.MakeIterator ();
        if (iterator.QueryCount () != 1)
            {
            EXTRACTOR_E ("The package does not contain a single embedded file.");
            return BSIERROR;
            }

        DbEmbeddedFileTable::Iterator::Entry entry = iterator.begin ();
        id = embeddedFiles.QueryFile (entry.GetNameUtf8 ());
        if (id.IsValid ())
            {
            EXTRACTOR_V ("Found a single embedded file <%s>", entry.GetNameUtf8 ());
            embeddedFileName.AssignUtf8 (entry.GetNameUtf8 ());
            return SUCCESS;
            }
        else
            {
            EXTRACTOR_E ("Did not find an embedded file");
            return BSIERROR;
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static BentleyStatus resolveOutputFilePath (BeFileNameR outputFilePath, WStringR embeddedFileName, WCharCP inputFilePath, WCharCP outputPath, BeSQLite::Db& db)
    {
    WString outputFileName;
    outputFilePath = BeFileName (outputPath);
    if (!outputFilePath.IsEmpty ())
        {
        EXTRACTOR_V ("Found output path <%s>", Utf8String (outputPath).c_str ());

        // if the user specified the output path, it's either a path to a directory or a file
        WString outputDevice;
        WString outputDirectory;
        WString outputFileExtension;
        BeFileName::ParseName (&outputDevice, &outputDirectory, &outputFileName, &outputFileExtension, outputPath);

        // if the extension is empty, assume that the specified path is a directory path without the trailing slash
        if (outputFileExtension.empty ())
            {
            outputDirectory.append (outputFileName.c_str ());
            outputFileName.clear ();
            }
        else 
            {
            // if the extension is not empty but the file name is empty, assume it's a directory
            if (outputFileName.empty ())
                {
                outputDirectory.append (L".");
                outputDirectory.append (outputFileExtension.c_str ());
                outputFileExtension.clear ();
                }
            // if we have both name and extension, just append the extension to name
            else
                {
                outputFileName.append (L".");
                outputFileName.append (outputFileExtension);
                }
            }
        outputFilePath.BuildName (outputDevice.c_str (), outputDirectory.c_str (), outputFileName.c_str (), NULL);

        // if device is empty, assume that the specified path is relative to input path
        if (outputDevice.empty ())
            {
            EXTRACTOR_V ("Relative output path. Constructing full path...");
            BeFileName devAndDir (BeFileName::DevAndDir, inputFilePath);
            devAndDir.AppendToPath (outputFilePath);
            outputFilePath = devAndDir;
            }
        }
    else
        {
        EXTRACTOR_V ("Output path not specified. Using the same directory as input...");
        outputFilePath = BeFileName (BeFileName::DevAndDir, inputFilePath);
        }

    // we're supposed to have an output path at this moment
    BeAssert (!outputFilePath.IsEmpty ());

    // get the name of the embedded file
    embeddedFileName.AssignOrClear (outputFileName.c_str ());
    if (SUCCESS != getEmbeddedFileName (embeddedFileName, db))
        return BSIERROR;

    // only override the file name if wasn't specified
    if (outputFileName.empty ())
        outputFilePath.OverrideNameParts (embeddedFileName.c_str ());
    EXTRACTOR_V ("Resolved output file path to <%s>", Utf8String (outputFilePath.GetName ()).c_str ());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
static BentleyStatus extractFromPackage (WCharCP input, WCharCP output, bool overwrite)
    {
    // open the package
    BeSQLite::Db   db;
    DbResult       result;
    Db::OpenParams openParams (Db::OpenMode::Readonly);
    EXTRACTOR_V ("Opening package database...");
    if (BE_SQLITE_OK != (result = db.OpenBeSQLiteDb (Utf8String (input).c_str (), openParams)))
        {
        EXTRACTOR_E ("Failed to open package database. Error <%s>", Db::InterpretDbResult (result));
        return BSIERROR;
        }
    EXTRACTOR_V ("Package database opened");

    // validate package schema version
    EXTRACTOR_V ("Validating package schema version...");
    if (BE_SQLITE_OK != checkPackageVersion (db, openParams))
        {
        EXTRACTOR_E ("Invalid package schema version");
        return BSIERROR;
        }
    EXTRACTOR_V ("Package schema version is valid");

    // use the full path of the open Db rather than the original "input" (which may have been just a file name in the current directory)
    BeFileName inputFilePath (db.GetDbFileName(), BentleyCharEncoding::Utf8);
    BeFileName outputFilePath;
    WString    embeddedFileName;

    if (SUCCESS != resolveOutputFilePath (outputFilePath, embeddedFileName, inputFilePath.GetName(), output, db))
        return BSIERROR;

    if (SUCCESS != validateOutputFilePath (outputFilePath, overwrite))
        return BSIERROR;
    
    EXTRACTOR_V ("Extracting <%s> to <%s>", Utf8String (embeddedFileName.c_str ()).c_str (), outputFilePath.GetNameUtf8 ().c_str ());
    if (BE_SQLITE_OK != (result = db.EmbeddedFiles().Export (outputFilePath.GetNameUtf8 ().c_str (), Utf8String (embeddedFileName).c_str ())))
        {
        EXTRACTOR_E ("Failed to extract the file");
        return BSIERROR;
        }
    EXTRACTOR_V ("Extraction finished.");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Grigas.Petraitis     04/2013
//---------------------------------------------------------------------------------------
int wmain (int argc, WCharCP argv[])
    {
    // parse command line arguments
    IModelExtractorCmdParser parser (argc, argv);
    if (ERROR == parser.Parse ())
        {
        EXTRACTOR_E ("Invalid arguments");
        printUsage (argv[0]);
        return 1;
        }

    // help command
    if (parser.ShowHelp ())
        {
        printUsage (argv[0]);
        return 0;
        }

    // initialization
    initialize (parser.Verbose ());
    
    // make sure the input file exists
    WString inputFilePath (parser.GetInput ());
    if (SUCCESS != validateInputFilePath (inputFilePath.c_str ()))
        return 1;

    // extract the file from the package
    if (SUCCESS == extractFromPackage (inputFilePath.c_str (), parser.GetOutput (), parser.Overwrite ()))
        {
        EXTRACTOR_I ("The package extracted successfully.");
        return 0;
        }
    else
        {
        EXTRACTOR_I ("Failed to extract the package.");
        return 1;
        }
    }
