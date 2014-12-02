/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/PlatformTextServices.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

// Windows-only

#include <Windows.h>
#include <objbase.h>
#include <usp10.h>

#include "DgnPlatformInternal.h"
#include <DgnPlatformInternal/DgnCore/PlatformTextServices.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
BentleyStatus UniscribeServices::ItemizeString(WCharCP unicodeString, size_t numCharacters, T_ScriptItemVectorR scriptItems, T_OpenTypeTagVectorR scriptTags, size_t& numScriptItems)
    {
    //...............................................................................................................................................
    // I think we want digit substitution... so set this up for ScriptItemize.

    SCRIPT_CONTROL scriptControl;
    memset(&scriptControl, 0, sizeof (scriptControl));
    
    SCRIPT_STATE scriptState;
    memset(&scriptState, 0, sizeof (scriptState));

    POSTCONDITION(S_OK == ::ScriptApplyDigitSubstitution(NULL, &scriptControl, &scriptState), ERROR)

    // It worries me that I have to set this. You can see the effects of this in, say, notepad, when you enter Arabic text followed by a neutral character (e.g. a space, 0x0020). If this is 0 (default; LTR reading direction), the space will appear to the visual right of the Arabic text. If set to 1 (RTL reading direction), the space will appear to the visual left. Word always seems to display it to the left (regardless of specified reading direction), and I feel we always want that as well. I do not understand why this is not the default, but I am setting it always until I discover I can't.
    scriptControl.fInvertPostBoundDir = 1;

    // Again, having this drives me nuts because isn't this Uniscribe's job? As documented, ::ScriptItemize needs an initial state for uBidiLevel for bi-directional contexts... how should we detect bi-directional contexts?!
    // Google Chrome does this by asking the ICU library to classify the beginning of the string as LTR or RTL... hopefully this check is "good enough" so we don't have to include another third-party library.
    if (DgnFontManager::IsUsingAnRtlLocale() || (LANG_ARABIC == scriptControl.uDefaultLanguage) || (LANG_HEBREW == scriptControl.uDefaultLanguage))
        scriptState.uBidiLevel = 1;

    //...............................................................................................................................................
    // Use ScriptItemize to break the string into items (e.g. runs of single script and direction).

    HRESULT hr;
    int     numScriptItemsI;

    // There is no way to know up-front how many script items are required. The docs say to keep calling/reallocating until it doesn't return E_OUTOFMEMORY. SCRIPT_ITEM is 8 bytes; preferring to allocate a bunch initially instead of going through the loop to save a little memory. Brett comments that Uniscribe can internally use up more items than you actually get as output (another reason to not be too stingy).
    if (scriptItems.size() < UniscribeServices::GetRecommendedScripItemVectorSize())
        scriptItems.resize(UniscribeServices::GetRecommendedScripItemVectorSize());

    scriptTags.resize(scriptItems.size());

    while (S_OK != (hr = ::ScriptItemizeOpenType(unicodeString, (int)numCharacters, (int)(scriptItems.size() - 1), &scriptControl, &scriptState, &scriptItems[0], &scriptTags[0], &numScriptItemsI)))
        {
        POSTCONDITION (E_OUTOFMEMORY == hr, ERROR)
        
        scriptItems.resize(2 * scriptItems.size());
        scriptTags.resize(scriptItems.size());
        }

    numScriptItems = (size_t)numScriptItemsI;

    return SUCCESS;
    }
