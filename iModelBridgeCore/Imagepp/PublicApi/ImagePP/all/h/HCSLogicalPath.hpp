//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HCSLogicalPath.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "HFCURLFile.h"

//------------------------------------------------------------------------------
// Public
// Default constructor
//------------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPath<Properties>::HCSLogicalPath()
    {
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPath<Properties>::HCSLogicalPath(const WString&    pi_rLogical,
                                                  const WString&    pi_rPhysical,
                                                  const Properties& pi_rProps)
    : m_Logical(pi_rLogical),
      m_Physical(pi_rPhysical),
      m_Properties(pi_rProps)
    {
    HPRECONDITION(!pi_rLogical.empty());
    HPRECONDITION(!pi_rPhysical.empty());
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPath<Properties>::HCSLogicalPath(const HCSLogicalPath& pi_rObj)
    : m_Logical(pi_rObj.m_Logical),
      m_Physical(pi_rObj.m_Physical),
      m_Properties(pi_rObj.m_Properties)
    {
    HPRECONDITION(!pi_rObj.m_Logical.empty());
    HPRECONDITION(!pi_rObj.m_Physical.empty());
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPath<Properties>::~HCSLogicalPath()
    {
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline HCSLogicalPath<Properties>& HCSLogicalPath<Properties>::operator=(const HCSLogicalPath& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        m_Logical    = pi_rObj.m_Logical;
        m_Physical   = pi_rObj.m_Physical;
        m_Properties = pi_rObj.m_Properties;
        }

    return (*this);
    }



//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline bool HCSLogicalPath<Properties>::operator< (const HCSLogicalPath& pi_rObj) const
    {
    return (m_Logical.compare(pi_rObj.m_Logical) < 0);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline bool HCSLogicalPath<Properties>::operator> (const HCSLogicalPath& pi_rObj) const
    {
    return (m_Logical.compare(pi_rObj.m_Logical) > 0);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline bool HCSLogicalPath<Properties>::operator==(const HCSLogicalPath& pi_rObj) const
    {
    return (m_Logical.compare(pi_rObj.m_Logical) == 0);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline bool HCSLogicalPath<Properties>::operator!=(const HCSLogicalPath& pi_rObj) const
    {
    return (m_Logical.compare(pi_rObj.m_Logical) != 0);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline const WString& HCSLogicalPath<Properties>::GetName() const
    {
    return (m_Logical);
    }

//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline const WString& HCSLogicalPath<Properties>::GetPhysicalPath() const
    {
    return (m_Physical);
    }


//------------------------------------------------------------------------------
// Public
//
//------------------------------------------------------------------------------
template<class Properties>
inline const Properties& HCSLogicalPath<Properties>::GetProperties() const
    {
    return (m_Properties);
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPath<Properties>::FormatLogicalPath(WString* po_pLogical)
    {
    HPRECONDITION(po_pLogical);

    // if the given path is empty, set as root "/"
    // if not, format.
    if (!po_pLogical->empty())
        {
        // Change all '\\' for '/'
        WString::size_type Pos;
        while ((Pos = po_pLogical->find(L'\\', 0)) != WString::npos)
            (*po_pLogical)[Pos] = L'/';

        // Verify that the logical path doesn't start with '/' except
        // on root path
        while ((po_pLogical->compare(L"/") != 0) &&
               ((*po_pLogical)[0] == L'/') )
            po_pLogical->erase(0, 1);

        // Verify that the logical path doesn't finish with '/' except
        // on root path
        while ((po_pLogical->compare(L"/") != 0) && ((*po_pLogical)[po_pLogical->size() - 1] == L'/'))
            po_pLogical->erase(po_pLogical->size() - 1 , 1);

        // lower-case the logical path name,
//        ctype<char> Converter;
//        Converter.tolower(po_pLogical->begin(), po_pLogical->end());
        }
    else
        *po_pLogical = L"/";
    }


//-----------------------------------------------------------------------------
// Public
//
//-----------------------------------------------------------------------------
template<class Properties>
inline void HCSLogicalPath<Properties>::FormatPhysicalPath(WString* po_pPhysical)
    {
    HPRECONDITION(po_pPhysical != 0);

    // find the scheme separator ("://").  If none can be found, then add
    // the file:// scheme to the string
    WString::size_type StartPos = 0;
    StartPos = po_pPhysical->find(L"://");
    if (StartPos == string::npos)
        {
        po_pPhysical->insert(0, HFCURLFile::s_SchemeName() + L"://");
        StartPos = HFCURLFile::s_SchemeName().size();
        }

    // aside from the current position in case of a UNC path, remove any dual "//"
    StartPos += 4;  // skip the "://" and the first char that may be / in //
    WString::size_type Pos;
    while ( ((Pos = po_pPhysical->find(L"//",   StartPos)) != WString::npos) ||
            ((Pos = po_pPhysical->find(L"/\\",  StartPos)) != WString::npos) ||
            ((Pos = po_pPhysical->find(L"\\/",  StartPos)) != WString::npos) ||
            ((Pos = po_pPhysical->find(L"\\\\", StartPos)) != WString::npos) )
        po_pPhysical->erase(Pos, 1);
    }


