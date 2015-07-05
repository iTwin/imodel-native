/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/BeFile_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (__unix__)

#include <Bentley/BeFile.h>
#include <Bentley/Bentley.h>
#include  <Bentley/BeTest.h>
USING_NAMESPACE_BENTLEY

struct BeFileTests : public testing::Test
    {
public:
    BeFile m_file;
    BeFile m_file2;
    bvector <WCharCP> m_testData;

    //---------------------------------------------------------------------------------------
    // Initializes two file objects
    //
    // @bsimethod                          Julija.Suboc                            02/13
    //---------------------------------------------------------------------------------------
    void SetUp()
        {
        //Will test files with these extensions
        m_testData.push_back(L"py");
        m_testData.push_back(L"cpp");
        m_testData.push_back(L"txt");
        }

    //---------------------------------------------------------------------------------------
    // Closes files
    //
   // @bsimethod                                        Julija.Suboc                02/13
    //---------------------------------------------------------------------------------------
    void TearDown()
        {
        CloseFiles();
        }

    //---------------------------------------------------------------------------------------
    // Closes two class files if open
    //
    // @bsimethod                                        Julija.Suboc                02/13
    //---------------------------------------------------------------------------------------
    void CloseFiles()
        {
        if(m_file.IsOpen())
            CloseFile(&m_file);
        if(m_file2.IsOpen())
            CloseFile(&m_file2);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Julija.Suboc                02/13
    //---------------------------------------------------------------------------------------
    void CloseFile(BeFile* file)
        {
        BeFileStatus status = file->Close();
        ASSERT_TRUE (BeFileStatus::Success == status);
        }

    //---------------------------------------------------------------------------------------------
    // Prepares file for test with data specified
    // @param filePath      path where file should be placed/found
    // @param buf           string to write to file
    // @param repeatContent     how many times write buf string to file
    // @param reCreate      reCreate file if exists, if true will not change file if it exists
    //
    // @bsimethod                                        Julija.Suboc                02/13
    //---------------------------------------------------------------------------------------------
    void PrepareFile(WCharCP  filePath, char const* buf, int repeatContent, bool reCreate = false)
        {
        BeFile  newFile;
        BeFileStatus status;
        if(!reCreate)
            {
            status = newFile.Open(filePath, BeFileAccess::Write);
            newFile.Close();
            }
        else
            {
            status = BeFileStatus::UnknownError;
            }
        if (status != BeFileStatus::Success)
            {
            status = newFile.Create(filePath, true);
            ASSERT_TRUE(status == BeFileStatus::Success);
            uint32_t byteCountToCopy;
                uint32_t byteCount = 0;
                for(int i = 0; i<repeatContent; i++)
                    {
                    byteCountToCopy = (uint32_t) strlen(buf);
                    status = newFile.Write(&byteCount, buf, byteCountToCopy);
                    EXPECT_EQ(byteCount, byteCountToCopy);
                    ASSERT_TRUE(status == BeFileStatus::Success);
                    }
            newFile.Close();
            }
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Julija.Suboc                02/13
    //---------------------------------------------------------------------------------------
    void CreateFile(BeFile* file, WCharCP filePath, bool createAlways = true)
        {
        BeFileStatus status = file->Create(filePath, createAlways);
        ASSERT_TRUE(status == BeFileStatus::Success)<<"Failed to create file"<< filePath;
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Julija.Suboc                02/13
    //---------------------------------------------------------------------------------------
    void OpenFile(BeFile* file,  WCharCP filePath, BeFileAccess mode)
        {
        BeFileStatus status = BeFileStatus::Success;
        status = file->Open(filePath, mode);
        ASSERT_TRUE(status == BeFileStatus::Success)<<"Failed to open file, file: "<<filePath;
        ASSERT_TRUE(mode == file->GetAccess())<<"Access mode does not match";
        ASSERT_TRUE(file->IsOpen())<<"Should have status opened";
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                        Julija.Suboc                02/13
    //---------------------------------------------------------------------------------------
    WCharCP CreatePathForTempFile(BeFileName * tmp, WCharCP fileName, WCharCP fileExtension)
        {
        BeTest::GetHost().GetOutputRoot (*tmp);
        tmp->AppendToPath(fileName);
        tmp->AppendExtension(fileExtension);
        return tmp->GetName();
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, CreateFile)
    {
    while(!m_testData.empty())
        {
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();

        PrepareFile(filePath, "", 0, false);
        //Create file
        BeFileStatus status = m_file.Create(filePath, true);
        ASSERT_TRUE(status == BeFileStatus::Success)<<"Failed to create file, file: "<<filePath;
        EXPECT_TRUE(m_file.IsOpen())<<"Open?";
        m_file.Close();
        status = m_file2.Create(filePath, true);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to re-create file";
        //Try to create file with set parameter to not create file if it already exists
        m_file2.Close();
        CloseFiles();
        }
    }

//---------------------------------------------------------------------------------------
// Gets size for the file with one set of data in it, and then adds more data and 
// compares new size to old one
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, GetSize)
    {
    while(!m_testData.empty())
        {
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        //Fill file with data and get size
       char const* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        PrepareFile(filePath, buf, 10, true);
        OpenFile(&m_file, filePath, BeFileAccess::Read);
        uint64_t size1;
        BeFileStatus status =  m_file.GetSize(size1);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get size of file";
        m_file.Close();
        //fill data with 12 times more of data
        PrepareFile(filePath, buf, 120, true);
        OpenFile(&m_file, filePath, BeFileAccess::Read);
        uint64_t size2;
        status =  m_file.GetSize(size2);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get size of file";
        m_file.Close();
        //Verify that first file was 12 times smaller than second
        EXPECT_EQ(size1*12, size2)<<"size1 should be 12 times smaller than size2. File tested: "<<filePath;
        }
    }

//---------------------------------------------------------------------------------------
// Sets new size for the file
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, SetSize)
    {
    while(!m_testData.empty())
        {
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        CreateFile(&m_file, filePath, true);
        uint64_t size1;
        BeFileStatus status =  m_file.GetSize(size1);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get size of file";
        //Set new size for file
        status =  m_file.SetSize((size1+100)*2);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to set size of file";
        m_file.Close();
        m_file.Open(filePath, BeFileAccess::Read);
        uint64_t size2;
        status =  m_file.GetSize(size2);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get size of file";
        EXPECT_EQ(size2, (size1+100)*2);
        m_file.Close();
        }
    }


//---------------------------------------------------------------------------------------
//Reads entire file
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, ReadEntireFile)
    {
    while(!m_testData.empty())
        {
        // Prepre file with one line
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        char const* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        PrepareFile(filePath, buf, 1, true);
        //------Test steps--------------
        BeFileAccess access= BeFileAccess::Read;
        BeFileStatus status =m_file.Open(filePath, access);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to open file, file: "<<filePath;
        //Read entire file
        bvector<Byte> entireFile;
        status =m_file.ReadEntireFile(entireFile);
        //-------Verification-----------
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to read entire file, file: "<<filePath;
        EXPECT_EQ(strlen(buf), entireFile.size())<<"Should had read all file.";
        m_file.Close();
        }
}

//---------------------------------------------------------------------------------------
//Write to file
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, Write)
    {   
    while(!m_testData.empty())
        {
        //------Preparations----------
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        CreateFile(&m_file, filePath, true);
        //Write to file
        const char* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        uint32_t byteCountToCopy = (uint32_t) strlen(buf);
        uint32_t bytesWritten;
        BeFileStatus status = m_file.Write(&bytesWritten, buf, byteCountToCopy);
        //---Verification--------------
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to write to file, File: "<<filePath;
        EXPECT_EQ(bytesWritten, byteCountToCopy)<<"Failed to write bytes count specified. File: "<<filePath;
        m_file.Close();
        }
    }

//---------------------------------------------------------------------------------------
//Test setting and getting pointers
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, SetAndGetPointer)
    {
    while(!m_testData.empty())
        {
        //------Preparations----------
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        char const* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        PrepareFile(filePath, buf, 2, true);
        //Open file
        BeFileAccess access= BeFileAccess::Write;
        BeFileStatus status =m_file.Open(filePath, access);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to open file, file: "<<filePath;
        //Get pointer, should be zero as file was just opened
        uint64_t currentPosition;
        uint64_t newPosition;
        m_file.GetPointer(currentPosition);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get file pointer, file: "<<filePath;
        EXPECT_TRUE(currentPosition == 0)<<"Current pointer position should be 0 not "<<currentPosition;
        //Set pointer with begin seek and verify if it was performed correctly
        newPosition = 100;
        m_file.SetPointer(newPosition, BeFileSeekOrigin::Begin);
        m_file.GetPointer(currentPosition);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get file pointer, file: "<<filePath;
        EXPECT_TRUE(currentPosition == newPosition)<<"Pointer should be set to "<<newPosition<<" not "<<currentPosition<<
                                                     " Pointer was set with current seek";
        //Set pointer with current seek and verify if it was performed correctly
        m_file.SetPointer(newPosition, BeFileSeekOrigin::Current);
        m_file.GetPointer(currentPosition);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get file pointer, file: "<<filePath;
        EXPECT_TRUE(currentPosition == newPosition*2)<<"Pointer should be set to "<<newPosition*2<<" not "<<currentPosition<<
                                                       " Pointer was set with current seek ";
        //Set pointer to begining
        m_file.SetPointer(0, BeFileSeekOrigin::Begin);
        //Set pointer with end seek and verify if it was performed correctly
        uint64_t oldPosition;
        m_file.SetPointer(0, BeFileSeekOrigin::End);
        m_file.GetPointer(oldPosition);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get file pointer, file: "<<filePath;
        newPosition = 57;
        m_file.SetPointer(newPosition, BeFileSeekOrigin::End);
        m_file.GetPointer(currentPosition);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to get file pointer, file: "<<filePath;
        EXPECT_TRUE(currentPosition - oldPosition == newPosition)<<"Pointer should be set to "<<oldPosition<<" + "<<newPosition<<" not "<<
                                                                    currentPosition<<" It was set with end seek. ";
        m_file.Close();
        }
    }

