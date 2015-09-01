//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCIniFileBrowser.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Implementation of the HFCIniFileBrowser class.
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCIniFileBrowser.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HFCBinStream.h>

/**----------------------------------------------------------------------------
 Constructor for this class. The constructor opens the configuration
 file to be browsed.  It must be existent and it must contain at least
 one topic and follow the format described in the overview of this
 class.

 After construction there is no current topic that is selected, so
 variables cannot be read unless FirstTopic or FindTopic is called
 succesfully.

 @param pi_pFile Pointer to binary stream manager that opened the file to scan.
                 The file will be closed automatically at destruction time.
-----------------------------------------------------------------------------*/
HFCIniFileBrowser::HFCIniFileBrowser(HFCBinStream* pi_pFile)
    {
    HPRECONDITION(pi_pFile != 0);

    // Initialise members.
    m_pFile = pi_pFile;
    m_TopicBeginMark = '[';
    m_TopicEndMark   = ']';
    m_VariableMark   = '=';

#if 0
    // Open the specified file.
    string FileName(pi_rpURL->GetHost());
    FileName += "\\";
    FileName += pi_rpURL->GetPath();

    m_File.open(FileName.c_str());
    if (m_File.is_open() == 0)
        {
        throw HFCFileException(HFC_FILE_NOT_FOUND_EXCEPTION, FileName);
        }
#endif
    }

/**----------------------------------------------------------------------------
 Destructor for this class. The destructor closes the file.
-----------------------------------------------------------------------------*/
HFCIniFileBrowser::~HFCIniFileBrowser()
    {
#if 0
    // Close the raster file.
    m_File.close();
#endif
    }

/**----------------------------------------------------------------------------
 Changes the current topic to the first one found in the configuration
 file.  To get its name, call @r{GetTopicName}.  To continue the iteration
 through the configuration file, call @r{NextTopic}.

 @return true if a topic have been found, false otherwise.

 @see GetTopicName<
 @see NextTopic
-----------------------------------------------------------------------------*/
bool HFCIniFileBrowser::FirstTopic()
    {
    // Move to begin of file.
    m_TopicOffset = 0;
    m_TopicNameW  = L"";

    // Search the first topic.
    return NextTopic();
    }

/**----------------------------------------------------------------------------
 Searches the configuration file for the specified topic name, and
 changes the current topic to that one if it is found.  The search is
 case sensitive, this means the lowercase and uppercase and not
 considered to be the same.  If the topic is not found, there is no
 current topic anymore, so variables cannot be read unless @r{FirstTopic} or
 @r{FindTopic} is called succesfully.

 @param pi_rTopicName  Constant reference to a string that contains the
                       name of the topic to be found.

 @return true is the topic has been found and the current topic has been changed,
         false is the topic has not been found and the current topic became invalid.

 @see FirstTopic
 @see FindTopic
-----------------------------------------------------------------------------*/
bool HFCIniFileBrowser::FindTopic(const string& pi_rTopicName)
    {
    WString tempoStr;
    BeStringUtilities::CurrentLocaleCharToWChar( tempoStr,pi_rTopicName.c_str());

    return FindTopic(tempoStr);
    }

bool HFCIniFileBrowser::FindTopic(const WString& pi_rTopicName)
    {
    bool Found     = false;
    bool HaveTopic;

    // Get the first topic.
    HaveTopic = FirstTopic();

    // Search for the specified topic.
    while (HaveTopic)
        {
        // Check if we have found the topic.
        if (m_TopicNameW == pi_rTopicName)
            {
            HaveTopic = false;
            Found     = true;
            }
        else
            {
            HaveTopic = NextTopic();
            }
        }

    return Found;
    }

/**----------------------------------------------------------------------------
 Returns a constant reference to a string that contains the name of the
 current topic.  Behavior is undefined if there is no current topic.

 @return A constant reference to a string containing the name of the current topic.
-----------------------------------------------------------------------------*/
const WString& HFCIniFileBrowser::GetTopicName()
    {
    return m_TopicNameW;
    }

/**----------------------------------------------------------------------------
 Changes the current topic to the one that follows the current one in the
 configuration file.  If the current one was the last one, there is no
 current topic anymore, so variables cannot be read unless @r{FirstTopic} or
 @r{FindTopic} is called succesfully.  To get the name of the new topic
 found, call {GetTopicName}.

 @return true if the current topic was succesfully changed to the next one,
         false if the end of the file was reached before a new topic was found.

 @see FindTopic
 @see FindTopic
 @see GetTopicName
-----------------------------------------------------------------------------*/
bool HFCIniFileBrowser::NextTopic()
    {
    bool  Found = false;
    WString CurrentLine;

    // Go to the begin of the current topic.
    m_pFile->SeekToPos(m_TopicOffset);

    // Read the first line of the topic.
    ReadLine(CurrentLine);

    // Search the next topic.
    while (!CurrentLine.empty())
        {
        // Check if we have found a topic.
        if (ExtractTopicName(CurrentLine))
            {
            // Indicate that we have found a topic and get it's position.
            Found         = true;
            CurrentLine   = L"";
            m_TopicOffset = m_pFile->GetCurrentPos();
            }
        else
            {
            ReadLine(CurrentLine);
            }
        }

    return Found;
    }

