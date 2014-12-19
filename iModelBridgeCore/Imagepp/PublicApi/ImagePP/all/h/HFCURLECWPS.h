//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURLECWPS.h $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HFCURLECWPS
//-----------------------------------------------------------------------------
#pragma once

#include "HFCURLCommonInternet.h"

//:Ignore
// URL specification at this level is:
// http:[//][user[:password]@]host[:port][/[Path[?SearchPart]]]
//:End Ignore

/**

    This class defines the URL descriptor for the "http" scheme type.  That
    kind of URL is used to locate files that are accessible through HTTP
    server (web servers).  The structure of the text of the URLs follows the
    syntax of the common internet scheme types, with one particularity: the
    url-path section is divided into two components, one being the path
    itself, the second being a facultative search expression.  The resulting
    syntax is as follows:

    @code
    http://[@i{user}[:@i{password}]@]@i{host}[:@i{port}][/[@i{path}[?@i{search}]]]
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

class HFCURLECWPS : public HFCURLCommonInternet
    {
public:

    HDECLARE_CLASS_ID(1295, HFCURLCommonInternet);

    // Define the Scheme label
    static const WString& s_SchemeName()
        {   static const WString Val(L"ecwps");
        FREEZE_STL_STRING(Val);
        return Val;
        }

    //:> constructor
    _HDLLu                         HFCURLECWPS() { } //:> required for persistence
    _HDLLu                         HFCURLECWPS(const WString& pi_URL);
    _HDLLu                         HFCURLECWPS(const WString& pi_User,
                                               const WString& pi_Password,
                                               const WString& pi_Host,
                                               const WString& pi_Port,
                                               const WString& pi_Path,
                                               const WString& pi_SearchPart);
    _HDLLu virtual                 ~HFCURLECWPS();

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

    friend struct URLECWPSCreator;

    // Components of the URLPath part of the URL string.
    WString                  m_Path;
    WString                  m_SearchPart;

    // Disabled methods
    HFCURLECWPS(const HFCURLECWPS&);
    HFCURLECWPS& operator=(const HFCURLECWPS&);

    };


#include "HFCURLECWPS.hpp"

