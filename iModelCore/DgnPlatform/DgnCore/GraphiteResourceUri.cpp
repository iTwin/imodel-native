/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/GraphiteResourceUri.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define SCHEME_DgnResourceURI   "dgnresourceuri"
#define SCHEME_DgnResourceURI_Length 14

#define RT_DgnElements  "DgnElements"
#define RT_DgnElements_Length   11
#define RT_ECInstances  "ECInstances"
#define RT_ECInstances_Length   11

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnResourceURI
    {
    //! Well known resource types. See DgnResourceURI::GetWellKnownResourceType
    enum WellKnownResourceType
        {
        WELL_KNOWN_RESOURCE_TYPE_DgnElements=1,    //!< "DgnElements"
        WELL_KNOWN_RESOURCE_TYPE_ECInstances=2     //!< "ECInstances"
        };

    //! The result of parsing a Dgn URI string
    enum ParseStatus
        {
        PARSE_STATUS_SUCCESS=0,           //!< The URI seems to be valid
        PARSE_ERROR_InvalidSyntax=-1      //!< The URI contains invalid or unexpected characters. A URI should be of the form resource_type/id or resource_type?query
        };

private:
    Utf8String      m_targetFile; // The target file. Optional.
    Utf8String      m_path;     // The path portion of the URI (including a leading '/' prefix)
    Utf8String      m_query;    // Optional. The query parameters (including the '?' prefix). 
    Utf8String      m_fragment; // Optional. The fragment (including the '#' prefix)

public:
    struct UriToken;
    struct ParserBase;
    struct PathParser;
    struct QueryParser;

public:
    DGNPLATFORM_EXPORT DgnResourceURI ();

    //! Rarely used function to construct a DgnResourceURI by parsing a URI string. 
    //! @param uriString    An encode URI string.
    //! @return non-zero error status if the URI string is invalid.
    //! it easy to build a valid URI and takes care of details like encoding.
    //! @remarks The string must be a relative URI following the syntax of DgnResourceURI.
    //! @note The string must be URI-encoded.
    DGNPLATFORM_EXPORT ParseStatus FromEncodedString (Utf8CP uriString);

    //! Get this URI as a string. 
    //! @return the URI as a string in encoded form.
    DGNPLATFORM_EXPORT Utf8String ToEncodedString() const;

    //! Get a well-known resource type
    //! @param t    The type
    //! @return The string that identifies the type
    DGNPLATFORM_EXPORT static Utf8CP GetWellKnownResourceType (WellKnownResourceType t);

    //! Sets the scheme and the target file identifer as the path root.
    //! The scheme for a DgnResourceURI with an explicit target file is always "dgnresourceuri:".
    //! The target file becomes the root of the path. The target file is identified by its filename (not including its path), followed by its BeGuid.
    //! @remarks By default, a DgnResourceURI will have no scheme or file identifier. Often these qualifiers are not needed. 
    //! The scheme and target file identifer are added only if you call SetTargetFile. You only need to add these items if you must create
    //! a URI can be distinguished from URIs of other types.
    DGNPLATFORM_EXPORT void SetTargetFile (DgnDbR targetFile);

    //! Get the target file identifier, if present.
    DGNPLATFORM_EXPORT BentleyStatus GetTargetFile (Utf8StringR filename, BeGuid& guid) const;

    //! If this URI contains a target file identifier, then this function checks that the GUID in that identifier matches the GUID in file.
    //! @return non-zero error status if this URI has a target file GUID and it does not match the GUID of \a file. If this URI does not have
    //! a target file identifier, then this function returns SUCCESS.
    DGNPLATFORM_EXPORT BentleyStatus CheckTargetFileMatches (DgnDbR file) const;

    //! Get the encoded path portion of this URI. See DgnResourceURI::PathParser.
    DGNPLATFORM_EXPORT Utf8String GetPath() const;

    //! Get the encoded query portion of this URI. See DgnResourceURI::QueryParser. 
    DGNPLATFORM_EXPORT Utf8String GetQuery() const;

    //! Get the encoded fragment portion of this URI. See BeStringUtilities::UriDecode.
    DGNPLATFORM_EXPORT Utf8String GetFragment() const;
    };


