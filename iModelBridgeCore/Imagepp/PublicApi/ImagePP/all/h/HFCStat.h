//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCStat.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCStat
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HFCURL.h"

BEGIN_IMAGEPP_NAMESPACE
class HFCStatImpl;

class HFCStat
    {
    friend class  HFCStatImpl;
    friend struct HFCStatImplListDestroyer;

public:
//DM-Android    Not able to build if private member
    // static list of implementation objects
    typedef list<const HFCStatImpl*>    ImplList;       // From private
    static ImplList*        s_pImplList;                // From private


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

    IMAGEPP_EXPORT                         HFCStat(const HFCURL&         pi_rURL);
    IMAGEPP_EXPORT                         HFCStat(const HFCPtr<HFCURL>& pi_rpURL);
    IMAGEPP_EXPORT virtual                 ~HFCStat();


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

    IMAGEPP_EXPORT                     HFCStatImpl();
    IMAGEPP_EXPORT virtual             ~HFCStatImpl();


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

    IMAGEPP_EXPORT void                    RegisterImpl(const HFCStatImpl* pi_pImpl) const;
    IMAGEPP_EXPORT void                    UnregisterImpl(const HFCStatImpl* pi_pImpl) const;
    };

//------------------------------------------------------------------------------
// HFCStatFile 
//------------------------------------------------------------------------------
class HFCStatFile
    {
    public:
        IMAGEPP_EXPORT static void Register();
    };
//------------------------------------------------------------------------------
// HFCStatMemFile 
//------------------------------------------------------------------------------
class HFCStatMemFile
    {
    public:
        IMAGEPP_EXPORT static void Register();
    };
//------------------------------------------------------------------------------
// HFCStatEmbedFile 
//------------------------------------------------------------------------------
class HFCStatEmbedFile
    {
    public:
        IMAGEPP_EXPORT static void Register();
    };


//------------------------------------------------------------------------------
// HFCStatHttpFile 
//------------------------------------------------------------------------------
class HFCStatHttpFile
    {
    public:
        IMAGEPP_EXPORT static void Register();
    };

END_IMAGEPP_NAMESPACE

#include "HFCStat.hpp"

