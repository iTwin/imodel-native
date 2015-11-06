/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/BackDoor/BackDoor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicAPI/BackDoor/DgnProject/BackDoor.h"
#include <DgnPlatform/RealityDataCache.h>
#include <Logging/bentleylogging.h>

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

namespace BackDoor
{

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    Kevin.Nyman     05/11
+---------------+---------------+---------------+---------------+---------------+------*/
namespace DirectionParser 
{
    void SetTrueNorthValue (Dgn::DirectionParser& parser, double const& trueNorth)
        {
        parser.SetTrueNorthValue (trueNorth);
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsinamespace                                                 KevinNyman      11/09
+---------------+---------------+---------------+---------------+---------------+------*/
    namespace DgnModel
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        Dgn::DgnModelId GetModelId (DgnModelR model)
            {
            return model.GetModelId();
            }

    };

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 JoshSchifter    11/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace NonVisibleViewport
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        DgnViewportP Create (ViewControllerR viewInfo)
            {
            return new Dgn::NonVisibleViewport(viewInfo);
            }

        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            JoshSchifter    11/09
        +-------+---------------+---------------+---------------+---------------+------*/
        void Delete (DgnViewportP viewport)
            {
            delete viewport;
            }

    }; // NonVisibleViewport

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace                                                 KevinNyman      07/09
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace PolyfaceQueryP
    {
        /*-------------------------------------------------------------------------**//**
        * @bsimethod                                            KevinNyman      07/09
        +-------+---------------+---------------+---------------+---------------+------*/
        double SumTetrahedralVolumes (BentleyApi::PolyfaceQueryP p, DPoint3dCR origin)
            {
            return p->SumTetrahedralVolumes (origin);
            }
    };

    /*---------------------------------------------------------------------------------**//**
    * @bsinamespace
    +---------------+---------------+---------------+---------------+---------------+------*/
    namespace RealityData
    {
        /*---------------------------------------------------------------------------------**//**
        * @bsiclass                                     Grigas.Petraitis                07/15
        +---------------+---------------+---------------+---------------+---------------+------*/
        struct Work : RefCounted<RealityDataWork>
        {
        typedef std::function<void()> Handler;
        private:
            Handler m_handler;
            Work(Handler const& handler) : m_handler(handler) {}
        protected:
            virtual void _DoWork() override {m_handler();}
        public:
            static RefCountedPtr<Work> Create(Handler const& handler) {return new Work(handler);}
        };

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod                                    Grigas.Petraitis                07/15
        +---------------+---------------+---------------+---------------+---------------+------*/
        void RunOnAnotherThread(std::function<void()> const& handler)
            {
            RealityDataWorkerThreadPtr thread = RealityDataWorkerThread::Create();
            thread->Start();
            thread->DoWork(*Work::Create([handler, thread]()
                {
                handler();
                thread->Terminate();
                }));
            }

    }; // RealityData
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbTestDgnManager::FindTestData (BeFileName& fullFileName, WCharCP fileName, CharCP callerSourceFile)
    {
    return TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Majd.Uddin                      05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnDbTestDgnManager::GetTestDataOut (BeFileName& outFullFileName, WCharCP fileName, WCharCP outName, CharCP callerSourceFile)
    {
    BeFileName fullFileName;
    if (TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile)) != SUCCESS)
        return ERROR;
    //Copy this to out path and then set that path
    BeTest::GetHost().GetOutputRoot (outFullFileName);
    outFullFileName.AppendToPath (outName);
    BeFileName::CreateNewDirectory (BeFileName::GetDirectoryName(outFullFileName).c_str());
    if (BeFileName::BeCopyFile (fullFileName, outFullFileName) != BeFileNameStatus::Success)
        return ERROR;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetUtDatPath (CharCP callerSourceFile)
    {
    if (NULL == callerSourceFile || !*callerSourceFile)
        return BeFileName (L"DgnDb");   // just look in the docs root

    // start looking in the sub-directory that is specific to this UT. (Then look in each parent directory in turn.)
    BeFileName path (L"DgnDb");
    path.AppendToPath (L"Published");           // *** NB: This must agree with the directory structure of the DgnPlatformTest/DgnDbUnitTests repository
    path.AppendToPath (L"DgnHandlers");
    path.AppendToPath (BeFileName (BeFileName::Basename, BeFileName(callerSourceFile)).GetName());
    return path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetReadOnlyTestData (WCharCP fileName, CharCP callerSourceFile, bool forceMakeCopy)
    {
    BeFileName fullFileName;
    StatusInt status;
    if (forceMakeCopy)
        status = GetTestDataOut (fullFileName, fileName, fileName, callerSourceFile);
    else
        status = TestDataManager::FindTestData (fullFileName, fileName, GetUtDatPath(callerSourceFile));

    if (SUCCESS != status)
        {
        NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to find project name=\"%ls\" callerSourceFile=\"%ls\"", fileName, WString(callerSourceFile,BentleyCharEncoding::Utf8).c_str());
        BeAssert (false && "failed to open find input project file");
        return BeFileName(L"");
        }
    return fullFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeFileName DgnDbTestDgnManager::GetWritableTestData (WCharCP fileName, CharCP callerSourceFile)
    {
    BeFileName fullFileName;
    if (GetTestDataOut (fullFileName, fileName, fileName, callerSourceFile) != SUCCESS)
        {
        NativeLogging::LoggingManager::GetLogger (L"BeTest")->errorv (L"failed to find or copy project name=\"%ls\" callerSourceFile=\"%ls\"", fileName, WString(callerSourceFile,BentleyCharEncoding::Utf8).c_str());
        BeAssert (false && "failed to open find or copy input project file");
        return BeFileName(L"");
        }
    return fullFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbTestDgnManager::DgnDbTestDgnManager (WCharCP dgnfilename, CharCP callerSourceFile, BeSQLite::Db::OpenMode omode, DgnInitializeMode imode, bool forceMakeCopy) 
    : TestDgnManager ((BeSQLite::Db::OpenMode::Readonly==omode) ? GetReadOnlyTestData(dgnfilename,callerSourceFile, forceMakeCopy) : GetWritableTestData(dgnfilename,callerSourceFile), omode, imode) 
    {}

END_DGNDB_UNIT_TESTS_NAMESPACE