//---------------------------------------------------------------------------------------
//Write while changing pointer position
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, WriteWithPointerSet)
    {
    while(!m_testData.empty())
        {
        //------Preparations----------
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        CreateFile(&m_file, filePath, true);
        //Write to file
        const char* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        uint32_t byteCountToCopy = (uint32_t) strlen(buf);
        uint32_t bytesWritten;
        BeFileStatus status = m_file.Write(&bytesWritten, buf, byteCountToCopy);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to write to file, File: "<<filePath;
        EXPECT_EQ(bytesWritten, byteCountToCopy)<<"Failed to write bytes count specified. File: "<<filePath;
        //Set pointer to begining and write again
        const char* buf2 = "sadasdawqfkdv,n,fpoeojfeolkjmj";
        byteCountToCopy = (uint32_t) strlen(buf2);
        m_file.SetPointer(0, BeFileSeekOrigin::Begin);
        status = m_file.Write(&bytesWritten, buf2, byteCountToCopy);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to write to file, File: "<<filePath;
        EXPECT_EQ(bytesWritten, byteCountToCopy)<<"Failed to write bytes count specified. File: "<<filePath;
        
        m_file.SetPointer(0, BeFileSeekOrigin::End);
        status = m_file.Write(&bytesWritten, buf2, byteCountToCopy);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to write to file, File: "<<filePath;
        EXPECT_EQ(bytesWritten, byteCountToCopy)<<"Failed to write bytes count specified. File: "<<filePath;
        
        //Cunstruct expected string
        std::string expected = buf;
        expected.replace(0, strlen(buf2), buf2);
        expected.append(buf2);
        //Read file content
        char readBuf[1000];
        memset(readBuf, '\0', sizeof(readBuf));
        //Read file contend and verify that it matches expected string
        uint32_t byteCount;
        m_file.SetPointer(0, BeFileSeekOrigin::Begin);
        status = m_file.Read(readBuf, &byteCount, 1000);
        int cmp = strcmp(readBuf, expected.c_str());
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to read from file, File: "<<filePath;
        EXPECT_TRUE(byteCount == expected.length())<<"Readed other Byte count than expected, read "<<byteCount<<" Expected "<<expected.length();
        EXPECT_TRUE(cmp == 0)<<"File contains other data than expected. File: "<<filePath<<" Expected "<<expected<<" contains "<<readBuf;
        m_file.Close();
        }
    }


