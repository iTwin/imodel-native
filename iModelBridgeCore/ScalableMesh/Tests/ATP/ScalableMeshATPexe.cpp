#include <ScalableMeshATPPch.h>
#include "ScalableMeshATPexe.h"
#include "Initialize.h"
#include "Common/ATPUtils.h"
#include "Common/ATPFileFinder.h"


USING_NAMESPACE_BENTLEY_SCALABLEMESH
USING_NAMESPACE_BENTLEY_TERRAINMODEL    


namespace ScalableMeshATPexe
    {   


    BentleyStatus ScalableMeshATPexe::Initialize(int argc, WCharP argv[])
        {
        //BeFileName::FixPathName(m_outputName, m_outputName.c_str());
       // m_outputName.BeGetFullPathName();

        //BeFileName outputDir = BeFileName(BeFileName::GetDirectoryName(m_outputName.c_str()).c_str());

        /*if (m_pipe == NULL)
            {
            OpenPipe();
            }*/
        DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries
        InitializeATP(*this);
        setlocale(LC_CTYPE, "");
        return SUCCESS;
        }


    int ScalableMeshATPexe::PrintUsage(WCharCP programName)
        {
        WString exeName = BeFileName::GetFileNameAndExtension(programName);

        fwprintf(stderr,
                 L"\n\
                 Run ATP for ScalableMesh.\n\
                 \n Usage: \n\
                %ls runatp -i|--input= -c|--clean\n\
                --input=                (required)  path to XML file for ATP or directory with xml files for ATP. \n\
                --clean                 (optional)  Delete stm file (if -i is a directory delete all stm files). \n\
                ", programName, programName);

        return 1;
        }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
    WString ScalableMeshATPexe::GetArgValueW(WCharCP arg)
        {
        WString argValue(arg);
        argValue = argValue.substr(argValue.find_first_of('=', 0) + 1);
        argValue.Trim(L"\"");
        argValue.Trim();
        return argValue;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Sam.Wilson                      07/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    Utf8String ScalableMeshATPexe::GetArgValue(WCharCP arg)
        {
        return Utf8String(GetArgValueW(arg));
        }

    int ScalableMeshATPexe::ParseCommandLine(int argc, WCharP argv[])
        {
        if (argc < 2)
            return PrintUsage(argv[0]);
        bool isTestPlanMode = false;
        m_optionClean = false;
        if (0 == wcscmp(argv[1], L"runatp"))
            {
            isTestPlanMode = true;
            }
        if (!isTestPlanMode)
            {
            fwprintf(stderr, L"Unrecognized command: %ls\nTry 'runatp'\n.", argv[1]);
            return PrintUsage(argv[0]);
            }
        for (int iArg = 2; iArg < argc; ++iArg)
            {
            if (argv[iArg] == wcsstr(argv[iArg], L"--help") || argv[iArg] == wcsstr(argv[iArg], L"-h"))
                return PrintUsage(argv[0]);
            if (argv[iArg] == wcsstr(argv[iArg], L"--clean") || argv[iArg] == wcsstr(argv[iArg], L"-c"))
                {
                m_optionClean = true;
                continue;
                }
            if (argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i="))
                {
                BeFileName::FixPathName(m_inputFileName, GetArgValueW(argv[iArg]).c_str());
                if (!BeFileName::DoesPathExist(m_inputFileName.c_str()))
                    {
                    fwprintf(stderr, L"%ls is not an existing path\n", m_inputFileName.c_str());
                    return PrintUsage(argv[0]);
                    }
                /*if (!BeFileName::IsDirectory(m_inputFileName.c_str()))
                    return PrintUsage(argv[0]);*/

                continue;
                }

            /*if (argv[iArg] == wcsstr(argv[iArg], L"--output=") || argv[iArg] == wcsstr(argv[iArg], L"-o="))
                {
                m_outputName.SetName(GetArgValueW(argv[iArg]).c_str());
                continue;
                }*/

            fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
            return PrintUsage(argv[0]);
            }

        return SUCCESS;
        }

    void ScalableMeshATPexe::Start()
        {
        // Run ATP
        if (BeFileName::IsDirectory(m_inputFileName.c_str()))
            {
            ATPFileFinder fileFinder;

            WString filePaths;
            
            fileFinder.FindFiles(m_inputFileName, filePaths, true);

            WString firstPath;

            while (fileFinder.ParseFilePaths(filePaths, firstPath))
                {
                BeFileName name(firstPath.c_str());
                WString extension;
                name.ParseName(NULL, NULL, NULL, &extension);
                if (0 == BeStringUtilities::Wcsicmp(extension.c_str(), L"xml"))
                    {
                    if (m_optionClean)
                        RemoveStmFiles(name);
                    RunTestPlan(name);
                    }

                }
            }
        else
            {
            if (m_optionClean)
                RemoveStmFiles(m_inputFileName);
            RunTestPlan(m_inputFileName);
            }
        }
   }

int wmain(int argc, wchar_t* argv[])
    {
    ScalableMeshATPexe::ScalableMeshATPexe app;
    if (SUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    /*if (app.m_outputName.empty())
        {
        fwprintf(stderr, L"No output directory specified\n");
        return app.PrintUsage(argv[0]);
        }*/
    app.Initialize(argc, argv);
    app.Start();

    ScalableMeshATPexe::CloseATP();

    return 0;
    }