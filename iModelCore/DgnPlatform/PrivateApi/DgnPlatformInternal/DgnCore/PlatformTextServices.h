/*--------------------------------------------------------------------------------------+
|
|     $Source: PrivateApi/DgnPlatformInternal/DgnCore/PlatformTextServices.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

// WIP_NONPORT - Text
#if defined (BENTLEY_WIN32)

    //=======================================================================================
    //! Provides wrappers around Uniscribe to provide easier access to re-usable functionality.
    // @bsiclass                                                    Jeff.Marker     06/2010
    //=======================================================================================
    struct UniscribeServices
    {
        typedef bvector<struct tag_SCRIPT_ITEM> T_ScriptItemVector;
        typedef T_ScriptItemVector& T_ScriptItemVectorR;
        typedef bvector<unsigned long /*OPENTYPE_TAG*/> T_OpenTypeTagVector;
        typedef T_OpenTypeTagVector& T_OpenTypeTagVectorR;

    private:
        //! This is a static class; you are not intended to create instances of it.
        UniscribeServices() { }

    public:
        //! When creating a vector to give to ItemizeString, it is recommended to pre-allocate this much space.
        static size_t GetRecommendedScripItemVectorSize() { return 16; }

        //! Calls ::ScriptItemize successively until all script items can be created (or a real error is encountered). Also sets up digit substitution for ::ScriptItemize.
        DGNPLATFORM_EXPORT static BentleyStatus ItemizeString(WCharCP, size_t numCharacters, T_ScriptItemVectorR, T_OpenTypeTagVectorR, size_t& numScriptItems);
    }; // UniscribeServices

#endif

//=======================================================================================
//! Describes why you want a word boundary... affects where methods in WordBoundaryServices detect boundaries for different purposes.
// @bsiclass                                                    Jeff.Marker     07/2013
//=======================================================================================
enum class WordBoundaryReason
{
    WordWrapping, //!< Finds the nearest word break that you can break a line at. The goal of this type is to control where TextBlock (or similar) will break a line for word-wrapping.
    FindingWords, //!< Finds the nearest word break that actually delimits a word. The goal of this type is to isolate actual words (for things like spell checking and whole word search).
    CaretPositioning //!< Finds the nearest word break that you can position a caret at. The goal of this type is to control where Crtl+Left/Right ends up.
}; // WordBoundaryReason

//=======================================================================================
//! Provides platform-agnostic word boundary services.
// @bsiclass                                                    Jeff.Marker     07/2013
//=======================================================================================
struct WordBoundaryServices
{
    //! Finds the next boundary requested, looking backwards.
    DGNPLATFORM_EXPORT static size_t FindPreviousWordBoundary(WordBoundaryReason, WCharCP, size_t numCharacters, size_t offset);

    //! Finds the next boundary requested, looking forwards.
    DGNPLATFORM_EXPORT static size_t FindNextWordBoundary(WordBoundaryReason, WCharCP, size_t numCharacters, size_t offset);

    //! Determine if the given index is between words. This method assumes WordBoundaryReason::FindingWords, and as such, is intended to isolate actual words (for things like spell checking and whole word search).
    DGNPLATFORM_EXPORT static bool IsAtWordBoundary(WCharCP, size_t numCharacters, size_t offset);
}; // WordBoundaryServices

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
