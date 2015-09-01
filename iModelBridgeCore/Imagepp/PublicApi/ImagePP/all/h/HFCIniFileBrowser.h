//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCIniFileBrowser.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Definition of the HFCIniFileBrowser class.
//-----------------------------------------------------------------------------
#pragma once

//#include <fstream>
#include "HFCPtr.h"

BEGIN_IMAGEPP_NAMESPACE
class HFCURLFile;
class HFCBinStream;

/**

    This is a class that can be used to browse configuration files, often
    known as "INI" files because they use the ".ini" extension.  These are
    plain text files that list a set of variables and the values affected to
    them, where variables are grouped by topic.  This class defines the
    interface for objects that will be used to read these files (no edition
    or writing is possible through that interface).

    In a configuration file, each group begins by the name of the topic
    between brackets, then is followed by a list of variable definitions,
    each one being alone on a line, and composed of the name of the
    variable, followed by an equal sign and the value affected to it.  The
    value can be anything from a number to a text to an enumeration, and can
    even have more than one line.  Here is an example of a typical file:

    @code
|    [MemorySetting]
|    ObjectStoreLog_MemActif=0
|    ObjectStoreLog_Mem=1024
|    ObjectStoreLog_Tiles=30
|    BufferImgLog_MemActif=0
|    BufferImgLog_Mem=2048
|    BufferImgLog_Tiles=30
|
|    [Settings]
|    WindowPos=0,1,-1,-1,-1,-1,106,202,945,810
|
|    [ToolBar-Summary]
|    Bars=2
|    ScreenCX=1152
|    ScreenCY=864
|
|    [ToolBar-Bar0]
|    BarID=59393
|
|    [ToolBar-Bar1]
|    BarID=1000
|    XPos=-2
|    YPos=-2
|    Docking=1
    @end

    The topics can be used to distinguish different variables having the
    same name, as this example illustrates it by having two "BarID" values,
    one for each toolbar.  The structure of content of the file is often
    dependant on the values it contains : here we can see in the
    "Toolbar-Summary" topic that we have two toolbars ("bars = 2") so we can
    expect two topics, one for each toolbar.

    The interface of this class permits the browsing of a configuration file
    on a sequential basis, by finding the first topic, then getting its
    name, getting the values in it, then reading the next topic, and so on.
    It also permits the browsing as a random-access file, by selecting a
    topic by its name before getting values in it.  In any case there is
    always a "current topic" into which values can be read, and the current
    topic can be changed to the next one in file or to a specific one.

*/

class HFCIniFileBrowser
    {
public:
    //:> Constructor and destructor.
    IMAGEPP_EXPORT                     HFCIniFileBrowser(HFCBinStream* pi_rpFile);
    IMAGEPP_EXPORT virtual             ~HFCIniFileBrowser();

    //:> Topic methods.
    IMAGEPP_EXPORT bool               FirstTopic();
    IMAGEPP_EXPORT bool               FindTopic(const string& pi_rTopicName);
    IMAGEPP_EXPORT bool               FindTopic(const WString& pi_rTopicName);
    IMAGEPP_EXPORT const WString&      GetTopicName();
    IMAGEPP_EXPORT bool               NextTopic();
    IMAGEPP_EXPORT void                ReadLine(string&  po_rString,
                                        bool    pi_KeepLineSeparator = false,
                                        uint32_t pi_MaxSize = -1);
    IMAGEPP_EXPORT void                ReadLine(WString& po_rString,
                                        bool    pi_KeepLineSeparator = false,
                                        uint32_t pi_MaxSize = -1);

    //:> Variable methods.
    IMAGEPP_EXPORT bool               GetVariableValue(const string&  pi_rVariableName,
                                                string&        po_rValue,
                                                bool          pi_KeepLineSeparator = false);
    IMAGEPP_EXPORT bool               GetVariableValue(const WString& pi_rVariableName,
                                                WString&       po_rValue,
                                                bool          pi_KeepLineSeparator = false);

private:
    //:> Members.
    HFCBinStream*    m_pFile;
    WChar       m_TopicBeginMark;
    WChar       m_TopicEndMark;
    WString      m_TopicNameW;
    uint64_t    m_TopicOffset;
    WChar       m_VariableMark;

    //:> Line manipulation method.
    void                CleanUpString(WString& pi_rString,
                                      bool    pi_KeepLineSeparator = false);
    bool               ExtractTopicName(const WString& pi_rString);
    bool               ExtractVariableName(const WString& pi_rString, WString& pi_rVariableName);
    bool               IsValidChar(const WChar& pi_rChar,
                                    bool         pi_KeepLineSeparator = false);

    //:> Disabled method.
    HFCIniFileBrowser(const HFCIniFileBrowser& pi_rObj);
    HFCIniFileBrowser& operator=(const HFCIniFileBrowser& pi_rObj);
    };

END_IMAGEPP_NAMESPACE