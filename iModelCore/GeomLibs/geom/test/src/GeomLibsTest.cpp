/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "testHarness.h"
#include <Geom/GeomApi.h>
#include "BSIBaseGeomExaminerGtestHelper.h"
#include "StackExaminer.h"
#include "StackExaminerGtestHelper.h"

StackExaminer* g_stackExaminer = NULL;
BSIBaseGeomExaminer* g_bsiBaseGeomExaminer = NULL;
struct VuPrintFuncs : GeomPrintFuncs
{
public:
virtual void EmitChar (char ch)
    {
    printf ("%c", ch);
    }
};

int main(int argc, char **argv)
    {
#if defined (_DEBUG)
    int dbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);

    /*
     * Set the debug-heap flag to no longer keep freed blocks in the
     * heap's linked list and turn on Debug type allocations (CLIENT)
     */
    dbgFlag |= _CRTDBG_ALLOC_MEM_DF;
    dbgFlag |= _CRTDBG_DELAY_FREE_MEM_DF;

    // Clear the upper 16 bits and OR in the desired freqency
    dbgFlag = (dbgFlag & 0x0000FFFF) | _CRTDBG_CHECK_EVERY_1024_DF;

    _CrtSetDbgFlag(dbgFlag);
#endif

#ifdef argHandler
    ArgHandlerList handlers;
    Args arghandlers;
    arghandlers.AddArgsToList (handlers);

    CommandLineArgHandler args (handlers, argc, argv);

    int status = 0;
    if (args.ShouldRunTests())
        {
        ::testing::InitGoogleTest (args.GetCountP (), args.GetArgVP ());

        g_stackExaminer = new StackExaminer;  // this must be alloced because gtest frees it up.
        ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
        listeners.Append (g_stackExaminer);

        status = RUN_ALL_TESTS();
        }
#else

        ::testing::InitGoogleTest (&argc, argv);

        g_stackExaminer = new StackExaminer;  // this must be alloced because gtest frees it up.
        g_bsiBaseGeomExaminer = new BSIBaseGeomExaminer ();  // this must be alloced because gtest frees it up.
        ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
        listeners.Append (g_stackExaminer);
        listeners.Append (g_bsiBaseGeomExaminer);

#endif
//    VuPrintFuncs printer;
    static bool s_debug = true;
//    GeomPrintFuncs::SetDefault (&printer);
//    GeomPrintFuncs::EnableDefault (L"PolyfaceClip::AnalyzeCutPlaneLoops", s_debug);


    int stat = RUN_ALL_TESTS();
    g_stackExaminer->DumpStackInfo();

    printf (" BSIBaseGeom counters: (Malloc %ld) (Calloc %ld) (Realloc %ld) (Free %ld) (DIFF %ld)\n",
                    BSIBaseGeom::GetNumMalloc (),
                    BSIBaseGeom::GetNumCalloc (),
                    BSIBaseGeom::GetNumRealloc (),
                    BSIBaseGeom::GetNumFree (),
                    BSIBaseGeom::GetAllocationDifference ());

    return stat;
    }
