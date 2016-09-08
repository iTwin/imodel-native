/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/ValidatePublishing_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (DGNPLATFORM_HAVE_DGN_IMPORTER)

//files for Finding files stuff... need to put under only Windows
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>

#include "DgnHandlersTests.h"
#include <Bentley/BeTimeUtilities.h>
#include <DgnPlatform/ForeignFormat/DgnProjectImporter.h>
#include <DgnPlatform/ColorUtil.h>
#include <BeXml/BeXml.h>

using namespace std;
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

WString currentDateTime()
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    string str(buf);
    WString temp;
    temp.assign(str.begin(), str.end());
    return temp;
}

/*=================================================================================**//**
* @bsimethod                                                        Majd Uddin 10/10
+===============+===============+===============+===============+===============+======*/
std::vector<WString> GetFileNamesInFolder(WStringCR FolderName)
{
    std::vector<WString> params;
    //You need to set the DgnDbRegRoot environment variable in your shared shell, It's the path of your Dgn files.

    WString path;
    path.AppendA(getenv("DgnDbRegRoot"));
    path = path + FolderName + L"\\";

    //iterating through files and adding it to the list
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    char buffer[1024];
    WString dirToSearch = path + L"*.dgn";
    dirToSearch.ConvertToLocaleChars(buffer);
    hFind = FindFirstFile(buffer, &findFileData);
    if (INVALID_HANDLE_VALUE == hFind)
    {
        return params; //return empty vector.
    }
    do
    {
        //populate vector with name of the dgndb file
        WString fullPath(path);
        params.push_back(fullPath.AppendA(findFileData.cFileName));
    } while (0 != FindNextFile(hFind, &findFileData));

    FindClose(hFind);
    return params;
}

/*================================================================================**//**
* @bsiclass                                                     Majd Uddin      09/12
+===============+===============+===============+===============+===============+======*/
class ValidatePublishing : public ::testing::TestWithParam<WString>
{
public:
    WString             DgnDbsRoot;
    WString             XMLRoot;
    WString             DgnFullFileName;
    WString             DgnDbFullFileName;
    WString             XMLFullFileName;
    WString             DgnFileName;
    WString             DgnDbFileName;
    WString             XMLFileName;

    bvector<WString>    sourceSchemas;
    bvector<WString>    targetSchemas;

    StopWatch publishTimer;

    virtual void SetUp() override;

    void                    setupPathsNames();
    void                    saveFileName();
    DgnDbPtr           openDgnDb();
    void                    publishToDgnDb(bool NoAsserts = true);
    void                    writeToXml();
};

//=======================================================================================
// @bsimethod                                                    Majd Uddin   09/12
//=======================================================================================
void ValidatePublishing::SetUp()
{
    ScopedDgnHost           autoDgnHost;
    setupPathsNames();
    saveFileName();
}

//=======================================================================================
// @bsimethod                                                    Majd Uddin   09/12
//=======================================================================================
void ValidatePublishing::setupPathsNames()
{
    DgnFullFileName = GetParam();
    DgnFileName = DgnFullFileName.substr(DgnFullFileName.find_last_of(L"\\"), DgnFullFileName.length());
    DgnDbFileName = XMLFileName = DgnFileName.substr(0, DgnFileName.find_last_of(L"."));
    DgnDbFileName = DgnDbFileName.AppendA(".dgndb");
    XMLFileName = XMLFileName.AppendA(".xml");

    DgnDbsRoot = XMLRoot = DgnFullFileName.substr(0, DgnFullFileName.find_last_of(L"\\"));
    DgnDbsRoot.ReplaceAll(L"Dgns", L"DgnDbs");
    //if DgnDbsRoot folder is not there, create it once
    if (!BeFileName::DoesPathExist(DgnDbsRoot.GetWCharCP()))
        BeFileName::CreateNewDirectory(DgnDbsRoot.GetWCharCP());
    XMLRoot.ReplaceAll(L"Dgns", L"XMLs");

    DgnDbFullFileName = DgnDbsRoot + DgnDbFileName;
    XMLFullFileName = XMLRoot + XMLFileName;
}

/*=================================================================================**//**
* @bsimethod                                                        Majd Uddin 12/12
+===============+===============+===============+===============+===============+======*/
void ValidatePublishing::saveFileName()
{
    //Saving file name to the XML report
    char buffer4[1024];
    DgnFileName.ConvertToLocaleChars(buffer4);
    ::testing::Test::RecordProperty("DgnFile", buffer4);
}

//=======================================================================================
// @bsimethod                                                    Majd Uddin   01/13
//=======================================================================================
DgnDbPtr ValidatePublishing::openDgnDb()
{
    DgnDbPtr      dgnProj;
    DgnDbStatus status;
    dgnProj = DgnDb::OpenDgnDb(&status, BeFileName(DgnDbFullFileName.c_str()), DgnDb::OpenParams(BeSQLite::Db::OPEN_Readonly));
    EXPECT_EQ(DgnDbStatus::Success, status) << status;
    if (dgnProj != NULL)
        return dgnProj;
    else
        return NULL;
}

