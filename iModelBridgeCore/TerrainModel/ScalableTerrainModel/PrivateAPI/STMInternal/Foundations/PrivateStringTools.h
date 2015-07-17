/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Foundations/PrivateStringTools.h $
|    $RCSfile: PrivateStringTools.h,v $
|   $Revision: 1.3 $
|       $Date: 2011/10/20 18:47:26 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableTerrainModel/Foundations//Definitions.h>

BEGIN_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE

// TDORAY: Move all these to FoundationsPrivateTools.h

template <typename CharT>
struct CaseInsensitiveCharTools
    {
    struct CharLessThan
        {
        const std::ctype<CharT>& ct;
        explicit CharLessThan(const std::ctype<CharT>& c) : ct(c) {}

        bool operator()(CharT lhs, CharT rhs) const
            { return ct.toupper(lhs) < ct.toupper(rhs); }
        };

    struct CharEqualTo
        {
        const std::ctype<CharT>& ct;
        explicit CharEqualTo(const std::ctype<CharT>& c) : ct(c) {}

        bool operator()(CharT lhs, CharT rhs) const
            { return ct.toupper(lhs) == ct.toupper(rhs); }
        };

    // Character equality test predicate
    std::locale m_locale;
    const std::ctype<CharT>& m_ctype;


    explicit CaseInsensitiveCharTools (const std::locale& pi_locale = std::locale::classic())
        :   m_locale(pi_locale),
            m_ctype(std::use_facet<std::ctype<CharT>>(pi_locale)) 
        {}

    CharLessThan GetLessThan () const { return CharLessThan(m_ctype); }
    CharEqualTo GetEqualTo () const { return CharEqualTo(m_ctype); }
    };

template <typename CharT>
static const CaseInsensitiveCharTools<CharT>& GetDefaultCaseInsensitiveCharTools ()
    {
    static const CaseInsensitiveCharTools<CharT> CHAR_TOOLS;
    return CHAR_TOOLS;
    }

struct ReverseStringLessThan
    {
    bool                        operator ()                    (const WString&       lhs, 
                                                                const WString&       rhs) const
        {
        return lexicographical_compare(lhs.rbegin(), lhs.rend(), rhs.rbegin(), rhs.rend(), 
                                       GetDefaultCaseInsensitiveCharTools<WChar>().GetLessThan());
        }
    };

WString                      CreateStrFromWCStr             (const WChar*        cstr);
WString                      CreateWStrFrom                 (const WChar*         cstr);
WString                      CreateStrFrom                  (const WChar*        cstr);


END_BENTLEY_MRDTM_FOUNDATIONS_NAMESPACE
