/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeTextFile.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

//! @file BeTextFile.h Provides %Bentley specific text file read/write functions (Bentley/BeAssert.h).

#include    <Bentley\BeFile.h>
#include    <Bentley\RefCounted.h>

BEGIN_BENTLEY_NAMESPACE

//! @addtogroup BentleyLibrary
//! @beginGroup

//! The possible BeTextFile open modes.
enum class TextFileOpenType
    {
    Read   = 0,      //!< Open for read access.
    Write  = 1,      //!< Open for write access.
    Append = 2,      //!< Open for both read and write.
    };

//! The possible BeTextFile open modes.
enum class TextFileEncoding
    {
    CurrentLocale    = 0,      //!< Write current locale multibyte output file.
    Utf8             = 1,      //!< Write UTF-8 encoded output file.
    Utf16            = 2,      //!< Write little-endian UTF-16 encoded output file.
    };

//! The possible options for text file reading.
enum class TextFileOptions
    {
    None             = 0,      //!< Use the default newline handling.
    KeepNewLine      = 0x1,    //!< relevant only for getString, default behavior is to return the line without the newlines.
    NewLinesToSpace  = 0x5,    //!< relevant only for getString, also turns on KeepNewLine.
    };

//! The possible return values for text file reads.
enum class TextFileReadStatus
    {
    Success            = 0,      //!< Successful read 
    Eof,                         //!< Encountered the end of the file.
    BadParameter,                //!< A bad argument was passed to the method.
    };

//! The possible return values for text file writes.
enum class TextFileWriteStatus
    {
    Success           = 0,      //!< Successful write.
    Error,                      //!< An I/O error occurred while attempting to write.
    BadParameter,               //!< A bad argument was passed to the method.
    };

//! A reference counted pointer to a BeTextFile instance.
typedef RefCountedPtr<struct BeTextFile>  BeTextFilePtr;

/*=================================================================================**//**
* Reads and Writes Text Files regardless of whether they are encoded on disk as locale-encoded ASCII, UTF8, or UTF16.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  BeTextFile : public RefCountedBase
    {
    static const uint32_t BUFFER_SIZE = 100;
private:
    WString             m_fileSpec;
    TextFileOpenType    m_openType;
    BeFile*             m_file;
    TextFileEncoding    m_encoding;
    TextFileOptions     m_options;
    mbstate_t           m_mbState;
    uint32_t            m_startOfTextData;

    bool                m_reachedEnd;
    bool                m_noData;
    bool                m_atStart;
    Byte*               m_currentByte;
    Byte*               m_endBuffer;
    Byte                m_readBuffer[BUFFER_SIZE+2];  // used only for reading. 2 extra bytes are to make sure we can push a byte or WChar back into the buffer.

    BeFileStatus        ReadBuffer ();
    Byte                ByteFromBuffer ();
    uint16_t            UInt16FromBuffer ();
    TextFileEncoding    ReadFileEncoding (TextFileOpenType openType, TextFileEncoding proposedEncoding);
    void                ResetToFileStart ();
    void                RestoreByteToBuffer (uint16_t restore);
    void                RestoreUInt16ToBuffer (uint16_t restore);
    void                WriteEncodingBOM ();

protected:
    BENTLEYDLL_EXPORT   BeTextFile();
    BENTLEYDLL_EXPORT   ~BeTextFile();
    //! Internal method for opening a file.  Use the factory method BeTextFile::Open instead.
    BENTLEYDLL_EXPORT   BeFileStatus    OpenFile (WCharCP fullFileSpec, TextFileOpenType openType, TextFileOptions options, TextFileEncoding encoding);

public:
    //! Opens a text file for reading or writing.
    //! @param[out] status           BeFileStatus::Success on success or the file open error.
    //! @param[in]  fullFileSpec     Name of the file to open.
    //! @param[in]  openType         Whether to open the file for read, write or append.
    //! @param[in]  options          Options controlling how newline characters are treated on read.
    //! @param[in]  encoding         Encoding for the file; use TextFileEncoding::CurrentLocale for ASCII. This parameter is used only when openType is TextFileOpenType::Write.
    //! @remarks When opened for read or append, the text file encoding will be read from the beginning of the file (if it is there) and the contents will be read and converted from its encoding to Unicode.
    //! @remarks When opened for write, the \a encoding argument indicates how the output is to be written. If there is an existing file with the same name, that file is replaced by a new file.
    //! @return A pointer to the file.  If status is not BeFileStatus::Success then the pointer will fail the IsValid() check.
    BENTLEYDLL_EXPORT     static BeTextFilePtr    Open (BeFileStatus& status, WCharCP fullFileSpec, TextFileOpenType openType, TextFileOptions options, TextFileEncoding encoding = TextFileEncoding::CurrentLocale);

    //! Closes the text file.  
    //! @remarks This call is not usually necessary, as the file is closed automatically when the reference-counted pointer is released.
    BENTLEYDLL_EXPORT     void                    Close();

    //! Reads a line from the file.  
    //! @param[out] textLine     The buffer to read the line into.
    //! @remarks The line is always converted to Unicode in UTF-16.
    //! @remarks The handling of newlines is controlled by the options when opening the file.
    //! @return TextFileReadStatus::Success if the line was read successfully; otherwise TextFileReadStatus::Eof or an error code.
    BENTLEYDLL_EXPORT     TextFileReadStatus      GetLine (WStringR textLine);

    //! Writes a line to the file.
    //! @param[in] textLine             The buffer to read the line into.
    //! @param[in] addCarriageReturn    If true, appends a newline character to the input \a textLine.
    //! @remarks The data will be converted to the encoding specified when opening the file.
    //! @return TextFileWriteStatus::Success if the line was written successfully; otherwise an error code.
    BENTLEYDLL_EXPORT     TextFileWriteStatus     PutLine (WCharCP textLine, bool addCarriageReturn);

    //! Reads the next character from the file.
    //! @return The next character or WEOF.
    BENTLEYDLL_EXPORT     WChar                   GetChar ();

    //! Writes formatted output to the file.
    //! @param[in] toStdOutAlso         True to also print the information to stdout.  
    //! @param[in] format               The format string; see printf for the formats.  
    //! @remarks The format string is the same as would be used in a call to the standard C wprintf function.
    BENTLEYDLL_EXPORT     TextFileWriteStatus     PrintfTo (bool toStdOutAlso, WCharCP format, ...);


    //! Rewinds the file and prepare for reading from the beginning of the text data (i.e., just past the encoding signature, if there is one).
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus    Rewind ();

    //! Moves the file read/write position
    //! @param[in] position The new read/write position.
    //! @param[in] origin   The origin mode for the move.
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    //! @remarks Use Rewind to go to the beginning of rthe text data (rather than SetPointer (0, BeFileSeekOrigin::Begin) 
    //!          because Rewind accounts for the encoding signature that might be at the beginning of the file.
    BENTLEYDLL_EXPORT BeFileStatus SetPointer (uint64_t position, BeFileSeekOrigin origin);

    //! Gets the file read/write position.
    //! @param[out] position      the current read/write position
    //! @return BeFileStatus::Success if the operation was successful or non-zero if it failed.
    BENTLEYDLL_EXPORT BeFileStatus GetPointer (uint64_t& position);

    };

//! @endGroup

END_BENTLEY_NAMESPACE
