/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnResourceURI.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnPlatform.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/* Anrdroid resource URI
The format is:

"android.resource://[package]/[res id]"

[package] is your package name

[res id] is value of the resource ID, e.g. R.drawable.sample_1

to stitch it together, use

Uri path = Uri.parse("android.resource://your.package.name/" + R.drawable.sample_1);

source: http://stackoverflow.com/questions/4896223/how-to-get-an-uri-of-an-image-resource-in-android
*/

/* Android content URI

content://user_dictionary/words

source: http://developer.android.com/guide/topics/providers/content-provider-basics.html
*/

/**
 A DgnResourceURI is a URI that identifies one or more elements, ECInstances, or other resources within a DgnFile.

 The documentation below is intended for developers who must implement an element Handler or who need custom URIs.
 Normally, an application will call PersistentElementRef::CreateDgnResourceURI to create a DgnResourceURI on an element
 and DgnFile::GetElementByURI to resolve a URI into a ElementRef.
 
 @see PersistentElementRef::CreateDgnResourceURI for an example. 

 <h4>API Overview</h4>

 If you need to define a new kind of URI or must implement CreateDgnResourceURI for a Handler that cannot use the 
 default behavior, the follow API functions are available:

 - Use Builder to create a DgnResourceURI. That class makes it easy to build a valid URI and takes care of details like encoding.
 - Use PathParser to read the path portion of a DgnResourceURI.
 - Use DgnResourceURI::GetTargetFile to get the optional target file at the root of the path.
 - Use QueryParser to read the query portion of a DgnResourceURI.
 - Use DgnResourceURI::GetFragment to get the fragment portion.

 <h4>URI Backgrounder</h4>
 A URI has the following syntax:
 <pre>
    URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
 
    hier-part   = "//" authority path-abempty
                / path-absolute
                / path-rootless
                / path-empty
</pre>
 Example:
 @verbatim
      foo://example.com:8042/over/there?name=ferret#nose
      \_/   \______________/\_________/ \_________/ \__/
       |           |            |            |        |
    scheme     authority       path        query   fragment
 @endverbatim

 source: http://tools.ietf.org/html/rfc3986

 "authority" is also called "host". 

 <h4>DgnResourceURI as a URI</h4>
 A DgnResourceURI is a URI. It follows URI syntax.

 A DgnResourceURI always identifies resources within a dgndb project. Thus, the path is always a sequence of keywords and identifiers that are interpreted
 by domains, handlers, and apps to locate data within a dgndb project. A DgnResourceURI cannot be used to capture any other kind of URI. 

 A DgnResourceURI never has an authority or host.

 In a DgnResourceURI, the root of the path is always a dgndb project. The path root may be omitted if it is known by context.
 In a DgnResourceURI, the scheme is always "dgnresourceuri". The scheme may be omitted if the URI is known to be a DgnResourceURI. 
 See #SetTargetFile.
 
 <h4>Examples</h4>
 The following are examples of the kind of relative URIs that can be captured by DgnResourceURI.
 <DL>
 <DT>/DgnElements/2342</DT>                               <DD>identifies an element by its ElementId. It has a path and no query.</DD>
 <DT>/DgnElements?ECClass=op:Pump&BusinessKey=P-101</DT>  <DD>identifies an element by the business key of its primary instance. It has a path (DgnElements) and a query (?ECClass=...)</DD>
 <DT>/ECInstances/op:Pump/1234</DT>                       <DD>identifies an ECInstance by its ECInstanceId. It has a path and no query.</DD>
 <DT>/ECInstances/op:Pump?BusinessKey=P-101</DT>          <DD>identifies an ECInstance by its business key. It has a path (ECInstances/op:Pump) and a query (?BusinessKey=...).</DD>
 </DL>
 
 <h4>URI-Encoding</h4>
 Before they can be embedded in a URI, strings must be "encoded" so that they do not contain spaces or reserved characters. 
 Strings extracted from a URI must then be "decoded" again, so that they can be interpreted and used by the application.
 For the most part, DgnResourceURI and its PathParser, QueryParser and Builder classes take care of encoding and decoding strings automatically.
 @bsiclass                                                     
*/
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
    struct Builder;
    struct ParserBase;
    struct PathParser;
    struct QueryParser;

