/*--------------------------------------------------------------------------------------+
|
|     $Source: tools/SchemaResourceGenerator/SchemaResourceGenerator.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECObjects/ECObjectsAPI.h>
#include <BeXml/BeXml.h>
#include <Logging/bentleylogging.h>
#if defined (BENTLEY_WIN32)
// WIP_BEFILENAME_PORTABILITY - need better way of setting temp directory
#include <windows.h>
#endif

USING_NAMESPACE_BENTLEY_EC

BentleyApi::NativeLogging::ILogger* s_logger = BentleyApi::NativeLogging::LoggingManager::GetLogger("SchemaResourceGenerator");

#pragma region Commandline Parameters
struct Options
    {
    bvector<BeFileName> InputFiles;
    Utf8String CultureName;
    BeFileName OutDirectory;
    bool HasOutDirectorySet = false;
    bvector<BeFileName> ReferencePaths;
    Utf8String OutputFileName;
    int Precendence = 9900;
    bool NoSupplementation = false;
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
static void ShowUsage()
    {
    const char *usage =
#include "usage.txt"
        ;

    printf(usage);
    }

enum class InputParserState
    {
    InputFiles,
    Precedence,
    CultureName,
    ReferencePaths,
    OutDirectory,
    OutFile,
    NoSupplementation,
    Help,
    ExpectingSwitch
    };

static bmap<Utf8String, InputParserState> CreateSwitchMap()
    {
    bmap<Utf8String, InputParserState> m;
    m["/?"] = InputParserState::Help;
    m["-p"] = InputParserState::Precedence;
    m["-c"] = InputParserState::CultureName;
    m["-r"] = InputParserState::ReferencePaths;
    m["-o"] = InputParserState::OutDirectory;
    m["-of"] = InputParserState::OutFile;
    m["-ns"] = InputParserState::NoSupplementation;
    return m;
    };

static bmap<Utf8String, InputParserState> const ConsoleSwitches = CreateSwitchMap();

static bool TryParseInput(int argc, char** argv, Options& options)
    {
    if (argc < 2)
        {
        ShowUsage();
        return false;
        }

    InputParserState currentState = InputParserState::InputFiles;
    bset<InputParserState> usedSwitches;
    for (int i = 1; i < argc; i++)
        {
        Utf8CP arg = argv[i];
        InputParserState currentSwitch = InputParserState::InputFiles;
        auto const csw = ConsoleSwitches.find(arg);
        bool isSwitch = csw != ConsoleSwitches.end();

        if (isSwitch)
            {
            currentSwitch = csw->second;
            if (usedSwitches.find(currentSwitch) != usedSwitches.end())
                {
                s_logger->errorv("Switch %s is specified multiple times.", arg);
                return false;
                }

            usedSwitches.insert(currentSwitch);
            }

        switch (currentState)
            {
                case InputParserState::InputFiles: //still reading input files
                    if (!isSwitch)
                        {
                        s_logger->infov("Input File #%d: %s", i, arg);
                        BeFileName file;
                        file.AssignUtf8(arg);
                        if(!file.DoesPathExist())
                            { 
                            s_logger->errorv("Input file %s does not exist.", arg);
                            return false;
                            }

                        if (file.IsDirectory())
                            {
                            s_logger->errorv("Input file %s points to a directory, expected a file.", arg);
                            return false;
                            }

                        options.InputFiles.push_back(file);
                        continue;
                        }

                    if (options.InputFiles.size() <= 0)
                        {
                        s_logger->error("No input files specified.");
                        return false;
                        }
                    break;

                case InputParserState::CultureName:
                    options.CultureName = arg;
                    s_logger->infov("Culture: %s", arg);
                    currentState = InputParserState::ExpectingSwitch;
                    continue;

                case InputParserState::OutDirectory:
                    options.OutDirectory = BeFileName(arg, true);
                    s_logger->infov("Output Directory: %s", arg);
                    options.HasOutDirectorySet = true;
                    currentState = InputParserState::ExpectingSwitch;
                    continue;

                case InputParserState::ReferencePaths:
                    if (!isSwitch)
                        {
                        s_logger->infov("Reference Path #%d: %s", options.ReferencePaths.size(), arg);
                        BeFileName file;
                        file.AssignUtf8(arg);

                        if (!file.DoesPathExist())
                            {
                            s_logger->errorv("Reference path %s does not exist, ignoring it.", arg);
                            continue;
                            }

                        if (!file.IsDirectory())
                            {
                            s_logger->errorv("Reference path %s is not a directory, ignoring it.", arg);
                            continue;
                            }

                        options.ReferencePaths.push_back(file);
                        continue;
                        }

                    if (options.ReferencePaths.size() <= 0)
                        {
                        s_logger->error("No reference paths specified.");
                        return false;
                        }
                    break;

                case InputParserState::OutFile:
                    options.OutputFileName = arg;
                    s_logger->infov("Output File: %s", arg);
                    currentState = InputParserState::ExpectingSwitch;
                    continue;

                case InputParserState::Precedence:
                    Utf8P end = nullptr;
                    uint32_t p = strtoul(arg, &end, 10);
                    if (end == arg)
                        {
                        s_logger->errorv("Invalid Precedence. '%s' is not a number.", arg);
                        return false;
                        }

                    if (p < 9900 || p > 9999)
                        {
                        s_logger->errorv("Invalid Precedence: %d. Value must be between 9900 and 9999.", p);
                        return false;
                        }

                    options.Precendence = p;
                    s_logger->infov("Precedence: %d", p);
                    currentState = InputParserState::ExpectingSwitch;
                    continue;
            }

        if (!isSwitch)
            {
            s_logger->errorv("Unrecognized parameter: %s", arg);
            return false;
            }

        if (currentSwitch == InputParserState::NoSupplementation)
            {
            options.NoSupplementation = true;
            s_logger->info("No supplementation.");
            currentState = InputParserState::ExpectingSwitch;
            continue;
            }

        if (currentSwitch == InputParserState::Help)
            {
            ShowUsage();
            return false;
            }

        currentState = currentSwitch;
        }

    if (currentState == InputParserState::InputFiles || currentState == InputParserState::ExpectingSwitch)
        return true;

    if (currentState == InputParserState::ReferencePaths && options.ReferencePaths.size() > 0)
        {
        return true;
        }

    s_logger->error("Invalid call. The last console switch requires a following parameter.");
    return false;
    }

static bool PathPointsToSchema(BeFileName const& file)
    {
    return file.EndsWithI(L".ecschema.xml");
    }

static bool PathPointsToXliff(BeFileName const& file)
    {
    return file.EndsWithI(L".xliff");
    }
#pragma endregion

#pragma region Xliff Output
static void WriteResource(BeXmlWriter& writer, Utf8CP key, Utf8CP value)
    {
    if (Utf8String::IsNullOrEmpty(key) || Utf8String::IsNullOrEmpty(value))
        return;

    writer.WriteElementStart("trans-unit");
    writer.WriteAttribute("id", key);
    writer.WriteAttribute("resname", key);

    writer.WriteElementStart("source");
    writer.WriteAttribute("xml:space", "preserve");
    writer.WriteText(value);
    writer.WriteElementEnd();//source

    writer.WriteElementEnd();//trans-unit
    }

static bool WriteXliff(Options const& options)
    {
    BeFileName firstInputFile = options.InputFiles[0];
    BeFileName outFile = options.HasOutDirectorySet ? options.OutDirectory : firstInputFile.GetDirectoryName();
    Utf8String culture = options.CultureName;
    if (culture.empty())
        culture = "en";

    Utf8String outputFileName;
    if (!options.OutputFileName.empty())
        outputFileName = options.OutputFileName;
    else
        {
        Utf8String s(firstInputFile.GetFileNameAndExtension());
        outputFileName = s.substr(0, s.size() - 13);
        }

    outputFileName.append(".");
    outputFileName.append(culture);
    outputFileName.append(".xliff");

    outFile.AppendUtf8(outputFileName.c_str());
    s_logger->infov("Writing output file %s", outFile.GetNameUtf8().c_str());
    BeXmlWriterPtr xmlWriter = BeXmlWriter::CreateFileWriter(outFile.GetName());

    xmlWriter->SetIndentation(4);
    xmlWriter->WriteDocumentStart(XML_CHAR_ENCODING_UTF8);
    xmlWriter->WriteElementStart("xliff");
    //Using attributes to write namespaces seems somewhat hacky, but it's the only way to make BeXml produce valid xliff that looks exactly like our other xliff files.
    xmlWriter->WriteAttribute("xmlns", "urn:oasis:names:tc:xliff:document:1.2");
    xmlWriter->WriteAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    xmlWriter->WriteAttribute("xsi:schemaLocation", "urn:oasis:names:tc:xliff:document:1.2 xliff-core-1.2-strict.xsd");

    auto context = ECSchemaReadContext::CreateContext(true);
    for (auto const& refPath : options.ReferencePaths)
        {
        context->AddSchemaPath(refPath.GetName());
        }

    for (auto& inputFile : options.InputFiles)
        {
        Utf8String fileName(inputFile.GetFileNameAndExtension());
        s_logger->infov("Processing %s...", fileName.c_str());
        
        ECSchemaPtr schema;
        auto status = ECSchema::ReadFromXmlFile(schema, inputFile.GetName(), *context);
        if (status != SchemaReadStatus::Success)
            {
            s_logger->infov("Failed to load schema, return code was %d.", status);
            continue;
            }

        xmlWriter->WriteElementStart("file");
        xmlWriter->WriteAttribute("datatype", "plaintext");

        xmlWriter->WriteAttribute("original", fileName.c_str());
        xmlWriter->WriteAttribute("source-language", "en");
        xmlWriter->WriteElementStart("body");

        xmlWriter->WriteElementStart("group");
        xmlWriter->WriteAttribute("resname", "Schema");
        auto key = SchemaResourceKeyHelper::GetSchemaDisplayLabelKey(*schema);
        WriteResource(*xmlWriter, key.c_str(), schema->GetInvariantDisplayLabel().c_str());

        key = SchemaResourceKeyHelper::GetSchemaDescriptionKey(*schema);
        WriteResource(*xmlWriter, key.c_str(), schema->GetInvariantDescription().c_str());
        xmlWriter->WriteElementEnd();//group

        if (schema->GetClassCount() > 0)
            {
            xmlWriter->WriteElementStart("group");
            xmlWriter->WriteAttribute("resname", "Classes");

            for (ECClassCP c : schema->GetClasses())
                {
                key = SchemaResourceKeyHelper::GetTypeDisplayLabelKey(*c);
                WriteResource(*xmlWriter, key.c_str(), c->GetInvariantDisplayLabel().c_str());

                key = SchemaResourceKeyHelper::GetTypeDescriptionKey(*c);
                WriteResource(*xmlWriter, key.c_str(), c->GetInvariantDescription().c_str());

                for (ECPropertyCP p : c->GetProperties())
                    {
                    key = SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(*p);
                    WriteResource(*xmlWriter, key.c_str(), p->GetInvariantDisplayLabel().c_str());

                    key = SchemaResourceKeyHelper::GetTypeChildDescriptionKey(*p);
                    WriteResource(*xmlWriter, key.c_str(), p->GetInvariantDescription().c_str());
                    }

                if (c->IsRelationshipClass())
                    {
                    auto relC = c->GetRelationshipClassCP();
                    Utf8CP invariant = relC->GetSource().GetInvariantRoleLabel().c_str();
                    key = SchemaResourceKeyHelper::GetRelationshipSourceRoleLabelKey(*relC, invariant);
                    WriteResource(*xmlWriter, key.c_str(), invariant);

                    invariant = relC->GetTarget().GetInvariantRoleLabel().c_str();
                    key = SchemaResourceKeyHelper::GetRelationshipTargetRoleLabelKey(*relC, invariant);
                    WriteResource(*xmlWriter, key.c_str(), invariant);
                    }
                }
            }

            if (schema->GetEnumerationCount() > 0)
                {
                xmlWriter->WriteElementStart("group");
                xmlWriter->WriteAttribute("resname", "Enumerations");

                for (ECEnumerationCP e : schema->GetEnumerations())
                    {
                    key = SchemaResourceKeyHelper::GetTypeDisplayLabelKey(*e);
                    WriteResource(*xmlWriter, key.c_str(), e->GetInvariantDisplayLabel().c_str());

                    key = SchemaResourceKeyHelper::GetTypeDescriptionKey(*e);
                    WriteResource(*xmlWriter, key.c_str(), e->GetInvariantDescription().c_str());

                    for (ECEnumeratorCP ep : e->GetEnumerators())
                        {
                        key = SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(*ep);
                        WriteResource(*xmlWriter, key.c_str(), ep->GetInvariantDisplayLabel().c_str());
                        }
                    }
            
            xmlWriter->WriteElementEnd();//group
            }

        xmlWriter->WriteElementEnd();//body
        xmlWriter->WriteElementEnd();//file
        s_logger->infov("Finished processing %s.", fileName.c_str());
        }

    xmlWriter->WriteElementEnd();//xliff
    return true;
    }
#pragma endregion

#pragma region Sqlang Output

static bool WriteSqlang(Options const& options)
    {
    s_logger->error("Writing to sqlang is not yet supported by this application.");
    return false;
    }
#pragma endregion

#pragma region Main
//---------------------------------------------------------------------------------------
// @bsimethod                                                   BentleySystems
//---------------------------------------------------------------------------------------
int main(int argc, char** argv)
    {
#if defined(BENTLEY_WIN32)
    WChar exePathW[MAX_PATH];
    if (0 == ::GetModuleFileNameW(nullptr, exePathW, MAX_PATH))
        {
        fprintf(stderr, "Could not load logging config file");
        return -1;
        }

    BeFileName exePath(exePathW);
    BeFileName exeDirectory(exePath.GetDirectoryName());
    BeFileName logFilePath(exeDirectory);
    logFilePath.AppendToPath(L"SchemaResourceGenerator.logging.config.xml");
    logFilePath.BeGetFullPathName();
    BentleyApi::NativeLogging::LoggingConfig::SetOption(CONFIG_OPTION_CONFIG_FILE, logFilePath);
    BentleyApi::NativeLogging::LoggingConfig::ActivateProvider(NativeLogging::LOG4CXX_LOGGING_PROVIDER);
    ECSchemaReadContext::Initialize(exeDirectory);
#endif

    Options options;
    if (!TryParseInput(argc, argv, options))
        {
        return -1;
        }
    
    auto const& firstFile = options.InputFiles[0]; //TryParse made sure there is at least one input file
    bool inputPointsToSchemas = PathPointsToSchema(firstFile);
    if(inputPointsToSchemas)
        {
        if (!std::all_of(options.InputFiles.begin(), options.InputFiles.end(), [] (BeFileName const& f) { return PathPointsToSchema(f); }))
            {
            s_logger->error("Invalid call. All input files are expected to be ECSchemas, if the first is one.");
            return -2;
            }
        }
    else
        {
        if (!std::all_of(options.InputFiles.begin(), options.InputFiles.end(), [] (BeFileName const& f) { return PathPointsToXliff(f); }))
            {
            s_logger->error("Invalid call. All input files are expected to be Xliff, if the first is one.");
            return -2;
            }
        }

    if (inputPointsToSchemas)
        return WriteXliff(options) ? 0 : -3;
        
    return WriteSqlang(options) ? 0 : -4;
    }
#pragma endregion
