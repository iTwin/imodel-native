/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <signal.h>
#include <stdio.h>

namespace IModelJsNative {

//=======================================================================================
//! @bsiclass
//=======================================================================================
enum SignalType
{
    None = 0,
    Abort  = 1,             // Calls abort
    RaiseSigSev = 2,        // Calls raise(SIGSEV)
    DereferenceNull = 3,    // Dereferences null ptr
    DivideByZero = 4,       // Evaluates 1 / 0
    StackOverflow = 5,      // Causes StackOverflow
    ThreadDeadlock = 6      // Causes ThreadDeadlock
};

//=======================================================================================
//! @bsiclass
//=======================================================================================
struct SignalTestUtility
{
private:
    static void StackOverflow(int depth);
    static void ThreadDeadlock();
public:
    static bool Signal(SignalType signalType);
};

}
