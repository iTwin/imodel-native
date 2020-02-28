/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (ANDROID)
    #define USE_ANDROID_LOG
#elif defined (__APPLE__) || defined (__linux__)
    #define USE_SYSLOG
#elif defined (BENTLEY_WIN32)
    // use printf + OutputDebugStringW
    #include <windows.h>
#elif defined (BENTLEY_WINRT)
    // use OutputDebugStringW
    #include <windows.h>
#else
    #error unknown platform
#endif

#if defined (ANDROID)
    #include <android/log.h>
#elif defined (USE_SYSLOG)
    #include <syslog.h>

    #undef LOG_EMERG
    #undef LOG_ALERT
    #undef LOG_CRIT
    #undef LOG_ERR
    #undef LOG_WARNING
    #undef LOG_NOTICE
    #undef LOG_INFO
    #undef LOG_DEBUG

    enum SyslogLevels {
    SYSLOG_EMERG     = 0,       /* system is unusable */
    SYSLOG_ALERT     = 1,       /* action must be taken immediately */
    SYSLOG_CRIT      = 2,       /* critical conditions */
    SYSLOG_ERR       = 3,       /* error conditions */
    SYSLOG_WARNING   = 4,       /* warning conditions */
    SYSLOG_NOTICE    = 5,       /* normal but significant condition */
    SYSLOG_INFO      = 6,       /* informational */
    SYSLOG_DEBUG     = 7        /* debug-level messages */
    };

    static int s_syslog_ref_count;

#endif // defined (USE_SYSLOG)

