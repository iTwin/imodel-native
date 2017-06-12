/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityAdmin/RealityDataServiceConsole.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__


#include <ctime>

#include <Bentley/DateTime.h>
#include <RealityPlatform/RealityPlatformAPI.h>
#include <BeJsonCpp/BeJsonUtilities.h>
#include <RealityPlatform/RealityDataService.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct NodeList
    {
    NodeList() : parentNode(nullptr), childNode(nullptr)
    {}

    NodeList *parentNode;
    NavNode node;
    NodeList *childNode;
    };

enum class Command
    {
    Dummy,
    Quit,
    Retry,
    Error,
    Cancel,
    Help,
    ChoiceIndex,
    ChoiceValue,
    SetServer,
    SetProjectId,
    List,
    ListAll,
    Details,
    ChangeDir,
    Stat,
    Download,
    Upload,
    FileAccess,
    AzureAddress,
    ChangeProps,
    Delete,
    Filter,
    Relationships,
    CreateRD,
    Link,
    Unlink
    };

struct RealityDataConsole
    {
public:
    enum class DisplayOption
        {
        Info,
        Question,
        Tip,
        Error
        };

    REALITYDATAPLATFORM_EXPORT RealityDataConsole();

    REALITYDATAPLATFORM_EXPORT void Run();
    REALITYDATAPLATFORM_EXPORT void Run(BeFileName infile, BeFileName outfile);
    REALITYDATAPLATFORM_EXPORT void Run(Utf8String server, Utf8String projectId);
private:
    void _Run();
    void ConfigureServer();
    void SetProjectId();
    void PrintResults(bvector<Utf8String> results);
    void PrintResults(bmap<Utf8String, bvector<Utf8String>> results);
    void Usage();
    void Choice(bvector<Utf8String> options, Utf8StringR input);
    void InterpretCommand();
    void List();
    void ListAll();
    void ListRoots();
    void ChangeDir();
    void EnterpriseStat();
    void Download();
    void Upload();
    void Details();
    void DummyFunction(){}
    void InputError();
    void FileAccess();
    void AzureAddress();
    void ChangeProps();
    void Delete();
    void MassDelete();
    void Filter();
    void Relationships();
    void CreateRD();
    void Link();
    void Unlink();
    void MassUnlink();
    Utf8String MakeBuddiCall(int region = 0);

    void DisplayInfo(Utf8StringCR msg, DisplayOption option= DisplayOption::Info);

private:    
    typedef void (RealityDataConsole::*FUNCTION)();
    bmap<Command, FUNCTION> m_functionMap;

    Utf8String           m_lastInput;
    Command              m_lastCommand;
    WSGServer            m_server;
    bvector<NavNode>     m_serverNodes;
    bvector<Utf8String>  m_machineRepos;
    NodeList*            m_currentNode;

    bvector<Utf8String>  m_realityDataProperties;
    bvector<Utf8String>  m_visibilityOptions;
    bvector<Utf8String>  m_classificationOptions;
    bvector<Utf8String>  m_typeOptions;
    bvector<Utf8String>  m_filterProperties;
    Utf8String           m_nameFilter;
    Utf8String           m_groupFilter;
    Utf8String           m_typeFilter;
    Utf8String           m_ownerFilter;
    Utf8String           m_queryFilter;
    Utf8String           m_projectFilter;

    HANDLE        m_hConsole;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE