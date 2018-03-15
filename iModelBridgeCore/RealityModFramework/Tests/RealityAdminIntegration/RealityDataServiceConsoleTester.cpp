//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityAdminIntegration/RealityDataServiceConsoleTester.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST

#include <Bentley/BeTest.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityAdmin/RealityDataServiceConsole.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFilename.h>
#include <Bentley/Base64Utilities.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            04/2017
//=====================================================================================
class RealityDataServiceConsoleTestFixture : public testing::Test
    {
public:
    WCharCP GetDirectory()
        {
        BeFileName outDir;
        BeTest::GetHost().GetTempDir (outDir);
        outDir.AppendToPath(L"RealityDataServiceConsoleTestDirectory");
        return outDir;
        }

    void InitTestDirectory(WCharCP directoryname)
        {
        if (BeFileName::DoesPathExist(directoryname))
            BeFileName::EmptyAndRemoveDirectory(directoryname);
        BeFileName::CreateNewDirectory(directoryname);
        }

    void Execute(WCharCP directoryName, Utf8String commands, Utf8String& output)
        {
        BeFileName infile = BeFileName(directoryName);
        infile.AppendToPath(L"commandFile.txt");
        BeFileName outfile = BeFileName(directoryName);
        outfile.AppendToPath(L"consoleOutput.txt");
        BeFile commandFile;
        commandFile.Create(infile, true);
        BeFile outputFile;
        outputFile.Create(outfile, true);
        uint32_t bytesWritten;
        commandFile.Write(&bytesWritten, commands.c_str(), (uint32_t)(commands.length() * sizeof(char)));
        commandFile.Close();

        RealityDataConsole console = RealityDataConsole();
        console.Run(infile, outfile);

        // For some reason having a BeFile open, close then reopen the same file causes handle duplication
        // which makes the file impossible to delete, until the program closes. To workaround this, I use 
        // a new BeFile..
        BeFile outputFile2; 
        outputFile2.Open(outfile, BeFileAccess::Read);
        const uint32_t bufferSize = 32768;
        uint32_t bytesRead;
        char buffer[bufferSize];

        do {
            outputFile2.Read(buffer, &bytesRead, bufferSize);
            output.append(buffer);
        } while (bytesRead == bufferSize);

        outputFile2.Close();
        }
    };


