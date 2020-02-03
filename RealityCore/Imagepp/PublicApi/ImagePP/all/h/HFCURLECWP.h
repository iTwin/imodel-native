//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLECWP
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURLCommonInternet.h"

BEGIN_IMAGEPP_NAMESPACE
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

    HDECLARE_CLASS_ID(HFCURLId_ECWP, HFCURLCommonInternet);

    // Define the Scheme label
    static const Utf8String& s_SchemeName()
        {   
        static const Utf8String Val("ecwp");
        return Val;
        }

    //:> constructor
    IMAGEPP_EXPORT                         HFCURLECWP(const Utf8String& pi_URL);
    IMAGEPP_EXPORT                         HFCURLECWP(const Utf8String& pi_User,
                                              const Utf8String& pi_Password,
                                              const Utf8String& pi_Host,
                                              const Utf8String& pi_Port,
                                              const Utf8String& pi_Path,
                                              const Utf8String& pi_SearchPart);
    IMAGEPP_EXPORT virtual                 ~HFCURLECWP();

    //:> Content access methods
    IMAGEPP_EXPORT Utf8String         GetURL() const override;
    const Utf8String&          GetPath() const;
    const Utf8String&          GetSearchPart() const;

    //:> Overriden methods, used in relative path management
    IMAGEPP_EXPORT bool           HasPathTo(HFCURL* pi_pURL) override;
    IMAGEPP_EXPORT Utf8String         FindPathTo(HFCURL* pi_pDest) override;
    IMAGEPP_EXPORT HFCURL*         MakeURLTo(const Utf8String& pi_Path) override;

#ifdef __HMR_DEBUG_MEMBER
    virtual void            PrintState() const;
#endif

protected:

private:

    friend struct URLECWPCreator;

    // Components of the URLPath part of the URL string.
    Utf8String                  m_Path;
    Utf8String                  m_SearchPart;

    // Disabled methods
    HFCURLECWP(const HFCURLECWP&);
    HFCURLECWP& operator=(const HFCURLECWP&);

    };

END_IMAGEPP_NAMESPACE
#include "HFCURLECWP.hpp"

