//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityAdmin/RealityDataServiceConsoleTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST

#include <Bentley/BeTest.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityAdmin/RealityDataServiceConsole.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFilename.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//=====================================================================================
//! @bsiclass                          Spencer.Mason                            04/2017
//=====================================================================================
class RealityDataServiceConsoleTestFixture : public testing::Test
    {
public:
    WCharCP GetDirectory()
        {
        WChar exePath[MAX_PATH];
        GetModuleFileNameW(NULL, exePath, MAX_PATH);

        WString exeDir = exePath;
        size_t pos = exeDir.find_last_of(L"/\\");
        exeDir = exeDir.substr(0, pos + 1);

        BeFileName testPath(exeDir);
        testPath.AppendToPath(L"RealityDataServiceConsoleTestDirectory");
        return testPath;
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

    Utf8String commandString = "dev-realitydataservices-eus.cloudapp.net\n"
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

    Utf8String commandString = "dev-realitydataservices-eus.cloudapp.net\n"
                        "n\n"
                        "8\n"
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

    Utf8String commandString = "dev-realitydataservices-eus.cloudapp.net\n"
        "n\n"
        "8\n"
        "stat\n"
        "quit\n";
    Utf8String consoleOutput;

    Execute(directory.c_str(), commandString, consoleOutput);

    ASSERT_TRUE(consoleOutput.ContainsI("NbRealityData"));
    ASSERT_TRUE(consoleOutput.ContainsI("TotalSize"));

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

    Utf8String id = "945F9288 - 45C7 - 44ea - A9D4 - B05D015D4780";

    Utf8String commandString = "dev-realitydataservices-eus.cloudapp.net\n"
                                "n\n"
                                "8\n"
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
                    "Link\n"
                    "1\n"
                    "Unlink\n"
                    "1\n"
                    "changeprops\n"
                    "5\n"
                    "CHANGE PROP TEST\n"
                    "20\n"
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

    ASSERT_TRUE(consoleOutput.ContainsI("New RealityData created with GUID")); //upload
    ASSERT_TRUE(consoleOutput.ContainsI(Utf8PrintfString("Name : %s", id.c_str()))); //filter
    ASSERT_TRUE(consoleOutput.ContainsI("1 \t 945F9288 - 45C7 - 44ea - A9D4 - B05D015D4780")); //list
    ASSERT_TRUE(consoleOutput.ContainsI("Visibility         : PUBLIC")); //details
    ASSERT_TRUE(consoleOutput.ContainsI("DummyRootDocument.json 5000001 bytes"));
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile1.txt 1000001 bytes")); // Upload/ListAll
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile2.txt 1000001 bytes"));
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile3.txt 1000001 bytes"));
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile4.txt 1000001 bytes"));
    ASSERT_TRUE(consoleOutput.ContainsI("DummySubFolder/smallfile5.txt 1000001 bytes"));
    ASSERT_TRUE(consoleOutput.ContainsI("Description        : CHANGE PROP TEST")); //changeprops
    ASSERT_TRUE(consoleOutput.ContainsI("FileAccess.FileAccessKey?$filter=Permissions+eq+'Write'")); //FileAccess
    ASSERT_TRUE(consoleOutput.ContainsI("?sv=")); //AzureAddress
    ASSERT_TRUE(consoleOutput.ContainsI("ProjectId          : 1")); //link
    ASSERT_TRUE(consoleOutput.ContainsI("There seems to be no projects attached to this RealityData")); //Unlink
    Utf8String outRoot = dummyRoot.GetNameUtf8();
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummyRootDocument.json", outRoot))));
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile1.txt", outRoot))));//Download
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile2.txt", outRoot))));
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile3.txt", outRoot))));
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile4.txt", outRoot))));
    ASSERT_TRUE(BeFileName::DoesPathExist(BeFileName(Utf8PrintfString("%s/DummySubFolder/smallfile5.txt", outRoot))));

    ASSERT_TRUE(BeFileName::EmptyAndRemoveDirectory(directory.c_str()) == BeFileNameStatus::Success);
    }
