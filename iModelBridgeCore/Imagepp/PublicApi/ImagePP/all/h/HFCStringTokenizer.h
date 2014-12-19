//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStringTokenizer.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCStringTokenizer
//-----------------------------------------------------------------------------

#pragma once

class HFCStringTokenizer
    {
public:
    //--------------------------------------
    // Construction/destruction
    //--------------------------------------

    HFCStringTokenizer(const WString&   pi_rString,
                       const WString&   pi_rSeparator,
                       bool            pi_SeparatorIsAtomic = false);
    ~HFCStringTokenizer();

    //--------------------------------------
    // Methods
    //--------------------------------------

    // Performs one iterator of tokenization.  Result is
    // true while there is a token to extract.
    bool               Tokenize(WString& po_rToken);

    // Trims white spaces of token
    static void         Trim(WString& po_rToken);


private:
    //--------------------------------------
    // Attributes
    //--------------------------------------

    // For performance issues, we keep references to
    // the string to parse and the separator.
    const WString&      m_rString;
    const WString       m_Separator;
    WString::size_type  m_StartPos;
    WString::size_type  m_EndPos;

    // If this is true, the separator string is a single separator.
    // Otherwise, all characters of the string are separators.
    bool               m_SeparatorIsAtomic;
    WString::size_type  m_SeparatorSize;
    };

#include "HFCStringTokenizer.hpp"

