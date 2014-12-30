/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/strfunc.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <Bentley/Bentley.h>
#include    <Bentley/BeAssert.h>
//#include    <Bentley/BeDebugLog.h>
#include    <Bentley/PTypesU.h>
#include    <Bentley/BeStringUtilities.h>
#include    <Bentley/BeNumerical.h>
#include    <Bentley/WString.h>

BEGIN_BENTLEY_NAMESPACE

#ifdef BENTLEY_CPP_MISSING_WCHAR_SUPPORT

struct PrintSink
    {
    int     m_error;
    PrintSink();
    virtual void    OnError (int);
    virtual int     GetError() const;
    virtual size_t  GetCount() const = 0;
    virtual void    PutChar (char) = 0;
    virtual void    PutCharCP (CharCP) = 0;
    /*
    virtual void    PutWChar (wchar_t) = 0;
    virtual void    PutWCharCP (WCharCP);
    */
    };

struct WStringPrintSink : PrintSink
    {
    WString&  m_s;
    WStringPrintSink (WString& s);
    virtual void PutChar (char) override     ;
    virtual void PutCharCP (CharCP) override   ;
    /*
    virtual void PutWChar (wchar_t) override  ;
    virtual void PutWCharCP (WCharCP) override  ;
    */
    virtual size_t GetCount() const override;
    };

struct Utf8PrintSink : PrintSink
    {
    /*
    virtual void PutWChar (wchar_t) override  ;
    virtual void PutWCharCP (WCharCP) override  ;
    */
    };

struct Utf8StringPrintSink : Utf8PrintSink
    {
    Utf8String&  m_s;
    Utf8StringPrintSink (Utf8String& s);
    virtual void PutChar (char) override     ;
    virtual void PutCharCP (CharCP) override   ;
    virtual size_t GetCount() const override;
    };

struct ScanSource
    {
    int             m_error;
    bvector<char>   m_pushedBack;
    ScanSource();
    virtual ~ScanSource() {;}
    virtual void    OnError (int);
    virtual int     GetError() const;
    virtual int     Getc ();
    virtual int     GetNextChar () = 0;
    virtual void    PutBack (char);
    };

struct StringScanSource : ScanSource
    {
    CharCP          m_string;
    StringScanSource (CharCP);
    virtual int     GetNextChar () override;
    };

struct WStringScanSource : ScanSource
    {
    WCharCP          m_string;
    WStringScanSource (WCharCP);
    virtual int     GetNextChar () override;
    };

#endif // BENTLEY_CPP_MISSING_WCHAR_SUPPORT

//! Write formatted string to \a buffer
//! @param buffer the formatted string
//! @param fmt the formatting to apply
//! @param ap arguments to the format
//! @param initialLengthGuess If the caller can put a limit on how long the formatted string could be, pass it in here. Or, if the caller knows that it can
//! make two passes, then he can pass in a relatively small value for the initial guess, in order to cover common cases. If no guess is possible, then pass -1.
//! @return Upon successful return, the number of characters written to \a buffer (EXCLUDING the trailing 0-terminator).
int BeUtf8StringSprintf (Utf8String& buffer, CharCP fmt, va_list ap, int initialLengthGuess = -1);

//! Return at least a good guess at how long the formatted string will be. On Windows, this will be an exact guess. On *nix, this will probably be just a big number.
//! @note the return value does NOT include the trailing \0
int BeUtf8StringGuessLength (CharCP fmt, va_list ap);

//! Write formatted string to \a buffer
//! @param buffer the formatted string
//! @param fmt the formatting to apply
//! @param ap arguments to the format
//! @param initialLengthGuess If the caller can put a limit on how long the formatted string could be, pass it in here. Or, if the caller knows that it can
//! make two passes, then he can pass in a relatively small value for the initial guess, in order to cover common cases. If no guess is possible, then pass -1.
//! @return Upon successful return, the number of characters written to \a buffer (EXCLUDING the trailing 0-terminator).
int BeWStringSprintf (WString& buffer, WCharCP fmt, va_list ap, int initialLengthGuess = -1);

//! Return at least a good guess at how long the formatted string will be. On Windows, this will be an exact guess. On *nix, this will probably be just a big number.
//! @note the return value does NOT include the trailing \0
int BeWStringGuessLength (WCharCP fmt, va_list ap);

//! Write formatted string to \a buffer
//! @param buffer the formatted string
//! @param numCharsInBuffer the maximum number of characters that may be written to the buffer, including trailing \0
//! @param fmt the formatting to apply
//! @param ap arguments to the format
//! @return Upon successful return, the number of characters written to \a buffer (EXCLUDING the trailing 0-terminator).
//! @remarks This function does not write more than numCharsInBuffer bytes (including the terminating null byte ('\0')).
//! If the output was truncated due to this limit then the return value is the number of characters (EXCLUDING the terminating null byte)
//! which would have been written to the final string if enough space had been available. Thus, a return value of numCharsInBuffer or more means
//! that the output was truncated.
int Bevsnprintf (CharP buffer, size_t numCharsInBuffer, CharCP fmt, va_list ap);

//! Write formatted string to \a buffer
//! @param buffer the formatted string
//! @param numCharsInBuffer the maximum number of characters that may be written to the buffer, including trailing \0
//! @param fmt the formatting to apply
//! @param ap arguments to the format
//! @return Upon successful return, the number of characters written to \a buffer (EXCLUDING the trailing 0-terminator).
//! @remarks This function does not write more than numCharsInBuffer bytes (including the terminating null byte ('\0')).
//! If the output was truncated due to this limit then the return value is the number of characters (EXCLUDING the terminating null byte)
//! which would have been written to the final string if enough space had been available. Thus, a return value of numCharsInBuffer or more means
//! that the output was truncated.
int  Bevsnwprintf (WCharP buffer, size_t numCharsInBuffer, WCharCP fmt, va_list ap);

END_BENTLEY_NAMESPACE