//---------------------------------------------------------------------------------------
//Read from file while changing pointer position
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, ReadWithPointerSet)
    {
    while(!m_testData.empty())
        {
        //------Preparations----------
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        char const* buf = "Reader! Hello";
        PrepareFile(filePath, buf, 1, true);
        //Open file
        BeFileAccess access= BeFileAccess::Read;
        BeFileStatus status =m_file.Open(filePath, access);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to open file, file: "<<filePath;
        //Move pointer and read from file
        status =m_file.SetPointer(8, BeFileSeekOrigin::Current);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to move pointer with current seek, file: "<<filePath;
        uint32_t byteCount;
        char readBuf[1000];
        memset(readBuf, '\0', sizeof(readBuf));
        status =m_file.Read(readBuf, &byteCount, 5);
        char const* expected = "Hello";
        //Verify that reading suceeded
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to read from file, File: "<<filePath;
        EXPECT_TRUE(byteCount == 5)<<"Readed other Byte count than expected, file: "<<filePath<<"   Read "<<byteCount<<" Expected "<<5;
        int cmp = strcmp(readBuf, expected);
        EXPECT_TRUE(cmp == 0)<<"File contains other data than expected. File: "<<filePath;
        //move pointer again and read some more
        status =m_file.SetPointer(0, BeFileSeekOrigin::Begin);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to move pointer with begin seek, file: "<<filePath;
        memset(readBuf, '\0', sizeof(readBuf));
        status =m_file.Read(readBuf, &byteCount, 7);
        expected = "Reader!";
        //Verify that readed what expected
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to read from file, File: "<<filePath;
        EXPECT_TRUE(byteCount == 7)<<"Readed other Byte count than expected, file: "<<filePath<<" read "<<byteCount<<" Expected "<<7;
        cmp = strcmp(readBuf, expected);
        EXPECT_TRUE(cmp == 0)<<"File contains other data than expected. File: "<<filePath;

        m_file.Close();
        }
    }

