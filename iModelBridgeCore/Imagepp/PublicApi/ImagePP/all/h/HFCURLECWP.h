//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLECWP.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLECWP
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURLCommonInternet.h"

//:Ignore
// URL specification at this level is:
// ecwp:[//][user[:password]@]host[:port][/[Path[?SearchPart]]]
//:End Ignore

/**

    This class defines the URL descriptor for the "ecwp" scheme type.  That
    kind of URL is used to access streaming ECW files from an IWS. ECWP is layered
    over the standard HTTP.

    @code
    ecwp://[@i{user}[:@i{password}]@]@i{host}[:@i{port}][/[@i{path}[?@i{search}]]]
    @end

    where the text in italics has to be replaced by corresponding value, and
    text between brackets is facultative.

    This class adds, to inherited interface, new methods to access the
    parsed values extracted from the URL text (the path and search
    expression).  The inherited method GetURLPath is still available, in
    this case it returns the combination of the path, the question mark and
    the search expression.

    @see HFCURLCommonIntenret
    @see HFCURL

*/




class HFCURLECWP : public HFCURLCommonInternet
    {
public:

    HDECLARE_CLASS_ID(1294, HFCURLCommonInternet);

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"ecwp");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    //:> constructor
    _HDLLu                         HFCURLECWP() { } //:> required for persistence
    _HDLLu                         HFCURLECWP(const WString& pi_URL);
    _HDLLu                         HFCURLECWP(const WString& pi_User,
                                              const WString& pi_Password,
                                              const WString& pi_Host,
                                              const WString& pi_Port,
                                              const WString& pi_Path,
                                              const WString& pi_SearchPart);
    _HDLLu virtual                 ~HFCURLECWP();

    //:> Content access methods
    _HDLLu virtual WString         GetURL() const;
    const WString&          GetPath() const;
    const WString&          GetSearchPart() const;

    //:> Overriden methods, used in relative path management
    _HDLLu virtual bool           HasPathTo(HFCURL* pi_pURL);
    _HDLLu virtual WString         FindPathTo(HFCURL* pi_pDest);
    _HDLLu virtual HFCURL*         MakeURLTo(const WString& pi_Path);

#ifdef __HMR_DEBUG_MEMBER
    virtual void            PrintState() const;
#endif

protected:

private:

    friend struct URLECWPCreator;

    // Components of the URLPath part of the URL string.
    WString                  m_Path;
    WString                  m_SearchPart;

    // Disabled methods
    HFCURLECWP(const HFCURLECWP&);
    HFCURLECWP& operator=(const HFCURLECWP&);

    };

#include "HFCURLECWP.hpp"

