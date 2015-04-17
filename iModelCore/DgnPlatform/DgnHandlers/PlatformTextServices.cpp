/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/PlatformTextServices.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
 // WIP_NONPORT
#if defined (BENTLEY_WIN32)
    #include <Windows.h>
    #include <objbase.h>
    #include <usp10.h>
#elif defined (__APPLE__)
    #include <CoreFoundation/CoreFoundation.h>
#elif defined (ANDROID)
    #include <icu4c/unicode/brkiter.h>
    #include <icu4c/unicode/locid.h>

    // Conflicts with ICU.
    #define NO_UCHAR_TYPEDEF
#endif

#include "DgnPlatformInternal.h"
#include <DgnPlatformInternal/DgnCore/PlatformTextServices.h>
#include <GeneratedHeaders/WordBreakData.h>

//#define DEBUG_UNISCRIBE_WORD_BOUNDARIES

USING_NAMESPACE_BENTLEY_DGNPLATFORM

#if defined (BENTLEY_WIN32)

//---------------------------------------------------------------------------------------
// This implementation is based on Unicode TR 29 (http://www.unicode.org/reports/tr29/)
// Note: "It is not possible to provide a uniform set of rules that resolves all issues across languages or that handles all ambiguous situations within a given language. The goal for the specification presented in this annex is to provide a workable default; tailored implementations can be more sophisticated."
// See associated WordBreakProperty.txt and WordBreakDataGenerator.py for the data that backs this algorithm.
// This algorithm should be a pure implementation of TR 29; edits have been made, however, to its underlying data file.
//
// @bsimethod                                                   Jeff.Marker     02/2011
//---------------------------------------------------------------------------------------
static bool isAtWordBoundaryTR29 (WCharCP unicodeString, size_t numCharacters, size_t offset)
    {
    // Key:
    //  ÷   allow break
    //  ×   disallow break
    //  ->  transform
    
    // Break at the start and end of text.
    // WB1: sot ÷
    
    if (0 == offset)
        return true;
    
    // WB2: ÷ eot

    if (offset >= numCharacters)
        return true;
    
    // Nice because we now know we have a character to the left and right!

    WCharCP   lastChar    = (unicodeString + numCharacters - 1);
    WCharCP   lhs         = (unicodeString + offset - 1);
    WCharCP   rhs         = (unicodeString + offset);

    // Do not break within CRLF.
    // WB3: CR × LF

    bool    isLhsCR = WordBreakRangeUtilities::IsCR (*lhs);
    bool    isRhsLF = WordBreakRangeUtilities::IsLF (*rhs);

    if (isLhsCR && isRhsLF)
        return false;

    // Otherwise break before and after Newlines (including CR and LF).
    // WB3a: (Newline | CR | LF) ÷
    
    bool    isLhsNewLine    = WordBreakRangeUtilities::IsNewline (*lhs);
    bool    isLhsLF         = WordBreakRangeUtilities::IsLF (*lhs);
    
    if (isLhsNewLine || isLhsCR || isLhsLF)
        return true;
    
    // WB3b: ÷ (Newline | CR | LF)

    bool    isRhsNewLine    = WordBreakRangeUtilities::IsNewline (*rhs);
    bool    isRhsCR         = WordBreakRangeUtilities::IsCR (*rhs);
    
    if (isRhsNewLine || isRhsCR || isRhsLF)
        return true;

    // Ignore Format and Extend characters, except when they appear at the beginning of a region of text.
    // WB4: X (Extend | Format)* -> X
    
    bool    isRhsExtend = WordBreakRangeUtilities::IsExtend (*rhs);
    bool    isRhsFormat = WordBreakRangeUtilities::IsFormat (*rhs);
    
    while (isRhsExtend || isRhsFormat)
        {
        ++rhs;

        // Test end-of-text scenario (WB2).
        if (rhs > lastChar)
            return true;
        }

    // Do not break between most letters.
    // WB5: ALetter × ALetter

    bool    isLhsALetter = WordBreakRangeUtilities::IsALetter (*lhs);
    bool    isRhsALetter = WordBreakRangeUtilities::IsALetter (*rhs);

    if (isLhsALetter && isRhsALetter)
        return false;
    
    // Do not break letters across certain punctuation.
    // WB6: ALetter × (MidLetter | MidNumLet) ALetter
    
    bool    canPeekRhs      = (rhs < (lastChar - 1));
    bool    isRhsMidLetter  = WordBreakRangeUtilities::IsMidLetter (*rhs);
    bool    isRhsMidNumLet  = WordBreakRangeUtilities::IsMidNumLet (*rhs);

    if (canPeekRhs && isLhsALetter && (isRhsMidLetter || isRhsMidNumLet) && WordBreakRangeUtilities::IsALetter (*(rhs + 1)))
        return false;

    // WB7: ALetter (MidLetter | MidNumLet) × ALetter
    
    bool    canPeekLhs      = ((lhs - 1) >= unicodeString);
    bool    isLhsMidNumLet  = WordBreakRangeUtilities::IsMidNumLet (*lhs);
    bool    isLhsMidLitter  = WordBreakRangeUtilities::IsMidLetter (*lhs);

    if (canPeekLhs && WordBreakRangeUtilities::IsALetter (*(lhs - 1)) && (isLhsMidLitter || isLhsMidNumLet) && isRhsALetter)
        return false;

    // Do not break within sequences of digits, or digits adjacent to letters ("3a", or "A3").
    // WB8: Numeric × Numeric

    bool    isLhsNumeric    = WordBreakRangeUtilities::IsNumeric (*lhs);
    bool    isRhsNumeric    = WordBreakRangeUtilities::IsNumeric (*rhs);

    if (isLhsNumeric && isRhsNumeric)
        return false;

    // WB9: ALetter × Numeric

    if (isLhsALetter && isRhsNumeric)
        return false;
    
    // WB10: Numeric × ALetter

    if (isLhsNumeric && isRhsALetter)
        return false;

    // Do not break within sequences, such as "3.2" or "3,456.789".
    // WB11: Numeric (MidNum | MidNumLet) × Numeric

    bool isLhsMidNum = WordBreakRangeUtilities::IsMidNumLet (*lhs);

    if (canPeekLhs && WordBreakRangeUtilities::IsNumeric (*(lhs - 1)) && (isLhsMidNum || isLhsMidNumLet) && isRhsNumeric)
        return false;

    // WB12: Numeric × (MidNum | MidNumLet) Numeric
    
    bool isRhsMidNum = WordBreakRangeUtilities::IsMidNumLet (*rhs);

    if (canPeekRhs && isLhsNumeric && (isRhsMidNum || isRhsMidNumLet) && WordBreakRangeUtilities::IsNumeric (*(rhs + 1)))
        return false;

    // Do not break between Katakana.
    // WB13: Katakana × Katakana
    
    bool    isLhsKatakana   = WordBreakRangeUtilities::IsKatakana (*lhs);
    bool    isRhsKatakana   = WordBreakRangeUtilities::IsKatakana (*rhs);

    if (isLhsKatakana && isRhsKatakana)
        return false;

    // Do not break from extenders.
    // WB13a: (ALetter | Numeric | Katakana | ExtendNumLet) × ExtendNumLet

    bool    isLhsExtendNumLet   = WordBreakRangeUtilities::IsExtendNumLet (*lhs);
    bool    isRhsExtendNumLet   = WordBreakRangeUtilities::IsExtendNumLet (*rhs);

    if ((isLhsALetter || isLhsNumeric || isLhsKatakana || isLhsExtendNumLet) && isRhsExtendNumLet)
        return false;

    // WB13b: ExtendNumLet × (ALetter | Numeric | Katakana)
    
    if (isLhsExtendNumLet && (isRhsALetter || isRhsNumeric || isRhsKatakana))
        return false;

    // Otherwise, break everywhere (including around ideographs).
    // WB14: Any ÷ Any

    return true;
    }

//---------------------------------------------------------------------------------------
// This comes from empirical testing with Word.
// The intention is to negate some scenarios of isAtWordBoundaryTR29.
// @bsimethod                                                   Jeff.Marker     02/2011
//---------------------------------------------------------------------------------------
static bool shouldIngoreWordBoundary (WCharCP unicodeString, size_t offset)
    {
    if (0 == offset)
        return false;
    
    // Don't stop between successive punctuation.
    if (iswpunct (unicodeString[offset - 1]) && iswpunct (unicodeString[offset]))
        return true;
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2011
//---------------------------------------------------------------------------------------
static bool isQuote (WChar charValue)
    {
    // I think U+0022, U+201c, and U+201d are obvious; U+ff02 is also promising.
    //  The others I found via a text search of Unicode character descriptions as potential candidates, but am not including them without due cause.
    
    switch (charValue)
        {
        case 0x0022:    // QUOTATION MARK
        case 0x201c:    // LEFT DOUBLE QUOTATION MARK
        case 0x201d:    // RIGHT DOUBLE QUOTATION MARK
        //case 0x201e:    // DOUBLE LOW-9 QUOTATION MARK
        //case 0x201f:    // DOUBLE HIGH-REVERSED-9 QUOTATION MARK
        //case 0x275d:    // HEAVY DOUBLE TURNED COMMA QUOTATION MARK ORNAMENT
        //case 0x275e:    // HEAVY DOUBLE COMMA QUOTATION MARK ORNAMENT
        //case 0x2760:    // HEAVY LOW DOUBLE COMMA QUOTATION MARK ORNAMENT
        case 0xff02:    // FULLWIDTH QUOTATION MARK
            return true;
        }
    
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2011
//---------------------------------------------------------------------------------------
static bool isAtWordBoundaryCustom (WCharCP unicodeString, size_t numCharacters, size_t offset, WordBoundaryReason boundaryType)
    {
    if (!isAtWordBoundaryTR29 (unicodeString, numCharacters, offset))
        return false;
    
    if (shouldIngoreWordBoundary (unicodeString, offset))
        return false;
    
    if (WordBoundaryReason::WordWrapping != boundaryType)
        return true;
    
    // We have taken this rule out of the word break data file, because we want to use the generic TR29 algorithm for caret stops, but line breaks don't want to stop here (WB6 and WB7).
    
    if ((offset < (numCharacters - 1)) && WordBreakRangeUtilities::IsMidNumLetEx1 (unicodeString[offset]) && WordBreakRangeUtilities::IsALetter (unicodeString[offset + 1]))
        return false;

    if ((offset > 0) && WordBreakRangeUtilities::IsMidNumLetEx1 (unicodeString[offset - 1]) && WordBreakRangeUtilities::IsALetter (unicodeString[offset]))
        return false;

    return true;
    }

#endif

#if defined (BENTLEY_WIN32)

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2011
//---------------------------------------------------------------------------------------
static size_t findScriptItemForCharacterOffset (UniscribeServices::T_ScriptItemVector const& scriptItems, size_t numScriptItems, size_t characterOffset)
    {
    for (size_t iScriptItem = 0; iScriptItem < numScriptItems; ++iScriptItem)
        {
        // ::ScriptItemize will always put a trailing fake SCRIPT_ITEM into the array, pointing one past the last real item, allowing you to always look one past numScriptItems to compute number of characters.
        if (characterOffset >= (size_t)scriptItems[iScriptItem].iCharPos && characterOffset < (size_t)scriptItems[iScriptItem + 1].iCharPos)
            return iScriptItem;
        }
    
    return (numScriptItems - 1);
    }

//---------------------------------------------------------------------------------------
// Official Msdn docs: http://msdn.microsoft.com/en-us/library/dd374091(VS.85).aspx
// Additional documentation and examples from a Google Chrome developer (Brett Wilson): http://maxradi.us/documents/uniscribe/.
//
// This helper is only meant to support WordBoundaryReason::CaretPositioning and WordBoundaryReason::WordWrapping; behavior is undefined for other types.
//
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
static size_t findWordBoundary (WordBoundaryReason boundaryType, WCharCP unicodeString, size_t numCharacters, size_t offset, int walkIncrement)
    {
    //...............................................................................................................................................
    // Error checking...
    
    if ((NULL == unicodeString) || (0 == walkIncrement))
        { BeAssert (false); return 0; }

    //...............................................................................................................................................
    // Early (fast) return?
    
    if (offset > numCharacters)
        { BeAssert (false); return numCharacters; }

    bool isWalkingForward = (walkIncrement > 0);

    if ((0 == numCharacters) || (!isWalkingForward && (0 == offset)))
        return 0;

    if (isWalkingForward && (offset >= (numCharacters - 1)))
        return numCharacters;

    //...............................................................................................................................................
    UniscribeServices::T_ScriptItemVector   scriptItems     (UniscribeServices::GetRecommendedScripItemVectorSize ());
    UniscribeServices::T_OpenTypeTagVector  scriptTags      (UniscribeServices::GetRecommendedScripItemVectorSize());
    size_t                                  numScriptItems  = 0;
    
    if (SUCCESS != UniscribeServices::ItemizeString(unicodeString, numCharacters, scriptItems, scriptTags, numScriptItems))
        return ERROR;

    //...............................................................................................................................................
    // Determine where to start walking from.
    
    size_t originalScriptItem = findScriptItemForCharacterOffset (scriptItems, numScriptItems, offset);
    
    offset += walkIncrement;

    size_t iScriptItem = findScriptItemForCharacterOffset (scriptItems, numScriptItems, offset);

    // While we would prefer to always use Uniscribe, Uniscribe documentation explains that it is up to us to decide what to do between script items. WTF mate?!
    //  Note (Uniscribe): "Decide if the first code point of a range should be a word break point because the last character of the previous range requires it. For example, if one range ends in a comma, consider the first character of the next range to be a word break point."
    //  If we just crossed into a different script item, then simply let logic below find the next boundary within it instead.
    if (isWalkingForward && (originalScriptItem != iScriptItem) && isAtWordBoundaryCustom (unicodeString, numCharacters, offset, boundaryType))
        {
#ifdef DEBUG_UNISCRIBE_WORD_BOUNDARIES
        ::OutputDebugStringW (L"UniscribeServices: Found word boundary via TR29\n");
#endif
        return offset;
        }

    //...............................................................................................................................................
    // Analyze the script item and find a word break.
    
    for (; iScriptItem < numScriptItems; iScriptItem += walkIncrement)
        {
        size_t                      numCharsInScriptItem    = (scriptItems[iScriptItem + 1].iCharPos - scriptItems[iScriptItem].iCharPos);
        ScopedArray<SCRIPT_LOGATTR> logicalAttributesBuff   (numCharsInScriptItem);
        SCRIPT_LOGATTR*             logicalAttributes       = logicalAttributesBuff.GetData ();
        
        if (0 != ::ScriptBreak (unicodeString + scriptItems[iScriptItem].iCharPos, (int)numCharsInScriptItem, &scriptItems[iScriptItem].a, logicalAttributes))
            { BeAssert (false); return ERROR; }
        
        while ((offset >= (size_t)scriptItems[iScriptItem].iCharPos) && (offset < (size_t)scriptItems[iScriptItem + 1].iCharPos))
            {
            size_t                  offsetInCurrentScriptItem   = (offset - scriptItems[iScriptItem].iCharPos);
            SCRIPT_LOGATTR const&   logAttrs                    = logicalAttributes[offsetInCurrentScriptItem];
            bool                    shouldBreak                 = false;
        
            switch (boundaryType)
                {
                case WordBoundaryReason::CaretPositioning:
                    {
                    shouldBreak = logAttrs.fWordStop;

                    // This comes from empirical testing with Word.
                    // While we want to stop on both sides of a quote, the left-hand side typically (always?) sees a script item break, and thus our inter-script item logic will catch it.
                    // I suppose we could use ::GetStringTypeW with (C1_ALPHA | C1_DIGIT) instead of IsALetter, but this is easier at the moment and hopefully suffices.
                    if (!shouldBreak && (offset > 0) && isQuote (unicodeString[offset - 1]) && WordBreakRangeUtilities::IsALetter (unicodeString[offset]))
                        {
#ifdef DEBUG_UNISCRIBE_WORD_BOUNDARIES
                        ::OutputDebugStringW (L"UniscribeServices: Altering word boundary via custom heuristic\n");
#endif
                        shouldBreak = true;
                        }

                    break;
                    }
                
                case WordBoundaryReason::WordWrapping:
                    {
                    shouldBreak = (logAttrs.fSoftBreak || logAttrs.fWhiteSpace);
                    
                    // I'm surprised that Uniscribe doesn't allow line breaking at hyphens...
                    if (!shouldBreak && (offset > 0) && (0x002d == unicodeString[offset - 1]))
                        {
#ifdef DEBUG_UNISCRIBE_WORD_BOUNDARIES
                        ::OutputDebugStringW (L"UniscribeServices: Altering word boundary via custom heuristic\n");
#endif
                        shouldBreak = true;
                        }
                    
                    break;
                    }
                
                default:
                    {
                    BeAssert (false && L"Unknown or unexpected WordBoundaryReason");
                    break;
                    }
                }
            
            if (shouldBreak)
                {
#ifdef DEBUG_UNISCRIBE_WORD_BOUNDARIES
                ::OutputDebugStringW (L"UniscribeServices: Found word boundary via Uniscribe\n");
#endif
                return offset;
                }
        
            offset += walkIncrement;
            }
        
        // When we cross script items, it is up to us to decide what to do...
        if (isAtWordBoundaryCustom (unicodeString, numCharacters, (isWalkingForward ? offset : (offset + 1)), boundaryType))
            {
#ifdef DEBUG_UNISCRIBE_WORD_BOUNDARIES
            ::OutputDebugStringW (L"UniscribeServices: Found word boundary via TR29\n");
#endif
            return (isWalkingForward ? offset : (offset + 1));
            }
        }
    
#ifdef DEBUG_UNISCRIBE_WORD_BOUNDARIES
    ::OutputDebugStringW (L"UniscribeServices: Found word boundary via Uniscribe\n");
#endif

    return (isWalkingForward ? scriptItems[iScriptItem + 1].iCharPos : scriptItems[iScriptItem].iCharPos);
    }

#elif defined (__APPLE__)

//=======================================================================================
//! Ensures CFRelease is called on the provided object when this wrapper goes out of scope.
// @bsiclass                                                    Jeff.Marker     07/2013
//=======================================================================================
template<typename T>
struct AutoCFRelease
{
private:
    T m_object;

public:
    AutoCFRelease(T object) { m_object = object; }
    ~AutoCFRelease() { ::CFRelease(m_object); }
    operator T() { return m_object; }
    bool IsNull() { return (NULL == m_object); }
}; // AutoCFRelease

//---------------------------------------------------------------------------------------
// This helper is only meant to support WordBoundaryReason::CaretPositioning and WordBoundaryReason::WordWrapping; behavior is undefined for other types.
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
static size_t findWordBoundary(WordBoundaryReason boundaryType, WCharCP unicodeString, size_t numCharacters, size_t offset, int walkIncrement)
    {
    //...............................................................................................................................................
    // Error checking...
    
    if ((NULL == unicodeString) || (0 == walkIncrement))
        { BeAssert(false); return 0; }

    //...............................................................................................................................................
    // Early (fast) return?
    
    if (offset > numCharacters)
        { BeAssert(false); return numCharacters; }

    bool isWalkingForward = (walkIncrement > 0);

    if ((0 == numCharacters) || (!isWalkingForward && (0 == offset)))
        return 0;

    if (isWalkingForward && (offset >= (numCharacters - 1)))
        return numCharacters;

    //...............................................................................................................................................
    // Create a CFTokenzier to answer the question.

    CFOptionFlags tokenizerOptions;
    switch (boundaryType)
        {
        case WordBoundaryReason::WordWrapping: tokenizerOptions = kCFStringTokenizerUnitLineBreak; break;
        case WordBoundaryReason::CaretPositioning: tokenizerOptions = kCFStringTokenizerUnitWord; break;

        default:
            BeAssert(false); // Unknown or unexpected WordBoundaryReason
            tokenizerOptions = kCFStringTokenizerUnitWord;
            break;
        }

    AutoCFRelease<CFStringRef> cfString(::CFStringCreateWithBytesNoCopy(kCFAllocatorDefault,
                                                                            (uint8_t const*)unicodeString,
                                                                            (numCharacters * sizeof(WChar)),
                                                                            kCFStringEncodingUTF32LE,
                                                                            false,
                                                                            kCFAllocatorNull));
    if (cfString.IsNull())
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    CFRange tokenizerRange = ::CFRangeMake(0, numCharacters);
    
    AutoCFRelease<CFLocaleRef> currentLocale(::CFLocaleCopyCurrent());
    if (currentLocale.IsNull())
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    AutoCFRelease<CFStringTokenizerRef> tokenizer(::CFStringTokenizerCreate(kCFAllocatorDefault,
                                                                                cfString,
                                                                                tokenizerRange,
                                                                                tokenizerOptions,
                                                                                currentLocale));
    if (tokenizer.IsNull())
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    if (kCFStringTokenizerTokenNone == ::CFStringTokenizerGoToTokenAtIndex(tokenizer, offset))
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    CFRange startingTokenRange = ::CFStringTokenizerGetCurrentTokenRange(tokenizer);
    if (kCFNotFound == startingTokenRange.location)
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    //...............................................................................................................................................
    // Is offset in the middle of a token? Return the boundary of the token.

    if ((offset > startingTokenRange.location) && (offset < (startingTokenRange.location + startingTokenRange.length - 1)))
        {
        if (walkIncrement > 0)
            return (startingTokenRange.location + startingTokenRange.length);

        return startingTokenRange.location;
        }

    //...............................................................................................................................................
    // Otherwise walk to the next token to get the answer.
    
    offset += walkIncrement;

    if (kCFStringTokenizerTokenNone == ::CFStringTokenizerGoToTokenAtIndex(tokenizer, offset))
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    CFRange stepTokenRange = ::CFStringTokenizerGetCurrentTokenRange(tokenizer);
    if (kCFNotFound == stepTokenRange.location)
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    if (walkIncrement > 0)
        return (startingTokenRange.location + startingTokenRange.length);

    return startingTokenRange.location;
    }

#elif defined (ANDROID)

//---------------------------------------------------------------------------------------
// This helper is only meant to support WordBoundaryReason::CaretPositioning and WordBoundaryReason::WordWrapping; behavior is undefined for other types.
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
static size_t findWordBoundary(WordBoundaryReason boundaryType, WCharCP unicodeString, size_t numCharacters, size_t offset, int walkIncrement)
    {
    //...............................................................................................................................................
    // Error checking...
    
    if ((NULL == unicodeString) || (0 == walkIncrement))
        { BeAssert(false); return 0; }

    //...............................................................................................................................................
    // Early (fast) return?
    
    if (offset > numCharacters)
        { BeAssert(false); return numCharacters; }

    bool isWalkingForward = (walkIncrement > 0);

    if ((0 == numCharacters) || (!isWalkingForward && (0 == offset)))
        return 0;

    if (isWalkingForward && (offset >= (numCharacters - 1)))
        return numCharacters;

    //...............................................................................................................................................
    // Use ICU to answer the question.
    
    UnicodeString icuString((CharCP)unicodeString, (int32_t)(numCharacters * sizeof (WChar)), "UTF-32LE");
    if (icuString.isEmpty())
        { BeAssert(false); return (size_t)(offset + walkIncrement); }

    UErrorCode icuStatus = U_ZERO_ERROR;
    unique_ptr<BreakIterator> breakIter;
    switch (boundaryType)
        {
        case WordBoundaryReason::CaretPositioning:
            breakIter.reset(BreakIterator::createWordInstance(Locale::getDefault(), icuStatus));
            break;

        default:
            BeAssert(false); // Unknown or unexpected WordBoundaryReason
            // Fall through

        case WordBoundaryReason::WordWrapping:
            breakIter.reset(BreakIterator::createLineInstance(Locale::getDefault(), icuStatus));
            break;
        }

    if (!breakIter || U_FAILURE(icuStatus))
        { BeAssert(false); return (size_t)(offset + walkIncrement); }
    
    breakIter->setText(icuString);

    if (walkIncrement > 0)
        {
        int32_t nextStop = breakIter->following((int32_t)offset);

        if (BreakIterator::DONE == nextStop)
            return numCharacters;
        
        return (size_t)nextStop;
        }
    
    int32_t previousStop = breakIter->preceding((int32_t)offset);

    if (BreakIterator::DONE == previousStop)
        return 0;
    
    return (size_t)previousStop;
    }

#else

#pragma message("Warning: findWordBoundary not implemented on this platform!")

//---------------------------------------------------------------------------------------
// This helper is only meant to support WordBoundaryReason::CaretPositioning and WordBoundaryReason::WordWrapping; behavior is undefined for other types.
// @bsimethod                                                   Jeff.Marker     07/2013
//---------------------------------------------------------------------------------------
static size_t findWordBoundary (WordBoundaryReason boundaryType, WCharCP unicodeString, size_t numCharacters, size_t offset, int walkIncrement)
    {
    BeAssert(false); // Unimplemented on this platform.
    return (size_t)(offset + walkIncrement);
    }

#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
size_t WordBoundaryServices::FindPreviousWordBoundary (WordBoundaryReason boundaryType, WCharCP unicodeString, size_t numCharacters, size_t offset)
    {
    if (offset > numCharacters)
        { BeAssert (false); return numCharacters; }
    
    if (0 == offset)
        return offset;

    if (1 == offset)
        return 0;

    switch (boundaryType)
        {
        case WordBoundaryReason::CaretPositioning:
            {
            // Caret boundaries are the finest (e.g. they stop the most places), so findWordBoundary is really geared towards that out of necessity.
            offset = findWordBoundary (boundaryType, unicodeString, numCharacters, offset, -1);
            
            break;
            }
        
        case WordBoundaryReason::WordWrapping:
            {
            // Line breaks need to be a little coarser than caret boundaries, but we can still use findWordBoundary as a foundation.
            
            while (iswpunct (unicodeString[offset]) || iswspace (unicodeString[offset]))
                --offset;
            
            offset = findWordBoundary (boundaryType, unicodeString, numCharacters, offset, -1);

            break;
            }
                
        case WordBoundaryReason::FindingWords:
            {
            // The Word boundary type is designed to work at a higher level for things like spell check, and is coarse enough that we have come up with our own logic (e.g. it is easier to start from scratch than to run findWordBoundary and find all the exceptions). As such, it may have poor support for Asian scripts (or others that don't delimit words based on punctuation and whitespace), but hopefully we can get away with that because users of such scripts will not expect (or execute) related features.
            // The basic idea is to delimit on white space, but also honor trailing punctuation. For example, stop before a trailing comma or period, but capture things like the inner slashes and periods of URIs.
            
            bool startsWithWhiteSpace = (0 != iswspace (unicodeString[offset]));
            
            --offset;

            while ((offset < numCharacters) && ((0 != iswspace (unicodeString[offset])) == startsWithWhiteSpace))
                --offset;
            
            ++offset;

            while ((offset > 0) && iswpunct (unicodeString[offset - 1]))
                --offset;

            break;
            }
        
        default:
            {
            BeAssert (false && L"Unknown WordBoundaryReason");
            break;
            }
        }

    return offset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2010
//---------------------------------------------------------------------------------------
size_t WordBoundaryServices::FindNextWordBoundary (WordBoundaryReason boundaryType, WCharCP unicodeString, size_t numCharacters, size_t offset)
    {
    if (offset > numCharacters)
        { BeAssert (false); return numCharacters; }
    
    if (offset >= numCharacters)
        return offset;

    if (offset >= (numCharacters - 1))
        return numCharacters;
    
    switch (boundaryType)
        {
        case WordBoundaryReason::CaretPositioning:
            {
            // Caret boundaries are the finest (e.g. they stop the most places), so findWordBoundary is really geared towards that out of necessity.
            offset = findWordBoundary (boundaryType, unicodeString, numCharacters, offset, 1);
            
            break;
            }
        
        case WordBoundaryReason::WordWrapping:
            {
            // Line breaks need to be a little coarser than caret boundaries, but we can still use findWordBoundary as a foundation.
            offset = findWordBoundary (boundaryType, unicodeString, numCharacters, offset, 1);

            // *** NEEDS WORK: findWordBoundary should not return special values.
            if (offset == ERROR)
                return numCharacters;

            // Include any trailing sequential punctuation followed by continuous white space on the line.
            while (iswpunct (unicodeString[offset]))
                ++offset;

            while (iswspace (unicodeString[offset]))
                ++offset;
            
            break;
            }
                
        case WordBoundaryReason::FindingWords:
            {
            // The Word boundary type is designed to work at a higher level for things like spell check, and is coarse enough that we have come up with our own logic (e.g. it is easier to start from scratch than to run findWordBoundary and find all the exceptions). As such, it may have poor support for Asian scripts (or others that don't delimit words based on punctuation and whitespace), but hopefully we can get away with that because users of such scripts will not expect (or execute) related features.
            // The basic idea is to delimit on white space, but also honor trailing punctuation. For example, stop before a trailing comma or period, but capture things like the inner slashes and periods of URIs.
            
            bool startsWithWhiteSpace = (0 != iswspace (unicodeString[offset]));
            
            ++offset;

            while ((offset < numCharacters) && ((0 != iswspace (unicodeString[offset])) == startsWithWhiteSpace))
                ++offset;
            
            while ((offset > 0) && iswpunct (unicodeString[offset - 1]))
                --offset;

            break;
            }
        
        default:
            {
            BeAssert (false && L"Unknown WordBoundaryReason");
            break;
            }
        }

    return offset;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     02/2011
//---------------------------------------------------------------------------------------
bool WordBoundaryServices::IsAtWordBoundary (WCharCP unicodeString, size_t numCharacters, size_t offset)
    {
    // This method is geared towards interacting with WordBoundaryReason::FindingWords; see related comments in FindNextWordBoundary.
    
    if (0 == offset)
        return true;
    
    if (offset > numCharacters)
        { BeAssert (false); return true; }

    if (offset >= numCharacters)
        return true;
    
    return (iswspace (unicodeString[offset - 1]) != iswspace (unicodeString[offset]));
    }

#if defined (BENTLEY_WIN32)
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

#endif
