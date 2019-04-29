/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include "SMUnitTestUtil.h"
#include <ScalableMesh/IScalableMeshProgress.h>
#include <future>

#define NUMPROCESS 3
#define NUMSTEPS 3
#define NUMITERATIONS 100

using namespace ScalableMesh;

class ScalableMeshProgressTest : public ::testing::Test
    {
    virtual void SetUp()
        {}
    virtual void TearDown()
        {}

    public:
        static IScalableMeshPtr OpenMesh(BeFileName filename)
            {
            StatusInt status;
            IScalableMeshPtr myScalableMesh = IScalableMesh::GetFor(filename, true, true, status);
            BeAssert(status == SUCCESS);
            return myScalableMesh;
            }
    };

struct MockListener : IScalableMeshProgressListener
    {
    mutable int m_numCheckContinueOnProgressCalls = 0;
    virtual void CheckContinueOnProgress(IScalableMeshProgress* progress) const override
        {
        //auto progress_percent = progress->GetProgress() * 100.0;
        //std::cout << "\r" << "step [" << progress->GetProgressStepIndex() << "] : " << progress_percent;
        //if (progress_percent >= 100) std::cout << "\r";
        ++m_numCheckContinueOnProgressCalls;
        };
    };

struct MockListenerWithCancel : IScalableMeshProgressListener
    {
    virtual void CheckContinueOnProgress(IScalableMeshProgress* progress) const override
        {
        auto progress_percent = progress->GetProgress() * 100;
        if (progress_percent >= 50) progress->Cancel();
        };
    };

template <int NIterations>
struct MockWorker
    {
    std::future<bool> DoWorkAsync(IScalableMeshProgress* progress)
        {
        return std::async(std::launch::async, [this, &progress] 
            {
            return DoWork(progress);
            });
        }

    protected:

        bool DoWork(IScalableMeshProgressPtr progress)
            {
            for (int i = 1; i <= NIterations && !progress->IsCanceled(); i++)
                {
                progress->Progress() = (float)i / NIterations;
                progress->UpdateListeners();
                }
            return progress->GetProgress() == 1;
            }
    };

template <int NSteps, int NIterations>
struct MockMultiStepWorker : MockWorker<NIterations>
    {
    std::future<bool> DoWorkAsync(IScalableMeshProgressPtr progress)
        {
        progress->SetTotalNumberOfSteps(NSteps);
        progress->ProgressStep() = ScalableMeshStep::STEP_NOT_STARTED;
        return std::async(std::launch::async, [this, progress]
            {
            ScalableMeshStep steps[NSteps];
            switch (NSteps)
                {
                case 3:
                {
                steps[2] = ScalableMeshStep::STEP_TEXTURE;
                }
                case 2:
                {
                steps[0] = ScalableMeshStep::STEP_MESH;
                steps[1] = ScalableMeshStep::STEP_GENERATE_LOD;
                break;
                }
                default:
                {
                return false;
                }
                }
            for (int step = 1; step <= NSteps; step++)
                {
                progress->ProgressStepIndex() = step;
                progress->Progress() = 0.0;
                progress->ProgressStep() = steps[step - 1];
                progress->UpdateListeners();
                MockWorker<NIterations>::DoWork(progress);
                }
            return progress->GetProgress() == 1;
            });
        }
    };

template <int NProcess, int NIterations>
struct MockMultiProcessWorker : MockWorker<NIterations>
    {
    std::future<bool> DoWorkAsync(IScalableMeshProgress* progress)
        {
        progress->ProgressStepProcess() = ScalableMeshStepProcess::PROCESS_INACTIVE;
        return std::async(std::launch::async, [this, &progress]
            {
            ScalableMeshStepProcess processes[NProcess];
            switch (NProcess)
                {
                case 3:
                {
                processes[2] = ScalableMeshStepProcess::PROCESS_DETECT_GROUND;
                }
                case 2:
                {
                processes[0] = ScalableMeshStepProcess::PROCESS_GENERATION;
                processes[1] = ScalableMeshStepProcess::PROCESS_TEXTURING;
                break;
                }
                default:
                {
                return false;
                }
                }
            for (int process = 0; process < NProcess; process++)
                {
                progress->ProgressStepProcess() = processes[process];
                progress->Progress() = 0.0;
                progress->UpdateListeners();
                MockWorker<NIterations>::DoWork(progress);
                }
            return progress->GetProgress() == 1;
            });
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, Create)
    {
    auto progress = IScalableMeshProgress::Create();
    EXPECT_TRUE(progress != nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, CreateWithParam)
    {
    auto filenames = ScalableMeshGTestUtil::GetFiles(BeFileName(SM_DATA_PATH));
    StatusInt status;
    auto myScalableMesh = ScalableMesh::IScalableMesh::GetFor(filenames[0], true, true, status);

    auto progress = IScalableMeshProgress::Create(ScalableMeshProcessType::SAVEAS_3SM, myScalableMesh);
    EXPECT_TRUE(progress != nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, AddListener)
    {
    auto progress = IScalableMeshProgress::Create();
    ASSERT_TRUE(progress != nullptr);
    MockListener listener;

    bool succeeded = progress->AddListener(listener);
    EXPECT_TRUE(succeeded);
    EXPECT_FALSE(progress->IsCanceled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, Run)
    {
    auto progress = IScalableMeshProgress::Create();
    ASSERT_TRUE(progress != nullptr);

    MockWorker<NUMITERATIONS> worker;
    bool isDone = worker.DoWorkAsync(progress.get()).get();

    EXPECT_TRUE(isDone);
    EXPECT_FALSE(progress->IsCanceled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, RunWithListener)
    {
    auto progress = IScalableMeshProgress::Create();
    ASSERT_TRUE(progress != nullptr);

    MockListener listener;
    ASSERT_TRUE(progress->AddListener(listener));

    MockWorker<NUMITERATIONS> worker;
    bool isDone = worker.DoWorkAsync(progress.get()).get();

    EXPECT_GT(listener.m_numCheckContinueOnProgressCalls, 0);
    EXPECT_TRUE(isDone);
    EXPECT_FALSE(progress->IsCanceled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, RunThenCancel)
    {
    auto progress = IScalableMeshProgress::Create();
    ASSERT_TRUE(progress != nullptr);

    MockListenerWithCancel listener;
    ASSERT_TRUE(progress->AddListener(listener));

    MockWorker<NUMITERATIONS> worker;
    bool isDone = worker.DoWorkAsync(progress.get()).get();

    EXPECT_FALSE(isDone); // Work is being canceled, should never get to finish
    EXPECT_TRUE(progress->IsCanceled());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, RunMultiStep)
    {
    auto progress = IScalableMeshProgress::Create();
    ASSERT_TRUE(progress != nullptr);

    MockListener listener;
    ASSERT_TRUE(progress->AddListener(listener));

    MockMultiStepWorker<NUMSTEPS, NUMITERATIONS> worker;
    auto workFuture = worker.DoWorkAsync(progress.get());

    ASSERT_TRUE(progress->GetTotalNumberOfSteps() == 3);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    auto step = (ScalableMeshStep)progress->GetProgressStep();
    EXPECT_TRUE(step == ScalableMeshStep::STEP_NOT_STARTED || step == ScalableMeshStep::STEP_MESH || step == ScalableMeshStep::STEP_GENERATE_LOD || ScalableMeshStep::STEP_TEXTURE);

    bool isDone = workFuture.get();

    EXPECT_TRUE(isDone);
    EXPECT_FALSE(progress->IsCanceled());
    EXPECT_TRUE(progress->GetProgressStepIndex() == NUMSTEPS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ScalableMeshProgressTest, RunMultiProcess)
    {
    auto progress = IScalableMeshProgress::Create();
    ASSERT_TRUE(progress != nullptr);

    struct MockProcessListener : IScalableMeshProgressListener
        {
        mutable std::map<ScalableMeshStepProcess, int> m_ProcessCount;
        virtual void CheckContinueOnProgress(IScalableMeshProgress* progress) const override
            {
            auto process = (ScalableMeshStepProcess)progress->GetProgressStepProcess();
            if (m_ProcessCount.count(process) == 0) m_ProcessCount[process] = 0;
            ++m_ProcessCount[process];
            };

        bool Validate(const int& nProcess)
            {
            if (m_ProcessCount.size() != nProcess) return false;
            for (auto process : m_ProcessCount)
                {
                if (process.second == 0) return false;
                }
            return true;
            }
        };

    MockProcessListener listener;
    ASSERT_TRUE(progress->AddListener(listener));

    MockMultiProcessWorker<NUMPROCESS, NUMITERATIONS> worker;
    auto workFuture = worker.DoWorkAsync(progress.get());
    bool isDone = workFuture.get();

    EXPECT_TRUE(isDone);
    EXPECT_FALSE(progress->IsCanceled());
    EXPECT_TRUE(listener.Validate(NUMPROCESS));
    }
