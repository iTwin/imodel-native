/*--------------------------------------------------------------------------------------+
|
|  $Source: BentleyTest/PublicAPI/Misc/ArgParser.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once 
//#define _UNICODE 1
#include <Bentley\WString.h>
#include <Bentley\bmap.h>
#include <string>
#include <fstream>
/*
#include <DgnPlatform\Tools\papatch.h>
#include <DgnPlatform\Tools\pagalloc.fdf>

#include "DebugCrtLeakDetector.h"
#include "Common\FileIO.h"
#include "StackWalker.h"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
#define DVTBM_EXPORT extern "C" __declspec(dllexport)
*/
USING_NAMESPACE_BENTLEY

/*================================================================================**//**
* @bsiclass                                                     KevinNyman      09/10
+===============+===============+===============+===============+===============+======*/
struct  ArgInfo
{
enum Type { BOOL_TYPE, INT_TYPE, STRING_TYPE };
Type type;
WString defaultValue;
WString section;
WString description;
void* location;
bool passForward;
bool displayInUsage;
ArgInfo () {}
public: ArgInfo (Type t, WStringCR sec, WStringCR val, WStringCR descr, void* loc, bool pass=true, bool displayUsage=true) 
    : type(t)
    , defaultValue (val)
    , location (loc)
    , section (sec)
    , description (descr)
    , passForward (pass)
    , displayInUsage (displayUsage)
    {}

public: void LoadValueFromString (WCharCP str);

public: void ToString (WStringR str);
};

typedef Bentley::bmap<char*, ArgInfo> ArgInfoMap;

/*================================================================================**//**
* @bsiclass                                                     Kevin.Nyman     05/11
+===============+===============+===============+===============+===============+======*/
struct ArgParser 
{
ArgInfoMap m_argMap; 
Bentley::bvector<std::string> m_outArgs;
char    **m_argv;
int     m_argc;

public: ArgParser ();
public: virtual ~ArgParser();
protected: virtual void _InitializeArgumentMap () = 0;
protected: void InitializeArgumentMap ();

public: void PrintUsage ();
public: virtual void _PrintUsage () = 0;
public: virtual void _PrintExamples () {} 

public: bool LoadArg (CharCP arg);
public: bool LoadResponseFile (CharCP arg);
public: bool ParseArguments (int argc, char ** argv);
public: virtual void ProcessParsedArguments () {}
public: BentleyStatus MakeResponseFile (WCharCP filename, bool useDefaults);


public: int * GetCountP () {return &m_argc;}
public: char ** GetArgVP () const   {return m_argv;}
private: void FreeArgs ();
private: void CreateModifiedArgumentList ();

private: char* m_assemblyName;
public: WString responseFileOut;
public: bool m_wasHelpPresent;
}; // ArgParser

// NOTE: File must be ASCII
BentleyStatus argParser_loadLinesFromFile (Bentley::bvector<WString>& lines, WStringCR inputFilename);
