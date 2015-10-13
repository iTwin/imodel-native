/*--------------------------------------------------------------------------------------+
|
|     $Source: Bentley/nonport/BeTextFile.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <string.h>

#if defined (BENTLEY_WIN32)
# include    <mbstring.h>
# include    <mbctype.h>
#endif

#include    <Bentley\BeTextFile.h>
#include    <Bentley\BeAssert.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeTextFile::BeTextFile ()
    {
    m_file              = NULL;
    m_encoding          = TextFileEncoding::CurrentLocale;
    m_openType          = TextFileOpenType::Read;
    m_options           = TextFileOptions::None;
    m_noData            = false;
    m_reachedEnd        = false;
    m_currentByte       = m_readBuffer;
    m_endBuffer         = m_readBuffer;
    m_atStart           = true;
    m_startOfTextData   = 0;

    memset (&m_mbState, 0, sizeof (m_mbState));
    memset (&m_readBuffer, 0, sizeof (m_readBuffer));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeTextFile::~BeTextFile ()
    {
    Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
* This is the public static method.
+---------------+---------------+---------------+---------------+---------------+------*/
BeTextFilePtr   BeTextFile::Open (BeFileStatus& openStatus, WCharCP fullFileSpec, TextFileOpenType openType, TextFileOptions options, TextFileEncoding encoding)
    {
    BeTextFilePtr   textFile = new BeTextFile();

    if (BeFileStatus::Success != (openStatus = textFile->OpenFile (fullFileSpec, openType, options, encoding)))
        return NULL;

    return textFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeTextFile::Close ()
    {
    if (NULL != m_file)
        {
        delete m_file;
        m_file = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus    BeTextFile::OpenFile (WCharCP fullFileSpec, TextFileOpenType openType, TextFileOptions options, TextFileEncoding encoding)
    {
    WString         wOption;
    BeFileAccess    beFileAccess = (TextFileOpenType::Read == openType) ? BeFileAccess::Read : (TextFileOpenType::Append == openType) ? BeFileAccess::ReadWrite : BeFileAccess::Write;

    // save file spec and openType in case we need them later.
    m_fileSpec  = fullFileSpec;
    m_openType  = openType;
    m_options   = options;
    m_file      = new BeFile();

    BeFileStatus    status;
    if (BeFileAccess::Write == beFileAccess)
        status = m_file->Create (fullFileSpec);
    else
        status = m_file->Open (fullFileSpec, beFileAccess);

    if (BeFileStatus::Success != status)
        {
        // if we are opening for append, and we could not find the file, create it.
        if (TextFileOpenType::Read != openType)
            {
            if (BeFileStatus::Success != (status = m_file->Create (fullFileSpec)))
                return status;
            // if we created the file, its Write, not Append.
            openType = TextFileOpenType::Write;
            }
        else
            return status;
        }

    m_encoding  = ReadFileEncoding (openType, encoding);

    // if we're appending, set file pointer to the end of the file.
    if (TextFileOpenType::Append == openType)
        {
        m_file->SetPointer (0, BeFileSeekOrigin::End);
        uint64_t    currFilePos = 0;
        if (BeFileStatus::Success == m_file->GetPointer (currFilePos))
            m_atStart = (0 == currFilePos);
        }

    return BeFileStatus::Success;
    }

static Byte s_Utf8BOM[]     = {0xef, 0xbb, 0xbf};
static Byte s_Utf16BOM[]    = {0xff, 0xfe};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
TextFileEncoding    BeTextFile::ReadFileEncoding (TextFileOpenType openType, TextFileEncoding proposedEncoding)
    {
    m_startOfTextData = 0;

    if (TextFileOpenType::Write == openType)
        return proposedEncoding;

    // see if the first few bytes are a BOM indicating UTF-8 or UTF-16.
    Byte    signatureByte1 = ByteFromBuffer();
    if ( (0 == signatureByte1) && m_noData)
        {
        return proposedEncoding;
        }
    else if (s_Utf16BOM[0] == signatureByte1)
        {
        // UTF-16 is indicated by 0xff, 0xfe.
        if (s_Utf16BOM[1] == ByteFromBuffer())
            {
            m_startOfTextData = 2;
            return TextFileEncoding::Utf16;
            }
        }
    else if (s_Utf8BOM[0]== signatureByte1)
        {
        // UTF-8 is indicated by 0xef, 0xbb, 0xbf
        if ( (s_Utf8BOM[1] == ByteFromBuffer()) && (s_Utf8BOM[2] == ByteFromBuffer()) )
            {
            m_startOfTextData = 3;
            return TextFileEncoding::Utf8;
            }
        }

    // it's not UTF-16 or UTF-8 - go back to beinning of file, and return CurrentLocale.
    ResetToFileStart();
    return TextFileEncoding::CurrentLocale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
TextFileReadStatus  BeTextFile::GetLine (WStringR textLine)
    {
    if (m_noData)
        return TextFileReadStatus::Eof;

    textLine.clear();

    if (TextFileEncoding::CurrentLocale == m_encoding)
        {
        // Here we are reading multibyte sequences. However, according to Microsoft, "you can safely use bytewise scans for any control characters (less than 32)."
        AString         workString;
        for (;;)
            {
            Byte    thisByte = ByteFromBuffer();
            if (0 == thisByte)
                break;

            // treat any of \n, \r, \n\r, or \r\n as a new line.
            if ('\r' == thisByte || '\n' == thisByte)
                {
                Byte    nextByte = ByteFromBuffer();
                Byte    theOtherOne = '\r' == thisByte ? '\n' : '\r';

                // only eat nextByte if the sequence is \n\r or \r\n.  For example, if \n is followed
                // by any character other than \r, put that back so it can be processed as part of the
                // next line
                if ( (0 != nextByte) && (theOtherOne != nextByte) )
                    RestoreByteToBuffer (nextByte);

                if (0 != (static_cast<int>(m_options) & static_cast<int>(TextFileOptions::KeepNewLine)))
                    {
                    if (0 != (static_cast<int>(m_options) & static_cast<int>(TextFileOptions::NewLinesToSpace)))
                        thisByte = ' ';

                    workString.append (1, thisByte);
                    }
                break;
                }
            workString.append (1, thisByte);
            }

        BeStringUtilities::CurrentLocaleCharToWChar (textLine, workString.c_str());
        if (textLine.empty() && m_noData)
            return TextFileReadStatus::Eof;

        return TextFileReadStatus::Success;
        }

    else if (TextFileEncoding::Utf8 == m_encoding)
        {
        // with UTF-8, \r can never be part of a multibyte sequence, so we can just build up an UTF8-string until we hit one.
        Utf8String     workString;
        for (;;)
            {
            Byte    thisByte = ByteFromBuffer();
            if (0 == thisByte)
                break;

            if ('\r' == thisByte)
                {
                Byte    nextByte = ByteFromBuffer();
                if ( (0 != nextByte) && ('\n' != nextByte) )
                    RestoreByteToBuffer (nextByte);

                if (0 != (static_cast<int>(m_options) & static_cast<int>(TextFileOptions::KeepNewLine)))
                    {
                    if (0 != (static_cast<int>(m_options) & static_cast<int>(TextFileOptions::NewLinesToSpace)))
                        thisByte = ' ';
                    workString.append (1, thisByte);
                    }
                break;
                }
            workString.append (1, thisByte);
            }

        BeStringUtilities::Utf8ToWChar (textLine, workString.c_str());
        if (textLine.empty() && m_noData)
            return TextFileReadStatus::Eof;

        return TextFileReadStatus::Success;
        }

    else if (TextFileEncoding::Utf16 == m_encoding)
        {
        // with UTF-16, we can just look for \r.
        for (;;)
            {
            uint16_t  thisChar = UInt16FromBuffer();
            if (0 == thisChar)
                {
                if (textLine.empty() && m_noData)
                    return TextFileReadStatus::Eof;

                return TextFileReadStatus::Success;
                }

            if ('\r' == thisChar)
                {
                // if next char is a return, discard it.
                uint16_t nextChar = UInt16FromBuffer();
                if ( (0 != nextChar) && ('\n' != nextChar) )
                    RestoreUInt16ToBuffer (nextChar);

                if (0 != (static_cast<int>(m_options) & static_cast<int>(TextFileOptions::KeepNewLine)))
                    {
                    if (0 != (static_cast<int>(m_options) & static_cast<int>(TextFileOptions::NewLinesToSpace)))
                        thisChar = ' ';
                    }
                else
                    {
                    return TextFileReadStatus::Success;
                    }
                }

            textLine.append (1, thisChar);
            }
        }
    else
        {
        BeAssert (false);
        return TextFileReadStatus::BadParameter;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int      ByteCountFollowingUtf8 (Byte firstByte)
    {
    static Byte masks[]      = { 0x80, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe }; 
    static Byte signatures[] = { 0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc };
    for (int iBytesFollow = 0; iBytesFollow < _countof (masks); iBytesFollow++)
        {
        if ( (firstByte & masks[iBytesFollow]) == signatures[iBytesFollow])
            return iBytesFollow;
        }
    // should always match one of these, or we have a problem!
    BeAssert (false);
    return 0;
    }


#if !defined (BENTLEY_WIN32)
// found this on the internet. Used for other than Windows. This is probably not right for some locales,
// the Microsoft implementation is more complicated because of Locale.

#define _MBC_SINGLE     0
#define _MBC_LEAD       1
#define _MBC_TRAIL      2
#define _MBC_ILLEGAL    (-1)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int _mbbtype(unsigned char c, int type)
    {
        if (type == 1)
        {
            if ((c >= 0x20 && c <= 0x7e) || (c >= 0xa1 && c <= 0xdf))
                return _MBC_SINGLE;
            else if ((c >= 0x40 && c <= 0x7e) || (c >= 0x80 && c <= 0xfc))
                return _MBC_TRAIL;
            else
                return _MBC_ILLEGAL;
        }
        else
        {
            if ((c >= 0x20 && c <= 0x7e) || (c >= 0xa1 && c <= 0xdf))
                return _MBC_SINGLE;
            else if ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc))
                return _MBC_LEAD;
            else
                return _MBC_ILLEGAL;
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
WChar           BeTextFile::GetChar ()
    {
    if (m_noData)
        return WEOF;

    if (TextFileEncoding::CurrentLocale == m_encoding)
        {
        // Here we are reading multibyte sequences. However, according to Microsoft, "you can safely use bytewise scans for any control characters (less than 32)."
        Byte    mbString[3];
        mbString[1] = mbString[2] = 0;

        mbString[0] = ByteFromBuffer();
        if (0 == mbString[0])
            return WEOF;

        // determine whether the byte we just got is the start of a multibyte character.
        int byteType = _mbbtype (mbString[0], 0);
        if (_MBC_LEAD == byteType)
            {
            // current byte starts a multibyte. Get the next byte, which has to be a trail byte.
            mbString[1] = ByteFromBuffer();
            if (_MBC_TRAIL != _mbbtype (mbString[1], 1))
                {
                BeAssert (false);
                return WEOF;
                }
            }
        else if (_MBC_SINGLE == byteType)
            {
            // if it's less than 128, then it's ascii and no code page translation is necessary.
            if (mbString[0] < 0x80)
                return mbString[0];
            }
        else if (_MBC_ILLEGAL == byteType)
            {
            // this value happens for control characters.
            return mbString[0];
            }
        else
            {
            // not single, illegal, or lead. Something bad has happened.
            BeAssert (false);
            return 0;
            }

        // convert to unicode.
        WChar   unicodeString[2];
        BeStringUtilities::CurrentLocaleCharToWChar (unicodeString, (CharCP) mbString, _countof (unicodeString));
        return  unicodeString[0];
        }

    else if (TextFileEncoding::Utf8 == m_encoding)
        {
        // with UTF-8, you can tell from the first character how many characters follow.
        Utf8Char            utf8String[8];
        memset (utf8String, 0, sizeof (utf8String));

        utf8String[0] = ByteFromBuffer();
        if (0 == utf8String[0])
            return WEOF;

        // determine whether the byte we just got is the start of a multibyte character.
        int bytesToFollow = ByteCountFollowingUtf8 (utf8String[0]);
        if (bytesToFollow < 1)
            return utf8String[0];

        for (int iFollowing=1; iFollowing <= bytesToFollow; iFollowing++)
            {
            if (m_noData)
                {
                BeAssert (false);
                return WEOF;
                }

            utf8String[iFollowing] = ByteFromBuffer();
            }

        // convert to unicode.
        WChar   unicodeString[2];
        BeStringUtilities::Utf8ToWChar (unicodeString, utf8String, _countof (unicodeString));
        return  unicodeString[0];
        }

    else if (TextFileEncoding::Utf16 == m_encoding)
        {
        // Note: this isn't completely right if the characters aren't in the 16-bit base unicode set.
        uint16_t  thisChar;
        return (0 == (thisChar = UInt16FromBuffer())) ? WEOF : thisChar;
        }
    else
        {
        BeAssert (false);
        return WEOF;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
Byte                BeTextFile::ByteFromBuffer ()
    {
    if (m_currentByte < m_endBuffer)
        return *m_currentByte++;

    else if (!m_reachedEnd)
        {
        uint32_t    bytesRead;
        m_file->Read (m_readBuffer, &bytesRead, BUFFER_SIZE);
        m_currentByte = m_readBuffer;
        m_endBuffer = &m_readBuffer[bytesRead];
        if (bytesRead < BUFFER_SIZE)
            m_reachedEnd = true;
        if (0 == bytesRead)
            {
            m_noData = true;
            return 0;
            }
        else
            {
            return *m_currentByte++;
            }
        }
    else
        {
        // previously read to the end of the file, no more bytes in buffer, can't read more.
        m_noData = true;
        return 0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t            BeTextFile::UInt16FromBuffer ()
    {
    union
        {
        Byte        nextTwo[2];
        uint16_t    utf16Char;
        } u;

    u.nextTwo[0] = ByteFromBuffer();
    u.nextTwo[1] = ByteFromBuffer();
    if (m_noData)
        return 0;

    return u.utf16Char;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                BeTextFile::RestoreUInt16ToBuffer (uint16_t restore)
    {
    if (m_currentByte > &m_readBuffer[1])
        {
        m_currentByte -= 2;
        m_noData = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                BeTextFile::RestoreByteToBuffer (uint16_t restore)
    {
    if (m_currentByte > &m_readBuffer[0])
        {
        m_currentByte--;
        m_noData = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                BeTextFile::ResetToFileStart ()
    {
    m_noData            = false;
    m_reachedEnd        = false;
    m_currentByte       = m_readBuffer;
    m_endBuffer         = m_readBuffer;
    memset (&m_readBuffer, 0, sizeof (m_readBuffer));
    m_file->SetPointer (0, BeFileSeekOrigin::Begin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void                BeTextFile::WriteEncodingBOM ()
    {
    uint32_t      bytesWritten = 0;
    if (TextFileEncoding::Utf8 == m_encoding)
        {
        m_file->Write (&bytesWritten, s_Utf8BOM, sizeof (s_Utf8BOM));
        BeAssert (sizeof (s_Utf8BOM) == bytesWritten);
        }
    else if (TextFileEncoding::Utf16 == m_encoding)
        {
        m_file->Write (&bytesWritten, s_Utf16BOM, sizeof (s_Utf16BOM));
        BeAssert (sizeof (s_Utf16BOM) == bytesWritten);
        }

    // don't do it again.
    m_startOfTextData = bytesWritten;
    m_atStart         = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
TextFileWriteStatus BeTextFile::PutLine (WCharCP textLine, bool addCR)
    {
    if (m_atStart)
        WriteEncodingBOM ();

    uint32_t        bytesWritten;
    uint32_t        bytesToWrite;
    BeFileStatus    writeStatus;
    if (TextFileEncoding::CurrentLocale == m_encoding)
        {
        AString    currentLocaleString;
        BeStringUtilities::WCharToLocaleChar (currentLocaleString, LangCodePage::None, textLine);
        if (addCR)
            {
#if !defined (unix)
            currentLocaleString.append (1, '\r');
#endif
            currentLocaleString.append (1, '\n');
            }
        bytesToWrite = (uint32_t) currentLocaleString.size();
        writeStatus = m_file->Write (&bytesWritten, currentLocaleString.c_str(), bytesToWrite);
        }
    else if (TextFileEncoding::Utf8 == m_encoding)
        {
        Utf8String  utf8String;
        BeStringUtilities::WCharToUtf8 (utf8String, textLine);
        if (addCR)
            {
#if !defined (unix)
            utf8String.append (1, '\r');
#endif
            utf8String.append (1, '\n');
            }
        bytesToWrite = (uint32_t) utf8String.size();
        writeStatus = m_file->Write (&bytesWritten, utf8String.c_str(), bytesToWrite);
        }
    else if (TextFileEncoding::Utf16 == m_encoding)
        {
        bytesToWrite = (uint32_t) wcslen (textLine) * sizeof (uint16_t);
        writeStatus = m_file->Write (&bytesWritten, textLine, bytesToWrite);
        if (addCR)
#if !defined (unix)
            m_file->Write (NULL, L"\r\n", 4);
#else
            m_file->Write (NULL, L"\n", 2);
#endif
        }
    else
        {
        BeAssert (false);
        return TextFileWriteStatus::BadParameter;
        }

    if ( (BeFileStatus::Success != writeStatus) || (bytesWritten != bytesToWrite) )
        return TextFileWriteStatus::Error;

    return TextFileWriteStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus    BeTextFile::Rewind ()
    {
    return SetPointer (m_startOfTextData, BeFileSeekOrigin::Begin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus    BeTextFile::SetPointer (uint64_t pos, BeFileSeekOrigin origin)
    {
    BeFileStatus status = m_file->SetPointer (pos, origin);
    if (BeFileStatus::Success != status)
        return status;

    // Flush the buffer and reset state so we aren't looking at stale data.
    m_noData            = false;
    m_reachedEnd        = false;
    m_currentByte       = m_readBuffer;
    m_endBuffer         = m_readBuffer;
    
    if ((0 == pos) && (BeFileSeekOrigin::Begin == origin))
        m_atStart = true;

    return BeFileStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileStatus    BeTextFile::GetPointer (uint64_t& pos)
    {
    BeFileStatus fileStatus = m_file->GetPointer (pos);

    if (m_endBuffer > m_currentByte)
        pos -= m_endBuffer - m_currentByte;

    return fileStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/12
+---------------+---------------+---------------+---------------+---------------+------*/
TextFileWriteStatus BeTextFile::PrintfTo (bool toStdOutAlso, WCharCP format, ...)
    {
    WChar       message[4096];
    va_list     ap;

    va_start (ap, format);
    BeStringUtilities::Vsnwprintf (message, _countof(message), format, ap);
    va_end (ap);

    // replace \n with \r\n, or the files will not open in notepad correctly.
    WChar       processedMessage[4300];
    WChar       previousChar = 0;
    WCharCP     pIn;
    WCharP      pOut;
    for (pIn = message, pOut = processedMessage; (0 != *pIn) && (pOut < &processedMessage[_countof(processedMessage)-1]);  pIn++, pOut++)
        {
        if ( (*pIn == '\n') && (previousChar != '\r') )
            *pOut++ = '\r';
        *pOut = (previousChar = *pIn);
        }
    // NULL terminate
    *pOut = 0;

    if (toStdOutAlso)
        wprintf (processedMessage);

    return PutLine (processedMessage, false);
    }
