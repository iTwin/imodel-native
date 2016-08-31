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

#define RT_DgnDb           "DgnDb"         // DgnDb0601 Code or ECClass + Property
#define RT_DgnElements  "DgnElements"   // Graphite05xx ECClass + Property

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnResourceURI
    {
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
    struct Builder;
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
//! Helper class for building a DgnResourceURI.
//! <h4>API Reference</h4>
//! - #AppendPath adds a component to the path part of the URI.
//! - #AppendQueryParameter adds a name=value pair to the query part of the URI.
//! - #SetEncodedFragment adds a fragment to the URI.
//! - Call #ToDgnResourceURI to create a DgnResourceURI when done.
//! @nosubgrouping
//=======================================================================================
struct DgnResourceURI::Builder
    {
private:
    Utf8String m_schemeAndTargetFile;
    Utf8String m_path;
    Utf8String m_query;
    Utf8String m_fragment;

    void InitQuery ();

public:
    Utf8String ToEncodedString () const;
    void SetTargetFile (DgnDbR targetFile);
    void AppendEncodedPath (Utf8CP newSegment);
    void AppendPath (Utf8CP newSegment);
    void AppendPath (Utf8StringCR newSegment);
    void SetEncodedPath (Utf8CP newPath);
    void AppendQueryParameter (Utf8CP key, Utf8CP value);
    void AppendQueryParameter (Utf8StringCR key, Utf8StringCR value);
    void AppendAndOperator ();
    void SetEncodedQuery (Utf8CP newQuery);
    void SetEncodedFragment (Utf8CP newFragment);
    static Utf8String MakeFullECClassName (Utf8StringCR schemaName, Utf8StringCR className);
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
static BentleyStatus makeElementByProvenance(Utf8StringR uriStr, DgnElementCR el)
    {
#ifdef WIP_ELEMENTURI // *** reinstate file and element provenance tables
    DgnDbR db = el.GetDgnDb();

    Utf8String oldFileName;
    DgnProvenances prov = m_file.GetDgnProject().Provenance();

    ElementProvenance eprov = prov.QueryProvenance(eid);
    if (!eprov.IsValid() || SUCCESS != prov.QueryFileName(oldFileName, eprov.GetOriginalFileId()))
        return ERROR;

    DgnResourceURI::Builder builder;
    builder.AppendEncodedPath(RT_DgnDb);
    builder.AppendQueryParameter("SourceId", oldFileName);
    builder.AppendAndOperator();
    builder.AppendQueryParameter("ElementId", (Utf8CP)Utf8PrintfString("%lld", eprov.GetOriginalElementId()));

    uriStr = builder.ToEncodedString();
    return SUCCESS;
#endif
    return BSIERROR; // *** TBD
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementId findElementIdByProvenance(DgnDbR db, DgnResourceURI::QueryParser& queryParser, Utf8CP oldFileName)
    {
    //  Source=oldFileName&ElementId=oldElementId

    char logical;
    DgnResourceURI::UriToken elementidKeyword, oldElementId;
    if (queryParser.ParseQueryParameter(elementidKeyword, oldElementId, logical) != SUCCESS
        || logical != '&'
        || !elementidKeyword.GetString().EqualsI("ElementId")
        || oldElementId.GetType() != DgnResourceURI::UriToken::TYPE_UInt64)
        {
        BeDataAssert(false && "Invalid Provenance URI");
        return DgnElementId();
        }

    //  This query is one of mine. From here on, I return SUCCESS to indicate that I handled it (even if the provenance can't be found).

#ifdef WIP_ELEMENTURI // *** reinstate file and element provenance tables
    DgnProvenances prov = project.Provenance();

    //  Look up the file
    uint64_t oldFileId;
    if (prov.QueryFileId (oldFileId, oldFileName) != SUCCESS)
        return DgnElementId();

    //  Look up the element
    ElementProvenance eprov (oldFileId, oldElementId);
    eid = prov.GetElement (eprov);
    return eid.IsValid()? SUCCESS: ERROR;
#endif
    BeAssert(false && "TBD");
    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementId queryElementIdByCodeComponents(DgnDbR db, Utf8StringCR encodedCodeValue, DgnResourceURI::QueryParser& queryParser)
    {
    auto codeValue = BeStringUtilities::UriDecode(encodedCodeValue.c_str());

    DgnResourceURI::UriToken authkeyword, encodedauthname;
    char unused;
    if (queryParser.ParseQueryParameter(authkeyword, encodedauthname, unused, true) != SUCCESS)
        {
        BeDataAssert(false && "bad Code URI");
        return DgnElementId();
        }
    auto authorityName = BeStringUtilities::UriDecode(encodedauthname.GetString().c_str());

    DgnResourceURI::UriToken nskeyword, encodedns;
    if (queryParser.ParseQueryParameter(nskeyword, encodedns, unused, true) != SUCCESS)
        {
        BeDataAssert(false && "bad Code URI");
        return DgnElementId();
        }
    auto ns = BeStringUtilities::UriDecode(encodedns.GetString().c_str());

    auto authorityId = db.Authorities().QueryAuthorityId(authorityName.c_str());
    if (!authorityId.IsValid())
        return DgnElementId();

    CachedStatementPtr codestmt = db.GetCachedStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE(Code_AuthorityId=? AND Code_Namespace=? AND Code_Value=?)");
    codestmt->BindId(1, authorityId);
    codestmt->BindText(2, ns.c_str(), Statement::MakeCopy::No);
    codestmt->BindText(3, codeValue.c_str(), Statement::MakeCopy::No);
    if (BE_SQLITE_ROW == codestmt->Step())
        return codestmt->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementId queryElementIdByClassAndProperty(DgnDbR db, DgnResourceURI::QueryParser& queryParser, DgnResourceURI::PathParser& pathParser, Utf8StringCR ecClassFullName, bool fallBackOnCode)
    {
    // 
    //  Db/ECClass=schema.class&prop=value

    Utf8String ecSchemaName, ecClassName;
    if (pathParser.ParseFullECClassName(ecSchemaName, ecClassName, ecClassFullName.c_str()) != SUCCESS)
        {
        BeAssert(false && "invalid DgnElements/ECClass URI");
        return DgnElementId();
        }

    ECN::ECClassCP ecClass = db.Schemas().GetECClass(ecSchemaName.c_str(), ecClassName.c_str());
    if (ecClass == nullptr)
        return DgnElementId();

    DgnResourceURI::UriToken propname, propvalue;
    char unused;
    if (queryParser.ParseQueryParameter(propname, propvalue, unused, true) != SUCCESS) // NB: ECProperty values are always encoded as strings, never as integers
        {
        BeAssert(false && "invalid DgnElements/ECClass URI");
        return DgnElementId();
        }

    auto stmt = db.GetPreparedECSqlStatement(Utf8PrintfString("SELECT ECInstanceId FROM %s WHERE(%s=?)", ecClass->GetECSqlName().c_str(), propname.GetString().c_str()).c_str());
    stmt->BindText(1, propvalue.GetString().c_str(), EC::IECSqlBinder::MakeCopy::No);
    if (BE_SQLITE_ROW == stmt->Step())
        return stmt->GetValueId<DgnElementId>(0);

    if (fallBackOnCode)
        {
        // Maybe the property was identified in the V8 schema as the BusinessKey and that the converter converted it into the element's Code. Try that as a fallback.
        // Unfortunately, we have no way of knowing what namespace value to use, since that depends on the value of the "guest" command-line parameter that was 
        // passed to the converter, which we cannot detect now.
        CachedStatementPtr codestmt = db.GetCachedStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE(Code_Value=?)");
        codestmt->BindText(1, propvalue.GetString().c_str(), Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == codestmt->Step())
            return codestmt->GetValueId<DgnElementId>(0);
        }

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementId queryElementIdFromDgnDbURI(DgnDbR db, DgnResourceURI& uri, DgnResourceURI::PathParser& pathParser)
    {
    switch (pathParser.ParseNextToken().GetType())
        {
        default: //  Unsupported path 
            {
            BeAssert(false && "bad DgnDb URI - unrecognized resource type");
            return DgnElementId();
            }

        case DgnResourceURI::UriToken::TYPE_UInt64: //  /DgnElements/elementid
            {
            return DgnElementId(pathParser.GetCurToken().GetUInt64());
            }

        case DgnResourceURI::UriToken::TYPE_EOS:
            break;  // It's a query. Handled below.
        }

    DgnResourceURI::QueryParser queryParser(uri.GetQuery());
    DgnResourceURI::UriToken key, value;
    char logical;
    if (queryParser.ParseQueryParameter(key, value, logical) != SUCCESS)
        {
        BeAssert(false && "invalid Db URI");
        return DgnElementId();
        }

    if (key.GetString().EqualsI("Code")) //  Db/Code=authority/namespace/
        return queryElementIdByCodeComponents(db, value.GetString(), queryParser);
    
    if (key.GetString().EqualsI("SourceId"))
        return findElementIdByProvenance(db, queryParser, value.GetString().c_str());

    if (key.GetString().EqualsI("ECClass"))
        return queryElementIdByClassAndProperty(db, queryParser, pathParser, value.GetString(), false);

    // This domain does not recognize the query parameters. 
    BeAssert(false && "Unrecognized URI type");
    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnElementId queryElementIdFromGraphiteURI(DgnDbR db, DgnResourceURI& uri, DgnResourceURI::PathParser& pathParser)
    {
    switch (pathParser.ParseNextToken().GetType())
        {
        default: //  Unsupported path 
            {
            BeAssert(false && "bad DgnElements URI - unrecognized resource type");
            return DgnElementId();
            }
             
        case DgnResourceURI::UriToken::TYPE_UInt64: //  /DgnElements/elementid
            {
            //return DgnElementId(pathParser.GetCurToken().GetUInt64());
            BeDataAssert(false && "We can't resolve raw DgnDb ElementIds from old files");
            return DgnElementId();
            }

        case DgnResourceURI::UriToken::TYPE_EOS:
            break;  // It's a query. Handled below.
        }

    DgnResourceURI::QueryParser queryParser(uri.GetQuery());
    DgnResourceURI::UriToken key, value;
    char logical;
    if (queryParser.ParseQueryParameter(key, value, logical) != SUCCESS)
        {
        BeDataAssert(false && "invalid Graphite URI");
        return DgnElementId();
        }

    if (key.GetString().EqualsI ("SourceId"))
        return findElementIdByProvenance(db, queryParser, value.GetString().c_str());
    
    if (key.GetString().EqualsI ("ECClass"))
        return queryElementIdByClassAndProperty(db, queryParser, pathParser, value.GetString(), true);

    // This domain does not recognize the query parameters. 
    BeAssert(false && "Unrecognized URI type");
    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdByURI(Utf8CP uriStr) const
    {
    DgnResourceURI uri;
    if (DgnResourceURI::PARSE_STATUS_SUCCESS != uri.FromEncodedString(uriStr))
        return DgnElementId();

    DgnResourceURI::PathParser pathParser (uri.GetPath());

    Utf8String resourceType = pathParser.ParseNextToken().GetString();

    if (resourceType == RT_DgnDb)
        return queryElementIdFromDgnDbURI(GetDgnDb(), uri, pathParser);

    if (resourceType == RT_DgnElements)
        return queryElementIdFromGraphiteURI(GetDgnDb(), uri, pathParser);
    
    BeAssert(false && "Unsupported resource  type");
    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::InitQuery ()
    {
    if (m_query.empty())
        m_query.append ("?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::Builder::ToEncodedString () const
    {
    return m_schemeAndTargetFile + m_path + m_query + m_fragment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnResourceURI::Builder::MakeFullECClassName (Utf8StringCR schemaName, Utf8StringCR className)
    {
    Utf8String name (schemaName);
    name.append(":");
    name.append (className);
    return name;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetTargetFile (DgnDbR targetFile)
    {
    m_schemeAndTargetFile = SCHEME_DgnResourceURI ":/";
    m_schemeAndTargetFile.append (getEncodedTargetFile (targetFile));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendEncodedPath (Utf8CP newSegment)
    {
    if (*newSegment != '/')
        m_path.append ("/");
    m_path.append (newSegment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendPath (Utf8CP newSegment)
    {
    AppendEncodedPath (BeStringUtilities::UriEncode (newSegment).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendPath (Utf8StringCR newSegment)
    {
    AppendPath (newSegment.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetEncodedPath (Utf8CP newPath)
    {
    m_path = newPath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendQueryParameter (Utf8CP key, Utf8CP value)
    {
    InitQuery();
    BeAssert (*key != '?');
    m_query.append(BeStringUtilities::UriEncode(key)).append("=").append(BeStringUtilities::UriEncode(value));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendQueryParameter (Utf8StringCR key, Utf8StringCR value)
    {
    AppendQueryParameter (key.c_str(), value.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::AppendAndOperator ()
    {
    BeAssert (!m_query.empty());
    m_query.append ("&");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetEncodedQuery (Utf8CP newQuery)
    {
    if (*newQuery == '?')
        m_query = newQuery;
    else
        {
        InitQuery();
        m_query.append (newQuery);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnResourceURI::Builder::SetEncodedFragment (Utf8CP newFragment)
    {
    m_fragment = newFragment;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus makeElementByCode(Utf8StringR uriStr, DgnElementCR el)
    {
    DgnCode code = el.GetCode();
    if (!code.IsValid() || code.GetValue().empty())
        return BSIERROR;

    auto auth = el.GetCodeAuthority();
    if (!auth.IsValid())
        return BSIERROR;

    DgnResourceURI::Builder builder;
    builder.AppendEncodedPath(RT_DgnDb);
    builder.AppendQueryParameter("Code", BeStringUtilities::UriEncode(code.GetValueCP()));
    builder.AppendAndOperator();
    builder.AppendQueryParameter("A", BeStringUtilities::UriEncode(auth->GetName().c_str()).c_str());
    builder.AppendAndOperator();
    builder.AppendQueryParameter("N", BeStringUtilities::UriEncode(code.GetNamespace().c_str()).c_str());

    uriStr = builder.ToEncodedString();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getEcKeyPropertyName (ECN::ECClassCR ecClass, Utf8CP customAttributeName)
    {
    ECN::IECInstancePtr ca = ecClass.GetCustomAttribute (customAttributeName);
    if (!ca.IsValid())
        return "";
    ECN::ECValue v;
    ca->GetValue (v, "PropertyName");
    if (v.IsNull())
        {
        ca->GetValue (v, "Property");      // CustomAttributes are inconsistent about this
        if (v.IsNull())
            return "";
        }
    return v.ToString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String getPropertyAsString (DgnElementCR el, Utf8StringCR propName)
    {
    if (propName.length() == 0)
        return "";

    auto stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT %s FROM %s WHERE(ECInstanceId=?)", 
                                                                         propName.c_str(), 
                                                                         el.GetElementClass()->GetECSqlName().c_str()).c_str());
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return "";
    return stmt->GetValueText(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isValidUniqueKey(DgnDbR db, ECN::ECClassCR ecClass, Utf8StringCR keyname, Utf8StringCR keyvalue)
    {
    if (0 == keyname.length() || 0 == keyvalue.length())
        return false;

    auto stmt = db.GetPreparedECSqlStatement(Utf8PrintfString("SELECT ECInstanceId FROM %s WHERE([%s]=?)", 
                                                              ecClass.GetECSqlName().c_str(), 
                                                              keyname.c_str()).c_str());
    stmt->BindText(1, keyvalue.c_str(), EC::IECSqlBinder::MakeCopy::No);

    if (BE_SQLITE_ROW != stmt->Step())
        return false; // cannot find element by this property

    if (BE_SQLITE_ROW == stmt->Step())
        return false; // property is not unique

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus makeElementByWellKnownECProperty(Utf8StringR uriStr, DgnElementCR el)
    {
    DgnDbR db = el.GetDgnDb();
    auto ecClass = el.GetElementClass();

    Utf8String keyName = getEcKeyPropertyName(*ecClass, "SyncIDSpecification"); // prefer syncid
    Utf8String keyValue = getPropertyAsString(el, keyName);
    if (!isValidUniqueKey(db, *ecClass, keyName, keyValue))
        {
        keyName = getEcKeyPropertyName(*ecClass, "GlobalIdSpecification");      // second, try globalid
        keyValue = getPropertyAsString(el, keyName);
        if (!isValidUniqueKey(db, *ecClass, keyName, keyValue))
            {
            keyName = getEcKeyPropertyName(*ecClass, "BusinessKeySpecification"); // last resort, use buskey
            keyValue = getPropertyAsString(el, keyName);
            if (!isValidUniqueKey(db, *ecClass, keyName, keyValue))
                {
                return BSIERROR;
                }
            }
        }

    DgnResourceURI::Builder builder;
    builder.AppendEncodedPath (RT_DgnDb);
    Utf8String className = DgnResourceURI::Builder::MakeFullECClassName(ecClass->GetSchema().GetName(), ecClass->GetName());
    builder.AppendQueryParameter ("ECClass", className);
    builder.AppendAndOperator();
    builder.AppendQueryParameter (keyName, keyValue);
    uriStr = builder.ToEncodedString();
    return BSISUCCESS;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus makeElementById (Utf8StringR uriStr, DgnElementId eid)
    {
    DgnResourceURI::Builder builder;
    builder.AppendEncodedPath (RT_DgnDb);
    builder.AppendEncodedPath (Utf8PrintfString("%lld", eid.GetValue()).c_str());
    uriStr = builder.ToEncodedString();
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnElements::CreateElementUri (Utf8StringR uriStr, DgnElementCR el, bool fallBackOnV8Id, bool fallBackOnDgnDbId) const
    {
    // If the element has a primary ECInstance and if that instance has a business key or a guid, we normally prefer to base the URI on the instance
    if (makeElementByCode(uriStr, el) == BSISUCCESS)
        return BSISUCCESS;

    if (makeElementByWellKnownECProperty(uriStr, el) == BSISUCCESS)
        return BSISUCCESS;

    if (fallBackOnV8Id)
        {
        if (makeElementByProvenance(uriStr, el) == BSISUCCESS)
            return BSISUCCESS;
        }

    // If all else fails, we normally default to basing the URI on the current id
    if (fallBackOnDgnDbId)
        {
        makeElementById (uriStr, el.GetElementId());
        return SUCCESS;
        }

    //  I don't understand or can't impelement the strategy that the caller wants to use.
    return ERROR;
    }
