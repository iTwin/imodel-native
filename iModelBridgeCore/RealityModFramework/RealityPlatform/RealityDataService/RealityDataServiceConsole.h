/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataService/RealityDataServiceConsole.h $
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
    Quit,
    Retry,
    Error,
    Cancel,
    Help,
    ChoiceIndex,
    ChoiceValue,
    SetServer,
    List,
    ListAll,
    Details,
    ChangeDir,
    Stat,
    Download,
    Upload,
    FileAccess,
    AzureAdress,
    ChangeProps,
    Delete,
    Filter
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

    RealityDataConsole();

    void Run();
    void ConfigureServer();
    void PrintResults(bvector<Utf8String> results);
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
    void AzureAdress();
    void ChangeProps();
    void Delete();
    void Filter();

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
    bvector<Utf8String>  m_filterProperties;
    Utf8String           m_nameFilter;
    Utf8String           m_groupFilter;
    Utf8String           m_typeFilter;
    Utf8String           m_ownerFilter;

    HANDLE        m_hConsole;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE