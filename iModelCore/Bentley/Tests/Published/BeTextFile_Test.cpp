/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeTextFile_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32)

#include <Bentley/BeTest.h>
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <Bentley/bvector.h>
#include <wchar.h>
#include <Bentley/BeTextFile.h>
USING_NAMESPACE_BENTLEY

struct BeTextFileTests : public testing::Test
    {
     protected:
        BeFileName m_fileAppendName;
        BeFileName m_fileWriteName;
        BeFileName m_fileReadName;

        BeTextFilePtr m_fileReadPtr;
        BeTextFilePtr m_fileWritePtr;
        BeTextFilePtr m_fileAppendPtr;
       
        bvector<TextFileEncoding> m_encodingsToTest;
        WCharCP m_line;
        AString m_lineToCurrentLocale;
        Utf8String m_lineToUtf8;
        Utf16Buffer m_lineToUtf16;
        
        //---------------------------------------------------------------------------------------
        //Initializes values
        // @bsimethod                                        Julija.Suboc                02/13
        //---------------------------------------------------------------------------------------
        void SetUp() 
            {
            BeFileName tmp;
            BeTest::GetHost().GetOutputRoot (tmp);
            
            m_fileReadName.AppendToPath(tmp.GetName());
            m_fileReadName.AppendToPath (L"Read.txt");

            m_fileWriteName.AppendToPath(tmp.GetName());
            m_fileWriteName.AppendToPath (L"Write.txt");

            m_fileAppendName.AppendToPath(tmp.GetName());
            m_fileAppendName.AppendToPath (L"Append.txt");
            
            m_fileReadPtr = NULL;
            m_fileWritePtr = NULL;
            m_fileAppendPtr = NULL;
            //Push encodings that tests should be run with
            m_encodingsToTest.push_back(TextFileEncoding::Utf16);
            m_encodingsToTest.push_back(TextFileEncoding::CurrentLocale);
            m_encodingsToTest.push_back(TextFileEncoding::Utf8);
            //test line
            m_line = L"Bentley Systems, IncorporatedBentley is the global leader dedicated to providing architects,"
                    L" engineers, constructors, and owner-operators with comprehensive architecture and engineering"
                    L" software solutions for sustaining infrastructure. Founded in 1984, Bentley has nearly 3,000"
                    L" colleagues in more than 45 countries, $500 million in annual revenues, and, since 2001,"
                    L" has invested more than $1 billion";
            }
        //---------------------------------------------------------------------------------------
        //Closes files
        // @bsimethod                                        Julija.Suboc                02/13
        //---------------------------------------------------------------------------------------
        void TearDown()
            {
            CloseFiles();
            }
        void CloseFiles()
            {
            if(m_fileReadPtr != NULL)
                m_fileReadPtr -> Close();
            if(m_fileWritePtr != NULL)
                m_fileWritePtr -> Close();
            if(m_fileAppendPtr != NULL)
                m_fileAppendPtr -> Close();
            }

        //---------------------------------------------------------------------------------------
        //Creates file for reading
        // @bsimethod                                        Julija.Suboc                02/13
        //---------------------------------------------------------------------------------------
        void ReadFileSetUp(TextFileEncoding encoding)
            {
            //Prepare Read File
            BeFileStatus status;
            //Create file
            m_fileReadPtr =    BeTextFile::Open(status, m_fileReadName.GetName(), TextFileOpenType::Write, TextFileOptions::None, encoding);
            //Check if file was opened
            ASSERT_TRUE(BeFileStatus::Success ==status)<<"Failed to open file as write";
            TextFileWriteStatus writeStatus;
            //Add ten new lines to file
            for(int i = 0; i < 10; i++)
                {
                writeStatus = m_fileReadPtr->PutLine(m_line, true);
                EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file";
                }
            m_fileReadPtr->Close();
            m_fileReadPtr = NULL;
            }
                
        //---------------------------------------------------------------------------------------
        //Creates file for appending
        //
        // @bsimethod                                        Julija.Suboc                02/13
        //--------------------------------------------------------------------------------------
        void AppendFileSetUp (TextFileEncoding encoding)
            {
            BeFileStatus status;
            //Write info to Append File
            m_fileAppendPtr =    BeTextFile::Open(status, m_fileAppendName.GetName(), TextFileOpenType::Write, TextFileOptions::None, encoding);
            ASSERT_TRUE(BeFileStatus::Success ==status)<<"Failed to open file as write";
            //Add one line to file
            WCharCP appendLine = L"File to append";
            TextFileWriteStatus writeStatus = m_fileAppendPtr->PutLine(appendLine, true);
            ASSERT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file";
            m_fileAppendPtr->Close();
            m_fileAppendPtr = NULL;
            }

        //---------------------------------------------------------------------------------------
        // Creates file for writing
        // 
        // @bsimethod                                        Julija.Suboc                02/13
        //---------------------------------------------------------------------------------------
        void WriteFileSetUp (TextFileEncoding encoding)
            {
            BeFileStatus status;
            //Re-create empty file for writing
            m_fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Write, TextFileOptions::None, encoding);
            ASSERT_TRUE(BeFileStatus::Success ==status)<<"Failed to open file as write";
            m_fileWritePtr->Close();
            m_fileWritePtr = NULL;
            }
    };

//---------------------------------------------------------------------------------------
//Opens file for reading and reads first line
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, BeTextFileOpenToRead)
    {
    while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        WriteFileSetUp(encoding);
        ReadFileSetUp(encoding);
        BeFileStatus status;
        m_fileReadPtr = BeTextFile::Open(status, m_fileReadName.GetName(), TextFileOpenType::Read, TextFileOptions::NewLinesToSpace, encoding);
        ASSERT_TRUE(BeFileStatus::Success ==status)<<"Failed to open file as read-only, used encoding: "<<(uint32_t)encoding;
        //Read from file
        WString line;
        TextFileReadStatus read_status;
        read_status =  m_fileReadPtr->GetLine(line);
        EXPECT_TRUE(TextFileReadStatus::Success ==read_status)<<"Failed to read line from file";
        WString line2 = L"Bentley Systems, IncorporatedBentley is the global leader dedicated to providing architects, engineers,"
                        L" constructors, and owner-operators with comprehensive architecture and engineering software solutions"
                        L" for sustaining infrastructure. Founded in 1984, Bentley has nearly 3,000 colleagues in more than 45"
                        L" countries, $500 million in annual revenues, and, since 2001, has invested more than $1 billion ";
        int c = line.CompareTo(line2);
        EXPECT_TRUE (c == 0)<<c<<"Line comparison failed (Issue 2191), used encoding: "<<(uint32_t)encoding; //<<endl<<line.c_str()<<line2.c_str();
        CloseFiles();
        }
    }

//---------------------------------------------------------------------------------------
// Opens file for writing and writes line, then opens it for reading and gets some chars from it
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, BeTextFileOpenToWrite)
    {
     while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        BeFileStatus status;
        //Open file
        m_fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Write, TextFileOptions::None, encoding);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file as write";
        //Write to file
        TextFileWriteStatus writeStatus;
        WCharCP fileLine =  L"Hi, I am new line!";
        writeStatus = m_fileWritePtr->PutLine(fileLine, true);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //Close file
        m_fileWritePtr->Close();
        m_fileWritePtr = NULL;
    
        //Open file as read and check first char
        m_fileReadPtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Read, TextFileOptions::NewLinesToSpace, encoding);
        //Check if file was opened
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file as read-only, used encoding: "<<(uint32_t)encoding;
        WChar charFromFile = m_fileReadPtr->GetChar();
        Utf16Char charExpected = 0;
        //Verify according used encoding
        if(encoding == TextFileEncoding::CurrentLocale )
            {
            BeStringUtilities::WCharToCurrentLocaleChar(m_lineToCurrentLocale, fileLine);
            EXPECT_EQ(m_lineToCurrentLocale.at(0), charFromFile)<<"First char does not match, used encoding :"<<(uint32_t)encoding;
            }
        else if (encoding == TextFileEncoding::Utf8)
                {
                BeStringUtilities::WCharToUtf8(m_lineToUtf8, fileLine);
                EXPECT_EQ(m_lineToUtf8.at(0), charFromFile)<<"First char does not match, used encoding :"<<(uint32_t)encoding;
                }
        else if (encoding == TextFileEncoding::Utf16)
                {
                BeStringUtilities::WCharToUtf16 (m_lineToUtf16, fileLine, BeStringUtilities::AsManyAsPossible);
                charExpected = m_lineToUtf16.front();
                EXPECT_EQ(charExpected, charFromFile)<<"First char does not match, used encoding :"<<(uint32_t)encoding;
                }
        //Read first char again
        status = m_fileReadPtr->Rewind();
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to prepare to read from first position again, used encoding: "<<(uint32_t)encoding;
        charFromFile = m_fileReadPtr->GetChar();
        //If encoding was utf 8 or utf 16 first char ir encoding, so need to read one more
        if(encoding == TextFileEncoding::CurrentLocale )
            EXPECT_EQ(m_lineToCurrentLocale.at(0), charFromFile)<<"Failed to rewind file with encoding: "<<(uint32_t)encoding;
        else if (encoding == TextFileEncoding::Utf8)
            {
            charFromFile = m_fileReadPtr->GetChar();
            EXPECT_EQ(m_lineToUtf8.at(0), charFromFile)<<"Failed to rewind file with encoding: "<<(uint32_t)encoding;
            }
        else if (encoding == TextFileEncoding::Utf16)
            {
            charFromFile = m_fileReadPtr->GetChar();
            EXPECT_EQ(charExpected, charFromFile)<<"Failed to rewind file with encoding: "<<(uint32_t)encoding;
            }
        //Get current position
        uint64_t currentPos;
        status = m_fileReadPtr->GetPointer(currentPos);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to get pointer, used encoding: "<<(uint32_t)encoding;
        //Move to another position in file
        BeFileSeekOrigin position = BeFileSeekOrigin::Current;
        uint64_t movePos = 4;
        status = m_fileReadPtr->SetPointer(movePos, position);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to set pointer";
        //Get new position
        uint64_t newPos;
        status = m_fileReadPtr->GetPointer(newPos);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to get pointer, used encoding: "<<(uint32_t)encoding;
        EXPECT_EQ((newPos-movePos), currentPos)<<"New position should be higher than old one by 7, used encoding: "<<(uint32_t)encoding;
        CloseFiles();
        }
    }

//---------------------------------------------------------------------------------------
//Tries to insert line to read only file
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, BeTextFileTryToWriteToReadOnlyFile)
    {
    while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        WriteFileSetUp(encoding);
        BeFileStatus status;
        //Open empty file
        try
            {
            m_fileWritePtr = BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Read, TextFileOptions::None, encoding);
            ASSERT_TRUE(BeFileStatus::Success ==status)<<"Failed to open file as read, used encoding: "<<(uint32_t)encoding;
            //Fail to write to file
            TextFileWriteStatus writeStatus;
            WCharCP fileLine =  L"gi, I am new line!";
            writeStatus = m_fileWritePtr->PutLine(fileLine, false);
            EXPECT_TRUE(TextFileWriteStatus::Success != writeStatus)<<"Should not be able to write to read-only file, used encoding: "<<(uint32_t)encoding;
        
            //Fail to printf directly to file
            writeStatus = m_fileWritePtr->PrintfTo(false, L"Characters: %lc %lc ", L'a', 65);
            EXPECT_TRUE(TextFileWriteStatus::Success != writeStatus)<<"Suceeded to write to read-only file, used encoding: "<<(uint32_t)encoding;
    
            //Verify that there is no line in file
            WString getLine;
            TextFileReadStatus read_status =  m_fileWritePtr->GetLine(getLine);
            EXPECT_TRUE(TextFileReadStatus::Eof ==read_status)<<"File should be empty, used encoding: "<<(uint32_t)encoding<<"   "<<getLine.c_str();
            CloseFiles();
            }
        catch(wchar_t const*) 
            {
            CloseFiles();
            }
        }
    }

//---------------------------------------------------------------------------------------
//Uses GetChar to get char while changing pointer position 
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, BeTextFileChangePointerAndGetChar)
    {
    TextFileEncoding encoding = TextFileEncoding::CurrentLocale;
    ReadFileSetUp(encoding);
    BeFileStatus status;
    m_fileReadPtr =    BeTextFile::Open(status, m_fileReadName .GetName(), TextFileOpenType::Read, TextFileOptions::None, 
                                        TextFileEncoding::CurrentLocale);
    EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file as read, used encoding: "<<(uint32_t)encoding;
    WChar firstChar = m_fileReadPtr->GetChar();
    //Set pointer to begining and read first char again
    BeFileSeekOrigin position = BeFileSeekOrigin::Begin;
    uint64_t movePos = 0;
    status = m_fileReadPtr->SetPointer(movePos, position);
    EXPECT_TRUE(BeFileStatus::Success == status);
    WChar firstCharWithPointer = m_fileReadPtr->GetChar();
    EXPECT_TRUE(firstChar == firstCharWithPointer)<<"First char was "<<(char)firstChar<<" not "<<(char)firstCharWithPointer<<
                                                    " Failed to  set pointer to begining, used encoding: "<<(uint32_t)encoding;
    //Rewind file and read first char again
    m_fileReadPtr->Rewind();
    WChar firstCharWithRewind = m_fileReadPtr->GetChar();
    EXPECT_TRUE(firstChar == firstCharWithRewind)<<"First char was "<<(char)firstChar<<" not "<<(char)firstCharWithRewind<<
                                                    " Failed to  Rewind() file, used encoding: "<<(uint32_t)encoding;
    //get second char
    m_fileReadPtr->Rewind();
    position = BeFileSeekOrigin::Current;
    movePos = 2;
    status = m_fileReadPtr->SetPointer(movePos, position);
    WChar secondChar = m_fileReadPtr->GetChar();
    EXPECT_EQ(110, secondChar)<<"Second char is n not "<<(char)secondChar<<" Failed to  Rewind() file, used encoding: "<<(uint32_t)encoding;
    //Set pointer to end and get WEOF while trying to read char
    position = BeFileSeekOrigin::End;
    movePos = 0;
    status = m_fileReadPtr->SetPointer(movePos, position);
    EXPECT_TRUE(BeFileStatus::Success == status);
    WChar WEOFChar = m_fileReadPtr->GetChar();
    EXPECT_TRUE(WEOFChar == WEOF)<<"It should be WEOF not "<<(char)WEOFChar<<" Failed to set pointer to end position, used encoding: "<<(uint32_t)encoding;
    CloseFiles();
    }

//---------------------------------------------------------------------------------------
//Write to file while changing pointer position.
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------

TEST_F(BeTextFileTests, BeTextFileChangePointerAndWriteToFile)
    {
    while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        //----------Test steps--------------------------
        BeFileStatus status;
        //Open file
        m_fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Write, TextFileOptions::None, encoding);
        //Check if file was opened
        ASSERT_TRUE(m_fileWritePtr != NULL)<<"Failed to open file as write, used encoding: "<<(uint32_t)encoding;
        EXPECT_TRUE(BeFileStatus::Success == status);
        //Write to file
        TextFileWriteStatus writeStatus;
        WCharCP fileLine =  L"gi, I am new line!";
        writeStatus = m_fileWritePtr->PutLine(fileLine, false);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //Set position to begining and write one letter
        BeFileSeekOrigin position = BeFileSeekOrigin::Begin;
        uint64_t movePos = 0;
        status = m_fileWritePtr->SetPointer(movePos, position);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to set pointer, used encoding: "<<(uint32_t)encoding;
        fileLine =  L"P";
        writeStatus = m_fileWritePtr->PutLine(fileLine, false);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //Change pointer for current position and rewrite part of the text
        position = BeFileSeekOrigin::Current;
        //UTF16 takes 2 bytes, while utf8 takes 1 byte
        if(encoding ==TextFileEncoding::Utf16)
            movePos = 16;
        else
            movePos = 8;
        status = m_fileWritePtr->SetPointer(movePos, position);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to set pointer, used encoding: "<<(uint32_t)encoding;
        fileLine =  L"corrected line!";
        writeStatus = m_fileWritePtr->PutLine(fileLine, false);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //Set position to begining and correct spelling of "Pi" to "Hi"
        status = m_fileWritePtr->Rewind();
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to prepare to read from first position again, used encoding: "<<(uint32_t)encoding;
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to set pointer, used encoding: "<<(uint32_t)encoding;
        fileLine =  L"H";
        writeStatus = m_fileWritePtr->PutLine(fileLine, false);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //add smile to the end
        position = BeFileSeekOrigin::End;
        movePos = 0;
        status = m_fileWritePtr->SetPointer(movePos, position);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to set pointer, used encoding: "<<(uint32_t)encoding;
        fileLine =  L":)";
        writeStatus = m_fileWritePtr->PutLine(fileLine, false);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //------------Verification-----------------------------------
        //Verify that file only contains "Hi, I am corrected line!:)" text
        status = m_fileWritePtr->Rewind();
        //If is used utf 8 or utf  16 encoding first symbol is used to store encoding
        if(encoding ==TextFileEncoding::Utf8 || encoding ==TextFileEncoding::Utf16)
            {
            m_fileWritePtr->GetChar();
            }
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to prepare to read from first position again, used encoding: "<<(uint32_t)encoding;
        WString getLine;
        TextFileReadStatus read_status;
        read_status =  m_fileWritePtr->GetLine(getLine);
        EXPECT_TRUE(TextFileReadStatus::Success ==read_status)<<"Failed to read line from file, used encoding: "<<(uint32_t)encoding;
        WString expectedLine = L"Hi, I am corrected line!:)";
        int c = getLine.CompareTo(expectedLine);
        EXPECT_TRUE (c == 0)<<"Line comparison failed, used encoding: "<<(uint32_t)encoding; //<<"  "<<getLine.c_str()<<"  "<<expectedLine.c_str();
        //Verify that there is no another line
        read_status =  m_fileWritePtr->GetLine(getLine);
        EXPECT_TRUE(TextFileReadStatus::Eof ==read_status)<<"File should contain only two lines, used encoding: "<<(uint32_t)encoding;
        CloseFiles();
        }
    }

//---------------------------------------------------------------------------------------
//Open file to append a line and appends 2 lines, first with "PutLine" and second with "PrintfTo"
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, BeTextFileWriteToFileWithPutAndPrintf)
    {
    while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        //-------Test Steps----------------------------
        BeFileStatus status;
        //Open file
        m_fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Write, TextFileOptions::KeepNewLine, encoding);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file to write, used encoding: "<<(uint32_t)encoding;
        //Add line to file
        TextFileWriteStatus writeStatus;
        WCharCP line = L"Hi, I am appended line!";
        writeStatus = m_fileWritePtr->PutLine(line, true);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //printf directly to file
        writeStatus = m_fileWritePtr->PrintfTo(false, L"Characters: %lc %lc ", L'a', 65);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        m_fileWritePtr->Close();
        //--------------Verification-------------------------
        //Check file, it should contain two lines
        m_fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Read, TextFileOptions::KeepNewLine, encoding);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file to write, used encoding: "<<(uint32_t)encoding;
        WString getLine;
        TextFileReadStatus read_status;
        read_status =  m_fileWritePtr->GetLine(getLine);
        ASSERT_TRUE(TextFileReadStatus::Success == read_status)<<"Failed to read line from file, used encoding: "<<(uint32_t)encoding;
        WString line2 = L"Hi, I am appended line!\n";
        int c = getLine.CompareTo(line2);
        EXPECT_TRUE (c == 0)<<"Line comparison failed (issue 2191), used encoding: "<<(uint32_t)encoding; //<<"  "<<getLine.c_str()<<"   "<<line2.c_str();
        //Verify second line
        read_status =  m_fileWritePtr->GetLine(getLine);
        ASSERT_TRUE(TextFileReadStatus::Success == read_status)<<"Failed to read line from file(issue 2191), used encoding: "<<(uint32_t)encoding;
        line2 = L"Characters: a A ";
        c = getLine.CompareTo(line2);
        EXPECT_TRUE (c == 0)<<"Line comparison failed, used encoding: "<<(uint32_t)encoding; //<<" "<<getLine.c_str()<<" "<<line2.c_str();
        //Verify that there is no third line
        read_status =  m_fileWritePtr->GetLine(getLine);
        EXPECT_TRUE(TextFileReadStatus::Eof ==read_status)<<"File should contain only two lines, used encoding: "<<(uint32_t)encoding;
        CloseFiles();
        }
    }
    
//------------------------------------------------------------------------------------------------
//Tests PutLine and GetLine, checks if count of lines put match with count of line read from file
//
// @bsimethod                                        Julija.Suboc                02/13
//------------------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, CountLinesInFile)
    {
    //All encodings
    bvector<TextFileEncoding> encodingsToTest;
    encodingsToTest.push_back(TextFileEncoding::Utf16);
    encodingsToTest.push_back(TextFileEncoding::CurrentLocale);
    encodingsToTest.push_back(TextFileEncoding::Utf8);
    
    TextFileEncoding encoding;
    
    //Run test for every encoding
    while(!encodingsToTest.empty())
        {
        //All options
        encoding = encodingsToTest.back();
        encodingsToTest.pop_back();
        
        bvector<TextFileOptions> options;
        options.push_back(TextFileOptions::None);
        options.push_back(TextFileOptions::KeepNewLine);
        options.push_back(TextFileOptions::NewLinesToSpace);
        
        //Run test for every option
        while(!options.empty())
            {
            //----------Prepare file--------------
            TextFileOptions option = options.back();
            options.pop_back();
            
            //Open file
            BeFileStatus status;
            BeTextFilePtr fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Write, option, encoding);
            WCharCP line = L"Hi, I am line!";
            int linesWrote = 10;
            TextFileWriteStatus writeStatus;
            
            //Add line to file 10 times
            for(int i = 0; i < linesWrote; ++i)
                    writeStatus = fileWritePtr->PutLine(line, true);

            //one additional empty line because of second argument of PutLine(); 
            linesWrote++;
            fileWritePtr->Close();
            
            //--------------Verification-------------------------
            BeTextFilePtr fileReadPtr = BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Read, option, encoding);
            TextFileReadStatus read_status = TextFileReadStatus::Success;
            int linesRead = 0;
            WString getLine;
            
            //Count how many lines there is in file
            while(read_status != TextFileReadStatus::Eof)
                {
                read_status =  fileReadPtr->GetLine(getLine);
                linesRead++;
                }
            --linesRead;
            EXPECT_EQ(linesRead, linesWrote)<<"Count of lines read: "<<linesRead<<" count of lines that were written to file "<<
                                                linesWrote<<" used encoding: "<<(uint32_t)encoding<<" with option "<<(uint32_t)option<<" (Issue 2191)";
            fileReadPtr -> Close(); 
            }
        }
    }

TEST_F(BeTextFileTests, CountCharsInFile)
    {
    //Run test for every encoding
    while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        bvector<TextFileOptions> options;
        options.push_back(TextFileOptions::None);
        options.push_back(TextFileOptions::KeepNewLine);
        options.push_back(TextFileOptions::NewLinesToSpace);
        //Run test for every option
        while(!options.empty())
            {
            //----------Prepare file--------------
            TextFileOptions option = options.back();
            options.pop_back();
            //Open file
            BeFileStatus status;
            m_fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Write, option, encoding);
            EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file for writing, used encoding: "<<(uint32_t)encoding;
            WCharCP line = L"Hi";
            int linesWrote = 10;
            TextFileWriteStatus writeStatus;
            //Add line to file 10 times
            for(int i = 0; i<linesWrote; i++)
                {
                writeStatus = m_fileWritePtr->PutLine(line, true);
                EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
                }
            //one additiona empty line because of second argument of m_fileWritePtr->PutLine(line, true); 
            m_fileWritePtr->Close();
            //--------------Verification-------------------------
            m_fileReadPtr = BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Read, option, encoding);
            EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to prepare to read from first position again, used encoding: "<<(uint32_t)encoding;
            int charRead = 1;
            WChar getChar = m_fileReadPtr->GetChar();
            //Count how many lines there is in file
            while(getChar != WEOF)
                {
                getChar =  m_fileReadPtr->GetChar();
                charRead++;
                }
            --charRead;
            EXPECT_EQ(sizeof(linesWrote)*linesWrote, charRead)<<"Count of chars read: "<<charRead<<" count of chars that were written to  file "
                                                                <<sizeof(linesWrote)*linesWrote<<" used encoding: "<<(uint32_t)encoding<<" with option"<<(uint32_t)option;
            m_fileReadPtr -> Close();
            }
            }
        }

//---------------------------------------------------------------------------------------
//Reads line from one file and appends the same line to another file
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, BeTextFileReadFromOneAppendToAnother)
    {
    while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        ReadFileSetUp(encoding);
        AppendFileSetUp(encoding);
        //---------------Test steps-----------------------
        //Open file and read first line from it
        BeFileStatus status;
        m_fileReadPtr = BeTextFile::Open(status, m_fileReadName .GetName(), TextFileOpenType::Read, TextFileOptions::None, encoding);
        //Check if file was opened
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file as read-only, used encoding: "<<(uint32_t)encoding;
        //Read from file
        WString line;
        TextFileReadStatus read_status;
        read_status =  m_fileReadPtr->GetLine(line);
        ASSERT_TRUE(read_status == TextFileReadStatus::Success)<<"Failed to read line from file, used encoding: "<<(uint32_t)encoding;
        WString line2 = L"Bentley Systems, IncorporatedBentley is the global leader dedicated to providing architects, engineers,"
                        L" constructors, and owner-operators with comprehensive architecture and engineering software solutions"
                        L" for sustaining infrastructure. Founded in 1984, Bentley has nearly 3,000 colleagues in more than 45"
                        L" countries, $500 million in annual revenues, and, since 2001, has invested more than $1 billion";
        int c = line.CompareTo(line2);
        EXPECT_TRUE (c == 0)<<c<<"Line comparison failed"; //<<" "<<line.c_str()<<line2.c_str();
        //Open another file and append to it recently read line
        WCharCP fileLine = line.GetWCharCP();
        //Open another file
        BeFileStatus status2;
        m_fileAppendPtr =    BeTextFile::Open(status2, m_fileAppendName.GetName(), TextFileOpenType::Append, TextFileOptions::KeepNewLine, 
                                              encoding);
        //Check if file was opened
        EXPECT_TRUE(BeFileStatus::Success == status2)<<"Failed to open file as append, used encoding: "<<(uint32_t)encoding;
        //Write to file
        TextFileWriteStatus writeStatus2;
        writeStatus2 = m_fileAppendPtr->PutLine(fileLine, false);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus2)<<"Failed to append line to file, used encoding: "<<(uint32_t)encoding;
        //----------Verification------------------------------
        status = m_fileAppendPtr->Rewind();
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to prepare to read from first position again, used encoding: "<<(uint32_t)encoding;
        if(encoding ==TextFileEncoding::Utf8 || encoding ==TextFileEncoding::Utf16)
            m_fileAppendPtr->GetChar();
        WString getLine;
        WString lastLine;
        read_status =  m_fileAppendPtr->GetLine(getLine);
        ASSERT_TRUE(TextFileReadStatus::Success == read_status)<<"File should contain at least one line, used encoding: "<<(uint32_t)encoding;
        lastLine = getLine;
        while(read_status ==TextFileReadStatus::Success)
            {
            read_status =  m_fileAppendPtr->GetLine(getLine);
            if(TextFileReadStatus::Success == read_status)
                {
                lastLine = getLine;
                }
            }
        ASSERT_TRUE(TextFileReadStatus::Eof == read_status);
        c = getLine.CompareTo(line2);
        EXPECT_TRUE (c == 0)<<"Line comparison failed(issue 2191), used encoding: "<<(uint32_t)encoding; //<<" "<<getLine.c_str()<<" "<<line2.c_str();
        CloseFiles();
        }
    }

//---------------------------------------------------------------------------------------
//Set pointer to the file start with "Rewind" while writing to file
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeTextFileTests, BeTextFileRewind)
    {
    while(!m_encodingsToTest.empty())
        {
        TextFileEncoding encoding = m_encodingsToTest.back();
        m_encodingsToTest.pop_back();
        //--------Test steps----------------------
        BeFileStatus status;
        //Open file
        m_fileWritePtr =    BeTextFile::Open(status, m_fileWriteName.GetName(), TextFileOpenType::Write, TextFileOptions::None, encoding);
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to open file as write, used encoding: "<<(uint32_t)encoding;
        //Write to file two lines
        TextFileWriteStatus writeStatus;
        WCharCP fileLine =  L"Hi, I am old line!";
        writeStatus = m_fileWritePtr->PutLine(fileLine, true);
        writeStatus = m_fileWritePtr->PutLine(fileLine, false);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //Set pointer to begining and rewrite first line
        status = m_fileWritePtr->Rewind();
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to prepare to read from first position again, used encoding: "<<(uint32_t)encoding;
        fileLine =  L"Hi, I am new line!";
        writeStatus = m_fileWritePtr->PutLine(fileLine, true);
        EXPECT_TRUE(TextFileWriteStatus::Success ==writeStatus)<<"Failed to write to file, used encoding: "<<(uint32_t)encoding;
        //--------------Verification-----------------------
        //Check file, it should contain two lines
        //Set pointer to begining and read first line, verify it is correct
        status = m_fileWritePtr->Rewind();
        if(encoding ==TextFileEncoding::Utf8 || encoding ==TextFileEncoding::Utf16)
            m_fileWritePtr->GetChar();
        EXPECT_TRUE(BeFileStatus::Success == status)<<"Failed to prepare to read from first position again, used encoding: "<<(uint32_t)encoding;
        WString line;
        TextFileReadStatus read_status;
        read_status =  m_fileWritePtr->GetLine(line);
        EXPECT_TRUE(TextFileReadStatus::Success ==read_status)<<"Failed to read line from file, used encoding: "<<(uint32_t)encoding;
        WString line2 = L"Hi, I am new line!";
        int c = line.CompareTo(line2);
        EXPECT_TRUE (c == 0)<<"Line comparison failed, used encoding: "<<(uint32_t)encoding; //<<line.c_str()<<" "<<line2.c_str();
        //Verify second line
        read_status =  m_fileWritePtr->GetLine(line);
        EXPECT_TRUE(TextFileReadStatus::Success ==read_status)<<"Failed to read line from file, used encoding: "<<(uint32_t)encoding;
        line2 = L"Hi, I am old line!";
        c = line.CompareTo(line2);
        EXPECT_TRUE (c == 0)<<"Line comparison failed, used encoding: "<<(uint32_t)encoding; //<<" "<<line.c_str()<<" "<<line2.c_str();
        //Verify that there is no third line
        read_status =  m_fileWritePtr->GetLine(line);
        EXPECT_TRUE(TextFileReadStatus::Eof ==read_status)<<"File should contain only two lines.";
        CloseFiles();
        }
}

#endif