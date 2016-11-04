/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DirectoryObserverTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "DirectoryObserverTests.h"
#include <Bentley/BeThread.h>
#include <Bentley/Tasks/Tasks.h>

USING_NAMESPACE_BENTLEY_TASKS

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
void DirectoryObserverTests::BasicDirectoryObservation()
    {
    LOG_OBSERVER("[BasicDirectoryObservation]: [Start] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    StartBasicObservation();
    BeFileName testSubDir1 = m_testDir1;
    testSubDir1.AppendToPath(L"TestSubDir1");
    
    CreateFolder(testSubDir1);
    LOG_OBSERVER("[BasicDirectoryObservation: [Folder created]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    m_cv.WaitOnCondition(&m_predicate, 5000);
    
    ASSERT_TRUE (m_isCallbackWorking);                      
    ASSERT_EQ(testSubDir1, m_fileNameChanged);
    ASSERT_EQ(FileChanges::FileAdded, m_changeStatus);
    
    m_isCallbackWorking = false;                           
    m_testObserver->StopObserving ();
   
    RemoveFolder(testSubDir1);
    ASSERT_FALSE (m_isCallbackWorking);  
    StopBasicObservation();
    LOG_OBSERVER("[BasicDirectoryObservation]: [End] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   07/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, BasicDirectoryObservation_UIThread)
    {
    BasicDirectoryObservation();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   07/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, BasicDirectoryObservation_WorkThread)
    {
    WorkerThreadPtr workerThread = WorkerThread::Create("ObserverTests_WorkThread");
    workerThread->ExecuteAsync([&]()
        {
        BasicDirectoryObservation();
        })->Wait();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
void DirectoryObserverTests::BasicDirectoryObservationRepeat()
    {
    LOG_OBSERVER("[BasicDirectoryObservationRepeat]: [Start] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    StartBasicObservation();
    BeFileName testSubDir1 = m_testDir1;
    testSubDir1.AppendToPath(L"TestSubDir1");
    for (int i = 0; i < 5; i++)
        {
        CreateFolder(testSubDir1);
        LOG_OBSERVER("[BasicDirectoryObservationRepeat]: [Folder created]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        m_cv.WaitOnCondition(&m_predicate, 5000);

        ASSERT_TRUE(m_isCallbackWorking);
        m_isCallbackWorking = false;

        RemoveFolder(testSubDir1);
        LOG_OBSERVER("[BasicDirectoryObservationRepeat]: [Folder removed]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        m_cv.WaitOnCondition(&m_predicate, 5000);
        ASSERT_TRUE(m_isCallbackWorking);
        m_isCallbackWorking = false;
        }
    StopBasicObservation();
    LOG_OBSERVER("BasicDirectoryObservationRepeat]: [End] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, BasicDirectoryObservationRepeat_UIThread)
    {
    BasicDirectoryObservationRepeat();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, BasicDirectoryObservationRepeat_WorkThread)
    {
    WorkerThreadPtr workerThread = WorkerThread::Create("ObserverTests_WorkThread");
    workerThread->ExecuteAsync([&]()
        {
        BasicDirectoryObservationRepeat();
        })->Wait();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
void DirectoryObserverTests::AddObservedDirectory()
    {
    StartBasicObservation();
    BeFileName testDir2 = m_rootDir;
    testDir2.AppendToPath(L"TestDir2");

    BeFileName testSubDir2 = testDir2;
    testSubDir2.AppendToPath(L"TestSubDir2");

    CreateFolder(testSubDir2);
    ASSERT_FALSE(m_isCallbackWorking);                    
    
    ASSERT_EQ(BentleyStatus::SUCCESS, m_testObserver->AddObservedDirectory(testDir2));
    LOG_OBSERVER("[AddObservedDirectory]: [Directory added to list] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    RemoveFolder(testSubDir2);
    LOG_OBSERVER("[AddObservedDirectory]: [folder removed] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    m_cv.WaitOnCondition(&m_predicate, 5000);
    
    ASSERT_TRUE (m_isCallbackWorking);
    ASSERT_EQ(testSubDir2, m_fileNameChanged);
    ASSERT_EQ(FileChanges::FileRemoved, m_changeStatus);
    StopBasicObservation();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   07/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, AddObservedDirectory_UIThread)
    {
    AddObservedDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   07/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, AddObservedDirectory_WorkThread)
    {
    WorkerThreadPtr workerThread = WorkerThread::Create("ObserverTests_WorkThread");
    workerThread->ExecuteAsync([&]()
        {
        AddObservedDirectory();
        })->Wait();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
void DirectoryObserverTests::RemoveObservedDirectory()
    {
    StartBasicObservation();
    BeFileName testSubDir1 = m_testDir1;
    testSubDir1.AppendToPath(L"TestSubDir1");
    
    CreateFolder(testSubDir1);
    m_cv.WaitOnCondition(&m_predicate, 5000);   

    ASSERT_TRUE (m_isCallbackWorking);                      
    ASSERT_EQ(testSubDir1, m_fileNameChanged);
    ASSERT_EQ(FileChanges::FileAdded, m_changeStatus);

    m_isCallbackWorking = false;                            
                                                           
    ASSERT_EQ(BentleyStatus::SUCCESS, m_testObserver->RemoveObservedDirectory(m_testDir1));

    RemoveFolder(testSubDir1);
    ASSERT_FALSE (m_isCallbackWorking);   
    StopBasicObservation();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   07/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, RemoveObservedDirectory_UIThread)
    {
    RemoveObservedDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   07/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, RemoveObservedDirectory_WorkThread)
    {
    WorkerThreadPtr workerThread = WorkerThread::Create("ObserverTests_WorkThread");
    workerThread->ExecuteAsync([&]()
        {
        RemoveObservedDirectory();
        })->Wait();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Mantas.Ragauskas    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct RenamePredicate : IConditionVariablePredicate
    {
    BeAtomic<bool>& m_oldName;
    BeAtomic<bool>& m_newName;
    RenamePredicate(BeAtomic<bool>& oldName, BeAtomic<bool>& newName) : m_oldName(oldName), m_newName(newName) { }
    virtual bool _TestCondition(struct BeConditionVariable &cv) override { return m_oldName && m_newName; }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
void DirectoryObserverTests::RenameObservedDirectory()
    {
    BeAtomic<bool> oldFileNameUpdate(false);
    BeAtomic<bool> newFileNameUpdate(false);


    BeFileName testSubDir1 = m_testDir1;
    testSubDir1.AppendToPath(L"TestSubDir1");
    BeFileName testSubDir1NewName = m_testDir1;
    testSubDir1NewName.AppendToPath(L"TestSubDir1NewName");
    CreateFolder(testSubDir1);

    RenamePredicate predicate(oldFileNameUpdate, newFileNameUpdate);
    m_testObserver = DirectoryObserver::Create(m_filesList, [&](BeFileName changedDir, FileChanges changeStatus)
        {
        if (FileChanges::FileRemoved == changeStatus)
            oldFileNameUpdate.store(true);
        else if (FileChanges::FileAdded == changeStatus)
            newFileNameUpdate.store(true);
        m_cv.notify_all();
        LOG_OBSERVER("[RenameObservedDirectory]: [Callback called] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        });

    m_testObserver->StartObserving();
    LOG_OBSERVER("[RenameObservedDirectory]: [Observer started] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeMoveFile(testSubDir1, testSubDir1NewName));
    LOG_OBSERVER("[RenameObservedDirectory]: [folder renamed] %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());

    m_cv.WaitOnCondition(&predicate, 5000);

    ASSERT_TRUE(oldFileNameUpdate && newFileNameUpdate);
    StopBasicObservation();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, RenameObservedDirectory_UIThread)
    {
    RenameObservedDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, RenameObservedDirectory_WorkThread)
    {
    WorkerThreadPtr workerThread = WorkerThread::Create("ObserverTests_WorkThread");
    workerThread->ExecuteAsync([&]()
        {
        RenameObservedDirectory();
        })->Wait();
    }

#if !defined (__APPLE__)
//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
void DirectoryObserverTests::ModificationInObservedDirectory()
    {   
    StartBasicObservation();
    BeFileName testFileName = m_testDir1;
    testFileName.AppendToPath(L"TestFile.txt");

    BeFile testFile;          
    ASSERT_EQ(BeFileStatus::Success, testFile.Create(testFileName.GetName()));
    ASSERT_EQ(BeFileStatus::Success, testFile.Close());                         //We close the file, so that in case of test failure
    ASSERT_FALSE(testFile.IsOpen());                                            //It does not leave an unclosed handle

    m_cv.WaitOnCondition(&m_predicate, 5000);

    ASSERT_TRUE(m_isCallbackWorking);
    ASSERT_EQ(testFileName, m_fileNameChanged);
    ASSERT_EQ(FileChanges::FileAdded, m_changeStatus);

    m_isCallbackWorking = false;
    
    ASSERT_EQ(BeFileStatus::Success, testFile.Open(testFileName.GetName(), BeFileAccess::ReadWrite));

    Utf8String message("Message");
    testFile.Write(nullptr, (void*) message.c_str(), (uint32_t) message.length());
    testFile.Flush();
    ASSERT_EQ(BeFileStatus::Success, testFile.Close());
    ASSERT_FALSE(testFile.IsOpen());
  
    m_cv.WaitOnCondition(&m_predicate, 5000);

    ASSERT_TRUE(m_isCallbackWorking);
    ASSERT_EQ(testFileName, m_fileNameChanged);
    ASSERT_EQ(FileChanges::FileMofified, m_changeStatus);
    
    ASSERT_EQ(BeFileNameStatus::Success, testFileName.BeDeleteFile());
    StopBasicObservation();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, ModificationInObservedDirectory_UIThread)
    {
    ModificationInObservedDirectory();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
TEST_F(DirectoryObserverTests, ModificationInObservedDirectory_WorkThread)
    {
    WorkerThreadPtr workerThread = WorkerThread::Create("ObserverTests_WorkThread");
    workerThread->ExecuteAsync([&]()
        {
        ModificationInObservedDirectory();
        })->Wait();
    }
#endif // !(_APPLE_)

//---------------------------------------------------------------------------------------
// @bsimethod                                               Mantas.Ragauskas   06/2015
//---------------------------------------------------------------------------------------
TEST_F (DirectoryObserverTests, GetObservedDirectory)
    {
    StartBasicObservation();

    ASSERT_EQ(m_filesList, m_testObserver->GetObservedDirectories());   //Check if initialization value is returned correctly

    BeFileName testDir2 = m_rootDir;
    testDir2.AppendToPath(L"TestDir2");
    CreateFolder(testDir2);
    ASSERT_EQ(BentleyStatus::SUCCESS, m_testObserver->AddObservedDirectory(testDir2));
  
    bset<BeFileName> tempDirs = m_filesList;
    tempDirs.insert(testDir2);
    ASSERT_EQ(tempDirs, m_testObserver->GetObservedDirectories());     //Check if added value is returned correctly
    
    ASSERT_EQ(BentleyStatus::SUCCESS, m_testObserver->RemoveObservedDirectory(testDir2));
    ASSERT_EQ(m_filesList, m_testObserver->GetObservedDirectories());  //Check if removed value is returned correctly

    StopBasicObservation();
    }


