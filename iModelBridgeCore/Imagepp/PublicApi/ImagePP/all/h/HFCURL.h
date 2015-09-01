//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HFCURL.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HFCURL
//-----------------------------------------------------------------------------

#pragma once

#include "HFCPtr.h"
#include "HFCAccessMode.h"

BEGIN_IMAGEPP_NAMESPACE
//:Ignore
// URL specification at this level is:
// <scheme-type>:<scheme-specific-part>

// The scheme specific part will have to be analyzed by derived classes.
//:End Ignore



/**

    This class is designed to provide an object representation to "uniform
    resource locators", which are strings that are used to locate specific
    resources, like files, directories, servers, etc.  Instead of using URL
    as character strings (either C strings or STL strings), they become
    objects having their own identity and specific typing (because there are
    many "kinds" of URLs) that feature an extended interface wich permits an
    easier analysis of their content.

    The use of this class (and derived ones) change the way URLs are seen :
    instead of being a monolithic string, they can be seen as having many
    components that are structured hierarchically:

    @list{an URL is always composed of a scheme type specification followed by
    a part whose content and structure is specific to that scheme type;}

    @list{all kinds of URLs that locate internet resources, for example, have
    a common way to use that second part, with additional particularities
    for each kind of URL;}

    This class provides an interface that help the parsing of an URL to get
    its components.

    This class and its derived ones also provide a specific typing for URLs.
    Each instanciable derived class correspond to a specific kind of URL,
    and the interface of this class provides services specific to that kind
    of URL (usually specific parsing).  The hierarchy of classes correspond
    to the way the URLs are classified.

    @h3{Instanciation of HFCURL objects}

    An HFCURL object may be constructed to build a new URL or to "wrap" an
    already existent URL.  For a new URL, the explicit instanciation of the
    proper class is all that is needed to do.

    To obtain an HFCURL object from a string that contains an URL, there are
    two ways : either instanciating explicitly the proper class, when the
    type of the URL is already known (which is a rare case, without
    parsing), or automatic choice of the class by instanciating through a
    static method @k{Instanciate} that analyses the URL to discover its type and
    constructs the right kind of object.  See description for that function
    further in this document.

    In order to have proper behavior from that "factory", all classes that
    derives from HFCURL must be registered in a list.  See notes for derived
    classes below.

    @h3{Relative paths between URLs}

    This class also provides services to find the relationships between
    different URLs, under the form of "relative paths".  A relative path is
    the information that, after being combined with a URL, results in a
    second URL.  It describes the path to follow from the first resource to
    find the second one, if both are using the same media.  A common example
    is the relation between two files on the same disk: there is a path
    between these files that can be "walked" into the directory hierarchy of
    the disk.  By the way, the relative paths of URLs are using the same
    type of notation that is used for files in DOS and Unix systems, because
    they can be obtained only for medias that support hierarchical
    organisation into directories:  file systems, web servers, ftp servers,
    etc.

    To get a relative path from to URLs, they must point both to resources
    on same media, and that media must use directories.  To know if a
    relative path is available, the method @k{HasPathTo} can be used on the
    first URL, and the path description (which is a character string) can be
    obtained by calling the method @k{FindPathTo}.  The reverse procedure is
    possible by applying a path on an existent URL (by calling @k{MakeURLTo}) in
    order to obtain a new URL.

    @h3{Notes for designers of derived classes}

    A class that derives from HFCURL that is able to instanciate objects
    must correspond to one scheme type supported for the URL notation.  That
    class must define or override virtual methods (see @i{Inheritance notes} in
    the description of methods) and be registered in the mechanism
    used by static method Instanciate.

    To register a class, the programmer must define a private class, having
    friendship with the class to register, that derives from
    HFCURL::Creator, a class provided by HFCURL for this purpose, that has a
    virtual method @k{Create} that must be overriden.  There must be one static
    instance of that class, and its constructor inserts a reference to that
    instance in the list used by @k{Instanciate}, that can be reached by calling
    the protected method @k{GetSchemeList} defined in HFCURL (not described in
    this document).  Here is an example of all of this for the class
    HFCURLFile, which corrsponds to URLs of scheme type "file":

    @code
    |  static struct URLFileCreator : public HFCURL::Creator
    |  {
    |    URLFileCreator()
    |    {
    |      HFCURLFile::GetSchemeList().insert(
    |                     HFCURLFile::SchemeList::value_type("file", this));
    |    }
    |    virtual HFCURL* Create(const string& pi_RUL) const
    |    {
    |      return new HFCURLFile(pi_URL);
    |    }
    |  } s_URLFileCreator;
    @end

    For this example, the class @r{URLFileCreator} is a friend of HFCURLFile.
    The scheme list owned by HFCURL, reached by calling @k{GetSchemeList}, is a
    map, so it contains "pairs" where the first value is the key (the scheme
    type string) and the second value is the pointer to the creator.  The
    method Create receives a string containing an URL to wrap into an
    object.

    The programmer should also define a constant member in the class that
    contains the string of the scheme-type, for example : @c{char*
    HFCURLFile::s_SchemeType = "file";}

    When defining the virtual methods @k{FindPathTo} and @k{MakeURLTo}, the
    programmer may need path handling tools.  Two protected methods are
    provided for this purpose and are not described elsewhere in this
    document:

    @code
    static string AddPath(const string& pi_Source, const string& pi_Path);
    static string FindPath(const string& pi_Source, const string& pi_Dest);
    @end

    They work with resource locators without media identification (scheme
    type removed, media name removed), so they must begin without a slash or
    blackslash (both separators are supported).  @k{AddPath} uses the location
    of a resource as a basis (not only its path, but the object name too),
    uses a relative path and provides a resulting resource location.
    @k{FindPath} calculates the difference between two locations.

    @h3{Persistence notes}

    HFCURL is a descendant of the HPMPersistentObject class, making possible
    the persistence of typed and parsed URLs, usually as member of other
    persistent objects.  When creating a derived class, the rules for
    defining persistent objects must be observed, which is the use of macros
    for the class and for its members, and the presence of a default
    constructor.  See documentation of HPM library for details.

    @h3{Other notes}

    HFCURL is also a descendant of the HFCShareableObject class, but this is
    not specifically documented.  This ancestor simply allow instances of
    this class (and of derived classes) to be pointed to by "smart pointers"
    implemented with the HFCPtr class.

    @see HPMPersistentObject
    @see HFCURLFile
    @see HFCURLCommonInternet

*/

class HNOVTABLEINIT HFCURL : public HFCShareableObject<HFCURL>
    {
public:

    HDECLARE_BASECLASS_ID(HFCURLId_Base);

    //:> Primary methods.

    HFCURL(const WString& pi_URL);
    HFCURL(const WString& pi_SchemeType,
           const WString& pi_SchemeSpecificPart);
    HFCURL() { }  // required for persistence
    virtual                 ~HFCURL();

    //:> This static method replaces the constructor.  Use it to create
    //:> a correctly-typed object for specified URL.

    IMAGEPP_EXPORT static HFCURL*          Instanciate(const WString& pi_URL);
    IMAGEPP_EXPORT static HFCURL*          CreateFrom(const BeFileName& pi_beFilename);

    //:> Content access methods

    virtual WString         GetURL() const = 0;
    const WString&          GetSchemeType() const;
    const WString&          GetSchemeSpecificPart() const;

    //:> Specify if the URL is UTF8 or not, usefull when the URL is URL-Encoded
    IMAGEPP_EXPORT         void     SetUTF8URL (bool pi_UTF8);
    bool                   IsUTF8URL() const;

    IMAGEPP_EXPORT         void     SetEncodedURL(bool pi_Ecoded);
    IMAGEPP_EXPORT         bool    IsEncodedURL() const;


    //:> Methods used in relative path management

    virtual bool           HasPathTo(HFCURL* pi_pURL);
    virtual WString         FindPathTo(HFCURL* pi_pDest) = 0;
    virtual HFCURL*         MakeURLTo(const WString& pi_Path) = 0;

    // This is a utility class.  There will be a class that derives from this one
    // for each URL class.  It is used in scheme list.

    struct Creator
        {
        virtual HFCURL* Create(const WString& pi_URL) const = 0;
        };

#ifdef __HMR_DEBUG_MEMBER
    virtual void PrintState() const = 0;
#endif


//DM-Android     Not able to build if private members
    // The type of the scheme list
    typedef map<WString, Creator*, CaseInsensitiveStringCompare, allocator<Creator*> >
        SchemeList;    // The scheme list.
    static SchemeList*      s_pSchemeList;
    //:> Scheme list access
    static SchemeList&         GetSchemeList();     // from protected
protected:

    //:> *** Relative paths support ***

    //:> Utility functions provided as static methods available for derived classes.

    static WString           AddPath(const WString& pi_Source, const WString& pi_Path);
    static WString           FindPath(const WString& pi_Source, const WString& pi_Dest);

private:

    friend struct SchemeListDestroyer;

    // First level of decomposition of URL string

    WString                 m_SchemeType;
    WString                 m_SchemeSpecificPart;
    bool                   m_UTF8URL;
    bool                   m_EncodedURL;


    // Disabled methods

    HFCURL(const HFCURL&);
    HFCURL& operator=(const HFCURL&);

    };
END_IMAGEPP_NAMESPACE

#include "HFCURL.hpp"

