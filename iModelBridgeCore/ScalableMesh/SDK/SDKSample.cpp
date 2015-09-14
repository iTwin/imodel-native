#include "Initialize.h"
#include "SDKSample.h"
#include "SDKSampleImporter.h"

namespace ScalableMeshSDKSample
    {

    void ScalableMeshSDKSample::OpenPipe()
        {
        WString name = L"\\\\.\\pipe\\";
        name.append(BeFileName::GetFileNameWithoutExtension(m_outputName));

        m_pipe = CreateNamedPipe(
            name.c_str(),
            PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
            PIPE_TYPE_BYTE, // send data as a byte stream
            1, 
            0, 
            0, 
            0,
            NULL 
            );
        if (m_pipe == NULL || m_pipe == INVALID_HANDLE_VALUE) fwprintf(stderr, L"Error creating pipe\n");
        if (!ConnectNamedPipe(m_pipe, NULL)) fwprintf(stderr, L"No client connected\n");
        }

    BentleyStatus ScalableMeshSDKSample::Initialize(int argc, WCharP argv[])
        {
        BeFileName::FixPathName(m_outputName, m_outputName.c_str());
        m_outputName.BeGetFullPathName();

        BeFileName outputDir = BeFileName(BeFileName::GetDirectoryName(m_outputName.c_str()).c_str());

        if (m_pipe == NULL)
            {
            OpenPipe();
            }
        DgnViewLib::Initialize(*this, true); // this initializes the DgnDb libraries
        InitializeSDK(*this);
        setlocale(LC_CTYPE, "");
        return SUCCESS;
        }

    BentleyStatus ScalableMeshSDKSample::OpenScalableMesh(WCharCP path)
        {
        std::cout << "OPENING MESH " << path << std::endl;
        IScalableMeshPtr ptr = IScalableMesh::GetFor(path, true, true);
        m_sMesh = ptr;
        return SUCCESS;
        }


    size_t  ScalableMeshSDKSample::CountPoints()
        {
        if (m_sMesh == nullptr) std::cout << "ERROR, NO SCALABLE MESH " << std::endl;
        else
            {
            std::cout << "POINTS " << m_sMesh->GetPointCount() << std::endl;
            return m_sMesh->GetPointCount();
            }
        return 0;
        }


    HANDLE s_pipe;
    bool WritePointsCallback(const DPoint3d* points, size_t nbOfPoints, bool arePoints3d)
        {
        const double endOfPacket = DBL_MAX;
        const unsigned char pointPacketMagicNumber = 0xFF;
        size_t length = nbOfPoints*sizeof(DPoint3d) + sizeof(bool) + sizeof(double) + 1;
        unsigned char* transmission = new unsigned char[length];
        transmission[0] = pointPacketMagicNumber;
        DWORD  numBytesWritten;
        memcpy(transmission + 1, points, nbOfPoints*sizeof(DPoint3d));
        memcpy(transmission + 1 + nbOfPoints*sizeof(DPoint3d), &arePoints3d, sizeof(bool));
        memcpy(transmission + 1 + nbOfPoints*sizeof(DPoint3d) + sizeof(bool), &endOfPacket, sizeof(double));
        WriteFile(
            s_pipe,
            transmission,
            (DWORD)length,
            &numBytesWritten,
            NULL 
            );
        delete[] transmission;
        return true;
        }

    bool WriteFeatureCallback(const DPoint3d* featurePoints, size_t nbOfFeaturesPoints, DTMFeatureType featureType, bool isFeature3d)
        {
        const unsigned char featurePacketMagicNumber = 0xFA;
        const double endOfPacket = DBL_MAX;
        size_t length = nbOfFeaturesPoints*sizeof(DPoint3d) + sizeof(bool)+sizeof(DTMFeatureType) + sizeof(double) + 1;
        unsigned char* transmission = new unsigned char[length];
        DWORD  numBytesWritten;
        transmission[0] = featurePacketMagicNumber;
        memcpy(transmission + 1, &featureType, sizeof(DTMFeatureType));
        memcpy(transmission + 1 + sizeof(DTMFeatureType), featurePoints, nbOfFeaturesPoints*sizeof(DPoint3d));
        memcpy(transmission + 1 + sizeof(DTMFeatureType) + nbOfFeaturesPoints*sizeof(DPoint3d), &isFeature3d, sizeof(bool));
        memcpy(transmission + 1 + sizeof(DTMFeatureType) + nbOfFeaturesPoints*sizeof(DPoint3d) + sizeof(bool), &endOfPacket, sizeof(double));
        WriteFile(
            s_pipe,
            transmission,
            (DWORD)length,
            &numBytesWritten,
            NULL
            );
        delete[] transmission;
        return true;
        }

    BentleyStatus ScalableMeshSDKSample::ParseImportDefinition(BeXmlNodeP pTestNode)
        {
        BeXmlStatus status;


        Bentley::ScalableMesh::IScalableMeshSourceImporterPtr importerPtr(Bentley::ScalableMesh::IScalableMeshSourceImporter::Create());
        s_pipe = m_pipe;
        importerPtr->SetFeatureCallback(WriteFeatureCallback);
        importerPtr->SetPointsCallback(WritePointsCallback);

        if (importerPtr == 0)
            {
            printf("ERROR : cannot create importer\r\n");
            return ERROR;
            }
        else
            {
            WString gcsKeyName;

            status = pTestNode->GetAttributeStringValue(gcsKeyName, "gcsKeyName");

            if (status == BEXML_Success)
                {
                BaseGCSPtr baseGCSPtr(BaseGCS::CreateGCS(gcsKeyName.c_str()));
                StatusInt status = importerPtr->SetBaseGCS(baseGCSPtr);
                assert(status == SUCCESS);
                }

            if (ParseSourceSubNodes(importerPtr->EditSources(), pTestNode) == true)
                {

                return importerPtr->Import() ? SUCCESS : ERROR;
                }
            }

        return SUCCESS;
        }

    int ScalableMeshSDKSample::PrintUsage(WCharCP programName)
        {
        WString exeName = BeFileName::GetFileNameAndExtension(programName);

        fwprintf(stderr,
                 L"\n\
                Imports terrain data from various formats, including DgnV8.\n\
                 \n Usage: \n\
                %ls import -i|--input= -o|--output= \n\
                --input=                (required)  Configuration file listing the input files and options. \n\
                \n\
                --output=               (required)  Named pipe where data is to be transferred.\n\
                ", programName, programName);

        return 1;
        }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
    WString ScalableMeshSDKSample::GetArgValueW(WCharCP arg)
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
    Utf8String ScalableMeshSDKSample::GetArgValue(WCharCP arg)
        {
        return Utf8String(GetArgValueW(arg));
        }

    int ScalableMeshSDKSample::ParseCommandLine(int argc, WCharP argv[])
        {
        if (argc < 3)
            return PrintUsage(argv[0]);
        bool isImportMode = false;
        if (0 == wcscmp(argv[1], L"import"))
            {
            isImportMode = true;
            }
        if (!isImportMode)
            {
            fwprintf(stderr, L"Unrecognized command: %ls\nTry 'import'\n.", argv[1]);
            return PrintUsage(argv[0]);
            }
        for (int iArg = 2; iArg < argc; ++iArg)
            {
            if (argv[iArg] == wcsstr(argv[iArg], L"--input=") || argv[iArg] == wcsstr(argv[iArg], L"-i="))
                {
                BeFileName::FixPathName(m_inputFileName, GetArgValueW(argv[iArg]).c_str());
                if (BeFileName::IsDirectory(m_inputFileName.c_str()))
                    return PrintUsage(argv[0]);

                continue;
                }

            if (argv[iArg] == wcsstr(argv[iArg], L"--output=") || argv[iArg] == wcsstr(argv[iArg], L"-o="))
                {
                m_outputName.SetName(GetArgValueW(argv[iArg]).c_str());
                continue;
                }

            fwprintf(stderr, L"Unrecognized command line option: %ls\n", argv[iArg]);
            return PrintUsage(argv[0]);
            }
        return SUCCESS;
        }

    void ScalableMeshSDKSample::Start()
        {
        Import();
        }

    void ScalableMeshSDKSample::Import()
        {
        BeXmlStatus status;
        WString     errorMsg;

        BeXmlDomPtr pXmlDom = BeXmlDom::CreateAndReadFromFile(status, m_inputFileName, &errorMsg);

        if (pXmlDom == 0)
            {
            wprintf(L"ERROR : Cannot open input file (%s)", errorMsg.c_str());
            return;
            }

        BeXmlNodeP pRootNode(pXmlDom->GetRootElement());
        ParseImportDefinition(pRootNode);
        CloseHandle(m_pipe);
        }

   }


int wmain(int argc, wchar_t* argv[])
    {
    ScalableMeshSDKSample::ScalableMeshSDKSample app;
    if (SUCCESS != app.ParseCommandLine(argc, argv))
        return 1;

    if (app.m_outputName.empty())
        {
        fwprintf(stderr, L"No output directory specified\n");
        return app.PrintUsage(argv[0]);
        }
    app.Initialize(argc, argv);
    app.Start();
    return 0;
    }