//---------------------------------------------------------------------------------------
// Swaps two files and reads some data from file to verify that file swapping was 
//successfull
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, SwapFiles)
    {
    while(!m_testData.empty())
        {
        BeFileName fileName1;
        CreatePathForTempFile(&fileName1, L"Write", m_testData.back());
        BeFileName fileName2;
        CreatePathForTempFile(&fileName2, L"Write2", m_testData.back());
        m_testData.pop_back();

        CreateFile(&m_file, fileName1.GetName(), true);
        ASSERT_TRUE(m_file.IsOpen());
        CreateFile(&m_file2, fileName2.GetName(), true);
        ASSERT_TRUE(m_file2.IsOpen());
        uint32_t byteCountToCopy = 20;
        uint32_t byteCount;
        const char* buf = "Hello, its me Mario!";
        BeFileStatus status =m_file.Write(&byteCount, buf, byteCountToCopy);
        EXPECT_TRUE(status == BeFileStatus::Success);
        EXPECT_EQ(byteCountToCopy, byteCount);
    
        byteCountToCopy = 26;
        const char* buf2 = "Hi Mario, its me princess!";
        status =m_file2.Write(&byteCount, buf2, byteCountToCopy);
        ASSERT_TRUE (BeFileStatus::Success == status);
        ASSERT_EQ(byteCountToCopy, byteCount);

        m_file.Swap(m_file2);
        status =m_file2.Write(&byteCount, buf2, byteCountToCopy);
        ASSERT_TRUE (BeFileStatus::Success == status);
        byteCountToCopy = 20;
        status =m_file.Write(&byteCount, buf, byteCountToCopy);
        ASSERT_TRUE (BeFileStatus::Success == status);
        m_file.SetPointer(0, BeFileSeekOrigin::Begin);
        m_file2.SetPointer(0, BeFileSeekOrigin::Begin);
        char readBuf[100];
        memset(readBuf, '\0', sizeof(readBuf));
        char * expected = (char*) malloc(strlen(buf)+strlen(buf2)+1);
        strcpy(expected, buf);
        strcat(expected, buf2);
        status =m_file.Read(readBuf, &byteCount, 46);
        EXPECT_TRUE(status == BeFileStatus::Success);
        char readBuf2[100];
        memset(readBuf2, '\0', sizeof(readBuf2));
        char * expected2 = (char*) malloc(strlen(buf)+strlen(buf2)+1);
        strcpy(expected2, buf2);
        strcat(expected2, buf);
        status =m_file2.Read(readBuf2, &byteCount, 46);
        int cmp = strcmp(readBuf, expected2);
        EXPECT_EQ(cmp, 0)<<"File contains other data than expected. Found: "<<readBuf<<". Expected: "<<expected2;
    
        cmp = strcmp(readBuf2, expected);
        EXPECT_EQ(cmp, 0)<<"File2 contains other data than expected. Found: "<<readBuf2<<". Expected: "<<expected;
        free(expected);
        free(expected2);
        CloseFiles();
        }
    }

