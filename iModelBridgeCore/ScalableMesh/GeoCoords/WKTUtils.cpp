/*--------------------------------------------------------------------------------------+
|
|     $Source: GeoCoords/WKTUtils.cpp $
|    $RCSfile: WKTUtils.cpp,v $
|   $Revision: 1.3 $
|       $Date: 2011/12/20 16:24:01 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include <STMInternal/Foundations/PrivateStringTools.h>
#include "WktUtils.h"

BEGIN_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* FindWKTSectionKeyword (const WChar* wktBegin, const WChar* wktEnd)
    {
    assert(wktBegin <= wktEnd);

    struct IsWhiteSpace : std::unary_function<WChar, bool>
        {
        bool operator () (WChar c) const { return isspace(c); }
        };

    return std::find_if(wktBegin, wktEnd, not1(IsWhiteSpace()));
    }


namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* FindWKTSectionKeywordEnd (const WChar* wktKeywordBegin, const WChar* wktEnd)
    {
    assert(wktKeywordBegin <= wktEnd);

    struct IsKeywordEnd : std::unary_function<WChar, bool>
        {
        bool operator () (WChar c) const { return '[' == c || isspace(c); }
        };

    return std::find_if(wktKeywordBegin, wktEnd, IsKeywordEnd());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* FindWKTSectionFirstParameterBegin (const WChar* wktKeywordEnd, const WChar* wktEnd)
    {
    struct IsFirstParameterBegin : std::unary_function<WChar, bool>
        {
        bool operator () (WChar c) const { return !('[' == c || isspace(c)); }
        };

    return std::find_if(wktKeywordEnd, wktEnd, IsFirstParameterBegin());
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* FindWKTSectionNextParameterBegin (const WChar* wktParameterEnd, const WChar* wktEnd)
    {
    struct IsParamBegin : std::unary_function<WChar, bool>
        {
        bool operator () (WChar c) const { return !(',' == c || isspace(c)); }
        };

    return std::find_if(wktParameterEnd, wktEnd, IsParamBegin());
    }

}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKTRoot::WKTRoot   (const WChar* wktBegin,
                    const WChar* wktEnd)
    :   m_rootSectionP(0)
    {
    typedef std::reverse_iterator<const WChar*> RevIt;

    struct IsWhiteSpace : std::unary_function<WChar, bool>
        {
        bool operator () (WChar c) const { return isspace(c); }
        };
    m_begin = std::find_if(wktBegin, wktEnd, not1(IsWhiteSpace())) ;
    m_end = std::find_if(RevIt(wktEnd), RevIt(wktBegin), not1(IsWhiteSpace())).base();

    assert(m_begin <= m_end);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus WKTRoot::Parse()
    {
    const WChar* keywordBegin = FindWKTSectionKeyword(m_begin, m_end);
    if (m_end == keywordBegin)
        return S_SUCCESS; // Empty wkt

    const WChar* keywordEnd = FindWKTSectionKeywordEnd(keywordBegin, m_end);
    if (m_end == keywordEnd)
        return S_ERROR; // Could not find keyword end

    const WChar* sectionBegin = FindWKTSectionFirstParameterBegin(keywordEnd, m_end);
    if (m_end == sectionBegin)
        return S_ERROR;

    WKTSection section(keywordBegin, keywordEnd, sectionBegin);

    if (SMStatus::S_SUCCESS != section.Parse(*this, m_end))
        return S_ERROR;

    if (m_end != (section.strEnd() + 1))
        {
        assert(!"Invalid WKT expression");
        return S_ERROR;
        }

    m_sections.push_back(section);
    m_rootSectionP = &m_sections.back();

    return S_SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKTParameter::WKTParameter (const WChar* parameterBegin)
    :   m_sectionP(0),
        m_begin(parameterBegin),
        m_end(parameterBegin)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus WKTParameter::Parse(WKTRoot&        root,
                                            const WChar*     wktSectionEnd)
    {
    struct IsParameterEndOrSection : std::unary_function<WChar, bool>
        {
        bool operator () (WChar c) const { return ',' == c || ']' == c || '[' == c; }
        };
    const WChar* parameterTypeCheckIt = std::find_if(m_begin, wktSectionEnd, IsParameterEndOrSection());
    if (wktSectionEnd == parameterTypeCheckIt)
        return S_ERROR;

    typedef std::reverse_iterator<const WChar*> RevIt;

    struct IsWhiteSpace : std::unary_function<WChar, bool>
        {
        bool operator () (WChar c) const { return isspace(c); }
        };
    const WChar* parameterEnd = std::find_if(RevIt(parameterTypeCheckIt), RevIt(m_begin), not1(IsWhiteSpace())).base() ;

    if ('[' != *parameterTypeCheckIt)
        {
        // This is a value parameter. Return early.
        m_end = parameterEnd;
        return S_SUCCESS;
        }

    // This is a section parameter. Parse the section.
    const WChar* sectionBegin = FindWKTSectionFirstParameterBegin(parameterTypeCheckIt, wktSectionEnd);
    if (m_end == sectionBegin)
        return S_ERROR;

    WKTSection section(m_begin, parameterEnd, sectionBegin);

    if (SMStatus::S_SUCCESS != section.Parse(root, wktSectionEnd))
        return S_ERROR;
    
    root.m_sections.push_back(section);

    m_sectionP = &root.m_sections.back();
    assert(m_sectionP->strEnd() < wktSectionEnd);
    assert(']' == *m_sectionP->strEnd());

    m_end = m_sectionP->strEnd() + 1;

    return S_SUCCESS;

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WKTSection::WKTSection (const WChar*     keywordBegin,
                        const WChar*     keywordEnd,
                        const WChar*     sectionBegin)
    :   m_keywordBegin(keywordBegin),
        m_keywordEnd(keywordEnd),
        m_begin(sectionBegin),
        m_end(sectionBegin)

    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SMStatus WKTSection::Parse(WKTRoot&        root,
                                        const WChar*     sectionEnd)
    {
    assert(m_begin <= sectionEnd);
    assert(m_begin == FindWKTSectionFirstParameterBegin(m_begin, sectionEnd));

    const WChar* parameterBegin = m_begin;
    do 
        {
        WKTParameter parameter(parameterBegin);

        if (SMStatus::S_SUCCESS != parameter.Parse(root, sectionEnd))
            return S_ERROR;

        m_parameters.push_back(parameter);

        parameterBegin = FindWKTSectionNextParameterBegin(parameter.strEnd(), sectionEnd);
        }
    while (parameterBegin < sectionEnd && 
           ']' != *parameterBegin);
        
    if (']' != *parameterBegin)
        return S_ERROR;
    
    m_end = parameterBegin;
    return S_SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <size_t NAME_SIZE>
WKTKeyword::WKTKeyword (const WChar (&n)[NAME_SIZE], Type t)
    :   str(n),
        strEnd(n + (NAME_SIZE - 1)),
        strLen(NAME_SIZE - 1),
        type(t)
    {
    }






namespace {

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct WKTKeywordLess : std::binary_function<WKTKeyword, WKTKeyword, bool>
    {
    bool operator() (const WKTKeyword& lhs, const WKTKeyword& rhs) const        { return 0 > _wcsicmp(lhs.str, rhs.str); }
    bool operator() (const WChar* lhs, const WKTKeyword& rhs) const              { return 0 > _wcsnicmp(lhs, rhs.str, rhs.strLen); }
    bool operator() (const WKTKeyword& lhs, const WChar* rhs) const              { return 0 > _wcsnicmp(lhs.str, rhs, lhs.strLen); }
    } WKT_KEYWORD_LESS;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const struct WKTKeywordEqual : std::binary_function<WKTKeyword, WKTKeyword, bool>
    {
    bool operator() (const WKTKeyword& lhs, const WKTKeyword& rhs) const        { return 0 == _wcsicmp(lhs.str, rhs.str); }
    bool operator() (const WChar* lhs, const WKTKeyword& rhs) const              { return 0 == _wcsnicmp(lhs, rhs.str, rhs.strLen); }
    bool operator() (const WKTKeyword& lhs, const WChar* rhs) const              { return 0 == _wcsnicmp(lhs.str, rhs, lhs.strLen); }
    } WKT_KEYWORD_EQUAL;


const WKTKeyword WKT_NULL_KEY(L"", WKTKeyword::TYPE_NULL);
const WKTKeyword WKT_AUTHORITY_KEY(L"AUTHORITY", WKTKeyword::TYPE_AUTHORITY);
const WKTKeyword WKT_COMPD_CS_KEY(L"COMPD_CS", WKTKeyword::TYPE_COMPD_CS);
const WKTKeyword WKT_FITTED_CS_KEY(L"FITTED_CS", WKTKeyword::TYPE_FITTED_CS);
const WKTKeyword WKT_INVERSE_MT_KEY(L"INVERSE_MT", WKTKeyword::TYPE_INVERSE_MT);
const WKTKeyword WKT_LOCAL_CS_KEY(L"LOCAL_CS", WKTKeyword::TYPE_LOCAL_CS);
const WKTKeyword WKT_LOCAL_DATUM_KEY(L"LOCAL_DATUM", WKTKeyword::TYPE_LOCAL_DATUM);
const WKTKeyword WKT_PARAM_MT_KEY(L"PARAM_MT", WKTKeyword::TYPE_PARAM_MT);
const WKTKeyword WKT_PARAMETER_KEY(L"PARAMETER", WKTKeyword::TYPE_PARAMETER);
const WKTKeyword WKT_UNIT_KEY(L"UNIT", WKTKeyword::TYPE_UNIT);
const WKTKeyword WKT_UNKNOWN_KEY(L"", WKTKeyword::TYPE_UNKNOWN);


const WKTKeyword WKT_KEYWORDS[] =
    {
    WKT_NULL_KEY,
    WKT_AUTHORITY_KEY,
    WKT_COMPD_CS_KEY,
    WKT_FITTED_CS_KEY,
    WKT_INVERSE_MT_KEY,
    WKT_LOCAL_CS_KEY,
    WKT_LOCAL_DATUM_KEY,
    WKT_PARAM_MT_KEY,
    WKT_PARAMETER_KEY,
    WKT_UNIT_KEY,
    WKT_UNKNOWN_KEY,
    };

static_assert((WKTKeyword::TYPE_UNKNOWN + 1) == CARRAY_SIZE(WKT_KEYWORDS), "");

/*
 * Entries in this set are assumed to be manually ordered to the same result as
 * if it was sorted using std::sort(WKT_KEYWORDS_SET, WKT_KEYWORDS_SET_END, WKTKeywordLess()).
 */
const WKTKeyword* const WKT_KEYWORDS_SET = &WKT_KEYWORDS[WKTKeyword::TYPE_NULL] + 1;
const WKTKeyword* const WKT_KEYWORDS_SET_END = &WKT_KEYWORDS[WKTKeyword::TYPE_UNKNOWN];

}



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WKTKeyword& GetWKTKeyword (const WChar* keywordBegin)
    {
#ifndef NDEBUG
    // TDORAY: Use std::is_sorted when available.
    static const bool SET_SORTED 
        = (WKT_KEYWORDS_SET_END == std::adjacent_find(WKT_KEYWORDS_SET, WKT_KEYWORDS_SET_END, not2(WKT_KEYWORD_LESS)));
    assert(SET_SORTED);

    static const bool SET_CONTAINS_NULL_KEYS 
        = (WKT_KEYWORDS_SET_END != std::find_if(WKT_KEYWORDS_SET, WKT_KEYWORDS_SET_END, std::bind1st(WKT_KEYWORD_EQUAL, WKT_NULL_KEY)));
    assert(!SET_CONTAINS_NULL_KEYS);
#endif //!NDEBUG

    const WKTKeyword* foundIt 
        = lower_bound(WKT_KEYWORDS_SET, WKT_KEYWORDS_SET_END, keywordBegin, WKT_KEYWORD_LESS);

    if (foundIt == WKT_KEYWORDS_SET_END || !WKT_KEYWORD_EQUAL(*foundIt, keywordBegin))
        return WKT_UNKNOWN_KEY;

    return *foundIt;
    }


const WKTKeyword& GetWKTKeyword (WKTKeyword::Type type)
    {
    assert(type <= WKTKeyword::TYPE_UNKNOWN);
    return WKT_KEYWORDS[type];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ISMStore::WktFlavor GetWKTFlavor(WString* wktStrWithoutFlavor, const WString& wktStr)
    {
    assert(wktStr.size() > 0);
    ISMStore::WktFlavor fileWktFlavor = ISMStore::WktFlavor_Oracle9;

    size_t charInd;    

    for (charInd = wktStr.size() - 1; charInd >= 0; charInd--)
        {
        if (wktStr[charInd] == L']')
            {
            break;
            }
        else
        if (((short)wktStr[charInd] >= 1) || ((short)wktStr[charInd] < ISMStore::WktFlavor_End))
            {
            fileWktFlavor = (ISMStore::WktFlavor)wktStr[charInd];            
            }
        }

    if (wktStrWithoutFlavor != 0)
        {
        *wktStrWithoutFlavor = wktStr.substr(0, charInd + 1);
        }
    
    return fileWktFlavor;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Mathieu.St-Pierre   12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
WKTKeyword::Type GetWktType(WString wkt)
    {                       
    WString wktMb(wkt.GetWCharCP());
        
    const WChar* wktBegin = wktMb.c_str();
    const WChar* wktEnd = wktBegin + wktMb.size();
        
    const WChar* firstKeywordBegin = FindWKTSectionKeyword(wktBegin, wktEnd);
    if (wktEnd == firstKeywordBegin) 
        {
        return WKTKeyword::TYPE_NULL;        
        }

    const WKTKeyword& firstKeyword = GetWKTKeyword(firstKeywordBegin);    

    return firstKeyword.type;
    }

/*----------------------------------------------------------------------------+
|
+----------------------------------------------------------------------------*/ 
bool MapWktFlavorEnum(BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor& baseGcsWktFlavor, ISMStore::WktFlavor fileWktFlavor)
    {
    //Temporary use numeric value until the basegcs enum match the csmap's one.
    switch(fileWktFlavor)
        {
        case ISMStore::WktFlavor_Oracle9 :
            baseGcsWktFlavor = (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor)7;
            break;

        case ISMStore::WktFlavor_Autodesk :
            baseGcsWktFlavor = (BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCS::WktFlavor)8;
            break;

        default : 
            return false;
        }

    return true;        
    }

END_BENTLEY_SCALABLEMESH_GEOCOORDINATES_NAMESPACE
