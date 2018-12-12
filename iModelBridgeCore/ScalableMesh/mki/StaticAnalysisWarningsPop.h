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

#if _MSC_VER > 1800
    #pragma warning (suppress: 5031) // likely mismatch, popping warning state pushed in different file
    #pragma warning (pop)
#else
    #pragma warning (pop)
#endif