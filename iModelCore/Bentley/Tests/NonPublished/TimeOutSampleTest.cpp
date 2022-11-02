/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT) || defined (__unix__)

#include <Bentley/BeFile.h>
#include <Bentley/Bentley.h>
#include  <Bentley/BeTest.h>




//---------------------------------------------------------------------------------------
// Initializes two file objects
//
// @bsimethod
//---------------------------------------------------------------------------------------

//time out part
#include <time.h>
#include <functional>
#include <future>
#include <string>
#include <thread>

class TimedTask
{
public:
    virtual void Run () = 0;
};

template<typename... Function>
class TimedTaskImpl : public TimedTask
{
public:
    using BindType_t = decltype(std::bind (std::declval<std::function<void (Function...)>> (), std::declval<Function> ()...));
    template<typename... Args>
    TimedTaskImpl (std::function<void (Function...)> f, Args&&... args) :
        TimedTask (), mBind (std::move (f), std::forward<Args> (args)...)
    {
    }
    virtual ~TimedTaskImpl ()
    {
    }
    virtual void Run ()
    {
        mBind ();
    }
private:
    BindType_t mBind;
};

template<class Rep, class Period, typename Function, typename... Args>
static void EXPECT_COMPLETE (const std::chrono::duration<Rep, Period>& dur, Function&& f, Args&&... args)
{
    std::promise<bool> promisedFinished;
    auto futureResult = promisedFinished.get_future ();
    std::shared_ptr<TimedTask> task (new TimedTaskImpl<Args...> (std::forward<Function> (f), std::forward<Args> (args)...));

    std::thread ([](std::promise<bool>& finished, const std::shared_ptr<TimedTask>& _task)
    {
        _task->Run ();
        finished.set_value (true);
    }, std::ref (promisedFinished), task).detach ();

    EXPECT_TRUE (futureResult.wait_for (dur) != std::future_status::timeout);
}

template<class Rep, class Period, typename Function, typename... Args>
static void EXPECT_TIMEOUT (const std::chrono::duration<Rep, Period>& dur, Function&& f, Args&&... args)
{
    std::promise<bool> promisedFinished;
    auto futureResult = promisedFinished.get_future ();

    std::shared_ptr<TimedTask> task (new TimedTaskImpl<Args...> (std::forward<Function> (f), std::forward<Args> (args)...));

    std::thread ([](std::promise<bool>& finished, const std::shared_ptr<TimedTask>& _task)
    {
        _task->Run ();
        finished.set_value (true);
    }, std::ref (promisedFinished), task).detach ();

    EXPECT_FALSE (futureResult.wait_for (dur) != std::future_status::timeout);
}

//end time out part




PUSH_MSVC_IGNORE (6053 6054) // don't care about NULL termination warnings in tests...
USING_NAMESPACE_BENTLEY

struct BeFileTests1 : public testing::Test
{
public:
    BeFile m_file;
    BeFile m_file2;
    bvector <WCharCP> m_testData;


    //---------------------------------------------------------------------------------------
    // Initializes two file objects
    //
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void SetUp ()
    {
        //Will test files with these extensions
        m_testData.push_back (L"py");
        m_testData.push_back (L"cpp");
        m_testData.push_back (L"txt");

    }

    //---------------------------------------------------------------------------------------
    // Closes files
    //
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void TearDown ()
    {
        CloseFiles ();
    }

    //---------------------------------------------------------------------------------------
    // Closes two class files if open
    //
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void CloseFiles ()
    {
        if (m_file.IsOpen ())
            CloseFile (&m_file);
        if (m_file2.IsOpen ())
            CloseFile (&m_file2);
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void CloseFile (BeFile* file)
    {
        BeFileStatus status = file->Close ();
        ASSERT_TRUE (BeFileStatus::Success == status);
    }

    //---------------------------------------------------------------------------------------------
    // Prepares file for test with data specified
    // @param filePath      path where file should be placed/found
    // @param buf           string to write to file
    // @param repeatContent     how many times write buf string to file
    // @param reCreate      reCreate file if exists, if true will not change file if it exists
    //
    // @bsimethod
    //---------------------------------------------------------------------------------------------
    void PrepareFile (WCharCP  filePath, char const* buf, int repeatContent, bool reCreate = false)
    {
        BeFile  newFile;
        BeFileStatus status;
        if (!reCreate)
        {
            status = newFile.Open (filePath, BeFileAccess::Write);
            newFile.Close ();
        }
        else
        {
            status = BeFileStatus::UnknownError;
        }
        if (status != BeFileStatus::Success)
        {
            status = newFile.Create (filePath, true);
            ASSERT_TRUE (status == BeFileStatus::Success);
            uint32_t byteCountToCopy;
            uint32_t byteCount = 0;
            for (int i = 0; i<repeatContent; i++)
            {
                byteCountToCopy = (uint32_t)strlen (buf);
                status = newFile.Write (&byteCount, buf, byteCountToCopy);
                EXPECT_EQ (byteCount, byteCountToCopy);
                ASSERT_TRUE (status == BeFileStatus::Success);
            }
            newFile.Close ();
        }
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void CreateFile (BeFile* file, WCharCP filePath, bool createAlways = true)
    {
        BeFileStatus status = file->Create (filePath, createAlways);
        ASSERT_TRUE (status == BeFileStatus::Success) << "Failed to create file" << filePath;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    void OpenFile (BeFile* file, WCharCP filePath, BeFileAccess mode)
    {
        BeFileStatus status = BeFileStatus::Success;
        status = file->Open (filePath, mode);
        ASSERT_TRUE (status == BeFileStatus::Success) << "Failed to open file, file: " << filePath;
        ASSERT_TRUE (mode == file->GetAccess ()) << "Access mode does not match";
        ASSERT_TRUE (file->IsOpen ()) << "Should have status opened";
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod
    //---------------------------------------------------------------------------------------
    WCharCP CreatePathForTempFile (BeFileName * tmp, WCharCP fileName, WCharCP fileExtension)
    {
        BeTest::GetHost ().GetOutputRoot (*tmp);
        tmp->AppendToPath (fileName);
        tmp->AppendExtension (fileExtension);
        return tmp->GetName ();
    }
};


POP_MSVC_IGNORE
#endif // defined (BENTLEY_WIN32)