/**----------------------------------------------------------------------------
 Retrieves a value from the configuration file.  It looks for the entry
 labeled by the name specified by @{pi_rVarName} in the current topic
 section of the file, and copies its values (if found) into the string
 refered by @{po_rValue}.  The search is case sensitive, this means that
 lowercase and uppercase are not considered to be the same.

 Leading and trailing whitespaces are removed from the value before
 copying.  If the value expands over many lines, line separators are
 replaced by single space, and leading/trailing whitespaces for each line
 are removed.

 @param pi_rVariableName     Constant reference to a string that contains the name
                             of the variable to find in the current topic part of
                             the configuration file.
 @param po_rValue            Reference to a string that will receive the text of
                             the value affected to specified variable.
 @param pi_KeepLineSeparator If set to true, values that spread on more than one
                             line are kepts as "multi-line" values (carriage
                             returns and newlines codes are kept in the result).

 @return true if the variable has been found in the current topic section of
         the configuration file, false otherwise.
-----------------------------------------------------------------------------*/
bool HFCIniFileBrowser::GetVariableValue(const string& pi_rVariableName,
                                          string&       po_rValue,
                                          bool         pi_KeepLineSeparator)
    {

    WString tempoStr;
    BeStringUtilities::CurrentLocaleCharToWChar( tempoStr,pi_rVariableName.c_str());

    WString Value;
    bool Ret = GetVariableValue(tempoStr, Value, pi_KeepLineSeparator);

    size_t  destinationBuffSize = Value.GetMaxLocaleCharBytes();
    char*  ValueMBS= (char*)_alloca (destinationBuffSize);
    BeStringUtilities::WCharToCurrentLocaleChar(ValueMBS, Value.c_str(),destinationBuffSize);

    po_rValue = string(ValueMBS);
    return Ret;
    }
bool HFCIniFileBrowser::GetVariableValue(const WString& pi_rVariableName,
                                          WString&       po_rValue,
                                          bool          pi_KeepLineSeparator)
    {
    bool  Found = false;
    WString CurrentLine;

    // Go to the top of the current topic.
    m_pFile->SeekToPos(m_TopicOffset);

    // Read the first line.
    ReadLine(CurrentLine, pi_KeepLineSeparator);

    // Check that we have a line with data and that it isn't a topic.
    while (!Found && !CurrentLine.empty() && !ExtractTopicName(CurrentLine))
        {
        WString VariableName;

        if (ExtractVariableName(CurrentLine, VariableName) &&
            VariableName == pi_rVariableName)
            {
            WString Value;

            // Indicate the we have found the variable.
            Found = true;

            // Get the variable value.
            Value = CurrentLine.substr(CurrentLine.find_first_of(m_VariableMark)+1);
            CleanUpString(Value, pi_KeepLineSeparator);
            po_rValue = Value;

            // Check if we have a multi-line value by ready to
            // the next variable or topic.
            ReadLine(CurrentLine, pi_KeepLineSeparator);
            while (!CurrentLine.empty() && !ExtractTopicName(CurrentLine) &&
                   !ExtractVariableName(CurrentLine, Value))
                {
                // Add the value to the existing part.
                CleanUpString(CurrentLine, pi_KeepLineSeparator);
                po_rValue += L' ' + CurrentLine;

                // Read the next line.
                ReadLine(CurrentLine, pi_KeepLineSeparator);
                }
            CleanUpString(po_rValue, pi_KeepLineSeparator);
            }

        // We we don't have found the variable we need to read another line.
        if (!Found)
            {
            ReadLine(CurrentLine, pi_KeepLineSeparator);
            }
        }

    return Found;
    }

//-----------------------------------------------------------------------------
// Private
//-----------------------------------------------------------------------------
bool HFCIniFileBrowser::ExtractTopicName(const WString& pi_rString)
    {
    bool   Found = false;
    size_t  BeginPos;
    size_t  EndPos;

    // Search for the topic begin mark.
    BeginPos = pi_rString.find_first_of(m_TopicBeginMark);
    if (BeginPos != WString::npos)
        {
        // Search for the topic end mark.
        EndPos = pi_rString.find_first_of(m_TopicEndMark, BeginPos);
        if (EndPos != WString::npos)
            {
            // We have a topic, we need to extrude it.
            Found       = true;
            m_TopicNameW = pi_rString.substr(BeginPos+1, EndPos-BeginPos-1);
            CleanUpString(m_TopicNameW);
            }
        }

    return Found;
    }

//-----------------------------------------------------------------------------
// Private
// Extract the variable name.
//-----------------------------------------------------------------------------
bool HFCIniFileBrowser::ExtractVariableName(const WString& pi_rString,
                                             WString& pi_rVariableName)
    {
    bool  Found = false;
    size_t MarkPos;
    size_t HttpPos;

    // Check if the current line contain a variable mark.
    MarkPos = pi_rString.find_first_of(m_VariableMark);
    if (MarkPos != WString::npos)
        {
        HttpPos = pi_rString.find_first_of(L":");
        if ((HttpPos == WString::npos) || (HttpPos > MarkPos))
            {
            WString VariableName;

            // Extract the variable name.
            VariableName = pi_rString.substr(0, MarkPos);
            CleanUpString(VariableName);

            // Check that we have a variable name.
            if (!VariableName.empty())
                {
                Found            = true;
                pi_rVariableName = VariableName;
                }
            }
        }

    return Found;
    }

//-----------------------------------------------------------------------------
// Public
// Read a line of data from the file.  The empty line are flushed.
//-----------------------------------------------------------------------------
void HFCIniFileBrowser::ReadLine(string&    po_rString,
                                 bool      pi_KeepLineSeparator,
                                 uint32_t   pi_MaxSize)
    {
    WString Line;
    ReadLine(Line, pi_KeepLineSeparator, pi_MaxSize);

    size_t  destinationBuffSize = Line.GetMaxLocaleCharBytes();
    char*  LineMBS= (char*)_alloca (destinationBuffSize);
    BeStringUtilities::WCharToCurrentLocaleChar(LineMBS, Line.c_str(),destinationBuffSize);

    po_rString = string(LineMBS);
    }

void HFCIniFileBrowser::ReadLine(WString& po_rString,
                                 bool    pi_KeepLineSeparator,
                                 uint32_t pi_MaxSize)
    {
    HPRECONDITION(pi_MaxSize == -1 || pi_MaxSize < ULONG_MAX - 512);

    const int BufferSize = 512;
    char     Buffer[BufferSize + 1];
    WString   CurrentLine;

    // Read line until we found a line with data.
    // We also stop to read the file if we the current position in the file
    // is invalid.
    while (CurrentLine.empty() && !m_pFile->EndOfFile())
        {
        memset(Buffer, 0, BufferSize + 1);
        bool EndOfLine = false;
        if (pi_MaxSize == -1)
            {
            for (unsigned short i = 0; i < BufferSize && !EndOfLine; i++)
                {
                m_pFile->Read(&Buffer[i], 1);
                EndOfLine = Buffer[i] == '\n' || m_pFile->EndOfFile();
                }
            }
        else    
            {
            unsigned short ReadSize = (unsigned short)MIN(BufferSize, (long unsigned int)(pi_MaxSize - CurrentLine.length() - 1));
            for (unsigned short i = 0; i < ReadSize && !EndOfLine; i++)
                {
                m_pFile->Read(&Buffer[i], 1);
                EndOfLine = Buffer[i] == '\n' || m_pFile->EndOfFile();
                }
            }

        WString tempoStr;
        BeStringUtilities::Utf8ToWChar(tempoStr,Buffer);

        CurrentLine += tempoStr;

        if (EndOfLine)
            CleanUpString(CurrentLine, pi_KeepLineSeparator);
        }

    po_rString = CurrentLine;
    }

//-----------------------------------------------------------------------------
// Private
// Remove SPACE/TAB/ENTER from the begin and end of the file.
//-----------------------------------------------------------------------------
void HFCIniFileBrowser::CleanUpString(WString& pi_rString, bool pi_KeepLineSeparator)
    {
    size_t Pos = 0;

    // Remove the SPACE/TAB at the begin of the string.
    while (Pos < pi_rString.size() && !IsValidChar(pi_rString[Pos], pi_KeepLineSeparator))
        {
        Pos++;
        }
    pi_rString = pi_rString.substr(Pos);

    if (pi_rString.size() > 0)
        {
        // Remove the SPACE/TAB/ENTER at the end of the string.
        Pos = pi_rString.size()-1;
        while(Pos >= 0 && !IsValidChar(pi_rString[Pos], pi_KeepLineSeparator))
            {
            Pos--;
            }
        pi_rString = pi_rString.substr(0, Pos+1);
        }
    }

//-----------------------------------------------------------------------------
// Private
// Check if the specified character is valid.  Valid character exclude SPACE/
// TAB/ENTER.
//-----------------------------------------------------------------------------
bool HFCIniFileBrowser::IsValidChar(const WChar& pi_rChar,
                                     bool         pi_KeepLineSeparator)
    {
    bool IsValid = true;

    switch (pi_rChar)
        {
        case L'\n':
        case L'\r':
            IsValid = pi_KeepLineSeparator;
            break;
        case L' ':
        case L'\t':
            IsValid = false;
            break;
        }

    return IsValid;
    }
