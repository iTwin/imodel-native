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
    BeFileName InputFile;
    Utf8String CultureName;
    BeFileName OutDirectory;
    bvector<BeFileName> ReferencePaths;
    int Precendence = 9900;
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
    InputFile,
    Precedence,
    CultureName,
    ReferencePaths,
    OutDirectory,
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

    InputParserState currentState = InputParserState::InputFile;
    bset<InputParserState> usedSwitches;
    for (int i = 1; i < argc; i++)
        {
        Utf8CP arg = argv[i];
        InputParserState currentSwitch = InputParserState::InputFile;
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
                case InputParserState::InputFile:
                    if (!isSwitch)
                        {
                        s_logger->infov("Input File: %s", arg);
                        options.InputFile.AssignUtf8(arg);
                        if (!options.InputFile.DoesPathExist())
                            {
                            s_logger->errorv("Input file %s does not exist.", arg);
                            return false;
                            }

                        if (options.InputFile.IsDirectory())
                            {
                            s_logger->errorv("Input file %s points to a directory, expected a file.", arg);
                            return false;
                            }

                        currentState = InputParserState::ExpectingSwitch;
                        continue;
                        }

                    if (options.InputFile.IsEmpty())
                        {
                        s_logger->error("No input file specified.");
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

        if (currentSwitch == InputParserState::Help)
            {
            ShowUsage();
            return false;
            }

        currentState = currentSwitch;
        }

    if (currentState == InputParserState::InputFile || currentState == InputParserState::ExpectingSwitch)
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
static uint32_t _id = 1;
static void WriteResource(BeXmlWriter& writer, Utf8CP key, Utf8CP value)
    {
    if (Utf8String::IsNullOrEmpty(key) || Utf8String::IsNullOrEmpty(value))
        return;

    writer.WriteElementStart("trans-unit");
    writer.WriteAttribute("id", _id);
    _id++;
    writer.WriteAttribute("resname", key);

    writer.WriteElementStart("source");
    writer.WriteAttribute("xml:space", "preserve");
    writer.WriteText(value);
    writer.WriteElementEnd();//source

    writer.WriteElementEnd();//trans-unit
    }

static void WriteCustomAttributeValues(BeXmlWriter& writer, ECValuesCollectionR values, Utf8CP parentKey)
    {
    for (auto& propValue : values)
        {
        ECValueCR value = propValue.GetValue();
        if (value.IsNull())
            continue;

        if (propValue.HasChildValues())
            {
            if (value.IsStruct())
                {
                WriteCustomAttributeValues(writer, *propValue.GetChildValues(), parentKey);
                }
            else
                {
                WriteCustomAttributeValues(writer, *propValue.GetChildValues(), parentKey);
                }

            continue;
            }
        
        if (!value.IsUtf8())
            continue;

        Utf8CP stringValue = value.GetUtf8CP();
        if (Utf8String::IsNullOrEmpty(stringValue))
            continue;

        auto property = propValue.GetValueAccessor().GetECProperty();
        if (!property->IsDefined("CoreCustomAttributes", "Localizable") && !property->IsDefined("EditorCustomAttributes", "Localizable"))
            continue;

        auto completeKey = Utf8PrintfString("%s:%s:%s", parentKey, 
                                            propValue.GetValueAccessor().GetManagedAccessString().c_str(), SchemaResourceKeyHelper::ComputeHash(stringValue));
        WriteResource(writer, completeKey.c_str(), stringValue);
        }
    }

static void WriteCustomAttributeContainerResources(BeXmlWriter& writer, Utf8CP parentKey, IECCustomAttributeContainerCR container)
    {
    for (auto const& caInstance : container.GetPrimaryCustomAttributes(false))
        {
        ECClassCR ecClass = caInstance->GetClass();
        auto caKey = Utf8PrintfString("%s@%s:%s", parentKey, ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
        ECValuesCollectionPtr values = ECValuesCollection::Create(*caInstance);
        WriteCustomAttributeValues(writer, *values, caKey.c_str());
        }
    }

static bool WriteXliff(Options const& options)
    {
    BeFileName outFile = options.OutDirectory.IsEmpty() ? options.InputFile.GetDirectoryName() : options.OutDirectory;
    Utf8String culture = options.CultureName;
    if (culture.empty())
        culture = "en";

    Utf8String s(options.InputFile.GetFileNameAndExtension());
    Utf8String outputFileName = s.substr(0, s.size() - 13);

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

    auto& inputFile = options.InputFile;
    Utf8String fileName(inputFile.GetFileNameAndExtension());
    s_logger->infov("Processing %s...", fileName.c_str());

    ECSchemaPtr schema;
    auto status = ECSchema::ReadFromXmlFile(schema, inputFile.GetName(), *context);
    if (status != SchemaReadStatus::Success)
        {
        s_logger->infov("Failed to load schema, return code was %d.", status);
        return false;
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
    WriteCustomAttributeContainerResources(*xmlWriter, schema->GetName().c_str(), *schema);
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

                auto propertyKey = SchemaResourceKeyHelper::GetTypeChildKey(schema->GetName().c_str(), c->GetName().c_str(), p->GetName().c_str());
                WriteCustomAttributeContainerResources(*xmlWriter, propertyKey.c_str(), *p);
                }

            auto classKey = SchemaResourceKeyHelper::GetTypeKey(schema->GetName().c_str(), c->GetName().c_str());
            if (c->IsRelationshipClass())
                {
                auto relC = c->GetRelationshipClassCP();

                ECRelationshipConstraintR source = relC->GetSource();
                Utf8CP invariant = source.GetInvariantRoleLabel().c_str();
                key = SchemaResourceKeyHelper::GetRelationshipSourceRoleLabelKey(*relC, invariant);
                WriteResource(*xmlWriter, key.c_str(), invariant);
                Utf8PrintfString sourceConstraintKey("%s.Source", classKey);
                WriteCustomAttributeContainerResources(*xmlWriter, sourceConstraintKey.c_str(), source);

                ECRelationshipConstraintR target = relC->GetTarget();
                invariant = target.GetInvariantRoleLabel().c_str();
                key = SchemaResourceKeyHelper::GetRelationshipTargetRoleLabelKey(*relC, invariant);
                WriteResource(*xmlWriter, key.c_str(), invariant);
                Utf8PrintfString targetConstraintKey("%s.Target", classKey);
                WriteCustomAttributeContainerResources(*xmlWriter, targetConstraintKey.c_str(), source);
                }

            WriteCustomAttributeContainerResources(*xmlWriter, classKey.c_str(), *c);
            }

        xmlWriter->WriteElementEnd();//group
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

    xmlWriter->WriteElementEnd();//xliff
    return true;
    }
#pragma endregion

#pragma region Supplemental Schema Output

static bool WriteSchema(Options const& options)
    {
    //Compose output file name and schema keys
    BeFileName outFile = options.OutDirectory.IsEmpty() ? options.InputFile.GetDirectoryName() : options.OutDirectory;
    Utf8String nwe(options.InputFile.GetFileNameAndExtension());
    SchemaKey schemaKey;
    if (SchemaKey::ParseSchemaFullName(schemaKey, nwe.c_str()) != ECObjectsStatus::Success)
        {
        s_logger->errorv("Schema Name could not be extracted from file name '%s'. Expected Format: <Name>.<Major>.(<Write>.)<Minor>.<culture>.xliff", outFile.GetNameUtf8().c_str());
        return false;
        }

    Utf8String culture = options.CultureName;
    if (culture.empty())
        {
        bvector<Utf8String> tokens;
        BeStringUtilities::Split(nwe.c_str(), ".", tokens);
        if (tokens.size() < 3)
            {
            s_logger->errorv("Culture Name could not be extracted from file name '%s'. Expected Format: <Name>.<Major>.(<Write>.)<Minor>.<culture>.xliff", outFile.GetNameUtf8().c_str());
            return false;
            }

        culture = tokens[tokens.size() - 2];
        }

    Utf8String cultureForName = culture;
    cultureForName.ReplaceAll("-", "_");
    Utf8PrintfString supplementalSchemaName("%s_Supplemental_Localization_%s", schemaKey.GetName(), cultureForName.c_str());
    Utf8String encodedName = ECNameValidation::EncodeToValidName(supplementalSchemaName);
    SchemaKey supplementalKey(encodedName.c_str(), schemaKey.GetVersionMajor(), schemaKey.GetVersionWrite(), schemaKey.GetVersionMinor());

    //example: House_Supplemental_Localization_en_GB.01.00.ecschema.xml
    Utf8PrintfString outputFileName("%s.ecschema.xml", supplementalKey.GetFullSchemaName().c_str());
    outFile.AppendUtf8(outputFileName.c_str());

    //Load input file
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile(xmlStatus, options.InputFile);
    if ((xmlStatus != BEXML_Success) || !xmlDom.IsValid())
        {
        s_logger->errorv("Unable to read file %s", options.InputFile.GetNameUtf8().c_str());
        return false;
        }

    //Generate output schema
    ECSchemaPtr schema;
    Utf8PrintfString namespacePrefix("loc_%s", cultureForName.c_str());
    if (ECSchema::CreateSchema(schema, supplementalKey.GetName(), namespacePrefix,
                               supplementalKey.GetVersionMajor(), supplementalKey.GetVersionWrite(), supplementalKey.GetVersionMinor()) != ECObjectsStatus::Success || !schema.IsValid())
        {
        s_logger->errorv("Unable to create schema %s", supplementalKey.GetName().c_str());
        return false;
        }

    auto context = ECSchemaReadContext::CreateContext(true);
    for (auto const& refPath : options.ReferencePaths)
        {
        context->AddSchemaPath(refPath.GetName());
        }
    
    SchemaKey bsCAKey("Bentley_Standard_CustomAttributes", 1, 0, 0);
    ECSchemaPtr bsCASchema = ECSchema::LocateSchema(bsCAKey, *context);
    if (!bsCASchema.IsValid())
        {
        s_logger->errorv("Could not load standard schema '%s'.", bsCAKey.GetName().c_str());
        return false;
        }
    schema->AddReferencedSchema(*bsCASchema);

    auto supInfo = SupplementalSchemaMetaData::Create(schemaKey.GetName(), schemaKey.GetVersionMajor(), schemaKey.GetVersionMinor(), options.Precendence, "Localization", false);
    IECInstancePtr instance = supInfo->CreateCustomAttribute();
    schema->SetCustomAttribute(*instance);



    SchemaKey locCAKey("SchemaLocalizationCustomAttributes", 1, 0, 0);
    ECSchemaPtr locCASchema = ECSchema::LocateSchema(locCAKey, *context);
    if (!locCASchema.IsValid())
        {
        s_logger->errorv("Could not load standard schema '%s'.", locCAKey.GetName().c_str());
        return false;
        }
    schema->AddReferencedSchema(*locCASchema);

    //Create custom attribute
    auto locSpecEnabler = locCASchema->GetClassP("LocalizationSpecification")->GetDefaultStandaloneEnabler();
    IECInstancePtr caInstance = locSpecEnabler->CreateInstance();
    ECValue cultureV;
    cultureV.SetUtf8CP(culture.c_str());
    caInstance->SetValue("Locale", cultureV);
    
    auto locDataEnabler = locCASchema->GetClassP("LocalizationData")->GetDefaultStandaloneEnabler();
    uint32_t keyPropIndex;
    if (locDataEnabler->GetPropertyIndex(keyPropIndex, "Key") != ECObjectsStatus::Success)
        return false;

    uint32_t valuePropIndex;
    if (locDataEnabler->GetPropertyIndex(valuePropIndex, "Value") != ECObjectsStatus::Success)
        return false;

    uint32_t resourcePropIndex;
    if (locSpecEnabler->GetPropertyIndex(resourcePropIndex, "Resource") != ECObjectsStatus::Success)
        return false;

    ECValue locDataV;
    uint32_t currentIndex = 0;
    auto addValue = [&](Utf8CP key, Utf8CP value) -> void {
        IECInstancePtr instance = locDataEnabler->CreateInstance();
        ECValue v;
        v.SetUtf8CP(key);
        instance->SetValue(keyPropIndex, v);
        v.SetUtf8CP(value);
        instance->SetValue(valuePropIndex, v);

        caInstance->AddArrayElements(resourcePropIndex, 1);
        caInstance->GetValue(locDataV, resourcePropIndex);
        locDataV.SetStruct(instance.get());
        caInstance->SetValue(resourcePropIndex, locDataV, currentIndex);
        currentIndex++;
        };

    schema->SetCustomAttribute(*caInstance);

    //Add resources from xliff to supplemental

    //Document structure looks somewhat like this:
    /*
    <xliff xmlns="urn:oasis:names:tc:xliff:document:1.2" ...
    <file datatype="plaintext" original="ComprehensiveSchema.01.05.02.ecschema.xml" source-language="en">
        <body>
            <group resname="Schema">
                <trans-unit id="1" resname="ComprehensiveSchema.DisplayLabel:626f8965">
                    <source xml:space="preserve">Comprehensive Schema</source>
                    <target xml:space="preserve">abcde localized</target>
                </trans-unit>
    */

    BeXmlDom::IterableNodeSet transUnitNode;
    BeXmlNodeP xliffNode = xmlDom->GetRootElement();
    if (!xliffNode->IsIName("xliff"))
        {
        s_logger->error("Input file is missing the xliff root element.");
        return false;
        }

    xmlDom->RegisterNamespace("ns", xliffNode->GetNamespace());

    BeXmlDom::IterableNodeSet resNodes;
    
    xmlXPathContextPtr xPathContext = xmlDom->AcquireXPathContext(xliffNode);
    xmlDom->SelectNodes(resNodes, "//ns:trans-unit", xPathContext);
    for (BeXmlNodeP const res : resNodes)
        {
        Utf8String resName;
        if (res->GetAttributeStringValue(resName, "resname") != BeXmlStatus::BEXML_Success)
            continue;

        BeXmlNodeP valueNode = res->SelectSingleNode("ns:target");
        if (valueNode == nullptr)
            {
            valueNode = res->SelectSingleNode("ns:source");
            }

        if (valueNode == nullptr)
            continue; //skip this resource

        Utf8String resValue;
        valueNode->GetContent(resValue);
        if (resValue.empty())
            continue;

        s_logger->infov("%s - %s", resName.c_str(), resValue.c_str());
        addValue(resName.c_str(), resValue.c_str());
        }

    //Write output file
    s_logger->infov("Writing output file %s", outFile.GetNameUtf8().c_str());
    s_logger->infov("Wrote %d resources.", currentIndex);
    if (schema->WriteToXmlFile(outFile.GetName(), 3) != SchemaWriteStatus::Success)
        {
        s_logger->error("Failed to serialize schema.");
        return false;
        }

    return true;
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

    if (PathPointsToSchema(options.InputFile))
        {
        return WriteXliff(options) ? 0 : -3;
        }
    else if (PathPointsToXliff(options.InputFile))
        {
        return WriteSchema(options) ? 0 : -4;
        }
    else
        {
        s_logger->error("Invalid call. Input file needs to be either xliff or .ecschema.xml.");
        return -2;
        }
    }
#pragma endregion