public:
    DGNPLATFORM_EXPORT DgnResourceURI ();

    //! Rarely used function to construct a DgnResourceURI by parsing a URI string. 
    //! @param uriString    An encode URI string.
    //! @return non-zero error status if the URI string is invalid.
    //! @remarks Normally, callers should use DgnResourceURI::Builder to create a DgnResourceURI. That class makes 
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
    DGNPLATFORM_EXPORT void SetTargetFile (DgnProjectR targetFile);

    //! Get the target file identifier, if present.
    DGNPLATFORM_EXPORT BentleyStatus GetTargetFile (Utf8StringR filename, BeGuid& guid) const;

    //! If this URI contains a target file identifier, then this function checks that the GUID in that identifier matches the GUID in file.
    //! @return non-zero error status if this URI has a target file GUID and it does not match the GUID of \a file. If this URI does not have
    //! a target file identifier, then this function returns SUCCESS.
    DGNPLATFORM_EXPORT BentleyStatus CheckTargetFileMatches (DgnProjectR file) const;

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
/** @name Create a URI after building */
/** @{ */
    //! Get the URI as an encoded string
    DGNPLATFORM_EXPORT DgnResourceURI ToDgnResourceURI () const;

    //! Get the URI as an encoded string
    DGNPLATFORM_EXPORT Utf8String ToEncodedString () const;
/** @} */


/** @name Target File */
/** @{ */
    //! Sets the target file. 
    //! The scheme for a DgnResourceURI with an explicit target file is always "dgnresourceuri:".
    //! The target file is part of the path. It is identified by name, qualified by GUID.
    //! Scheme and target file are optional in a DgnResourceURI. A DgnResourceURI can be left as a URI reference with no scheme or file identifier, 
    //! if the holder knows that they it is a DgnResourceURI and not a URI following some other scheme and if the holder knows the target file.
    DGNPLATFORM_EXPORT void SetTargetFile (DgnProjectR targetFile);
/** @} */

/** @name Path */
/** @{ */
    //! Appends an encoded segment to the path
    //! @param newSegment   new segement to add to path. Must already be encoded.
    DGNPLATFORM_EXPORT void AppendEncodedPath (Utf8CP newSegment);

    //! Encodes and appends an segment to the path
    //! @param newSegment   new segement to add to path.
    DGNPLATFORM_EXPORT void AppendPath (Utf8CP newSegment);

    //! Encodes and appends an segment to the path
    //! @param newSegment   new segement to add to path.
    DGNPLATFORM_EXPORT void AppendPath (Utf8StringCR newSegment);

    //! Sets the path to \a newPath
    //! @param newPath    the new path. Must already be encoded.
    DGNPLATFORM_EXPORT void SetEncodedPath (Utf8CP newPath);
/** @} */

/** @name Query */
/** @{ */
    //! Appends a key=value component to the query. Encodes key and value before adding them to the query.
    //! @param key  The key. Will be encoded.
    //! @param value The target value. Will be encoded.
    DGNPLATFORM_EXPORT void AppendQueryParameter (Utf8CP key, Utf8CP value);

    //! Appends a key=value component to the query. Encodes key and value before adding them to the query.
    //! @param key  The key. Will be encoded.
    //! @param value The target value. Will be encoded.
    DGNPLATFORM_EXPORT void AppendQueryParameter (Utf8StringCR key, Utf8StringCR value);

    //! Encodes and then appends a logical operator to the query
    DGNPLATFORM_EXPORT void AppendAndOperator ();

    //! Sets the query to \a newQuery.
    //! @param newQuery    the new query. Must already be encoded.
    DGNPLATFORM_EXPORT void SetEncodedQuery (Utf8CP newQuery);
/** @} */

/** @name Fragment */
/** @{ */
    //! Set the fragment.
    DGNPLATFORM_EXPORT void SetEncodedFragment (Utf8CP newFragment);
/** @} */

/** @name EC Helper Methods */
/** @{ */
    //! Format the full name of an ECClass
    DGNPLATFORM_EXPORT static Utf8String MakeFullECClassName (WStringCR schemaName, WStringCR className);
/** @} */
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
        UInt64          m_uint64;
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
    void SetUInt64 (UInt64 v) {m_type=TYPE_UInt64; m_uint64=v;}

    //! Defines this token as a string. @remarks The UriToken will hold the two pointers supplied. Caller must guarantee that the referenced string is not destroyed while this UriToken exists.
    //! @private
    void SetString (Utf8CP s, Utf8CP e) {m_type=TYPE_String; m_substr.m_start=s; m_substr.m_end=e;}

    //! Gets the type of this token.
    Type GetType() const {return m_type;}

    //! Gets the value of this token as a UInt64. 
    //! @return the value of this token. 
    //! @remarks Returns 0 if the type of this token is not TYPE_UInt64. This function does not attempt to parse an integer from a string.
    UInt64 GetUInt64() const {BeAssert(m_type==TYPE_UInt64); return (m_type==TYPE_UInt64)? m_uint64: 0;}

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
    //! @return the token read.
    //! @see GetCurToken
    DGNPLATFORM_EXPORT UriToken ParseNextToken ();

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
    DGNPLATFORM_EXPORT BentleyStatus ParseQueryParameter (UriToken& propname, UriToken& propvalue, char& logical);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