//=======================================================================================
//! A token parsed from the path or query part of a DgnResorceURI. This class is used by the parser classes.  
//=======================================================================================
struct DgnResourceURI::UriToken
    {
    //! Identifies the type of data held or represented by this token.
    enum Type 
        {
        TYPE_Error,     //!< A parsing error
        TYPE_EOS,       //!< The end of the string being parsed
        TYPE_UInt64,    //!< A UInt64
        TYPE_String     //!< A Utf8String
        };

private:
    struct Utf8SubString
        {
        Utf8CP m_start, m_end;
        };

    Type m_type;
    union
        {
        Utf8SubString   m_substr;
        uint64_t        m_uint64;
        };

public:
    //! Constructs an invalid token. See SetUInt64, SetString, and SetEOS.
    //! @private
    UriToken () : m_type(TYPE_Error) {;}

    //! Defines this token as representing the end of the caller's string.
    //! @private
    void SetEOS() {m_type=TYPE_EOS;}

    //! Defines this token as representing a parsing error.
    //! @private
    void SetError() {m_type=TYPE_Error;}

    //! Defines this token as a UInt64.
    //! @private
    void SetUInt64 (uint64_t v) {m_type=TYPE_UInt64; m_uint64=v;}

    //! Defines this token as a string. @remarks The UriToken will hold the two pointers supplied. Caller must guarantee that the referenced string is not destroyed while this UriToken exists.
    //! @private
    void SetString (Utf8CP s, Utf8CP e) {m_type=TYPE_String; m_substr.m_start=s; m_substr.m_end=e;}

    //! Gets the type of this token.
    Type GetType() const {return m_type;}

    //! Gets the value of this token as a UInt64. 
    //! @return the value of this token. 
    //! @remarks Returns 0 if the type of this token is not TYPE_UInt64. This function does not attempt to parse an integer from a string.
    uint64_t GetUInt64() const {BeAssert(m_type==TYPE_UInt64); return (m_type==TYPE_UInt64)? m_uint64: 0;}

    //! Gets the value of this token as a non-encoded string.
    //! @remarks If the type of this token is TYPE_String, then this function will return it in decoded form.
    //! @remarks If the type of this token is TYPE_UInt64, then this function will return the formatted integer value.
    DGNPLATFORM_EXPORT Utf8String GetString() const;
    };

//=======================================================================================
//! Base class for URI parsers.
//! The string to be parsed must be in URI-encoded form. That is, it must not contain 
//! embedded blanks or any reserved characters as part of the tokens.
//! The string-valued tokens that are returned by the parser are automatically decoded.
//=======================================================================================
struct DgnResourceURI::ParserBase
    {
protected:
    Utf8String m_rawString;
    Utf8CP m_curpos;
    UriToken m_curtok;
    Utf8CP m_tokenDelimiters;
    
    DGNPLATFORM_EXPORT BentleyStatus ParseUInt64();
    DGNPLATFORM_EXPORT void ParseString();
    DGNPLATFORM_EXPORT void ToNextDelimiter();
    DGNPLATFORM_EXPORT void ToStartOfNextToken ();

public:
    //! Constructs a parser that breaks a string into a sequence of UriTokens.
    //! For example, if the string is /a/b/c and delims is "/" then ParseNextToken will yield three strings, "a", "b", and "c".
    //! @param str      The string to parse. Must be encoded.
    //! @param delims   The characters that delimit the tokens
    //! @remarks Call ParseNextToken to get the first token.
    //! @remarks \a str must be URI-encoded.
    DGNPLATFORM_EXPORT ParserBase (Utf8StringCR str, Utf8CP delims);
    
    //! Reads the next token from the string.
    //! @param mustBeString if true, then the next token is expected to be a string. 
    //! @return the token read.
    //! @see GetCurToken
    DGNPLATFORM_EXPORT UriToken ParseNextToken (bool mustBeString = false);

    //! Gets the most recently parsed token.
    //! @remarks String tokens are returned in decoded form.
    DGNPLATFORM_EXPORT UriToken GetCurToken() const;

/** @name EC Helper Methods */
/** @{ */
    //! Parses ecschemaname ':' ecclassname
    //! @param[out] ecSchemaName    The name of the schema
    //! @param[out] ecClassName     The name of the class
    //! @param[in] str              The full class name to be parsed
    //! @param[in] mustHaveSchema   If true, this function returns an error if \a str does not contain a schema name.
    //! @return non-zero error status if the string could not be parsed.
    DGNPLATFORM_EXPORT static BentleyStatus ParseFullECClassName (Utf8StringR ecSchemaName, Utf8StringR ecClassName, Utf8CP str, bool mustHaveSchema = true);
/** @} */
    };

//=======================================================================================
//! Parses the path part of a DgnResourceURI. 
//! @see DgnResourceURI::GetPath.
//=======================================================================================
struct DgnResourceURI::PathParser : DgnResourceURI::ParserBase
    {
    //! Constructs a parser for a URI path
    //! @param pathString      The URI path to parse. Must be encoded.
    //! @remarks \a pathString may contain a leading / or not. This parser ignores the leading / if found.
    //! @remarks \a pathString must be URI-encoded.
    DGNPLATFORM_EXPORT PathParser (Utf8StringCR pathString);
    };

//=======================================================================================
//! Parses the query part of a DgnResourceURI. 
//! @see DgnResourceURI::GetQuery.
//=======================================================================================
struct DgnResourceURI::QueryParser : DgnResourceURI::ParserBase
    {
    //! Constructs a parser for a URI query
    //! @param queryString      The URI path to parse. Must be encoded.
    //! @remarks \a queryString may contain a leading ? or not. This parser ignores the leading ? if found.
    //! @remarks \a queryString must be URI-encoded.
    DGNPLATFORM_EXPORT QueryParser (Utf8StringCR queryString);

    //! Parse the next name, value pair from the query.
    //! @param[out] propname     The name of the property
    //! @param[out] propvalue    The value of the property
    //! @param[out] logical      The value of the property
    //! @param[in] valueMustBeString If true, then the value is always a string
    DGNPLATFORM_EXPORT BentleyStatus ParseQueryParameter (UriToken& propname, UriToken& propvalue, char& logical, bool valueMustBeString = false);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getEncodedTargetFile (DgnDbR targetFile)
    {
    Utf8String filename (BeFileName (BeFileName::NameAndExt, targetFile.GetFileName().c_str()));
    filename.append (";");
    filename.append (targetFile.GetDbGuid().ToString());
    return BeStringUtilities::UriEncode(filename.c_str());
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_NOT_YET_SUPPORTED
static BentleyStatus findElementIdByProvenance (DgnElementId& eid, Utf8CP oldFileName, int64_t oldElementId, DgnProjectR project)
    {
    DgnProvenances prov = project.Provenance();

    //  Look up the file
    uint64_t oldFileId;
    if (prov.QueryFileId (oldFileId, oldFileName) != SUCCESS)
        return ERROR;

    //  Look up the element
    ElementProvenance eprov (oldFileId, oldElementId);
    eid = prov.GetElement (eprov);
    return eid.IsValid()? SUCCESS: ERROR;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::UriToken::GetString() const
    {
    switch (m_type)
        {
        case TYPE_String:   return BeStringUtilities::UriDecode (m_substr.m_start, m_substr.m_end);
        case TYPE_UInt64:   return Utf8PrintfString("%llu", m_uint64);
        }
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::ParserBase::ParseUInt64 ()
    {
    uint64_t v;
    if (1 != BE_STRING_UTILITIES_UTF8_SSCANF (m_curpos, "%" PRId64, &v))
        return ERROR;

    m_curtok.SetUInt64 (v);        

    ToNextDelimiter();
        
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::ParserBase::ParseString ()
    {
    Utf8CP p = m_curpos;
    ToNextDelimiter();
    m_curtok.SetString (p, m_curpos);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::ParserBase::ToNextDelimiter ()
    {
    // Skip over non-delimiters
    m_curpos += strcspn (m_curpos, m_tokenDelimiters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::ParserBase::ToStartOfNextToken ()
    {
    // Skip over delimiters
    while (*m_curpos && strchr(m_tokenDelimiters,*m_curpos))
        ++m_curpos;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::ParserBase::ParserBase (Utf8StringCR pathStr, Utf8CP delims) : m_tokenDelimiters(delims)
    {
    m_rawString = pathStr;
    m_curpos = &m_rawString[0];
    if (ispunct(*m_curpos))
        ++m_curpos; // step over initial / or ? or #
    m_curtok.SetError();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::UriToken DgnResourceURI::ParserBase::GetCurToken() const {return m_curtok;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::UriToken DgnResourceURI::ParserBase::ParseNextToken (bool mustBeString)
    {
    ToStartOfNextToken();
        
    if (!*m_curpos)
        {
        m_curtok.SetEOS();
        }
    else
        {
        if (mustBeString || (ParseUInt64() != SUCCESS))
            {
            ParseString();
            }
        }

    return m_curtok;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::ParserBase::ParseFullECClassName (Utf8StringR ecSchemaName, Utf8StringR ecClassName, Utf8CP str, bool mustHaveSchema)
    {
    Utf8CP colon = strstr (str+1, ":");

    if (colon != nullptr)
        {
        ecSchemaName.assign (str, colon);
        ecClassName.assign (colon+1);
        return SUCCESS;
        }
            
    if (mustHaveSchema)
        return ERROR;
            
    ecSchemaName.clear();
    ecClassName.assign (str);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::PathParser::PathParser (Utf8StringCR str) : DgnResourceURI::ParserBase(str, "/?#") {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::QueryParser::QueryParser (Utf8StringCR str) : DgnResourceURI::ParserBase(str, "&#") {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::QueryParser::ParseQueryParameter (UriToken& propname, UriToken& propvalue, char& logical, bool valueMustBeString)
    {
    if (*m_curpos == '&')
        logical = '&';
    else
        logical = 0;

    // parse everything up to the = sign
    Utf8CP delims = m_tokenDelimiters;
    m_tokenDelimiters = "&#=";

    propname = ParseNextToken();

    propvalue = ParseNextToken(valueMustBeString);

    m_tokenDelimiters = delims;

    return (propname.GetType()  == UriToken::TYPE_EOS || propname.GetType() == UriToken::TYPE_Error)
        || (propvalue.GetType() == UriToken::TYPE_EOS || propvalue.GetType() == UriToken::TYPE_Error)? ERROR: SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnResourceURI::GetWellKnownResourceType (WellKnownResourceType t) 
    {
    switch (t)
        {
        case WELL_KNOWN_RESOURCE_TYPE_DgnElements: return RT_DgnElements;
        case WELL_KNOWN_RESOURCE_TYPE_ECInstances: return RT_ECInstances;
        }
    BeAssert (false);
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::DgnResourceURI ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnResourceURI::ParseStatus DgnResourceURI::FromEncodedString (Utf8CP uriString)
    {
    m_path.clear();

    //  Pull off the optional scheme:/filename;fileguid prefix
    Utf8CP s = strstr (uriString, SCHEME_DgnResourceURI ":/");
    if (s != nullptr)
        {
        uriString += SCHEME_DgnResourceURI_Length + 2;  // skip over scheme:/
        // make sure target file is also in there! I hope that detecting a semicolon (%3B) is enough.
        Utf8CP semicolon = strstr (uriString, "%3B");
        if (semicolon == nullptr)
            return PARSE_ERROR_InvalidSyntax;
        Utf8CP slash = strstr (semicolon+3, "/");
        if (slash == nullptr)
            return PARSE_ERROR_InvalidSyntax;
        m_targetFile.assign (uriString, slash);
        uriString = slash;
        }

    //  Parse the path and optional query
    Utf8CP q = strstr (uriString, "?");
    if (q == nullptr)
        {
        m_path = uriString;
        if (*m_path.rbegin() == '/')     // drop trailing /
            m_path.resize (m_path.length() - 1);
        m_query.clear();
        }
    else
        {
        m_path.assign (uriString, q);
        m_query.assign(q);
        }

    if (m_path[0] != '/')
        m_path.insert (m_path.begin(), '/');

    //  Parse the optional fragment
    Utf8CP f = strstr (uriString, "#");
    if (f != nullptr)
        m_fragment = f;
    else
        m_fragment.clear();

    return PARSE_STATUS_SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::SetTargetFile (DgnDbR targetFile)
    {
    m_targetFile = getEncodedTargetFile (targetFile);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::GetTargetFile (Utf8StringR filename, BeGuid& guid) const
    {
    if (m_targetFile.empty())
        return ERROR;

    Utf8String targetFileDecoded = BeStringUtilities::UriDecode (m_targetFile.c_str());

    size_t semicolon = targetFileDecoded.find (";");
    BeAssert (semicolon != Utf8String::npos && "Invalid target file string was accepted by FromEncodedString");

    filename.assign (targetFileDecoded.substr (0, semicolon));

    guid.FromString (targetFileDecoded.substr (semicolon+1).c_str());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnResourceURI::CheckTargetFileMatches (DgnDbR file) const
    {
    Utf8String filename;
    BeGuid guid;
    if (GetTargetFile (filename, guid) != SUCCESS)
        return SUCCESS;

    return (guid == file.GetDbGuid())? SUCCESS: ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::GetPath() const
    {
    return m_path;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::GetQuery() const
    {
    return m_query;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::GetFragment() const
    {
    return m_fragment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdGraphiteURI(Utf8CP uriStr) const
    {
    DgnResourceURI uri;
    if (DgnResourceURI::PARSE_STATUS_SUCCESS != uri.FromEncodedString(uriStr))
        return DgnElementId();

    //BeAssert (uri.CheckTargetFileMatches(GetDgnDb()) == SUCCESS);

    //  -----------------------------  
    //  Path
    //  -----------------------------  
    DgnResourceURI::PathParser pathParser (uri.GetPath());

    // /DgnElements... is the only kind of path that I support
    if (pathParser.ParseNextToken().GetString() != RT_DgnElements)
        return DgnElementId();

    switch (pathParser.ParseNextToken().GetType())
        {
        default: //  Unsupported path 
            return DgnElementId();
             
        case DgnResourceURI::UriToken::TYPE_UInt64: //  /DgnElements/elementid
#ifdef WIP_NOT_YET_SUPPORTED
            eid = DgnElementId (pathParser.GetCurToken().GetUInt64());
            // *** WIP_URI - check that element exists!
#endif
            return DgnElementId();
        
        case DgnResourceURI::UriToken::TYPE_EOS:
            break;  // try query below
        }

    //  -----------------------------  
    //  Query          
    //  -----------------------------  
    DgnResourceURI::QueryParser queryParser (uri.GetQuery());
    DgnResourceURI::UriToken key, value;
    char logical;
    if (queryParser.ParseQueryParameter (key, value, logical) != SUCCESS)
        return DgnElementId();       // no query supplied? I don't have enough of a path to locate an element.

    if (key.GetString().EqualsI ("SourceId"))
        {
#ifdef WIP_NOT_YET_SUPPORTED
        //  DgnElements/Source=oldFileName&DgnElementId=oldElementId

        Utf8String oldFilename = value.GetString();

        DgnResourceURI::UriToken elementidKeyword, oldElementId;
        if (queryParser.ParseQueryParameter (elementidKeyword, oldElementId, logical) != SUCCESS
         || logical != '&'
         || !elementidKeyword.GetString().EqualsI ("DgnElementId")
         || oldElementId.GetType() != DgnResourceURI::UriToken::TYPE_UInt64)
            return ERROR;

        //  This query is one of mine. From here on, I return SUCCESS to indicate that I handled it (even if the provenance can't be found).

        return findElementIdByProvenance (eid, oldFilename.c_str(), oldElementId.GetUInt64(), GetDgnDb().GetDgnProject());
#endif
        return DgnElementId();
        }
    else if (key.GetString().EqualsI ("ECClass"))
        {
        //  DgnElements/ECClass=schema.class&prop=value

        Utf8String ecClassFullName = value.GetString();

        Utf8String ecSchemaName, ecClassName;
        if (pathParser.ParseFullECClassName (ecSchemaName, ecClassName, ecClassFullName.c_str()) != SUCCESS)
            return DgnElementId();

        //  This query is one of mine. From here on, I return SUCCESS to indicate that I handled it (even if the class or instance can't be found).

        ECN::ECClassCP ecClass = GetDgnDb().Schemas().GetECClass(ecSchemaName.c_str(), ecClassName.c_str());
        if (ecClass == nullptr)
            return DgnElementId();

        DgnResourceURI::UriToken propname, propvalue;
        char unused;
        if (queryParser.ParseQueryParameter (propname, propvalue, unused, true) != SUCCESS) // NB: ECProperty values are always encoded as strings, never as integers
            return DgnElementId();

        auto stmt = GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT ECInstanceId FROM %s WHERE(%s=?)", ecClass->GetECSqlName().c_str(), propname.GetString().c_str()).c_str());
        stmt->BindText(1, propvalue.GetString().c_str(), EC::IECSqlBinder::MakeCopy::Yes);
        if (BE_SQLITE_ROW == stmt->Step())
            return stmt->GetValueId<DgnElementId>(0);
        }

    // This domain does not recognize the query parameters. 
    return DgnElementId();
    }