//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataServiceConsoleTestFixture, ConnectivityTest)
    {
    WString directory(GetDirectory());
    directory.append(L"ConnectivityTest");
    InitTestDirectory(directory.c_str());

    Utf8String commandString = "\n"
        "n\n"
        "quit\n";
    Utf8String consoleOutput;

    Execute(directory.c_str(), commandString, consoleOutput);

    //if there is more than one repository all subsequent test will need to be updated
    ASSERT_TRUE(consoleOutput.ContainsI("Only one repository found"));
    //if the Repo structure changes, all subsequent tests will fail as well
    ASSERT_TRUE(consoleOutput.ContainsI("8 \t S3MX"));

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataServiceConsoleTestFixture, HelpTest)
    {
    WString directory(GetDirectory());
    directory.append(L"HelpTest");
    InitTestDirectory(directory.c_str());

    Utf8String commandString = "\n"
                        "n\n"
                        "S3MX\n"
                        "y\n"
                        "n\n"
                        "help\n"
                        "quit\n";
    Utf8String consoleOutput;

    Execute(directory.c_str(), commandString, consoleOutput);

    //Ensure all available commands are listed
    ASSERT_TRUE(consoleOutput.ContainsI("Available Commands"));
    ASSERT_TRUE(consoleOutput.ContainsI("Quit"));
    ASSERT_TRUE(consoleOutput.ContainsI("Retry"));
    ASSERT_TRUE(consoleOutput.ContainsI("Help"));
    ASSERT_TRUE(consoleOutput.ContainsI("SetServer"));
    ASSERT_TRUE(consoleOutput.ContainsI("SetProjectId"));
    ASSERT_TRUE(consoleOutput.ContainsI("List"));
    ASSERT_TRUE(consoleOutput.ContainsI("Dir"));
    ASSERT_TRUE(consoleOutput.ContainsI("Filter"));
    ASSERT_TRUE(consoleOutput.ContainsI("cd"));
    ASSERT_TRUE(consoleOutput.ContainsI("ListAll"));
    ASSERT_TRUE(consoleOutput.ContainsI("Details"));
    ASSERT_TRUE(consoleOutput.ContainsI("Stat"));
    ASSERT_TRUE(consoleOutput.ContainsI("Download"));
    ASSERT_TRUE(consoleOutput.ContainsI("Upload"));
    ASSERT_TRUE(consoleOutput.ContainsI("FileAccess"));
    ASSERT_TRUE(consoleOutput.ContainsI("AzureAddress"));
    ASSERT_TRUE(consoleOutput.ContainsI("ChangeProps"));
    ASSERT_TRUE(consoleOutput.ContainsI("Relationships"));
    ASSERT_TRUE(consoleOutput.ContainsI("Link"));
    ASSERT_TRUE(consoleOutput.ContainsI("Unlink"));
    ASSERT_TRUE(consoleOutput.ContainsI("CreateRD"));
    ASSERT_TRUE(consoleOutput.ContainsI("Delete"));

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataServiceConsoleTestFixture, StatTest)
    {   
    WString directory(GetDirectory());
    directory.append(L"StatTest");
    InitTestDirectory(directory.c_str());

    Utf8String commandString = "\n"
        "n\n"
        "S3MX\n"
        "y\n"
        "n\n"
        "stat\n"
        "quit\n";
    Utf8String consoleOutput;

    Execute(directory.c_str(), commandString, consoleOutput);

    ASSERT_TRUE(consoleOutput.ContainsI("NbRealityData"));
    ASSERT_TRUE(consoleOutput.ContainsI("TotalSize"));
    ASSERT_TRUE(consoleOutput.ContainsI("OrganizationId"));
    ASSERT_TRUE(consoleOutput.ContainsI("UltimateId"));
    ASSERT_TRUE(consoleOutput.ContainsI("UltimateSite"));

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataServiceConsoleTestFixture, CompleteTest)
    {
    WString directory(GetDirectory());
    directory.append(L"CompleteTest");
    InitTestDirectory(directory.c_str());

    BeFileName dummyRoot = BeFileName(directory.c_str());
    dummyRoot.AppendToPath(L"DummyFolder");
    BeFileName::CreateNewDirectory(dummyRoot.c_str());

    BeFileName dummyFile = BeFileName(dummyRoot.c_str());
    dummyFile.AppendToPath(L"DummyRootDocument.json");

    FILE* pFile;
    pFile = fopen (dummyFile.GetNameUtf8().c_str(), "w");
    fseek (pFile, 5000000, SEEK_SET); //5Mb
    fwrite ("1", sizeof(char), 1, pFile);
    fclose (pFile);

    dummyFile.PopDir(); // .../DummyFolder/
    dummyFile.AppendToPath(L"DummySubFolder");
    BeFileName::CreateNewDirectory(dummyFile.c_str());
    dummyFile.AppendToPath(L"smallfile1.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile2.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile3.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile4.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    dummyFile.PopDir(); // .../DummySubFolder/
    dummyFile.AppendToPath(L"smallfile5.txt");

    pFile = fopen(dummyFile.GetNameUtf8().c_str(), "w");
    fseek(pFile, 1000000, SEEK_SET); //1Mb
    fwrite("1", sizeof(char), 1, pFile);
    fclose(pFile);

    Utf8String token = RequestConstructor().GetToken();
    token.ReplaceAll("Authorization: Token ", "");
    Utf8String decodedToken = Base64Utilities::Decode(token);
    const char* charstring = decodedToken.c_str();
    Utf8String keyword = "givenname";
    const char* attributePosition = strstr(charstring, keyword.c_str());
    keyword = "<saml:AttributeValue>";
    const char* valuePosition = strstr(attributePosition, keyword.c_str());
    valuePosition += keyword.length();
    Utf8String idString = Utf8String(valuePosition);
    
    bvector<Utf8String> lines;
    BeStringUtilities::Split(idString.c_str(), "< ", lines);
    Utf8String id = lines[0];
    
    int64_t currentTime;
    DateTime::GetCurrentTimeUtc().ToUnixMilliseconds(currentTime);
    
    id.append(Utf8PrintfString("%ldbuildTest(DELETE THIS)", currentTime));

    Utf8String commandString = "dev\n"
                                "n\n"
                                "S3MX\n"
                                "y\n"
                                "y\n"
                                "72524420-7d48-4f4e-8b0f-144e5fa0aa22\n"
                                "upload\n";
    commandString.append(dummyRoot.GetNameUtf8().c_str());
    commandString.append("\n");
    commandString.append(id);
    commandString.append("\n");
    Utf8String commandString2 = "1\n"
                                "3\n"
                                "1\n"
                                "DummyRootDocument.json\n"
                                "filter\n"
                                "1\n";

    commandString.append(commandString2);
    commandString.append(id.c_str());
    commandString.append("\n");

    dummyRoot.PopDir();
    dummyRoot.AppendToPath(L"DownloadFolder");
    BeFileName::CreateNewDirectory(dummyRoot.c_str());

    commandString2 = "7\n"
                    "dir\n"
                    "cd 1\n"
                    "details\n"
                    "listall\n"
                    "\n"
                    "fileAccess w\n"
                    "azureaddress r\n"
                    "Relationships\n"
                    "Unlink\n"
                    "72524420-7d48-4f4e-8b0f-144e5fa0aa22\n"
                    "changeprops\n"
                    "Description\n"
                    "y\n"
                    "CHANGE PROP TEST\n"
                    "-Finish-\n"
                    "y\n"
                    "download\n";

    commandString.append(commandString2);
    commandString.append(dummyRoot.GetNameUtf8().c_str());
    commandString.append("\n");
    commandString2 = "delete\n"
                    "y\n"
                    "quit\n";
    
    commandString.append(commandString2);

    Utf8String consoleOutput;

    Execute(directory.c_str(), commandString, consoleOutput);

    ASSERT_TRUE(consoleOutput.ContainsI("New RealityData created with GUID")) << "createRD failed";
    ASSERT_TRUE(consoleOutput.ContainsI(Utf8PrintfString("Name : %s", id.c_str()))) << "filter failed";
    ASSERT_TRUE(consoleOutput.ContainsI(Utf8PrintfString("1 \t %s", id.c_str()))) << "list did not return created RD";
    ASSERT_TRUE(consoleOutput.ContainsI("Visibility         : ENTERPRISE")) << "details did not return expected results";
    ASSERT_TRUE(consoleOutput.ContainsI("DummyRootDocument.json 5000001 bytes")) << "upload was unsuccessful";
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile1.txt 1000001 bytes")) << "upload was unsuccessful";
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile2.txt 1000001 bytes")) << "upload was unsuccessful";
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile3.txt 1000001 bytes")) << "upload was unsuccessful";
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile4.txt 1000001 bytes")) << "upload was unsuccessful";
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile5.txt 1000001 bytes")) << "upload was unsuccessful";
    ASSERT_TRUE(consoleOutput.ContainsI("Description        : CHANGE PROP TEST")) << "properties weren't properly changed";
    ASSERT_TRUE(consoleOutput.ContainsI("FileAccess.FileAccessKey?$filter=Permissions+eq+'Write'")) << "FileAccess didn't return url";
    ASSERT_TRUE(consoleOutput.ContainsI("?sv=")) << "FileAccess didn't return url";
    ASSERT_TRUE(consoleOutput.ContainsI("RelatedId          : 72524420-7d48-4f4e-8b0f-144e5fa0aa22")) << "didn't list relationship";
    ASSERT_TRUE(consoleOutput.ContainsI("There seems to be no relation attached to this RealityData")) << "didn't unlink RD from project";
    Utf8String outRoot = dummyRoot.GetNameUtf8();
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummyRootDocument.json", outRoot)))) << "a document failed to download";
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile1.txt", outRoot)))) << "a document failed to download";
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile2.txt", outRoot)))) << "a document failed to download";
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile3.txt", outRoot)))) << "a document failed to download";
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile4.txt", outRoot)))) << "a document failed to download";
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile5.txt", outRoot)))) << "a document failed to download";

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);
    }
