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

enum class Command
    {
    Quit,
    Retry,
    Error,
    Help,
    ChoiceIndex,
    ChoiceValue,
    SetServer,
    List,
    ListAll,
    ChangeDir,
    ChangeDirIndex,
    AllGood
    };

/*struct ConsoleFunction;

struct ConsoleManager
    {
    void InterpretInput(Utf8String input);

    ConsoleFunction* func;
    };

struct ConsoleFunction
    {

    };*/

struct RealityDataConsole
    {
public:
    RealityDataConsole();

    void Run();
    Command ConfigureServer();
    void PrintResults(bvector<Utf8String> results);
    void Usage();
    Command Choice(bvector<Utf8String> options, Utf8StringR input);
    Command InterpretCommand(Utf8StringR entry, int argc = 1);
    Command List();
    Command ChangeDir(Utf8String newNode);
    Command ChangeDir(uint64_t choice);

private:    
    WSGServer            m_server;
    bvector<NavNode>     m_serverNodes;
    bvector<Utf8String>  m_machineRepos;
    Utf8String           m_currentNode;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE