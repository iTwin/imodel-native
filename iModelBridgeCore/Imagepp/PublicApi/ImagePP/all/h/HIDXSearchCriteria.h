//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXSearchCriteria.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXSearchCriteria
//-----------------------------------------------------------------------------
// Group of criterias
//
// NEVER add a virtual method : NOVTABLE used
//-----------------------------------------------------------------------------
#pragma once

BEGIN_IMAGEPP_NAMESPACE

class HIDXCriteria;
class HIDXIndex;


class HNOVTABLEINIT HIDXSearchCriteria
    {
public:
    IMAGEPP_EXPORT                 HIDXSearchCriteria();
    IMAGEPP_EXPORT                 HIDXSearchCriteria(void const* pi_Index, HIDXCriteria* pi_pNewCriteria);

    // Not virtual since this class should not have children
    IMAGEPP_EXPORT                 ~HIDXSearchCriteria();

    // Give new criteria (possessed by SearchCriteria)
    IMAGEPP_EXPORT void            AddCriteria(void const* pi_Index, HIDXCriteria* pi_pNewCriteria);

    // Retrieve a criteria
    IMAGEPP_EXPORT HIDXCriteria*   GetCriteria(void const* pi_Index) const;


private:

    // Copy ctor and assignment are disabled
    HIDXSearchCriteria(const HIDXSearchCriteria& pi_rObj);
    HIDXSearchCriteria& operator=(const HIDXSearchCriteria& pi_rObj);

    typedef map < void const*, HIDXCriteria*, less<void const*>, allocator<HIDXCriteria*> >
    CriteriaMap;

    // The list of criterias
    CriteriaMap    m_Criterias;

    };

END_IMAGEPP_NAMESPACE