//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hfc/src/HFCURL.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Methods for class HFCURL
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HFCURL.h>

// Static member initialization

HFCURL::SchemeList* HFCURL::s_pSchemeList = 0;

//:Ignore

// The destroyer that frees the scheme list.  The creators registered in it are
// not deleted because these are static objects.
static struct SchemeListDestroyer
    {
    ~SchemeListDestroyer()
        {
        if (HFCURL::s_pSchemeList)
            delete HFCURL::s_pSchemeList;
        }
    } s_SchemeListDestroyer;

//:End Ignore

/**----------------------------------------------------------------------------
 This constructor configures the object from the detached parts of the URL
 specification.

 This class is an abstract one and is not instanciable alone.

 Copy constructor and assignment operators are disabled for this class.

 @param pi_SchemeType         Constant reference to a string that contains the
                              name of a scheme type, without the colon ":".

 @param pi_SchemeSpecificPart Constant reference to a string that contains the
                              rest of the URL specification (what is after the
                              colon).

 @inheritance Usually the constructor of a derived class performs the
              analysis of the string to parse it into its components.
              The constructor of the ancestor did that too: it is a good
              idea to recycle the results.  At this level, the parsing
              has splitted the URL string into two basic components :
              the scheme type (available through GetSchemeType) and the
              rest, which is identified as the "scheme specific part",
              available through GetSchemeSpecificPart.  So the
              constructor of the derived class should analyse the string
              returned by the latter.

              The derived class will usually feature more than one
              constructor : the others receives the URL in detached
              parts.  Assemble them into less components and pass them
              to the ancestor's constructor that also use detached
              parts.  Here it is the third version, that receives it
              into two sections.

 @see GetSchemeSpecificPart
 @see GetSchemeType
-----------------------------------------------------------------------------*/
HFCURL::HFCURL(const WString& pi_SchemeType, const WString& pi_SchemeSpecificPart)
    : m_SchemeType(pi_SchemeType), m_SchemeSpecificPart(pi_SchemeSpecificPart)
    {
    FREEZE_STL_STRING(m_SchemeType);
    FREEZE_STL_STRING(m_SchemeSpecificPart);

    m_UTF8URL = false;
    m_EncodedURL = false;
    }

