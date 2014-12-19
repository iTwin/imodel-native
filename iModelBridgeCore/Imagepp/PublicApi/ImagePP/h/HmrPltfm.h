//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/h/HmrPltfm.h $
//:>
//:>  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

/*====================================================================
** Defines specific to the operating system
**====================================================================*/
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android

// TR 159688 : Fix copy constructor of templated class.
// The bug can be considedered as fixed if you remove this define and you get a compile error in
// HGF2DHoledFence.h because of this line : IF THIS RETURNS AN ERROR THEN IT ATTEMPTED TO COMPILE BODY ... YOUPPIE!
// If it compiles the bug is STILL there.
// To validate the fix, make sure the copy constructor is called in HGF2DHoledFence<DataType>::Clone().

#elif defined (_WIN32)
/*====================================================================
** Defines specific to some compiler. Bugs or specifics features
**====================================================================*/
/* Inline methods up to 64 levels deep */
#   pragma inline_depth(64)

#   if _MSC_VER >= 1400
/* This is Visual C++ 8.0 */


#   elif 
    // MSVC++ 7.1  _MSC_VER = 1310 
    // MSVC++ 7.0  _MSC_VER = 1300 
    // MSVC++ 6.0  _MSC_VER = 1200 
    // MSVC++ 5.0  _MSC_VER = 1100 
#       error Unsupported compiler
#   endif

#endif /* _MSC_VER */
