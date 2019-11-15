/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//#if _MSC_VER > 1800
//    #pragma warning (suppress: 5031) // likely mismatch, popping warning state pushed in different file
//    #pragma warning (pop)
//#endif

//#if _MSC_VER > 1600
//    #if _MSC_VER > 1800
//        #pragma warning (suppress: 5031) // likely mismatch, popping warning state pushed in different file
//        #pragma warning (pop)
//    #else
//        #pragma warning (pop)
//    #endif
//#endif

#ifdef WIN32
#if _MSC_VER > 1800
    #pragma warning (suppress: 5031) // likely mismatch, popping warning state pushed in different file
    #pragma warning (pop)
#else
    #pragma warning (pop)
#endif
#endif
