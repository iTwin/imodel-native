/*--------------------------------------------------------------------------------------+
|
|  $Source: DgnV8/Tests/DataValidationTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ConverterTestsBaseFixture.h"
#include "BeXmlCompareHelper.h"
#include <cctype>
#include <wctype.h>

/*================================================================================**//**
* @bsiclass                                                     Majd Uddin      04/16
+===============+===============+===============+===============+===============+======*/
class XmlReporter
{
private:
    BentleyApi::BeXmlDomPtr     m_xmlDomPtr;
    DgnDbPtr        m_dgnProject;

public:
    XmlReporter(DgnDbPtr project);
    BentleyApi::BeXmlStatus Save(BentleyApi::WStringCR filename);
    BentleyApi::BeXmlNodeP  GetRootNodeP();
    BentleyApi::BeXmlDomPtr GetXmlDome() { return this->m_xmlDomPtr; };

    void ReportModels();
    void ReportLineStyles();
    void ReportMaterials();
    void ReportCategories();
    void ReportViews();
    void ReportECSchemas();
    void ReportElements(BentleyApi::BeXmlNodeP parentNode, DgnElementMap elements);
    void ReportElements(BentleyApi::BeXmlNodeP parentNode, BentleyApi::Dgn::DgnModelId modelId);

    // Single method to report all of the above
    void DoReport();
};

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
XmlReporter::XmlReporter(DgnDbPtr project)
    : m_dgnProject(project)
{
    m_xmlDomPtr = BentleyApi::BeXmlDom::CreateEmpty();
    m_xmlDomPtr->AddNewElement("DgnDbDetails", nullptr, this->GetRootNodeP());
    BentleyApi::BeXmlNodeP fileNode = this->GetRootNodeP()->AddEmptyElement("DgnDbFile");


    fileNode->AddAttributeStringValue("Name", project->GetFileName().GetFileNameAndExtension().c_str());
    fileNode->AddAttributeStringValue("SchemaVersion", project->GetProfileVersion().ToString().c_str());
}

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyApi::BeXmlStatus XmlReporter::Save(BentleyApi::WStringCR filename)
{
    BentleyApi::BeXmlDom::ToStringOption options = static_cast<BentleyApi::BeXmlDom::ToStringOption>(
        static_cast<uint64_t>(BentleyApi::BeXmlDom::ToStringOption::TO_STRING_OPTION_Formatted) | static_cast<uint64_t>(BentleyApi::BeXmlDom::ToStringOption::TO_STRING_OPTION_Indent)
        );
    return m_xmlDomPtr->ToFile(filename, options, BentleyApi::BeXmlDom::FileEncodingOption::FILE_ENCODING_Utf8);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyApi::BeXmlNodeP XmlReporter::GetRootNodeP()
{
    return m_xmlDomPtr->GetRootElement();
}
//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  06/16
//+---------------+---------------+---------------+---------------+---------------+------
int isNotAlphaNum(BentleyApi::WChar c)
{
    return !iswalnum(c);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::DoReport()
{
    ReportModels();
    ReportLineStyles();
    ReportMaterials();
    ReportCategories();
    ReportViews();
    ReportECSchemas();
}
//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportECSchemas()
{
    BentleyApi::BeXmlNodeP schemasNode = this->GetRootNodeP()->AddEmptyElement("ECSchemas");
    int schemaCount = 0;

    BentleyApi::BeSQLite::EC::ECSqlStatement schemaStatement;
    ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, schemaStatement.Prepare(*m_dgnProject.get(), "SELECT Name, VersionMajor, VersionWrite, VersionMinor FROM meta.ECSchemaDef"));
    while (schemaStatement.Step() != BE_SQLITE_DONE)
    {
        BentleyApi::WString schemaFullName = BentleyApi::WString(schemaStatement.GetValueText(0));
        BentleyApi::ECN::ECSchemaCP schemaCP = m_dgnProject->Schemas().GetSchema(schemaStatement.GetValueText(0));
        schemaFullName.append(L"_");
        schemaFullName.AppendA(schemaStatement.GetValueText(1));
        schemaFullName.append(L"_");
        schemaFullName.AppendA(schemaStatement.GetValueText(2));
        schemaFullName.append(L"_");
        schemaFullName.AppendA(schemaStatement.GetValueText(3));
        
        BentleyApi::BeXmlNodeP schemaNode = schemasNode->AddEmptyElement(schemaFullName.c_str());
        schemaNode->AddAttributeStringValue("Type", "ECSchema");
        schemaNode->AddAttributeBooleanValue("Standard", schemaCP->IsStandardSchema());

        schemaCount++;

        BentleyApi::BeSQLite::EC::ECSqlStatement classStatement;
        BentleyApi::Utf8String ecsql = "SELECT c.Name FROM meta.ECSchemaDef s JOIN meta.ECClassDef c USING meta.SchemaOwnsClasses WHERE s.Name = '";
        ecsql.append(schemaStatement.GetValueText(0));
        ecsql.append("'");
        ASSERT_EQ(BentleyApi::BeSQLite::EC::ECSqlStatus::Success, classStatement.Prepare(*m_dgnProject.get(), ecsql.c_str())) << ecsql.c_str();
        int classCount = 0;
        while (classStatement.Step() != BE_SQLITE_DONE)
        {
            BentleyApi::BeXmlNodeP classNode = schemaNode->AddEmptyElement(BentleyApi::WString(classStatement.GetValueText(0)).c_str());
            classNode->AddAttributeStringValue("Type", "ECClass");
            classCount++;
        }
        schemaNode->AddAttributeInt32Value("ClassesCount", classCount);
    }
    schemasNode->AddAttributeInt32Value("Count", schemaCount);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportModels()
{
    BentleyApi::BeXmlNodeP modelsNode = this->GetRootNodeP()->AddEmptyElement("Models");
    int modelCount = 0;

    for (BentleyApi::Dgn::ModelIteratorEntryCR modelEntry : m_dgnProject->Models().MakeIterator(BIS_SCHEMA(BIS_CLASS_Model)))
    {
        BentleyApi::Dgn::DgnModelPtr model = m_dgnProject->Models().GetModel(modelEntry.GetModelId());
        if (model.IsValid())
        {
            BentleyApi::WString modelNodeName = BentleyApi::WString(m_dgnProject->Schemas().GetClass(model->GetClassId())->GetName().c_str());
            modelNodeName.AppendA("_");
            BentleyApi::WString modelCode = BentleyApi::WString(model->GetName().c_str());
            std::replace_if(modelCode.begin(), modelCode.end(), isNotAlphaNum, '_');
            modelNodeName.append(modelCode);
            BentleyApi::BeXmlNodeP modelNode = modelsNode->AddEmptyElement(modelNodeName.c_str());
            ReportElements(modelNode, model->GetModelId());
            modelCount++;
        }
    }
    modelsNode->AddAttributeInt32Value("Count", modelCount);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportCategories()
    {
    BentleyApi::BeXmlNodeP catsNode = this->GetRootNodeP()->AddEmptyElement("Categories");
    int catCount = 0;

    for (ElementIteratorEntry categoryEntry : SpatialCategory::MakeIterator(*m_dgnProject))
        {
        DgnCategoryId catId = categoryEntry.GetId<DgnCategoryId>();
        BentleyApi::WString catNodeName = L"Category_";
        SpatialCategoryCPtr category = SpatialCategory::Get(*m_dgnProject, catId);
        BentleyApi::WString catName = BentleyApi::WString(category->GetCategoryName().c_str());
        std::replace_if(catName.begin(), catName.end(), isNotAlphaNum, '_');
        catNodeName.append(catName);
        BentleyApi::BeXmlNodeP catNode = catsNode->AddEmptyElement(catNodeName.c_str());
        BentleyApi::WString catCode = BentleyApi::WString(category->GetCode().GetValueUtf8CP());
        std::replace_if(catCode.begin(), catCode.end(), isNotAlphaNum, '_');
        catNode->AddAttributeStringValue("Code", catCode.c_str());
        catCount++;
        }
    catsNode->AddAttributeInt32Value("Count", catCount);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportLineStyles()
{
    BentleyApi::BeXmlNodeP lstylesNode = this->GetRootNodeP()->AddEmptyElement("LineStyles");
    int lsCount = 0;

    //Get line styles
    LsCacheP cache = &m_dgnProject->LineStyles().GetCache();
    LsCacheStyleIterator iterLineStyles = cache->begin();
    LsCacheStyleIterator iterLineStyles_end = cache->end();

    while (iterLineStyles != iterLineStyles_end)
    {
        LsCacheStyleEntry const& entry = *iterLineStyles;
        BentleyApi::WString lsNodeName = L"LineStyle_";
        BentleyApi::WString lsName = BentleyApi::WString(entry.GetStyleName());
        std::replace_if(lsName.begin(), lsName.end(), isNotAlphaNum, '_');
        lsNodeName.append(lsName);
        BentleyApi::BeXmlNodeP lstyleNode = lstylesNode->AddEmptyElement(lsNodeName.c_str());
        lstyleNode->AddAttributeStringValue("StyleName", lsName.c_str());
        ++iterLineStyles;
        lsCount++;
    }
    lstylesNode->AddAttributeInt32Value("Count", lsCount);
}
//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportMaterials()
{
    BentleyApi::BeXmlNodeP materialsNode = this->GetRootNodeP()->AddEmptyElement("Materials");
    int matCount = 0;

    for (RenderMaterial::Entry entry : RenderMaterial::MakeIterator(*m_dgnProject))
    {
        BentleyApi::WString matNodeName = L"Material_";
        BentleyApi::WString matName = BentleyApi::WString(entry.GetName());
        std::replace_if(matName.begin(), matName.end(), isNotAlphaNum, '_');
        matNodeName.append(matName);
        BentleyApi::BeXmlNodeP matNode = materialsNode->AddEmptyElement(matNodeName.c_str());
        BentleyApi::WString matDesc = BentleyApi::WString(entry.GetDescription());
        std::replace_if(matDesc.begin(), matDesc.end(), isNotAlphaNum, '_');
        matNode->AddAttributeStringValue("Description", matDesc.c_str());
        matCount++;
    }
    materialsNode->AddAttributeInt32Value("Count", matCount);
}

//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportViews()
{
    BentleyApi::BeXmlNodeP viewsNode = this->GetRootNodeP()->AddEmptyElement("Views");
    int viewCount = 0;
    for (ViewDefinition::Entry entry : ViewDefinition::MakeIterator(*m_dgnProject))
    {
        BentleyApi::WString viewNodeName = L"View_";
        BentleyApi::WString viewName = BentleyApi::WString(entry.GetName());
        std::replace_if(viewName.begin(), viewName.end(), isNotAlphaNum, '_');
        viewNodeName.append(viewName);
        BentleyApi::BeXmlNodeP viewNode = viewsNode->AddEmptyElement(viewNodeName.c_str());
        BentleyApi::WString viewDesc = BentleyApi::WString(entry.GetDescription());
        std::replace_if(viewDesc.begin(), viewDesc.end(), isNotAlphaNum, '_');
        viewNode->AddAttributeStringValue("Description", viewDesc.c_str());
        viewCount++;
    }
    viewsNode->AddAttributeInt32Value("Count", viewCount);
}
//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportElements(BentleyApi::BeXmlNodeP parentNode, DgnElementMap elements)
{
    BentleyApi::BeXmlNodeP elemsNode = parentNode->AddEmptyElement("Elements");
    int elmCount = 0;
    for (auto element : elements)
    {
        BentleyApi::Dgn::DgnElementId elementId = element.first;
        BentleyApi::Dgn::DgnElementCPtr elementCPtr = m_dgnProject->Elements().GetElement(elementId);
        BentleyApi::WString elementNodeName = L"Element";
        elementNodeName.AppendA("_");
        BentleyApi::WString elemCode = BentleyApi::WString(elementCPtr->GetCode().GetValueUtf8().c_str());
        std::replace_if(elemCode.begin(), elemCode.end(), isNotAlphaNum, '_');
        elementNodeName.append(elemCode);
        BentleyApi::BeXmlNodeP elemNode = elemsNode->AddEmptyElement(elementNodeName.c_str());
        elemNode->AddAttributeStringValue("ECClass", BentleyApi::WString(elementCPtr->GetElementClass()->GetName().c_str()).c_str());
        elmCount++;
    }
    parentNode->AddAttributeInt32Value("ElementsCount", elmCount);
}
//---------------------------------------------------------------------------------------
// @bsimethod                                      Majd.Uddin                  04/16
//+---------------+---------------+---------------+---------------+---------------+------
void XmlReporter::ReportElements(BentleyApi::BeXmlNodeP parentNode, BentleyApi::Dgn::DgnModelId modelId)
{
    Statement clsStatement;
    BentleyApi::Utf8String sql = "SELECT DISTINCT ECClassId FROM dgn_element WHERE ModelId=";
    sql.append(modelId.ToString());
    sql.append(" ORDER BY ECClassId");
    ASSERT_EQ(BE_SQLITE_OK, clsStatement.Prepare(*m_dgnProject.get(), sql.c_str())) << sql.c_str();
    int elemClassCount = 0;
    while (clsStatement.Step() != BE_SQLITE_DONE)
    {
        BentleyApi::ECN::ECClassId classId = clsStatement.GetValueId<BentleyApi::ECN::ECClassId>(0);
        BentleyApi::BeXmlNodeP classNode = parentNode->AddEmptyElement(m_dgnProject->Schemas().GetClass(classId)->GetName().c_str());

        elemClassCount++;

        Statement elemStatement;
        BentleyApi::Utf8String sql2 = "SELECT Id, CodeValue FROM dgn_element WHERE ModelId=" + modelId.ToString() + " AND ECClassId=" + classId.ToString() + " ORDER by CodeValue";
        ASSERT_EQ(BE_SQLITE_OK, elemStatement.Prepare(*m_dgnProject.get(), sql2.c_str())) << sql2.c_str();
        int elemCount = 0;
        while (elemStatement.Step() != BE_SQLITE_DONE)
        {
            BentleyApi::Dgn::DgnElementCPtr elementCPtr = m_dgnProject->Elements().GetElement(elemStatement.GetValueId<BentleyApi::Dgn::DgnElementId>(0));
            BentleyApi::WString elementNodeName = L"Element";
            elementNodeName.AppendA("_");
            BentleyApi::WString elemCode = BentleyApi::WString(elementCPtr->GetCode().GetValueUtf8().c_str());
            std::replace_if(elemCode.begin(), elemCode.end(), isNotAlphaNum, '_');
            elementNodeName.append(elemCode);
            BentleyApi::BeXmlNodeP elemNode = classNode->AddEmptyElement(elementNodeName.c_str());
            elemNode->AddAttributeStringValue("ECClass", BentleyApi::WString(elementCPtr->GetElementClass()->GetName().c_str()).c_str());
            elemCount++;
        }
        classNode->AddAttributeInt32Value("ElementCount", elemCount);
    }
    parentNode->AddAttributeInt32Value("ElementClassCount", elemClassCount);
}

// End of Reporter class

enum class DataValidationStatus
{
    SUCCESS,
    ERROR_DgnDbOpen,
    ERROR_DumpXml,
    ERROR_BenchmarkMissing,
    ERROR_XmlMismatch,
    ERROR_FileOpen_Benchmark,
    ERROR_FileOpen_Current
};


/*================================================================================**//**
* @bsiclass                                                     Majd Uddin      04/15
+===============+===============+===============+===============+===============+======*/
class DataValidationTests : public ConverterTestBaseFixture
{
public:
    BentleyApi::BeFileName   m_InputFiles;
    BentleyApi::BeFileName   m_CurrentXML;
    BentleyApi::BeFileName   m_BenchmarkXML;
    BentleyApi::BeFileName   m_Result;

    virtual void SetUp();

    Utf8CP                      GetStatusString(DataValidationStatus stat);
    DataValidationStatus        DumpXmlAndCompare(BentleyApi::BeFileName DgnDbName);
    DataValidationStatus        DumpXmlDgn(BentleyApi::BeFileName DgnName);
    void                 LogErrorToFile(BentleyApi::WString errorMsg, BentleyApi::WString DgnDbName);
};

static BentleyApi::WString nodeToSkip;
/*=================================================================================**//**
* Callback to get failure info.
* @bsimethod                                                        Majd Uddin 06/16
+===============+===============+===============+===============+===============+======*/
BentleyApi::StatusInt FailureHandler(BentleyApi::WString value1, BentleyApi::WString value2, 
    BentleyApi::WString currentLocation, BentleyApi::WString currentKey)
{
    if (currentLocation.Contains(L"ECSchemas")) //The diff is in ECSchemas
    {
        if (currentKey.Equals(L"ClassesCount")) //Class Count is not matching
        {
            //Add Schema node to be skipped
            BentleyApi::WString sub = currentLocation.substr(currentLocation.find(L"ECSchema"), currentLocation.size());
            BentleyApi::WString sub2 = sub.substr(sub.find(L"/") + 1, sub.size());
            BentleyApi::WString sub3 = sub2.substr(0, sub2.find(L"["));
            nodeToSkip = sub3;

            //Record it in Common Errors
            BentleyApi::WString errorMsg;
            errorMsg = errorMsg + L"ECSchema: " + sub3.c_str() + L" has difference in class count. Expected: " + value1.c_str() + L" Actual: " + value2.c_str();
            BentleyApi::BeFileName commonErrorsLog;
            commonErrorsLog.AppendA(getenv("DGNDB_TEST_ROOT"));
            commonErrorsLog = commonErrorsLog.AppendToPath(L"Results\\CommonErrors.Log");
            FILE *f;
            f = fopen(commonErrorsLog.GetNameUtf8().c_str(), "a");
            if (f != NULL)
                fprintf(f, BentleyApi::AString(errorMsg.c_str()).c_str());
            else
                EXPECT_TRUE(false) << "Error creating Common Error log file";
        }
    }
    return ERROR;
}
/*=================================================================================**//**
* @bsimethod                                                        Majd Uddin 05/16
+===============+===============+===============+===============+===============+======*/
void DataValidationTests::SetUp()
{
    ConverterTestBaseFixture::SetUp();
    BentleyApi::BeFileName root;
    //Set environment variable of DgnDbRegRoot with InputFiles, CurrentXML, BenchmarkXML, Results subfolders
    root.AppendA(getenv("DGNDB_TEST_ROOT"));
    m_InputFiles = m_CurrentXML = m_BenchmarkXML = m_Result = root;
    m_InputFiles = m_InputFiles.AppendToPath(L"InputFiles");
    m_BenchmarkXML = m_BenchmarkXML.AppendToPath(L"BenchmarkXML");
    m_CurrentXML = m_CurrentXML.AppendToPath(L"CurrentXML");
    m_Result = m_Result.AppendToPath(L"Results");
}

/*=================================================================================**//**
* @bsimethod                                                        Majd Uddin 05/16
+===============+===============+===============+===============+===============+======*/
DataValidationStatus DataValidationTests::DumpXmlDgn(BentleyApi::BeFileName DgnName)
{
    V8FileEditor v8editor;
    v8editor.Open(DgnName);
    bvector<DgnV8Api::SchemaInfo> infos;

    DgnECManagerR schemaManager = DgnV8Api::DgnECManager::GetManager();
    schemaManager.DiscoverSchemas(infos, *v8editor.m_file); // , DgnV8Api::ECSchemaPersistence::ECSCHEMAPERSISTENCE_All);
    //DgnV8Api::DgnECManager::GetManager().DiscoverSchemasForModel(infos, *v8editor.m_defaultModel);
    EXPECT_EQ(0, infos.size());
    for (bvector<DgnV8Api::SchemaInfo>::iterator it = infos.begin(); it != infos.end(); ++it) 
    {
        EXPECT_TRUE(false) << it->GetSchemaName();
    }
    return DataValidationStatus::SUCCESS;
}

/*=================================================================================**//**
* @bsimethod                                                        Majd Uddin 05/16
+===============+===============+===============+===============+===============+======*/
DataValidationStatus DataValidationTests::DumpXmlAndCompare(BentleyApi::BeFileName DgnDbName)
{
    DgnDbPtr project = OpenExistingDgnDb(DgnDbName, BentleyApi::BeSQLite::Db::OpenMode::Readonly);
    if (project.IsNull())
        return DataValidationStatus::ERROR_DgnDbOpen;

    XmlReporter report = XmlReporter(project);
    report.DoReport();

    //Save File with same name and .xml extension in CurrentXML folder
    BentleyApi::BeFileName xmlFileName = m_CurrentXML;
    xmlFileName.AppendToPath(DgnDbName.GetFileNameWithoutExtension().c_str());
    xmlFileName.append(L".xml");
    if (report.Save(xmlFileName.c_str()) != BentleyApi::BeXmlStatus::BEXML_Success)
        return DataValidationStatus::ERROR_DumpXml;

    //Compare it with existing XML
    BentleyApi::BeFileName benchXmlFile = m_BenchmarkXML;
    benchXmlFile.AppendToPath(DgnDbName.GetFileNameWithoutExtension().c_str());
    benchXmlFile.append(L".xml");
    if (! BentleyApi::BeFileName::DoesPathExist(benchXmlFile.c_str()))
        return DataValidationStatus::ERROR_BenchmarkMissing;

    BeXmlCompare compare;
    XmlCompareOptions compareOptions;

    //First attempt to open both XML files
    BentleyApi::WString errorMsg = L"";
    BentleyApi::BeXmlDomPtr dom1, dom2;
    BentleyApi::BeXmlStatus beStatus1, beStatus2;
    dom1 = BentleyApi::BeXmlDom::CreateAndReadFromFile(beStatus1, benchXmlFile.c_str(), &errorMsg);
    if (beStatus1 != BEXML_Success)
    {
        DataValidationTests::LogErrorToFile(errorMsg, DgnDbName.GetFileNameWithoutExtension());
        return DataValidationStatus::ERROR_FileOpen_Benchmark;
    }
    dom2 = BentleyApi::BeXmlDom::CreateAndReadFromFile(beStatus2, xmlFileName.c_str(), &errorMsg);
    if (beStatus2 != BEXML_Success)
    {
        DataValidationTests::LogErrorToFile(errorMsg, DgnDbName.GetFileNameWithoutExtension());
        return DataValidationStatus::ERROR_FileOpen_Current;
    }

    //Failure Handler for common errors
    compare.RegisterComparisonFailedEvent(FailureHandler);
    compareOptions.m_matchChildren = true;
    compare.SetComparisonOption(&compareOptions);
    if (!BentleyApi::WString::IsNullOrEmpty(nodeToSkip.c_str()))
    {
        compareOptions.AddSkipNodeName(nodeToSkip);
        compare.setSkipNode(true);
    }

    //Do comparison
    BentleyApi::StatusInt status = compare.CompareXml(dom1, dom2);
    if (status == BentleyApi::ERROR || compare.m_isComparisonFailed)
    {
        DataValidationTests::LogErrorToFile(compare.GetError(), DgnDbName.GetFileNameWithoutExtension());
        return DataValidationStatus::ERROR_XmlMismatch;
    }
    else
        return DataValidationStatus::SUCCESS;
}

//=======================================================================================
// @bsimethod                                                    Majd Uddin   05/16
//=======================================================================================
Utf8CP DataValidationTests::GetStatusString(DataValidationStatus stat)
{
    switch (stat)
    {
    case DataValidationStatus::SUCCESS:
        return "SUCCESS";
        break;
    case DataValidationStatus::ERROR_DgnDbOpen:
        return "ERROR_DgnDbOpen";
        break;
    case DataValidationStatus::ERROR_DumpXml:
        return "ERROR_DumpXml";
        break;
    case DataValidationStatus::ERROR_BenchmarkMissing:
        return "ERROR_BenchmarkMissing";
        break;
    case DataValidationStatus::ERROR_XmlMismatch:
        return "ERROR_XmlMismatch";
        break;
    case DataValidationStatus::ERROR_FileOpen_Benchmark:
        return "ERROR_FileOpen_Benchmark";
        break;
    case DataValidationStatus::ERROR_FileOpen_Current:
        return "ERROR_FileOpen_Benchmark";
        break;
    default:
        return "Bad Enum Value";
        break;
    }
}

//=======================================================================================
// @bsimethod                                                    Majd Uddin   05/16
//=======================================================================================
void DataValidationTests::LogErrorToFile(BentleyApi::WString errorMsg, BentleyApi::WString DgnDbName)
{
    BentleyApi::BeFileName logFilePath = m_Result;
    logFilePath.AppendToPath(DgnDbName.c_str());
    logFilePath.append(L".log");

    FILE *f;
    f = fopen(logFilePath.GetNameUtf8().c_str(), "w");
    if (f != NULL)
    {
        fprintf(f, BentleyApi::AString(errorMsg.c_str()).c_str());
    }
    else
        ASSERT_TRUE(false) << "Error creating log file";
}

//=======================================================================================
// Dumps data to XML and compares with benchmark XML
// @bsimethod                                                    Majd Uddin   05/16
//=======================================================================================
TEST_F(DataValidationTests, CompareXML)
{
    //Setup results.csv
    BentleyApi::BeFileName resultsFilePath = m_Result;
    resultsFilePath.AppendToPath(L"DataValidationResults.csv");
    FILE *f;
    f = fopen(resultsFilePath.GetNameUtf8().c_str(), "w");
    if (f != NULL)
    {
        fprintf(f, "FileName, DataValidationStatus\n");
    }
    else
        ASSERT_TRUE(false) << "Error opening csv file. Perhaps the DGNDB_TEST_ROOT variable is not set";

    //Iterate all files
    BentleyApi::BeFileName wildcard(NULL, m_InputFiles, L"*", NULL);
    BentleyApi::BeFileListIterator it(wildcard, /*recursive*/true);
    BentleyApi::BeFileName name;
    int counter = 0;
    while (it.GetNextFileName(name) == BentleyApi::SUCCESS)
    {
        if (name.GetExtension().Equals(L"idgndb") || name.GetExtension().Equals(L"dgndb")) // Only parse DgnDb files
        {
            wprintf(L"\n*** Comparing File:  %ls \n\n", name.c_str());
            counter++;
            wprintf(L"\n*** File count: %d \n\n", counter);
            DataValidationStatus status = DumpXmlAndCompare(name);
            if (status != DataValidationStatus::SUCCESS)
                EXPECT_TRUE(false)<< GetStatusString(status);
            fprintf(f, "%ls, %s\n", name.GetFileNameAndExtension().c_str(), GetStatusString(status));
        }
    }
}

//=======================================================================================
// Dumps data to XML and compares with benchmark XML
// @bsimethod                                                    Majd Uddin   05/16
//=======================================================================================
TEST_F(DataValidationTests, DumpXMLFromIDgn)
{
    //Iterate all files
    BentleyApi::BeFileName root;
    //Set environment variable of DgnDbRegRoot with InputFiles, CurrentXML, BenchmarkXML, Results subfolders
    root.AppendA(getenv("DGNDB_TEST_ROOT"));
    BentleyApi::BeFileName   dgnFiles = root;
    dgnFiles = dgnFiles.AppendToPath(L"DgnFiles");;
    BentleyApi::BeFileName wildcard(NULL, dgnFiles, L"*", NULL);
    BentleyApi::BeFileListIterator it(wildcard, /*recursive*/true);
    BentleyApi::BeFileName name;
    while (it.GetNextFileName(name) == BentleyApi::SUCCESS)
    {
        if (name.GetExtension().Equals(L"dgn") || name.GetExtension().Equals(L"i.dgn")) // Only parse Dgn files
        {
            wprintf(L"\n*** Comparing File:  %ls *** \n\n", name.c_str());
            DataValidationStatus status = DumpXmlDgn(name);
            if (status != DataValidationStatus::SUCCESS)
                EXPECT_TRUE(false) << GetStatusString(status);
        }
    }
}


