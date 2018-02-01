/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Bentley/BeDebugUtilities.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Bentley.h"
#include "WString.h"
#include <time.h>

BEGIN_BENTLEY_NAMESPACE

//=======================================================================================    
//! Debugging related utilities.
//! Enabled for debug builds, disabled for release builds to avoid additional linkage.
//=======================================================================================    
struct BeDebugUtilities
    {
    //! Information about specific frame in stack
    struct StackFrameInfo
        {
        Utf8String functionName;
        Utf8String fileName;
        uint64_t fileLine = 0;
        bool IsValid() const { return !functionName.empty() && !fileName.empty(); };
        };

    //! Get stack trace formatted into newlines. Function is not included in frames.
    //! NOTE: output depends on platform, build configuration, and could be not available as well. 
    //! @param maxFrames maximum number of frames (depth) to return
    //! @return newline seperated stack frame descriptions. Empty if not implemented or could not get information.     
    //! Each line in should contain following information: {Frame number 1..n} {Module} {Address} {Symbol name}
    BENTLEYDLL_EXPORT static Utf8String GetStackTraceDescription(size_t maxFrames);
    
    //! Get stack frame information at specific index. 0 is current frame.
    //! NOTE: implemented for Windows x64 only
    BENTLEYDLL_EXPORT static StackFrameInfo GetStackFrameInfoAt(size_t frameIndex);

    //! Get process memory use in bytes
    //! @return bytes currently allocated by process. Zero if not implemented or could not get information.
    BENTLEYDLL_EXPORT static size_t GetMemoryUsed();
    };

END_BENTLEY_NAMESPACE
