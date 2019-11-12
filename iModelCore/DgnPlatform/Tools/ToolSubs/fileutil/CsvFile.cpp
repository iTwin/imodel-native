/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DesktopTools/CsvFile.h>
#include    <string.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>


BEGIN_BENTLEY_DGN_NAMESPACE

/*----------------------------------------------------------------------+
|                                                                       |
|   Local defines                                                       |
|                                                                       |
+----------------------------------------------------------------------*/
#define     DOUBLE_QUOTE_CHAR       '"'
#define     LEFT_DOUBLE_QUOTE_CHAR       0x201C
#define     RIGHT_DOUBLE_QUOTE_CHAR      0x201D

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
CsvFile::CsvFile ()
    {
    m_delimiter = ',';
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
CsvFile::~CsvFile ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
CsvFilePtr      CsvFile::Open (BeFileStatus& openStatus, WCharCP fullFileSpec)
    {
    CsvFilePtr  csvFile = new CsvFile();

    if (BeFileStatus::Success != (openStatus = csvFile->OpenFile (fullFileSpec, TextFileOpenType::Read, TextFileOptions::None, TextFileEncoding::CurrentLocale)))
        return NULL;

    return csvFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CsvFile::IsCommentOrBlankLine (WStringCR line)
    {
    // find first non-whitespace.
    size_t  iPos;
    for (iPos=0; iPos < line.length(); iPos++)
        {
        if (!iswspace (line[iPos]))
            break;
        }
    if (iPos >= line.length())
        return true;

    return (iPos == line.find (L"%COMMENT", iPos));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool            CsvFile::IsSectionHeader (WStringCR line)
    {
    // skip past initial whitespace.
    size_t  iPos;
    for (iPos=0; iPos < line.length(); iPos++)
        {
        if (!iswspace (line[iPos]))
            break;
        }
    // must start with %SECTION, and should not be case sensitive
    WString tempLine(line);
    tempLine.ToUpper();
    return (iPos == tempLine.find (L"%SECTION", iPos));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
TextFileReadStatus  CsvFile::GetLine (WStringR line, bool stopAtSectionHeader)
    {
    TextFileReadStatus  status;
    while (TextFileReadStatus::Success == (status = BeTextFile::GetLine (line)))
        {
        if (IsCommentOrBlankLine (line))
            continue;

        // if we hit another section header without encountering a column label line, didn't find it.
        if (stopAtSectionHeader && IsSectionHeader (line))
            {
            line.clear();
            return TextFileReadStatus::Eof;
            }

        break;
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            CsvFile::SetDelimiter (WChar delimiter)
    {
    m_delimiter = delimiter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Output, typename T_Input> void separateToken (T_Output& tokenOut, T_Input& stringIn, bool copyInternalDoubleQuotesIn, WChar delimiter)
    {
    tokenOut.clear();
    stringIn.Trim();

    if (stringIn.empty())
        return;

    WCharCP     pInString = stringIn.c_str();
    WCharCP     pEndPtr   = pInString + stringIn.length();

    bool        isFirstChar = true;
    bool        inDoubleQuotes = false;
    bool        inInternalDoubleQuotes = false;
    while (pInString < pEndPtr)
        {
        WCharCP pCurrentChar = pInString;
        pInString++;

        if (inDoubleQuotes)
            {
            if ( (DOUBLE_QUOTE_CHAR == *pCurrentChar) || (RIGHT_DOUBLE_QUOTE_CHAR == *pCurrentChar) )
                {
                if (DOUBLE_QUOTE_CHAR == *pInString)    /* If the next character is also a DOUBLE_QUOTE_CHAR */
                    {
                    tokenOut.append (1, *pCurrentChar);
                    pInString++;
                    }
                else
                    {
                    inDoubleQuotes = false;
                    if (inInternalDoubleQuotes)
                        {
                        inInternalDoubleQuotes = false;
                        tokenOut.append (1, *pCurrentChar);
                        }
                    }
                }
            else
                {
                tokenOut.append (1, *pCurrentChar);
                }
            }
        else if ( (DOUBLE_QUOTE_CHAR == *pCurrentChar) || (LEFT_DOUBLE_QUOTE_CHAR == *pCurrentChar) )
            {
            // if there are two quotes in a row, don't set inDoubleQuotes, just copy one of the quotes to the output.
            if (DOUBLE_QUOTE_CHAR == *pInString)
                {
                tokenOut.append (1, *pCurrentChar);
                pInString++;
                }
            else
                {
                inDoubleQuotes = true;
                if (copyInternalDoubleQuotesIn && !isFirstChar)
                    {
                    tokenOut.append (1, *pCurrentChar);
                    inInternalDoubleQuotes = true;
                    }
                }
            }
        else if (delimiter == *pCurrentChar)
            {
            break;
            }
        else
            tokenOut.append (1, *pCurrentChar);

        isFirstChar = false;
        }

    // erase up to our current position from the input string.
    stringIn.erase (0, pInString - stringIn.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            CsvFile::SeparateToken (WStringR tokenOut, WStringR stringIn, bool copyInternalDoubleQuotesIn, WChar delimiter)
    {
    separateToken (tokenOut, stringIn, copyInternalDoubleQuotesIn, delimiter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            CsvFile::SeparateToken (WStringR tokenOut, WStringR stringIn, bool copyInternalDoubleQuotesIn)
    {
    SeparateToken (tokenOut, stringIn, copyInternalDoubleQuotesIn, m_delimiter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void    CsvFile::BuildToken
(
WStringR    tokenOut,   /* OUT tokenized string */
WCharCP     valueIn,    /* => IN string */
WChar       separator   /* => IN field separator character */
)
    {
    tokenOut.clear();

    if ( (NULL == valueIn) || (0 == *valueIn) )
        return;

    WCharCP currentChar = valueIn;
    bool    needQuotes = false;

    if (NULL != ::wcschr (valueIn, separator))
        needQuotes = true;

    if (needQuotes)
        tokenOut.append (1, DOUBLE_QUOTE_CHAR);

    while (0 != *currentChar)
        {
        // need to turn " into "".
        tokenOut.append (1, *currentChar);

        if (DOUBLE_QUOTE_CHAR == *currentChar)
            tokenOut.append (1, *currentChar);
        currentChar++;
        }

    if (needQuotes)
        tokenOut.append (1, DOUBLE_QUOTE_CHAR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CsvFile::FindSection (uint64_t& filePos, WStringP sectionHeader, WCharCP sectionName)
    {
    WString rowString;

    filePos = 0;

    // always start at beginning of file.
    SetPointer (0, BeFileSeekOrigin::Begin);

    // Look for section header.
    while (TextFileReadStatus::Success == GetLine (rowString, false))
        {
        if (NULL != sectionHeader)
            sectionHeader->assign (rowString);

        rowString.Trim();

        if (rowString.empty())
            continue;

        WString firstToken;
        SeparateToken (firstToken, rowString, false);

        if (0 == firstToken.CompareToI (L"%SECTION"))
            {
            // we want the position after the SECTION line, which is where it currently is.
            GetPointer (filePos);

            WString sectionNameToken;
            SeparateToken (sectionNameToken, rowString, false);
            if (0 == sectionNameToken.CompareToI (sectionName))
                return SUCCESS;
            }
        }

    if (NULL != sectionHeader)
        sectionHeader->clear();

    return CSVERR_NOSECTION;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CsvFile::GetColumnNames (T_WStringVector& columnNames, uint64_t sectionFilePos)
    {
    columnNames.clear();
    SetPointer (sectionFilePos, BeFileSeekOrigin::Begin);

    WString rowString;
    if (TextFileReadStatus::Success != GetLine (rowString, true))
        return CSVERR_BADSTARTPOS;

    WString columnToken;
    while (!rowString.empty())
        {
        SeparateToken (columnToken, rowString, false);
        columnNames.push_back (columnToken);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CsvFile::GetColumnContentsFromSection (T_WStringVector& columnContents, uint64_t sectionFilePos, WCharCP columnName)
    {
    columnContents.clear();
    SetPointer (sectionFilePos, BeFileSeekOrigin::Begin);

    // Read section header to find column */
    WString rowString;
    if (TextFileReadStatus::Success != GetLine (rowString, true))
        return CSVERR_NOSECTION;

    int     columnNum = 0;
    bool    bColumnExists(false);
    WString token;
    while (!rowString.empty())
        {
        SeparateToken (token, rowString, false);

        if (0 == token.CompareToI (columnName))
            {
            bColumnExists = true;
            break;
            }
        columnNum++;
        }

    if (!bColumnExists)
        return CSVERR_NOCOLUMN;

    // read data rows, extract the column.
    while (TextFileReadStatus::Success == GetLine (rowString, true))
        {
        int iColumn;

        for (iColumn = 0; iColumn <= columnNum; iColumn++)
            {
            if (rowString.empty())
                {
                token.clear();
                break;
                }
            else
                {
                SeparateToken (token, rowString, false);
                }
            }
        columnContents.push_back (token);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CsvFile::GetColumnContentsFromSection (T_WStringVector& columnContents, WCharCP sectionName, WCharCP columnName)
    {
    uint64_t      sectionFilePos;
    StatusInt   status;
    if (SUCCESS != (status = FindSection (sectionFilePos, NULL, sectionName)))
        return status;

    return GetColumnContentsFromSection (columnContents, sectionFilePos, columnName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       CsvFile::GetSections (CsvSectionInfoVector& sectionNames)
    {
    // read data rows, extract the column.
    WString rowString;
    while (TextFileReadStatus::Success == GetLine (rowString, false))
        {
        rowString.Trim();

        if (rowString.empty())
            continue;

        WString firstToken;
        SeparateToken (firstToken, rowString, false);

        if (0 == firstToken.CompareToI (L"%SECTION"))
            {
            WString sectionNameToken;
            SeparateToken (sectionNameToken, rowString, false);
            if (!sectionNameToken.empty())
                {
                uint64_t  filePos;
                GetPointer (filePos);
                CsvSectionInfo info (sectionNameToken, filePos);
                sectionNames.push_back (info);
                }
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Deepak.Malkan                   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
void            CsvFile::ConvertStringToWStrings (bvector<WString>& vector, WStringR stringIn, WChar separatorCharIn, bool copyInternalDoubleQuotesIn)
    {
    WString token;
    while (!stringIn.empty())
        {
        SeparateToken (token, stringIn, copyInternalDoubleQuotesIn, separatorCharIn);
        vector.push_back (token);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        CsvFile::CountTokens (WStringR stringIn, WChar separatorCharIn, bool copyInternalDoubleQuotesIn)
    {
    // avoid unnecessary string copying/modification, we're only counting...
    struct TokenConsumer
        {
        void        clear() const { }
        void        append (size_t n, WChar const& c) const { }
        };

    struct LineAdapter
        {
    private:
        WCharCP         m_begin;
        WCharCP         m_end;
    public:
        LineAdapter (WStringCR str) : m_begin(str.begin()), m_end(str.end()) { }

        size_t  length() const                  { return m_end - m_begin; }
        bool    empty() const                   { return m_end <= m_begin; }
        WCharCP c_str() const                   { return m_begin; }
        void    erase (size_t pos, size_t n)    { BeAssert(0 == pos); m_begin += n; }
        };

    TokenConsumer token;
    uint32_t count = 0;
    while (!stringIn.empty())
        {
        separateToken (token, stringIn, copyInternalDoubleQuotesIn, separatorCharIn);
        ++count;
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void            CsvFile::GetRowAndMaxColumnCount (uint32_t& rowCount, uint32_t& colCount, bool copyInternalDoubleQuotesIn)
    {
    rowCount = colCount = 0;

    // GetPointer() and GetLine() are non-const, so this method is too...but let's set the file pointer back where it was when we're done anyway.
    uint64_t savedFilePos;
    GetPointer (savedFilePos);
    SetPointer (0, BeFileSeekOrigin::Begin);

    WString rowString;
    while (TextFileReadStatus::Success == GetLine (rowString, false))
        {
        if (IsSectionHeader (rowString))
            continue;

        uint32_t nCols = CountTokens (rowString, m_delimiter, copyInternalDoubleQuotesIn);
        if (nCols > colCount)
            colCount = nCols;

        ++rowCount;
        }

    SetPointer (savedFilePos, BeFileSeekOrigin::Begin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
CsvSectionInfo::CsvSectionInfo (WStringCR name, uint64_t filePos)
    {
    m_name = name;
    m_filePos = filePos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
CsvSectionInfo*    CsvSectionInfoVector::FindSection (WCharCP name)
    {
    for (CsvSectionInfoVector::iterator it = begin(); it != end(); it++)
        {
        CsvSectionInfo&  sectionInfo = *it;
        if (0 == sectionInfo.m_name.CompareToI (name))
            return &sectionInfo;
        }
    return NULL;
    }

END_BENTLEY_DGN_NAMESPACE
