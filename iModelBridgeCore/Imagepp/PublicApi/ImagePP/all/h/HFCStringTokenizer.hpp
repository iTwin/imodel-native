//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStringTokenizer.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCStringTokenizer
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCStringTokenizer::HFCStringTokenizer(const WString&    pi_rString,
                                              const WString&    pi_rSeparator,
                                              bool             pi_SeparatorIsAtomic)
    : m_rString(pi_rString),
      m_Separator(pi_rSeparator)
    {
    HPRECONDITION(!pi_rSeparator.empty());

    m_StartPos = 0;
    m_EndPos   = string::npos;

    m_SeparatorIsAtomic = pi_SeparatorIsAtomic;
    m_SeparatorSize = m_SeparatorIsAtomic ? pi_rSeparator.size() : 1;
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline HFCStringTokenizer::~HFCStringTokenizer()
    {
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline bool HFCStringTokenizer::Tokenize(WString& po_rToken)
    {
    bool Result = false;

    if (Result = (m_StartPos != WString::npos))
        {
        // find the next separator from the current start pos
        if (m_SeparatorIsAtomic)
            m_EndPos = m_rString.find(m_Separator, m_StartPos);
        else
            m_EndPos = m_rString.find_first_of(m_Separator, m_StartPos);

        // build the request if any
        if (m_EndPos != m_StartPos)
            {
            if (m_EndPos != WString::npos)
                po_rToken = WString(m_rString, m_StartPos, m_EndPos - m_StartPos);
            else
                po_rToken = m_rString.substr(m_StartPos);
            }
        else
            {
            // The current starting item in the separator.  Set an emtpy token
            // and proceed to the next item.
            po_rToken = L"";
            }

        // setup for the next iteration
        if (m_EndPos == WString::npos || m_EndPos >= m_rString.size() - m_SeparatorSize)
            {
            m_StartPos = WString::npos;
            }
        else
            {
            // Set the next starting position
            m_StartPos = m_EndPos + m_SeparatorSize;
            }
        }

    return (Result);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
inline void HFCStringTokenizer::Trim(WString& po_rToken)
    {
    // trim leading spaces
    while ((po_rToken.size() > 0) && (*po_rToken.begin() == L' '))
        po_rToken.erase(po_rToken.begin());

    // trim trailing spaces
    while ((po_rToken.size() > 0) && (*po_rToken.rbegin() == L' '))
        po_rToken.erase(po_rToken.size()-1);
    }