//---------------------------------------------------------------------------------------
//Try to write to read only file and get last error function
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, GetLastError)
    {
    while(!m_testData.empty())
        {
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        uint32_t bytesWritten;
        const char* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        uint32_t numBytes = (uint32_t)strlen(buf) * sizeof(char);
        BeFileStatus status;
        BeFileStatus status2;
        status =m_file.Write(&bytesWritten, buf, numBytes);
        EXPECT_TRUE(status != BeFileStatus::Success)<<"Write operation suceeded without opening file";
        status2 =m_file.GetLastError();
        EXPECT_TRUE(status ==status2)<<"Last error does not match after trying to write to not opened file.";

        status =m_file.Open(filePath, BeFileAccess::Read);
        status =m_file.Write(&bytesWritten, buf, numBytes);
        EXPECT_TRUE(status != BeFileStatus::Success)<<"Suceeded to write to file that was opened for reading, file: "<<filePath;
        status2 =m_file.GetLastError();
        EXPECT_TRUE(status == status2)<<"Last error does not match after trying to write to read only file, file: ."<<filePath;
        m_file.Close();
        status =m_file.Open(filePath, BeFileAccess::Write);
        status =m_file.Write(&bytesWritten, buf, numBytes);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to write to file that was opened for writing, file: "<<filePath;
        status2 =m_file.GetLastError();
        EXPECT_TRUE(status != status2)<<"Last error should not disappear after successfull operation.";
        m_file.Close();
        }
    }

//---------------------------------------------------------------------------------------
//Reads some data from file
//
// @bsimethod                                        Julija.Suboc                02/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, Read)
    {
    while(!m_testData.empty())
        {
        //Prepare file with one line
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        char const* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        PrepareFile(filePath, buf, 1, true);
        //-------------Test steps---------
        BeFileAccess access= BeFileAccess::Read;
        BeFileStatus status =m_file.Open(filePath, access);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to open file, file: "<<filePath;
        m_file.SetPointer(0, BeFileSeekOrigin::Begin);
        char readBuf[100];
        memset(readBuf, '\0', sizeof(readBuf));
        char expected[100];
        memset(expected, '\0', sizeof(expected));
        //TODO strn copy was used with char const *
        strncpy(expected, buf, 10);
        uint32_t byteCount;
        status =m_file.Read(readBuf, &byteCount, 10);
        //-----------Verification-----------------
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to read from file, file: "<<filePath;
        int cmp = strcmp(readBuf, expected);
        EXPECT_EQ(cmp, 0)<<"Should read 10 first symbols from file. File "<<filePath;
        EXPECT_EQ(byteCount, 10)<<"Read Byte count does not match";
        m_file.Close();
        }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                        Julija.Suboc                08/13
//---------------------------------------------------------------------------------------
TEST_F(BeFileTests, Flush)
    {   
        //------Preparations----------
        BeFileName fileName;
        CreatePathForTempFile(&fileName, L"Write", m_testData.back());
        WCharCP filePath = fileName.GetName();
        m_testData.pop_back();
        CreateFile(&m_file, filePath, true);
        //Write to file
        const char* buf = "QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!Q QWERTYUIOP QWERTYUJHG !@#$%^&*() 1234567890 !Q!Q!Q!Q!QE";
        uint32_t byteCountToCopy = (uint32_t) strlen(buf);
        uint32_t bytesWritten;
        //Try to flush after opening file
        BeFileStatus status = m_file.Flush();
        EXPECT_TRUE(status == BeFileStatus::Success);
        //Write to file
        status = m_file.Write(&bytesWritten, buf, byteCountToCopy);
        EXPECT_TRUE(status == BeFileStatus::Success)<<"Failed to write to file, File: "<<filePath;
        EXPECT_EQ(bytesWritten, byteCountToCopy)<<"Failed to write bytes count specified. File: "<<filePath;
        //Flush after writing
        status = m_file.Flush();
        EXPECT_TRUE(status == BeFileStatus::Success);
        m_file.Close();
        //Flush after closing
        status = m_file.Flush();
        EXPECT_TRUE(BeFileStatus::Success != status);
        }

#endif // defined (BENTLEY_WIN32)