/*=================================================================================**//**
* @bsimethod                                                        Majd Uddin 10/10
+===============+===============+===============+===============+===============+======*/
void ValidatePublishing::publishToDgnDb(bool NoAsserts)
{
    //Building publisher command
    WString publisherExe;
    publisherExe.AppendA(getenv("OutRoot"));
    publisherExe += L"Winx64\\Product\\SampleIDgnToIDgnDbPublisher\\SampleIDgnToIDgnDbPublisher.exe";
    WString publishCmd(publisherExe);
    publishCmd += L" --input=";
    publishCmd += DgnFullFileName;
    publishCmd += L" --output=";
    publishCmd += DgnDbsRoot;
    if (NoAsserts)
        publishCmd += L" --no-assert-dialogs";
    //The command is ready, let's run it
    //publishCmd += L" --no-compress"; 
    printf("\nStart Publishing for: %ls \n", publishCmd);
    char buffer[2048];
    publishCmd.ConvertToLocaleChars(buffer);
    int i;

    WString date = currentDateTime();
    char buffer4[1024];
    date.ConvertToLocaleChars(buffer4);
    ::testing::Test::RecordProperty("TestDate", buffer4);

    publishTimer.Start();
    i = system(buffer);
    publishTimer.Stop();
    //making sure that publishing was done
    if (i != 0)
        EXPECT_TRUE(false) << "The Publisher could not be executed. The command was: " << buffer;
    else
        printf("\nPublishing done\n");

}

/*=================================================================================**//**
* @bsimethod                                                        Majd Uddin 07/14
+===============+===============+===============+===============+===============+======*/
void ValidatePublishing::writeToXml()
{
    double  publishTime = publishTimer.GetElapsedSeconds();
    ::testing::Test::RecordProperty("PublishTimeInSeconds", int(publishTime));
    BeFile dgnFile;
    if (dgnFile.Open(DgnFullFileName.c_str(), BeFileAccess::Read) == BeFileStatus::Success)
    {
        uint64_t fileSize;
        if (dgnFile.GetSize(fileSize) == BeFileStatus::Success)
            ::testing::Test::RecordProperty("DgnFileSizeInBytes", int(fileSize));
    }
    BeFile dgnDbDFile;
    if (dgnDbDFile.Open(DgnDbFullFileName.c_str(), BeFileAccess::Read) == BeFileStatus::Success)
    {
        uint64_t dbfileSize;
        if (dgnDbDFile.GetSize(dbfileSize) == BeFileStatus::Success)
            ::testing::Test::RecordProperty("DgnDbSizeInBytes", int(dbfileSize));
    }
    DgnDbPtr dgnProject = openDgnDb();
    ASSERT_TRUE(dgnProject != NULL);
    //Rewrite this using ECSQL against the MetaSchema ECSchema
    /*bvector<ECN::ECSchemaCP> schemas;
    auto schemaStaus = dgnProject->Schemas().GetECSchemas(schemas, false);
    ASSERT_EQ(schemaStaus, SUCCESS);
    size_t schemaCount = schemas.size();

    ::testing::Test::RecordProperty("TotalSchemas", int(schemaCount));
    DbECClassKeys classKeys;
    auto classCount = 0;
    for (auto& schemaKey : schemaKeys)
    {
        schemaStaus = dgnProject->Schemas().GetECClassKeys(classKeys, schemaKey);
        size_t noOfClass = classKeys.size();
        classCount += int(noOfClass);
    }
    ::testing::Test::RecordProperty("TotalClasses", int(classCount));
    */
}

//=======================================================================================
// A test to that publishes with Asserts
// @bsimethod                                                    Majd Uddin   12/12
//=======================================================================================
TEST_P(ValidatePublishing, PublishDgnDbWithAsserts)
{
    publishToDgnDb(true);
    writeToXml();
}

INSTANTIATE_TEST_CASE_P(imodelsHigh1, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"High1")));
INSTANTIATE_TEST_CASE_P(imodelsHigh2, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"High2")));

INSTANTIATE_TEST_CASE_P(imodelsMedium1, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium1")));
INSTANTIATE_TEST_CASE_P(imodelsMedium2, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium2")));
INSTANTIATE_TEST_CASE_P(imodelsMedium3, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium3")));
INSTANTIATE_TEST_CASE_P(imodelsMedium4, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium4")));
INSTANTIATE_TEST_CASE_P(imodelsMedium5, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium5")));
INSTANTIATE_TEST_CASE_P(imodelsMedium6, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium6")));
INSTANTIATE_TEST_CASE_P(imodelsMedium7, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium7")));
INSTANTIATE_TEST_CASE_P(imodelsMedium8, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium8")));
INSTANTIATE_TEST_CASE_P(imodelsMedium9, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Medium9")));

INSTANTIATE_TEST_CASE_P(imodelsLow1, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Low1")));
INSTANTIATE_TEST_CASE_P(imodelsLow2, ValidatePublishing, ::testing::ValuesIn(GetFileNamesInFolder(L"Low2")));
#endif // defined (DGNPLATFORM_HAVE_DGN_IMPORTER)
