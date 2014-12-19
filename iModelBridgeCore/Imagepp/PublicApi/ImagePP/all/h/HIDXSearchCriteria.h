//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HIDXSearchCriteria.h $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HIDXSearchCriteria
//-----------------------------------------------------------------------------
// Group of criterias
//
// NEVER add a virtual method : NOVTABLE used
//-----------------------------------------------------------------------------
#pragma once


class HIDXCriteria;
class HIDXIndex;


class HNOVTABLEINIT HIDXSearchCriteria
    {
public:
    _HDLLu                 HIDXSearchCriteria();
    _HDLLu                 HIDXSearchCriteria(void const* pi_Index, HIDXCriteria* pi_pNewCriteria);

    // Not virtual since this class should not have children
    _HDLLu                 ~HIDXSearchCriteria();

    // Give new criteria (possessed by SearchCriteria)
    _HDLLu void            AddCriteria(void const* pi_Index, HIDXCriteria* pi_pNewCriteria);

    // Retrieve a criteria
    _HDLLu HIDXCriteria*   GetCriteria(void const* pi_Index) const;


private:

    // Copy ctor and assignment are disabled
    HIDXSearchCriteria(const HIDXSearchCriteria& pi_rObj);
    HIDXSearchCriteria& operator=(const HIDXSearchCriteria& pi_rObj);

    typedef map < void const*, HIDXCriteria*, less<void const*>, allocator<HIDXCriteria*> >
    CriteriaMap;

    // The list of criterias
    CriteriaMap    m_Criterias;

    };

