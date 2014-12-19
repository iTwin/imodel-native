//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStat.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCStat
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"

class HFCStatImpl;

class HFCStat
    {
    friend class  HFCStatImpl;
    friend struct HFCStatImplListDestroyer;

public:
    enum AccessStatus
        {
        AccessGranted = 0,
        AccessDenied,
        TargetNotFound,
        AccessError,
        };

    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    _HDLLu                         HFCStat(const HFCURL&         pi_rURL);
    _HDLLu                         HFCStat(const HFCPtr<HFCURL>& pi_rpURL);
    _HDLLu virtual                 ~HFCStat();


    //--------------------------------------
    // Methods
    //--------------------------------------


    // Resource time
    time_t                 GetCreationTime     () const;
    time_t                 GetLastAccessTime   () const;
    time_t                 GetModificationTime () const;
    void                   SetModificationTime (time_t    pi_NewTime) const;

    // Resource size
    uint64_t               GetSize() const;

    // Resource existence
    bool                   IsExistent() const;

    AccessStatus            DetectAccess() const;

    // Resource access mode
    HFCAccessMode           GetAccessMode() const;



private:
    //--------------------------------------
    // Methods
    //--------------------------------------

    const HFCStatImpl*      FindImplementation(const HFCURL& pi_rURL) const;


    //--------------------------------------
    // Attributes
    //--------------------------------------

    // URL of the resource
    HFCPtr<HFCURL>          m_pURL;
    const HFCStatImpl*      m_pImpl;

    // static list of implementation objects
    typedef list<const HFCStatImpl*>
    ImplList;
    static ImplList*        s_pImplList;
    };

//--------------------------------------
// Implementation object definition
//--------------------------------------

class HNOVTABLEINIT HFCStatImpl
    {
public:
    //--------------------------------------
    // Construction / Destruction
    //--------------------------------------

    _HDLLu                     HFCStatImpl();
    _HDLLu virtual             ~HFCStatImpl();


    //--------------------------------------
    // Methods
    //--------------------------------------

    // Indicates if an impl can handle an URL
    virtual bool       CanHandle(const HFCURL& pi_rURL) const = 0;

    // Resource time
    virtual time_t      GetCreationTime    (const HFCURL& pi_rURL) const = 0;
    virtual time_t      GetLastAccessTime  (const HFCURL& pi_rURL) const = 0;
    virtual time_t      GetModificationTime(const HFCURL& pi_rURL) const = 0;
    virtual void        SetModificationTime(const HFCURL& pi_rURL,
                                            time_t        pi_NewTime) const = 0;

    // Resource size
    virtual uint64_t   GetSize(const HFCURL& pi_rURL) const = 0;

    // Resource existence
    virtual bool       IsExistent(const HFCURL& pi_rURL) const;

    virtual HFCStat::AccessStatus
    DetectAccess(const HFCURL& pi_rURL) const = 0;

    // Resource access mode
    virtual HFCAccessMode
    GetAccessMode(const HFCURL& pi_rURL) const = 0;


protected:
    //--------------------------------------
    // Methods
    //--------------------------------------

    _HDLLu void                    RegisterImpl(const HFCStatImpl* pi_pImpl) const;
    };


#include "HFCStat.hpp"