/**----------------------------------------------------------------------------
 This constructor configure the object from the complete and weel defined URL
 specification.

 This class is an abstract one and is not instanciable alone.

 Copy constructor and assignment operators are disabled for this class.

 @param pi_URL Constant reference to a string that contains a complete URL
               specification.

 @inheritance Usually the constructor of a derived class performs the
              analysis of the string to parse it into its components.
              The constructor of the ancestor did that too: it is a good
              idea to recycle the results.  At this level, the parsing
              has splitted the URL string into two basic components :
              the scheme type (available through GetSchemeType) and the
              rest, which is identified as the "scheme specific part",
              available through GetSchemeSpecificPart.  So the
              constructor of the derived class should analyse the string
              returned by the latter.

              The derived class will usually feature more than one
              constructor : the others receives the URL in detached
              parts.  Assemble them into less components and pass them
              to the ancestor's constructor that also use detached
              parts.  Here it is the third version, that receives it
              into two sections.

 @see GetSchemeSpecificPart
 @see GetSchemeType
-----------------------------------------------------------------------------*/
HFCURL::HFCURL(const WString& pi_URL)
    {
    WString::size_type ColonPos = pi_URL.find(L':');
    if (ColonPos != WString::npos)
        {
        m_SchemeType = pi_URL.substr(0, ColonPos);
        m_SchemeSpecificPart = pi_URL.substr(ColonPos+1,
                                             pi_URL.length() - ColonPos - 1);
        }

    FREEZE_STL_STRING(m_SchemeType);
    FREEZE_STL_STRING(m_SchemeSpecificPart);

    m_UTF8URL = false;
    m_EncodedURL = false;
    }

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HFCURL::~HFCURL()
    {
    // Nothing to do here.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HFCURL* HFCURL::CreateFrom(const BeFileName& pi_beFilename)
    {
    HFCURL* pURL = HFCURL::Instanciate(pi_beFilename.GetName());
    if(pURL == NULL)
        {
        WString newUrl(WString(L"file://"));
        WString InputFixed;
        if (BeFileNameStatus::Success == BeFileName::FixPathName(InputFixed,pi_beFilename.GetName()))
            {
            newUrl += InputFixed;
            pURL = HFCURL::Instanciate(newUrl.c_str());
            }
        }

    return pURL;
    }


/**----------------------------------------------------------------------------
 This is a kind of "virtual constructor".  To be used instead of constructor
 of this class or of any derived class when come the time to get an object
 that defines an URL.  It creates a correctly-typed object for the given URL.
 Return zero if the scheme type is unknown, which means that the string is a
 relative path.

 @param pi_rURL Constant reference to a string that contains a complete URL
                specification.

 @return A pointer to a newly constructed URL descriptor.  That object is
         allocated on the heap and must be deleted by the caller of this
         method.

 @inheritance See the overview description of this class.
-----------------------------------------------------------------------------*/
HFCURL* HFCURL::Instanciate(const WString& pi_URL)
    {
    HFCURL* pNewObj = 0;
    WString::size_type ColonPos = pi_URL.find(L':');
    if (ColonPos != WString::npos)
        {
        WString SchemeType(pi_URL, 0, ColonPos);
        SchemeList::iterator itr = GetSchemeList().find(SchemeType);
        if (itr != GetSchemeList().end())
            {
            pNewObj = (*itr).second->Create(pi_URL);
            }
        }
    return pNewObj;
    }

/**----------------------------------------------------------------------------
 Static method that calculates a new path from a source path specification
 and a relative path.
-----------------------------------------------------------------------------*/
WString HFCURL::AddPath(const WString& pi_Source, const WString& pi_Path)
    {
    // If second path is from root, return it!

    if ((pi_Path.size() > 0) && ((pi_Path[0] == L'\\') || (pi_Path[0] == L'/')))
        return pi_Path;

    // First step : we create a list of path entries for the source.

    list<WString> EntryList;
    WString::size_type Pos = 0;
    if (!pi_Source.empty())
        {
        while (Pos != WString::npos)
            {
            WString::size_type FoundPos = pi_Source.find_first_of(L"\\/", Pos);
            if (FoundPos != WString::npos)
                {
                EntryList.push_back(pi_Source.substr(Pos, FoundPos - Pos));
                Pos = FoundPos+1;
                }
            else
                Pos = WString::npos;
            }
        }

    // We scan the relative path to construct the new path in the list.

    Pos = 0;
    while (Pos != WString::npos)
        {
        WString::size_type FoundPos = pi_Path.find_first_of(L"\\/", Pos);
        if (FoundPos != WString::npos)
            {
            WString PathStep(pi_Path.substr(Pos, FoundPos - Pos));
            if (PathStep.compare(L"..") == 0)
                {
                HASSERT(EntryList.size() > 0);
                EntryList.pop_back();
                }
            else if (PathStep.compare(L".") == 0)
                {
                EntryList.push_back(PathStep);
                }
            Pos = FoundPos+1;
            }
        else
            {
            if (Pos < pi_Path.length())
                EntryList.push_back(pi_Path.substr(Pos, pi_Path.length() - Pos));
            Pos = WString::npos;
            }
        }

    // The new path is created

    WString Result;
    list<WString>::iterator itr = EntryList.begin();
    while (itr != EntryList.end())
        {
        if (!Result.empty())
            Result += L"/";
        Result += *itr;
        ++itr;
        }
    return Result;
    }

/**----------------------------------------------------------------------------
 Static method that find the relative path that can describe the difference
 between a source and a destination specified as standard paths.
 There must be a way to get from source to dest : use HasPathTo to check
 this.
-----------------------------------------------------------------------------*/
WString HFCURL::FindPath(const WString& pi_Source, const WString& pi_Dest)
    {
    // First step : we create a list of path entries for the source.

    list<WString> EntryList;
    WString::size_type Pos = 0;
    if (!pi_Source.empty())
        {
        while (Pos != WString::npos)
            {
            WString::size_type FoundPos = pi_Source.find_first_of(L"\\/", Pos);
            if (FoundPos != WString::npos)
                {
                EntryList.push_back(pi_Source.substr(Pos, FoundPos - Pos));
                Pos = FoundPos + 1;
                }
            else
                Pos = WString::npos;
            }
        }

    // Next we scan the destination path to count the similar entries from
    // the beginning, removing them from the destination path

    WString Result(pi_Dest);
    unsigned short Count = 0;
    if (EntryList.size() > 0)
        {
        list<WString>::iterator itr = EntryList.begin();
        while (Result.length() && (itr != EntryList.end()))
            {
            WString::size_type FoundPos = Result.find_first_of(L"\\/");
            if (FoundPos != WString::npos)
                {
                WString PathStep(Result.substr(0, FoundPos));
                if (PathStep == *itr)
                    {
                    Result.erase(0, FoundPos+1);
                    ++Count;
                    ++itr;
                    }
                else
                    itr = EntryList.end();
                }
            else
                itr = EntryList.end();
            }
        }

    // We complete the remaining part of destination path with relative path
    // entries.

    for (; Count < EntryList.size(); Count++)
        Result = L"../" + Result;

    return Result;
    }

/**----------------------------------------------------------------------------
 Returns true only if a relative path can be calculated between this URL
 and the specified one.  A relative path is a string that describes how
 the location of a given resource can be described according to the
 position of another resouce.  This is possible if both resources are
 placed on the same media (i.e. same disk or same server address) and if
 the media support hierarchical organisation of its resouces into
 directories.  This also implies that both URLs are of same scheme type.

 @param pi_pDest Pointer to another URL descriptor that designates the
                 destination of the relative path to establish.

 @return true if a relative path exists between this URL and the specified
         one, false otherwise.

 @inheritance This virtual method can be overriden.  In the derived class
              version, call the ancestor's version of this method first, if it
              returned false stop and return false too, if it returned true
              further verification may be performed.

 @see FindPathTo
-----------------------------------------------------------------------------*/
bool HFCURL::HasPathTo(HFCURL* pi_pDest)
    {
    return GetSchemeType() == pi_pDest->GetSchemeType();
    }



/**----------------------------------------------------------------------------
 Specify if the URL is stored in UTF8, usefull when the URL is URL-Encoded.

 @param pi_UTF8 true : It is UTF8.

 @return None
-----------------------------------------------------------------------------*/
void  HFCURL::SetUTF8URL (bool pi_UTF8)
    {
    m_UTF8URL = pi_UTF8;
    }


/**----------------------------------------------------------------------------
Specify if the URL is encoded.

@param pi_Encoded true : Encoded URL.

@return None
-----------------------------------------------------------------------------*/
void  HFCURL::SetEncodedURL(bool pi_Encoded)
    {
    m_EncodedURL = pi_Encoded;
    }

//:Ignore
#if 0
//:End Ignore

/**----------------------------------------------------------------------------
 Returns a string that contains the standardized URL specification
 described by this object.  In other word, converts into a character
 string this URL.

 @return A string containing a complete and normalized URL specification.

 @inheritance This pure virtual method must be overriden by derived classes.
              The method has to construct a valid URL string from the components
              stored internally.
-----------------------------------------------------------------------------*/
WString HFCURL::GetURL() const { }

/**----------------------------------------------------------------------------
 Calculates the relative path that is found between this URL and the
 specified one.  A path must exist between both URLs before calling this
 method : this can be verified by first calling @k{HasPathTo}.

 A relative path is a string that describes how the location of a given
 resource can be described according to the position of another resouce.
 This is possible if both resources are placed on the same media (i.e.
 same disk or same server address) and if the media support hierarchical
 organisation of its resouces into directories.

 The relative path can be used with the method @k{MakeURLTo} to reconstruct a
 new URL from the combination of a source URL and the path.

 @param pi_pDest Pointer to another URL descriptor that designates the
                 destination of the relative path to establish.

 @return A string containing the relative path description.  The notation it uses
         is the same than for DOS and Unix file systems.

 @inheritance This pure virtual method must be overriden by derived
              classes.  It is suggested to use the protected method
              @k{FindPath} with URLs reduced to string without scheme
              specification and without media specification (only paths
              from the root of the media).  See info about this in the
              overview of the class.

 @see HasPathTo
-----------------------------------------------------------------------------*/
WString HFCURL::FindPathTo(HFCURL* pi_pDest) { }

/**----------------------------------------------------------------------------

 Constructs a new URL by combining this one with a relative path.  The
 relative path specification should have been produced by the method
 @k{FindPathTo}.

 @param pi_rPath Constant reference to a string that contains the relative path
                 specification between the resource pointed to by this URL and
                 an another one, for which a URL will be produced.

 @return A newly constructed URL descriptor of same type, that will point to
         the resource designated by the relative path starting from this URL.
         That object is allocated on the heap and must be deleted by the caller
         of this method.

 @inheritance This pure virtual method must be overriden by derived classes.
              It is suggested to use the protected method @k{AddPath} by reducing
              this URL to a string without scheme specification and without
              media specification (only paths from the root of the media).
              See info about this in the overview of the class.

 @see FindPathTo
-----------------------------------------------------------------------------*/
HFCURL* HFCURL::MakeURLTo(const WString& pi_Path) { }

//:Ignore
#endif
//:End Ignore

