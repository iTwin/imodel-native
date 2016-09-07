/*----------------------------------------------------------------------+
|
|   $Source: PublicAPI/DgnPlatform/DesktopTools/CsvFile.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <DgnPlatform/DgnPlatform.h>
#include <Bentley/bvector.h>
#include <Bentley/BeTextFile.h>

DGNPLATFORM_TYPEDEFS(CsvFile)
DGNPLATFORM_REF_COUNTED_PTR(CsvFile)

BEGIN_BENTLEY_DGN_NAMESPACE
//__PUBLISH_SECTION_END__

enum CsvError
    {
    CSVERR_NOCOLUMN     = -4001,
    CSVERR_NOSECTION    = -4002,
    CSVERR_BADSTARTPOS  = -4003,
    };

struct  CsvSectionInfo
    {
    CsvSectionInfo (WStringCR name, uint64_t filePos);
    WString m_name;
    uint64_t  m_filePos;
    };

struct  CsvSectionInfoVector : public bvector<CsvSectionInfo>
    {
    DGNPLATFORM_EXPORT  CsvSectionInfo*  FindSection (WCharCP sectionName);
    };

/*__PUBLISH_SECTION_START__*/
/*=================================================================================**//**
* Read CSV Files. Works regardless of whether they are stored on disk as ASCII, UTF8, or UTF16.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  CsvFile : public BeTextFile
    {
//__PUBLISH_SECTION_END__
private:
    WChar                       m_delimiter;
    CsvFile();
    static bool                 IsCommentOrBlankLine (WStringCR in);
    static bool                 IsSectionHeader (WStringCR in);

protected:
    ~CsvFile();

/*__PUBLISH_SECTION_START__*/
public:
    //! Open a CSV file for reading.  Handle Locale, Utf8 or Utf16 encoding.
    //! @param status           BeFileStatus::Success on success or the file open error.
    //! @param fullFileSpec     Name of the file to open.
    //! @return A pointer to the file.  If status is not BeFileStatus::Success then the pointer will fail the IsValid() check.
    DGNPLATFORM_EXPORT  static CsvFilePtr     Open (BeFileStatus& status, WCharCP fullFileSpec);
//__PUBLISH_SECTION_END__

    //! Find a section in this CSV file, identified by the sectionName.
    //! @param filePos          The location of the section in the file.
    //! @param sectionHeader    Optionally returns the section header located from sectionName.
    //! @param sectionName      The name of the section to find.
    //! @param separator        The delimiter, usually ','.
    //! @return SUCCESS if the section is in the file.
    DGNPLATFORM_EXPORT  StatusInt   FindSection (uint64_t& filePos, WStringP sectionHeader, WCharCP sectionName);

#if 0
    //! Returns the strings in a column of the file. The input file should not separate sections within it.
    //! @param columnString     A string concatenating all the strings from each column, separated by separator.
    //! @param columnName       The name of the column, which we try to find in the first row read.
    //! @param startPos         The file position to start reading.
    //! @param separator        The delimiter, usually ','.
    //! @return SUCCESS if the section is in the file.
    DGNPLATFORM_EXPORT  StatusInt   GetColumnString (WStringR columnString, WCharCP columnName, uint64_t startPos=0);
#endif

    //! Returns the strings in a column of a section within this Csvfile, that starts at the given startPos.
    //! @param columnString     The strings in the identified column.
    //! @param columnName       The name of the column, which we try to find in the first row read.
    //! @param sectionFilePos   The file position to start reading, obtained using the FindSection method.
    //! @param separator        The delimiter, usually ','.
    //! @return SUCCESS if the section is in the file.
    DGNPLATFORM_EXPORT  StatusInt   GetColumnContentsFromSection (T_WStringVector& columnContents, uint64_t sectionFilePos, WCharCP columnName);

    //! Returns the strings in a column of a section within this CsvFile, identified by name.
    //! @param columnString     The strings in the identified column.
    //! @param sectionHeader    Optionally returns the section header located from sectionName.
    //! @param sectionName      The name of the section to find.
    //! @param columnName       The name of the column, which we try to find in the first row read.
    //! @param separator        The delimiter, usually ','.
    //! @return SUCCESS if the section is in the file.
    DGNPLATFORM_EXPORT  StatusInt   GetColumnContentsFromSection (T_WStringVector& columnContents, WCharCP sectionName, WCharCP columnName);

    //! Returns all the section names found in this CsvFile.
    //! @param sectionNames     The section names in the file.
    DGNPLATFORM_EXPORT  StatusInt   GetSections (CsvSectionInfoVector& sectionInfo);

    //! Returns all the column names found at startFilePos in this CsvFile.
    //! @param columnNames      The column names found in the row at the file position specified.
    //! @param sectionFilePos   The file position to start reading, obtained using the FindSection method.
    DGNPLATFORM_EXPORT  StatusInt   GetColumnNames (T_WStringVector& columnNames, uint64_t sectionFilePos=0);

    //! Returns the number of non-comment/section/blank rows and maximum number of columns in any such row
    //! @param rowCount         The number of rows found
    //! @param colCount         The largest number of columns found
    //! @param copyInternalDoubleQuotes If true, quotes in the interior of a token are retained.
    DGNPLATFORM_EXPORT void         GetRowAndMaxColumnCount (uint32_t& rowCount, uint32_t& colCount, bool copyInternalDoubleQuotesIn);

    //! Returns the current line in the CSV file, optionally stopping when a new section header is encountered.
    DGNPLATFORM_EXPORT  TextFileReadStatus  GetLine (WStringR outLine, bool stopAtSectionHeader);

    //! Sets the delimiter, in case something other than the default value of comma is needed.
    DGNPLATFORM_EXPORT  void        SetDelimiter (WChar delimiter);

    //! Divides a CSV delimited string into its first token and the remainder of the string.
    //! @param tokenOut              The first token (the portion before the first delimiter)
    //! @param stringIn              On input, the entire line, on ouput, the line with the first token and its trailing quote removed.
    //! @param copyInternalQuotes    If true, quotes in the interior of a token are retained.
    //! @param separator             The separator character, usually the comma character.
    DGNPLATFORM_EXPORT  static void SeparateToken (WStringR tokenOut, WStringR stringIn, bool copyInternalQuotes, WChar separator);

    //! Divides a CSV delimited string into its first token and the remainder of the string.
    //! @param tokenOut              The first token (the portion before the first delimiter)
    //! @param stringIn              On input, the entire line, on ouput, the line with the first token and its trailing quote removed.
    //! @param copyInternalQuotes    If true, quotes in the interior of a token are retained.
    DGNPLATFORM_EXPORT  void        SeparateToken (WStringR tokenOut, WStringR stringIn, bool copyInternalDoubleQuotesIn);

    //! Create a CSV token, inserting quotes if needed.
    //! @param tokenOut         The token, including quotes as needed.
    //! @param inValue          The string to be built into a token.
    //! @param separator        The separator character, usually the comma character.
    DGNPLATFORM_EXPORT  static void     BuildToken (WStringR tokenOut, WCharCP inValue, WChar separator);

    DGNPLATFORM_EXPORT  static void     ConvertStringToWStrings (bvector<WString>& vector, WStringR stringIn, WChar separatorCharIn, bool copyInternalDoubleQuotesIn);

    //! Return the number of tokens in the supplied CSV delimited string.
    DGNPLATFORM_EXPORT  static uint32_t   CountTokens (WStringR stringIn, WChar separatorCharIn, bool copyInternalDoubleQuotesIn);
/*__PUBLISH_SECTION_START__*/
    };

END_BENTLEY_DGN_NAMESPACE
