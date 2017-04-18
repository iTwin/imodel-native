//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/RealityPlatform/RealityDataServiceConsoleTester.cpp $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//#ifdef REALITYMODFRAMEWORK_LOCALTEST

#include <Bentley/BeTest.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityDataServiceConsole.h>
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
        commandFile.Create(outfile, true);
        commandFile.Open(infile, BeFileAccess::Write);
        uint32_t bytesWritten;
        commandFile.Write(&bytesWritten, commands.c_str(), (uint32_t)(commands.length() * sizeof(char)));
        commandFile.Close();

        RealityDataConsole console = RealityDataConsole();
        console.Run(infile, outfile);

        BeFile outputFile;
        outputFile.Open(outfile, BeFileAccess::Read);
        const uint32_t bufferSize = 32768;
        uint32_t bytesRead;
        char buffer[bufferSize];

        do {
            outputFile.Read(buffer, &bytesRead, bufferSize);
            output.append(buffer);
        } while (bytesRead == bufferSize);

        outputFile.Close();
        }
    };


//-------------------------------------------------------------------------------------
// @bsimethod                          Spencer.Mason                            04/2017
//-------------------------------------------------------------------------------------
TEST_F(RealityDataServiceConsoleTestFixture, ConnectivityTest)
    {
    WString directory(GetDirectory());
    directory.append(L"ConnectionTest");
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

    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
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

    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
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

    BeFileName::EmptyAndRemoveDirectory(directory.c_str());
    }
