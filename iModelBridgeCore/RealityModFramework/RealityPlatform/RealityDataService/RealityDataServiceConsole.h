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
    ChangeDirIndex,
    Stat,
    Download,
    Upload,
    AllGood
    };

struct RealityDataConsole
    {
public:
    RealityDataConsole();

    void Run();
    void ConfigureServer();
    void PrintResults(bvector<Utf8String> results);
    void Usage();
    void Choice(bvector<Utf8String> options, Utf8StringR input);
    void InterpretCommand();
    void List();
    void ListAll();
    void ChangeDir();
    void EnterpriseStat();
    void Download();
    void Upload();
    void Details();

private:    
    typedef void (RealityDataConsole::*FUNCTION)();
    bmap<Command, FUNCTION> m_functionMap;

    Utf8String           m_lastInput;
    Command              m_lastCommand;
    WSGServer            m_server;
    bvector<NavNode>     m_serverNodes;
    bvector<Utf8String>  m_machineRepos;
    //Utf8String           m_currentNode;
    NodeList*            m_currentNode;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE