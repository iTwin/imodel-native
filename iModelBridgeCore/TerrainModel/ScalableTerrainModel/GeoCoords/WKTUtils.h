/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/GeoCoords/WKTUtils.h $
|    $RCSfile: WKTUtils.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/12/20 16:24:03 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/GeoCoords/Definitions.h>
#include <STMInternal/GeoCoords/WKTUtils.h>
#include <deque>

BEGIN_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE

struct WKTRoot;
struct WKTSection;

#define isspace(c)      (((c) =='\t') \
                     || ((c) == '\r') \
                     || ((c) == '\n') \
                     || ((c) == ' '))

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct WKTParameter
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        };

private:
    const WKTSection*                   m_sectionP;
    const WChar*                         m_begin;
    const WChar*                         m_end;

public:
    explicit                            WKTParameter                           (const WChar*             parameterBegin);


    size_t                              strLen                                 () const { return m_end - m_begin; }

    const WChar*                         strBegin                               () const { return m_begin; }
    const WChar*                         strEnd                                 () const { return m_end; }

    bool                                IsQuoted                               () const 
        {
        return ((m_end - m_begin) < 2) ? false : ('\"' == *m_begin && '\"' == *(m_end - 1)); 
        }

    const WChar*                         strBeginInsideQuote                    () const 
        { 
        assert((2 <= std::distance(m_begin, m_end)) && '\"' == *m_begin);
        return (m_begin == m_end) ? m_begin : m_begin + 1; 
        }
    const WChar*                         strEndInsideQuote                      () const 
        {
        assert((2 <= std::distance(m_begin, m_end)) && '\"' == *(m_end - 1));
        return (m_begin == m_end) ? m_end : m_end - 1; 
        }


    Status                              Parse                                  (WKTRoot&                root,
                                                                                const WChar*             parameterEnd);

    bool                                IsSection                              () const { return 0 != m_sectionP; }
    const WKTSection&                   GetSection                             () const { assert(IsSection()); return *m_sectionP; } 
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct WKTSection
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        };

private:
    typedef bvector<WKTParameter>        ParameterList;

    //friend struct                       WKTParameter;

    const WChar*                         m_keywordBegin;
    const WChar*                         m_keywordEnd;
    const WChar*                         m_begin;
    const WChar*                         m_end;
    ParameterList                       m_parameters;

public:
    typedef const WKTParameter*         const_iterator;

    explicit                            WKTSection                             (const WChar*             keywordBegin,
                                                                                const WChar*             keywordEnd,
                                                                                const WChar*             sectionBegin);

    const WChar*                         keywordBegin                           () const { return m_keywordBegin; }
    const WChar*                         keywordEnd                             () const { return m_keywordEnd; }

    size_t                              strLen                                 () const { return m_end - m_begin; }

    const WChar*                         strBegin                               () const { return m_begin; }
    const WChar*                         strEnd                                 () const { return m_end; }

    const_iterator                      begin                                  () const { return &*m_parameters.begin(); }
    const_iterator                      end                                    () const { return &*m_parameters.end(); }

    size_t                              GetSize                                () const { return m_parameters.size(); }

    const WKTParameter&                 operator[]                             (size_t                  index) const
        {
        assert(index < GetSize());
        return m_parameters[index];
        }


    Status                              Parse                                  (WKTRoot&                root,
                                                                                const WChar*             sectionEnd);
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct WKTRoot
    {
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        };

private:
    // Choice of deque as a container is not random here. Indeed, deque's push
    // back guarantee that previously held reference to items of the deque are 
    // not invalidated (which is not he case for vector). This is an essential 
    // property for this class to work properly so consider this thoroughly  
    // prior to making any changes of container.
    typedef std::deque<WKTSection>      SectionList;

    friend struct                       WKTSection;
    friend struct                       WKTParameter;

    SectionList                         m_sections;
    const WKTSection*                   m_rootSectionP;

    const WChar*                         m_begin;
    const WChar*                         m_end;
public:
    explicit                            WKTRoot                                (const WChar*             wktBegin,
                                                                                const WChar*             wktEnd);

    size_t                              strLen                                 () const { return m_end - m_begin; }

    const WChar*                         strBegin                               () const { return m_begin; }
    const WChar*                         strEnd                                 () const { return m_end; }

    bool                                IsEmpty                                () const { return 0 == m_rootSectionP; }
    const WKTSection&                   GetSection                             () const { assert(!IsEmpty()); return *m_rootSectionP; }

    Status                              Parse                                  ();
    };

END_BENTLEY_MRDTM_GEOCOORDINATES_NAMESPACE
