/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/DirectoryObserverTests.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/DirectoryObserver.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>

//#define OBSERVER_DEBUG 1
#if defined (OBSERVER_DEBUG)
#define LOG_OBSERVER DGNCLIENTFX_LOGE 
#else
#define LOG_OBSERVER(...) {}
#endif

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Mantas.Ragauskas    06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct ObserverCallbackPredicate : IConditionVariablePredicate
    {
    bool& m_isCallbackWorkingRef;
    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    ObserverCallbackPredicate(bool& isCallbackWorkingRef) : m_isCallbackWorkingRef(isCallbackWorkingRef) { }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    06/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual bool _TestCondition(struct BeConditionVariable &cv) override
        {
        return m_isCallbackWorkingRef;
        }
    };

//=======================================================================================
// @bsiclass                                        Mantas.Ragauskas            02/2015
//=======================================================================================
class DirectoryObserverTests : public ::testing::Test
    {
public:
    bset<BeFileName>      m_filesList;
    BeFileName            m_rootDir;
    BeFileName            m_testDir1;
    DirectoryObserverPtr  m_testObserver;
    
    ObserverCallbackPredicate m_predicate;
    BeConditionVariable   m_cv;
    
    bool                  m_isCallbackWorking;
    BeFileName            m_fileNameChanged;
    FileChanges           m_changeStatus;

    void BasicDirectoryObservation();
    void BasicDirectoryObservationRepeat();
    void AddObservedDirectory();
    void RemoveObservedDirectory();
    void RenameObservedDirectory();
    void GetObservedDirectory();
#if !defined (__APPLE__)
    void ModificationInObservedDirectory();
#endif

    DirectoryObserverTests() : m_predicate(m_isCallbackWorking) { };
    ~DirectoryObserverTests() { };

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    02/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void SetUp() override
        {
        LOG_OBSERVER("[Test SetUp]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        
        BeTest::GetHost().GetTempDir(m_rootDir);
        m_rootDir.AppendToPath(L"DirectoryObserverTesting_Root");
        
        if (m_rootDir.DoesPathExist())
            RemoveFolder(m_rootDir);

        m_testDir1 = m_rootDir;
        m_testDir1.AppendToPath(L"TestDir1");

        CreateFolder(m_testDir1);
        m_filesList.insert(m_testDir1);
        
        m_isCallbackWorking = false;
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void StartBasicObservation()
        {
        m_testObserver = DirectoryObserver::Create(m_filesList, [&](BeFileName changedDir, FileChanges changeStatus) 
            {
            LOG_OBSERVER("[Callback called]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
            m_isCallbackWorking = true;
            m_fileNameChanged = changedDir;
            m_changeStatus = changeStatus;
            m_cv.notify_all();
            });

        m_testObserver->StartObserving();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    07/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void StopBasicObservation()
        {
        m_testObserver->StopObserving();
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    02/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    virtual void TearDown () override
        {
        m_testObserver->StopObserving();
        if (m_rootDir.DoesPathExist())
            RemoveFolder (m_rootDir);
        
        LOG_OBSERVER("[Test TearDown]: %llu", BeTimeUtilities::GetCurrentTimeAsUnixMillis());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    02/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void CreateFolder(BeFileNameCR fileName)
        {
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(fileName));
        ASSERT_TRUE(fileName.DoesPathExist());
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                                 Mantas.Ragauskas    02/2015
    +---------------+---------------+---------------+---------------+---------------+------*/
    void RemoveFolder(BeFileNameCR fileName)
        {
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(fileName));
        ASSERT_FALSE(fileName.DoesPathExist());
        }
    };