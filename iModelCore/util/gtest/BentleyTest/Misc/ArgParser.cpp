/*--------------------------------------------------------------------------------------+
|
|  $Source: BentleyTest/Misc/ArgParser.cpp $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#define UNICODE 1
#define _UNICODE 1
#include <Windows.h>

#include <BentleyTest\Misc\ArgParser.h>

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus argParser_appendStrToFile (WCharCP line, WCharCP filename)
    {
    FILE* file = _wfopen (filename, L"a, ccs=UNICODE");
    if (NULL == file)
        {
        printf ("ERROR: Failed to open %ls file.\n", filename);
        return ERROR;
        }
    fwprintf (file, L"%ls", line);
    fclose (file);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* This only works on ascii files.
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus argParser_loadLinesFromFile (Bentley::bvector<WString>& lines, WStringCR inputFilename)
    {
    std::ifstream fin (inputFilename.c_str());
    if (!fin.is_open())
        return ERROR;

    std::string buffer;
    while (getline(fin, buffer))
        lines.push_back (WString(buffer.c_str()));

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus argParser_createTextFile (WCharCP filename)
    {
    FILE* file = _wfopen (filename, L"w");
    if (NULL == file)
        {
        wprintf (L"ERROR: Failed to create file %ls.\n", filename);
        return ERROR;
        }
    fclose (file);
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgInfo::LoadValueFromString (WCharCP str)
    {
    if (L'=' == str[0])
        str++;

    if (BOOL_TYPE == type)
        {
        if (0 == wcscmp (L"yes", str) || 0 == wcscmp (L"true", str))
            *(bool*)location = true;
        else if (0 == wcscmp (L"no", str) || 0 == wcscmp (L"false", str))
            *(bool*)location = false;
        else   // if present, assign it to true
            *(bool*)location = true;
        }
    else if (INT_TYPE == type)
        {
        int val = 0;
        if (1 != swscanf (str, L"%d", &val))
            swscanf (defaultValue.c_str(), L"%d", &val);
        *(int*)location = val;
        }
    else if (STRING_TYPE == type)
        {
        if (wcslen (str) > 0)
            *(WString*)location = str;
        else
            *(WString*)location = defaultValue.c_str();
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgInfo::ToString (WStringR str)
    {
    if (BOOL_TYPE == type)
        {
        str = *(bool*)location ? L"yes" : L"no";
        }

    else if (INT_TYPE == type)
        {
        WChar buffer[1024];
        wsprintf(buffer, L"%d", *(int*)location);
        str = buffer;
        }

    else if (STRING_TYPE == type)
        {
        str = *(WString*)location;
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void argParser_parseLineRemoveCommentAndTrim (WStringR output , WCharCP value)
    {
    const int sz = 1024;
    WChar buffer[sz];
    wcscpy (buffer, value);
    if (L';' == buffer[0] || L'#' == buffer[0])
        {
        output = L"";
        return;
        }

    WCharP front = wcstok (buffer, L";#");
    
    while (L'\0' != front && L' ' == *front)
        front ++;  

    if (NULL == front)
        return;

    WCharP back = &front[wcslen(front)-1];
    while (back > front && L' ' == *back)
        back--;
    *(back+1) = L'\0';
    output = front;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArgParser::LoadArg (CharCP arg)
    {
    if ('@' == arg[0])
        {
        if (!LoadResponseFile (arg))
            return false;
        }

    if (0 == strcmpi (arg, "--help")) // help needs to be passed forward, we don't know who will use it downstream.
        {
        PrintUsage ();
        m_wasHelpPresent = true;
        //m_outArgs.push_back (m_assemblyName);
        m_outArgs.push_back (arg);
        return true;
        }

    char tmp[1024];
    char adjustedArg[1024];
    bool wasFound = false;
    ArgInfoMap::iterator iter;
    for (iter = m_argMap.begin(); iter != m_argMap.end(); ++iter)
        {
        size_t  length = strlen (arg);
        if (length >= 1024)
            printf ("stack overflow");
        strcpy (tmp, arg);
        sprintf (adjustedArg, "--%s", iter->first);
        if(strlen (tmp) > strlen(adjustedArg))
            tmp[strlen(adjustedArg)] = NULL;
        if (0 == strcmpi (tmp, adjustedArg))
            {
            iter->second.LoadValueFromString (WString(&arg[strlen(adjustedArg)]).c_str());
            if (0 == strcmpi (tmp, "--makeresponse"))
                {
                MakeResponseFile (responseFileOut.c_str(), true);
                return false;
                }
            wasFound = true;
            if (iter->second.passForward)
                m_outArgs.push_back (arg);
            break;
            }
        }

    m_outArgs.push_back (arg);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgParser::PrintUsage ()
    {
    _PrintUsage(); 
    ArgInfoMap::iterator iter;
    size_t largest = 0;
    for (iter = m_argMap.begin(); iter != m_argMap.end(); ++iter)
        {
        if (!iter->second.displayInUsage)
            continue;

        WString name("--");
        name += WString (iter->first);
        name += L"=";
        name += iter->second.defaultValue.c_str();
        if (largest < name.length())
            largest = name.length();
        }

    WChar fmt[1024];
    wsprintf (fmt, L"   %%%ds    %%s\n", largest);
    for (iter = m_argMap.begin(); iter != m_argMap.end(); ++iter)
        {
        if (!iter->second.displayInUsage)
            continue;

        WString name("--");
        name += WString (iter->first);
        name += L"=";
        name += iter->second.defaultValue.c_str();
        wprintf  (fmt, name.c_str(), iter->second.description.c_str());  
        }

    _PrintExamples();

    wprintf  (L"\n\n\n");
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArgParser::LoadResponseFile (CharCP arg) 
    {
    if (2 > strlen (arg))
        {
        wprintf (L"Invalid response file...\n");
        return false;
        }
    WString filename (&arg[1]);

    Bentley::bvector<WString> lines;
    if (SUCCESS != argParser_loadLinesFromFile (lines, filename))
        {
        wprintf (L"Invalid response file [%s]\n", arg);
        return false;
        }
    
    size_t i, sz = lines.size();
    size_t const bsz = 1025;
    char buffer[bsz];
    for (i = 0; i < sz; ++i)
        {
        WString trimmedLine;
        argParser_parseLineRemoveCommentAndTrim (trimmedLine, lines[i].c_str());
        if (L"" == trimmedLine)
            continue;
        trimmedLine.ConvertToLocaleChars (buffer, bsz-1);
        if (!LoadArg (buffer))
            return false;
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArgParser::ParseArguments (int argc, char ** argv) 
    {
    m_assemblyName = argv[0];

    if (0 == m_outArgs.size())
        m_outArgs.push_back (m_assemblyName);

    for (int i = 1; i < argc; ++i)
        {
        if (!LoadArg (argv[i]))
            return false;
        }
    ProcessParsedArguments();
    CreateModifiedArgumentList();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgParser::CreateModifiedArgumentList () 
    {
    size_t i, sz = m_outArgs.size();
    m_argc = (int)sz;
    size_t realsize = sz+1;
    m_argv = new char*[realsize]; // NOTE: PLUS 1 is because gtest likes to move the pointer past end of argv!!! pagalloc does its job and stops execution.

    for (i = 0; i < sz; i++)
        {
        size_t size = m_outArgs[i].size() + 1;
        m_argv[i] = new char[size];
        strcpy (m_argv[i], m_outArgs[i].c_str());
        //m_outArgs[i].ConvertToLocaleChars (m_argv[i], size);
        }
    
    m_argv[realsize-1] = new char[20];
    m_argv[realsize-1][0] = 0;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
ArgParser::ArgParser ()
    : m_argc (0),
      m_argv (NULL),
      m_wasHelpPresent (false)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
ArgParser::~ArgParser ()
    {
    FreeArgs();
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgParser::FreeArgs()
    {
    if (NULL == m_argv)
        return;

    int i, sz = m_argc + 1;
    for (i = 0; i < sz; ++i)
        if (NULL != m_argv[i])
            delete [] m_argv[i];
    delete [] m_argv;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ArgParser::InitializeArgumentMap ()
    {
    _InitializeArgumentMap();

    // Initialize defaults.
    ArgInfoMap::iterator iter;
    for (iter = m_argMap.begin(); iter != m_argMap.end(); ++iter)
        {
        iter->second.LoadValueFromString (iter->second.defaultValue.c_str());
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    KevinNyman      09/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ArgParser::MakeResponseFile (WCharCP filename, bool useDefaults)
    {
    wprintf (L"Attempting to create response file %s.\n", filename);

    if (SUCCESS != argParser_createTextFile (filename))
        return ERROR; 

    wchar_t buffer[1024];
    ArgInfoMap::iterator iter;
    for (iter = m_argMap.begin(); iter != m_argMap.end(); ++iter)
        {
        if (0 == strcmpi("makeresponse", iter->first))
           continue; 

        WString line ("--");
        line += WString (iter->first);
        line += L"=";
        if (useDefaults)
            line += iter->second.defaultValue.c_str();
        else 
            {
            WString value;
            iter->second.ToString(value);
            line += value;
            }
        wsprintf  (buffer, L"%s\n", line.c_str());  
        argParser_appendStrToFile (buffer, filename);
        }

    wprintf (L"Response file written to %s.\n", filename);
    return SUCCESS;
    }