#include "bsilogprivate.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_LOGGING
USING_NAMESPACE_BENTLEY_LOGGING_PROVIDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP BentleyApi::NativeLogging::Provider::GetSeverityText (SEVERITY sev)
    {
    switch ( sev )
        {
        case LOG_FATAL:     return LOG_TEXT_FATAL;
        case LOG_ERROR:     return LOG_TEXT_ERROR;
        case LOG_WARNING:   return LOG_TEXT_WARNING;
        case LOG_INFO:      return LOG_TEXT_INFO;
        case LOG_DEBUG:     return LOG_TEXT_DEBUG;
        case LOG_TRACE:     return LOG_TEXT_TRACE;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP BentleyApi::NativeLogging::Provider::GetSeverityUtf8Text (SEVERITY sev)
    {
    switch ( sev )
        {
        case LOG_FATAL:     return LOG_UTF8TEXT_FATAL;
        case LOG_ERROR:     return LOG_UTF8TEXT_ERROR;
        case LOG_WARNING:   return LOG_UTF8TEXT_WARNING;
        case LOG_INFO:      return LOG_UTF8TEXT_INFO;
        case LOG_DEBUG:     return LOG_UTF8TEXT_DEBUG;
        case LOG_TRACE:     return LOG_UTF8TEXT_TRACE;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SEVERITY BentleyApi::NativeLogging::Provider::GetSeverityFromText
(
WCharCP sev
)
    {
    SEVERITY  severity = LOG_WARNING;

    if ( NULL == sev )
        {
        return severity;
        }

    if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_FATAL ) )
        {
        severity = LOG_FATAL;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_ERROR ) )
        {
        severity = LOG_ERROR;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_WARNING ) )
        {
        severity = LOG_WARNING;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_INFO ) )
        {
        severity = LOG_INFO;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_DEBUG ) )
        {
        severity = LOG_DEBUG;
        }
    else if ( 0 == BeStringUtilities::Wcsicmp ( sev, LOG_TEXT_TRACE ) )
        {
        severity = LOG_TRACE;
        }

    return severity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::SeverityMap ( void ) : m_defaultSeverity ( LOG_ERROR )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::SeverityMap ( SEVERITY defaultSeverity ) : m_defaultSeverity ( defaultSeverity )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SeverityMap::~SeverityMap ( void )
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
SEVERITY SeverityMap::GetDefaultSeverity ( void )
    {
    return m_defaultSeverity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void SeverityMap::SetDefaultSeverity
(
SEVERITY    severity
)
    {
    m_defaultSeverity = severity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int SeverityMap::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    BeAssert ( NULL != nameSpace );

    BeMutexHolder lock(m_lock);

    bmap<WString,SEVERITY>::iterator it = m_severity.find ( nameSpace );

    if ( it != m_severity.end() )
        {
        (*it).second = severity;
        }
    else
        {
        m_severity.insert ( bmap<WString,SEVERITY>::value_type(nameSpace,severity) );
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool SeverityMap::IsSeverityEnabled
(
WCharCP     nameSpace,
SEVERITY    sev
)
    {
    BeAssert ( NULL != nameSpace );

    BeMutexHolder lock(m_lock);

    SEVERITY    severity = m_defaultSeverity;

    bmap<WString,SEVERITY>::iterator it = m_severity.find ( nameSpace );

    if ( it != m_severity.end() )
        {
        severity = (*it).second;
        }

    if ( sev < severity )
        {
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ConsoleProvider::ConsoleProvider ( void ) : m_severity ( LOG_ERROR )
    {
#if defined (USE_SYSLOG)
    if (0 == s_syslog_ref_count++)
        openlog ("Be", (LOG_CONS|LOG_PERROR), LOG_USER);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ConsoleProvider::~ConsoleProvider ( void )
    {
#if defined (USE_SYSLOG)
    if (0 == --s_syslog_ref_count)
        closelog ();
#endif
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::Initialize ( void )
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/04
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::Uninitialize ( void )
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::CreateLogger
(
WCharCP                 nameSpace,
ILogProviderContext**   ppContext
)
    {
    if ( NULL == ppContext || NULL == nameSpace )
        {
        return ERROR;
        }

    *ppContext = reinterpret_cast<ILogProviderContext*>(new WString ( nameSpace ));

    if ( NULL == *ppContext )
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::DestroyLogger
(
ILogProviderContext*    pContext
)
    {
    if ( NULL == pContext )
        {
        return ERROR;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    if ( NULL == pNs )
        {
        return ERROR;
        }

    delete pNs;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE ConsoleProvider::LogMessage
(
ILogProviderContext*    pContext,
SEVERITY                sev,
WCharCP                 msg
)
    {
    if ( NULL == pContext || NULL == msg )
        {
        return;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    if ( !m_severity.IsSeverityEnabled (pNs->c_str(), sev) )
        {
        return;
        }

#if defined (BENTLEY_WIN32)

    // Because it is so easy for developers to screw up printf format strings, we're
    // going to protect ourselves from an exception and log a message saying that
    // our developers are stupid and to please contact support
    BSILOG_TRY
        {
        WPrintfString formattedMessage ( L"%-8ls %-20ls %ls\n", GetSeverityText(sev), pNs->c_str(), msg );
        OutputDebugStringW (formattedMessage.c_str());
        fwprintf ( stdout, L"%ls", formattedMessage.c_str() );
        }
    BSILOG_CATCH
        {
        fwprintf ( stdout, L"Formating output string caused an exception!!!" );
        }

#elif defined (BENTLEY_WINRT)

    WPrintfString formattedMessage ( L"%-8ls %-20ls %ls\n", GetSeverityText(sev), pNs->c_str(), msg );
    OutputDebugStringW (formattedMessage.c_str());

#elif defined (__unix__)

    LogMessage (pContext, sev, Utf8String(msg).c_str());

#else

    #error unknown platform

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE ConsoleProvider::LogMessage
(
ILogProviderContext*    pContext,
SEVERITY                sev,
Utf8CP                  msg
)
    {
    if ( NULL == pContext || NULL == msg )
        {
        return;
        }

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    if ( !m_severity.IsSeverityEnabled (pNs->c_str(), sev) )
        {
        return;
        }

    Utf8String nsUtf8 (pNs->c_str());

#if defined (USE_ANDROID_LOG)

    int asev;
    switch (sev)
        {
        case LOG_FATAL:   asev = ANDROID_LOG_FATAL;   break;
        case LOG_ERROR:   asev = ANDROID_LOG_ERROR;   break;
        case LOG_WARNING: asev = ANDROID_LOG_WARN;    break;
        case LOG_INFO:    asev = ANDROID_LOG_INFO;    break;
        case LOG_DEBUG:   asev = ANDROID_LOG_DEBUG;   break;
        case LOG_TRACE:   asev = ANDROID_LOG_VERBOSE; break;
        default:          asev = ANDROID_LOG_INFO;    break;
        }

    __android_log_print (asev, nsUtf8.c_str (), "%s", msg);

#elif defined (USE_SYSLOG)

    int asev;
    switch (sev)
        {
        case LOG_FATAL:   asev = SYSLOG_EMERG;   break;
        case LOG_ERROR:   asev = SYSLOG_ERR;     break;
        case LOG_WARNING: asev = SYSLOG_WARNING; break;
        case LOG_INFO:    asev = SYSLOG_INFO;    break;
        case LOG_DEBUG:   asev = SYSLOG_DEBUG;   break;
        case LOG_TRACE:   asev = SYSLOG_DEBUG;   break;
        default:          asev = SYSLOG_INFO;    break;
        }

    syslog (asev, "%s: %s", nsUtf8.c_str(), msg);

#else

    LogMessage (pContext, sev, WString(msg,true).c_str());

#endif
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::SetOption
(
WCharCP attribName,
WCharCP attribValue
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_DEFAULT_SEVERITY ) )
        {
        m_severity.SetDefaultSeverity ( GetSeverityFromText(attribValue) );
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::GetOption
(
WCharCP attribName,
WCharP  attribValue,
uint32_t valueSize
)
    {
    BeAssert ( NULL != attribName );
    BeAssert ( NULL != attribValue );

    if ( 0 == wcscmp ( attribName, CONFIG_OPTION_DEFAULT_SEVERITY ) )
        {
        BeStringUtilities::Wcsncpy ( attribValue, valueSize, GetSeverityText(m_severity.GetDefaultSeverity()) );
        }
    else
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE ConsoleProvider::SetSeverity
(
WCharCP     nameSpace,
SEVERITY    severity
)
    {
    BeAssert ( NULL != nameSpace );

    return m_severity.SetSeverity (nameSpace,severity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    TonyCleveland   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool STDCALL_ATTRIBUTE ConsoleProvider::IsSeverityEnabled
(
ILogProviderContext*    pContext,
SEVERITY                severity
)
    {
    BeAssert ( NULL != pContext );

    WString* pNs = reinterpret_cast<WString*>(pContext);

    BeAssert ( NULL != pNs );

    return m_severity.IsSeverityEnabled (pNs->c_str(),severity);
    }

#if defined (BENTLEY_WIN32)

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
struct SplitConsoleProvider : ILogProvider
{
    typedef SHORT dim_t;

    static const dim_t s_maxMessageLength = 0x300;

    struct Coord : COORD
    {
        Coord(dim_t x, dim_t y) { X = x; Y = y; }
        Coord() : Coord(0, 0) { }
    };

    struct Rect : SMALL_RECT
    {
        Rect(dim_t l, dim_t t, dim_t r, dim_t b)
            {
            Left = l;
            Top = t;
            Right = r;
            Bottom = b;
            }

        dim_t GetWidth() const { return Right - Left + 1; }
        dim_t GetHeight() const { return Bottom - Top + 1; }
        Coord GetTopLeft() const { return Coord(Left, Top); }
    };

    struct Component
    {
        virtual void Log(WCharCP msg, SEVERITY sev, WCharCP nameSpace) = 0;
    };

    struct Pane : Component
    {
        WString                 m_name;
        Rect                    m_rect; // writable area
        SplitConsoleProvider*   m_provider;
        CHAR_INFO               m_buffer[s_maxMessageLength];
        bool                    m_isDefault = false;

        Pane(WStringCR name, SplitConsoleProvider& provider) : m_name(name), m_rect(0,0,20,10), m_provider(&provider) { }
        Pane() : m_rect(0,0,10,10), m_provider(nullptr) { }

        virtual void Log(WCharCP msg, SEVERITY sev, WCharCP nameSpace) override;
        dim_t PrepareMessage(WCharCP msg, SEVERITY sev, WCharCP nameSpace, WCharCP& nextSegment);
        static WORD GetAttributesForSeverity(SEVERITY sev);
    };

    struct Title : Component
    {
        WString m_name;

        Title()
            {
            // getenv is considered unsafe in VS 2017.
            WCharP envTitle;
            if (0 == _wdupenv_s(&envTitle, nullptr, L"BENTLEY_SPLIT_CONSOLE_TITLE") && nullptr != envTitle)
                {
                m_name = envTitle;
                free(envTitle);
                }
            }

        bool Matches(WCharCP name) const { return !m_name.empty() && nullptr != name && m_name.Equals(name); }

        virtual void Log(WCharCP msg, SEVERITY, WCharCP) override
            {
            SetConsoleTitleW(msg);
            }
    };

    struct CharInfo : CHAR_INFO
    {
        CharInfo(WChar ch, WORD attributes) { Char.UnicodeChar = ch; Attributes = attributes; }
    };
private:
    static const dim_t s_charBufferSize = 500;

    bvector<Pane>   m_panes;
    SeverityMap     m_severity;
    HANDLE          m_screenBuffer;
    Pane*           m_defaultPane;
    dim_t           m_screenBufferWidth;
    dim_t           m_screenBufferHeight;
    CHAR_INFO       m_charBuffer[s_charBufferSize];
    Title           m_title;
    BeAtomic<uint32_t>  m_sequenceId;
    bool                m_wantSequenceId;

    static WCharCP GetNameSpace(ILogProviderContext* pContext)
        {
        auto pWString = reinterpret_cast<WString*>(pContext);
        return nullptr != pWString ? pWString->c_str() : nullptr;
        }

    static bool GetMaxPaneWidth(dim_t& maxWidth)
        {
        int value;
PUSH_DISABLE_DEPRECATION_WARNINGS
        WString envVar(getenv("BENTLEY_SPLIT_CONSOLE_MAX_WIDTH"), BentleyCharEncoding::Utf8);
POP_DISABLE_DEPRECATION_WARNINGS
        if (!envVar.empty() && 1 == WString::Swscanf_safe(envVar.c_str(), L"%d", &value) && value > 0 && value <= maxWidth)
            {
            maxWidth = static_cast<dim_t>(value);
            return true;
            }

        return false;
        }

    Component* GetComponent(WCharCP name)
        {
        if (m_title.Matches(name))
            return &m_title;

        auto found = std::find_if(m_panes.begin(), m_panes.end(), [&](Pane const& arg) { return arg.m_name.Equals(name); });
        return m_panes.end() != found ? &(*found) : m_defaultPane;
        }

    void InitPanes(bvector<WString> const& paneNames);

    virtual int STDCALL_ATTRIBUTE Initialize () override;
    virtual int STDCALL_ATTRIBUTE Uninitialize () override;
    virtual int STDCALL_ATTRIBUTE CreateLogger ( WCharCP nameSpace, ILogProviderContext ** ppContext ) override;
    virtual int STDCALL_ATTRIBUTE DestroyLogger ( ILogProviderContext * pContext ) override;
    virtual int STDCALL_ATTRIBUTE SetOption ( WCharCP attribName, WCharCP attribValue ) override;
    virtual int STDCALL_ATTRIBUTE GetOption ( WCharCP attribName, WCharP attribValue, uint32_t valueSize ) override;
    virtual int STDCALL_ATTRIBUTE SetSeverity ( WCharCP _namespace, SEVERITY severity ) override;
    virtual bool STDCALL_ATTRIBUTE IsSeverityEnabled ( ILogProviderContext * context, SEVERITY sev ) override;

    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, WCharCP msg ) override;
    virtual void STDCALL_ATTRIBUTE LogMessage ( ILogProviderContext * context, SEVERITY sev, Utf8CP msg ) override;       // we have an optimized version for Android and iOS
public:
PUSH_DISABLE_DEPRECATION_WARNINGS
    SplitConsoleProvider(bvector<WString> const& paneNames) : m_severity(LOG_TRACE), m_defaultPane(nullptr),
        m_sequenceId(0), m_wantSequenceId(nullptr != getenv("BENTLEY_SPLIT_CONSOLE_SHOW_SEQUENCE"))
        {
        InitPanes(paneNames);
        }
POP_DISABLE_DEPRECATION_WARNINGS

    virtual ~SplitConsoleProvider(void) { }

    HANDLE GetScreenBuffer() { return m_screenBuffer; }
    dim_t GetBufferHeight() const { return m_screenBufferHeight; }
    dim_t GetBufferWidth() const { return m_screenBufferWidth; }
    uint32_t GetSequenceId() { return m_sequenceId++; }
    bool WantSequenceId() const { return m_wantSequenceId; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SplitConsoleProvider::Pane::Log(WCharCP msg, SEVERITY sev, WCharCP nameSpace)
    {
    WCharCP nextSegment = msg;
    dim_t nRows;
    while (nullptr != msg && 0 < (nRows = PrepareMessage(msg, sev, nameSpace, nextSegment)))
        {
        // Scroll existing text to make room
        static const CharInfo s_fillChar(' ', 0);
        Rect scrollRect = m_rect;
        scrollRect.Top += nRows;
        ScrollConsoleScreenBuffer(m_provider->GetScreenBuffer(), &scrollRect, &m_rect, m_rect.GetTopLeft(), &s_fillChar);

        // Write new text at bottom
        Rect writeRect = m_rect;
        writeRect.Top = writeRect.Bottom - nRows;
        WriteConsoleOutput(m_provider->GetScreenBuffer(), m_buffer, Coord(writeRect.GetWidth(), nRows), Coord(0,0), &writeRect);

        // If the message included a new line...
        msg = nextSegment;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
SplitConsoleProvider::dim_t SplitConsoleProvider::Pane::PrepareMessage(WCharCP rawMsg, SEVERITY sev, WCharCP, WCharCP& nextSegment)
    {
    nextSegment = nullptr;
    WCharCP msg = rawMsg;
    size_t nPrefixChars = 0;

    WString formattedMsg;
    if (m_provider->WantSequenceId() && nullptr != rawMsg)
        {
        formattedMsg.Sprintf(L"[%llu] %ls", m_provider->GetSequenceId(), rawMsg);
        msg = formattedMsg.c_str();
        }

    WORD attributes = GetAttributesForSeverity(sev);
    CHAR_INFO* pBuf = m_buffer;
    CHAR_INFO* pEnd = m_buffer + _countof(m_buffer);

    dim_t width = m_rect.GetWidth();
    dim_t nRows = 0;
    WCharCP pChar = msg;

    dim_t nChars = 0;
    bool done = false;
    while (!done)
        {
        // If the last row contains more characters than we have room for in our buffer, caller needs to call us again to print the remainder
        if ((pEnd - pBuf) < width)
            {
            done = true;
            size_t dist = pChar - msg + nPrefixChars;
            nextSegment = rawMsg + dist;
            break;
            }

        // Fill the entire row, appending blanks if necessary
        for (dim_t pos = 0; pos < width; pos++)
            {
            CHAR_INFO& buf = *pBuf++;
            if (!done && '\n' == *pChar)
                {
                done = true;
                size_t dist = pChar+1 - msg + nPrefixChars;
                nextSegment = rawMsg + dist;
                }

            buf.Char.UnicodeChar = done ? ' ' : *pChar++;
            buf.Attributes = attributes;
            done = done || '\0' == *pChar || (++nChars > s_maxMessageLength);
            }

        ++nRows;
        }

    return nRows;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
WORD SplitConsoleProvider::Pane::GetAttributesForSeverity(SEVERITY sev)
    {
    switch (sev)
        {
        case LOG_ERROR:
        case LOG_FATAL:
            return FOREGROUND_RED | FOREGROUND_INTENSITY;
        case LOG_WARNING:
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        case LOG_INFO:
        case LOG_DEBUG:
            return FOREGROUND_GREEN | FOREGROUND_INTENSITY;
        default:
            return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SplitConsoleProvider::InitPanes(bvector<WString> const& paneNames)
    {
    m_panes.reserve(paneNames.size());
    for (auto const& name : paneNames)
        m_panes.push_back(Pane(name, *this));

    // any messages without a dedicated pane go in the "Default" pane if one exists
    auto defaultIter = std::find_if(m_panes.begin(), m_panes.end(), [&](Pane const& arg) { return arg.m_name.EqualsI(L"default"); });
    if (m_panes.end() != defaultIter)
        {
        m_defaultPane = &(*defaultIter);
        m_defaultPane->m_isDefault = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SplitConsoleProvider::Initialize()
    {
    HWND console = GetConsoleWindow();
    if (NULL == console && AllocConsole())
        console = GetConsoleWindow();

    if (NULL == console)
        return ERROR;

    m_screenBuffer = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
    if (NULL == m_screenBuffer)
        return ERROR;

    // Set the size of the window
    COORD maxWindowSize = GetLargestConsoleWindowSize(m_screenBuffer);
    dim_t maxWindowWidth = maxWindowSize.X; // msdn documentation lies about the meaning of X and Y here...
    m_screenBufferHeight = s_charBufferSize;    // so we can scroll window up to see history...
    m_screenBufferWidth = maxWindowWidth;

    if (!SetConsoleScreenBufferSize(m_screenBuffer, Coord(m_screenBufferWidth, m_screenBufferHeight)))
        return ERROR;

#ifdef SET_WINDOW_SIZE
    CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
    if (!GetConsoleScreenBufferInfo(m_screenBuffer, &screenBufferInfo))
        return ERROR;

    screenBufferInfo.dwSize = maxWindowSize;
    auto srWindow = screenBufferInfo.srWindow;
    dim_t windowHeight = srWindow.Bottom - srWindow.Top;
    screenBufferInfo.srWindow.Bottom = m_screenBufferHeight;
    screenBufferInfo.srWindow.Top = m_screenBufferHeight - windowHeight;

    if (!SetConsoleWindowInfo(m_screenBuffer, TRUE, &srWindow))
        return ERROR;
#else
    // the above returns TRUE but has no effect. Setting the cursor pos instead has the desired result of focusing window on bottom of screen buffer.
    if (!SetConsoleCursorPosition(m_screenBuffer, Coord(0, m_screenBufferHeight-1)))
        return ERROR;
#endif

    // Set up the dimensions of each pane
    dim_t nPanes = static_cast<dim_t>(m_panes.size());
    dim_t nVerticalBars = nPanes - 1;   // vertical bar between each pane
    dim_t paneWidthsTotal = m_screenBufferWidth - nVerticalBars;
    dim_t minPaneWidth = paneWidthsTotal / nPanes;
    bool limitWidths = GetMaxPaneWidth(minPaneWidth);
    dim_t maxPaneWidth = limitWidths ? minPaneWidth : minPaneWidth + (paneWidthsTotal % nPanes); // right-most pane gets any extra space

    for (dim_t i = 0; i < nPanes; i++)
        {
        dim_t left = i * (minPaneWidth + 1);    // + 1 = include vertical bars
        dim_t width = (nPanes-1 == i) ? maxPaneWidth : minPaneWidth;
        m_panes[i].m_rect = Rect(left, 0, left+width-1, GetBufferHeight()-2); // -1: bottom row reserved for pane names
        }

    // Draw the static content
    // Vertical bar between panes
    static const WORD BACKGROUND_WHITE = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_GREEN;
    CharInfo barInfo(' ', BACKGROUND_WHITE);
    for (dim_t i = 0; i < GetBufferHeight(); i++)
        m_charBuffer[i] = barInfo;

    dim_t bufHeight = GetBufferHeight();
    for (dim_t i = 0; i < nPanes-1; i++)
        {
        Rect rect = m_panes[i].m_rect;
        Rect barRect(rect.Right+1, 0, rect.Right+2, bufHeight-1);
        WriteConsoleOutput(GetScreenBuffer(), m_charBuffer, Coord(1, bufHeight), Coord(0,0), &barRect);
        }

    // Name at bottom of pane
    for (auto const& pane : m_panes)
        {
        dim_t nameLen = static_cast<dim_t>(pane.m_name.length());
        for (dim_t i = 0; i < pane.m_rect.GetWidth(); i++)
            m_charBuffer[i] = CharInfo(i < nameLen ? pane.m_name[i] : ' ', BACKGROUND_WHITE);

        Rect rect(pane.m_rect.Left, bufHeight-1, pane.m_rect.Right, bufHeight);
        WriteConsoleOutput(GetScreenBuffer(), m_charBuffer, Coord(rect.GetWidth(), 1), Coord(0,0), &rect);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SplitConsoleProvider::Uninitialize()
    {
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SplitConsoleProvider::CreateLogger(WCharCP nameSpace, ILogProviderContext** ppContext)
    {
    if (nullptr == ppContext || nullptr == nameSpace || nullptr == GetComponent(nameSpace))
        return ERROR;

    *ppContext = reinterpret_cast<ILogProviderContext*>(new WString(nameSpace));
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SplitConsoleProvider::DestroyLogger(ILogProviderContext* pContext)
    {
    auto pStr = reinterpret_cast<WString*>(pContext);
    if (nullptr == pStr)
        return ERROR;

    delete pStr;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE SplitConsoleProvider::LogMessage(ILogProviderContext* pContext, SEVERITY sev, WCharCP msg)
    {
    WCharCP nameSpace = GetNameSpace(pContext);
    auto log = GetComponent(nameSpace);
    if (nullptr != log)
        log->Log(msg, sev, nameSpace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void STDCALL_ATTRIBUTE SplitConsoleProvider::LogMessage(ILogProviderContext* pContext, SEVERITY sev, Utf8CP msg)
    {
    WString wMsg(msg, true);
    LogMessage(pContext, sev, wMsg.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SplitConsoleProvider::SetOption(WCharCP name, WCharCP value)
    {
    if (0 == wcscmp(name, CONFIG_OPTION_DEFAULT_SEVERITY) && nullptr != GetComponent(name))
        {
        m_severity.SetDefaultSeverity(GetSeverityFromText(value));
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SplitConsoleProvider::GetOption(WCharCP name, WCharP value, uint32_t size)
    {
    if (0 == wcscmp(name, CONFIG_OPTION_DEFAULT_SEVERITY) && nullptr != GetComponent(name))
        {
        BeStringUtilities::Wcsncpy(value, size, GetSeverityText(m_severity.GetDefaultSeverity()));
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
int STDCALL_ATTRIBUTE SplitConsoleProvider::SetSeverity(WCharCP nameSpace, SEVERITY sev)
    {
    return nullptr != GetComponent(nameSpace) ? m_severity.SetSeverity(nameSpace, sev) : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool STDCALL_ATTRIBUTE SplitConsoleProvider::IsSeverityEnabled(ILogProviderContext* pContext, SEVERITY sev)
    {
    auto nameSpace = GetNameSpace(pContext);
    return nullptr != nameSpace && nullptr != GetComponent(nameSpace) && m_severity.IsSeverityEnabled(nameSpace, sev);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
ILogProvider* createConsoleLogger()
    {
    // getenv is considered unsafe in VS 2017.
    WCharP envNames;
    bvector<WString> names;
    if (0 == _wdupenv_s(&envNames, nullptr, L"BENTLEY_SPLIT_CONSOLE_LOG") && nullptr != envNames)
        {
        BeStringUtilities::Split(envNames, L",", names);
        free(envNames);
        }

    if (names.size() > 0)
        return new SplitConsoleProvider(names);
    else
        return new ConsoleProvider();
    }

#endif // BENTLEY_WIN32
