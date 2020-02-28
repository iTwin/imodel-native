/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <BeXml/BeXml.h>
#include <libxml/xmlsave.h>
#include <Bentley/WString.h>
#include <Bentley/BeStringUtilities.h>
#include <libxml/xpathInternals.h>
#include <Bentley/CatchNonPortable.h>
#include <Bentley/BeAssert.h>
#include <Bentley/BeThread.h>

PUSH_DISABLE_DEPRECATION_WARNINGS

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Daryl.Holmwood  04/2014
//--------------+------------------------------------------------------------------------
static int CodePageToUTF8 (unsigned char *out, int *outlen,
                        const unsigned char *in, int *inlen, LangCodePage codePage)
    {
    WString unicode;
    BeStringUtilities::LocaleCharToWChar (unicode, (CharCP)in, codePage, *inlen);
    Utf8String outstring;
    BeStringUtilities::WCharToUtf8 (outstring, unicode.GetWCharCP ());

    if (*outlen < (int)outstring.size ())
        return -1;

    *outlen = (int)outstring.size ();
    memcpy (out, outstring.c_str (), outstring.size ());
    return *outlen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Daryl.Holmwood  04/2014
//--------------+------------------------------------------------------------------------
static int UTF8ToCodePage (unsigned char *out, int *outlen,
                        const unsigned char *in, int *inlen, LangCodePage codePage)
    {
    WString unicode;
    BeStringUtilities::Utf8ToWChar (unicode, (Utf8CP)in, *inlen);
    AString outstring;
    BeStringUtilities::WCharToLocaleChar (outstring, codePage, unicode.GetWCharCP());

    if (*outlen < (int)outstring.size ())
        return -1;

    *outlen = (int)outstring.size ();
    memcpy (out, outstring.c_str (), outstring.size ());
    return *outlen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Daryl.Holmwood  04/2014
//--------------+------------------------------------------------------------------------
static int Windows1252ToUTF8 (unsigned char *out, int *outlen, const unsigned char *in, int *inlen)
    {
    return CodePageToUTF8 (out, outlen, in, inlen, (LangCodePage)1252);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Daryl.Holmwood  04/2014
//--------------+------------------------------------------------------------------------
static int UTF8ToWindows1252 (unsigned char *out, int *outlen, const unsigned char *in, int *inlen)
    {
    return UTF8ToCodePage (out, outlen, in, inlen, (LangCodePage)1252);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
static int transcodeString(unsigned char* out, int* outlen, CharCP outEncoding, const unsigned char* in, int* inlen, CharCP inEncoding)
    {
    if ((nullptr == out) || (nullptr == outlen) || (0 == *outlen) || (nullptr == in) || (nullptr == inlen) || (0 == *inlen))
        return -1;

    bvector<Byte> outBuff;
    if (SUCCESS != BeStringUtilities::TranscodeStringDirect(outBuff, outEncoding, in, (size_t)*inlen, inEncoding))
        return -2;

    size_t bytesToCopy = outBuff.size();
    if (*outlen < (int)bytesToCopy)
        return -1;

    memcpy(out, &outBuff[0], bytesToCopy);

    // inlen should be set to number of bytes read; assume we read them all and leave as-is
    *outlen = (int)bytesToCopy;

    return *outlen;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
static int utf8ToUtf32LE(unsigned char* out, int* outlen, const unsigned char* in, int* inlen) { return transcodeString(out, outlen, "UTF-32LE", in, inlen, "UTF-8"); }
static int utf32LEToUtf8(unsigned char* out, int* outlen, const unsigned char* in, int* inlen) { return transcodeString(out, outlen, "UTF-8", in, inlen, "UTF-32LE"); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     08/2015
//---------------------------------------------------------------------------------------
static void initializeExtraEncodingHandlers()
    {
    static bool s_hasRun = false;
    if (s_hasRun)
        return;

    s_hasRun = true;

    xmlNewCharEncodingHandler ("windows-1252", &Windows1252ToUTF8, &UTF8ToWindows1252);
    xmlNewCharEncodingHandler("UTF-32LE", &utf32LEToUtf8, &utf8ToUtf32LE);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- BeXmlDom ------------------------------------------------------------------------------------------------------- BeXmlDom --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    01/2012
//--------------+------------------------------------------------------------------------
static unsigned long BeWcsToul(const wchar_t *nptr, wchar_t **endptr, int base)
    {
#if defined (ANDROID)
    // ANDROID_NONPORT_WIP - kludge while deciding on appropriate approach for wcstoul
    AString aStr;
    BeStringUtilities::WCharToLocaleChar (aStr, LangCodePage::None, nptr);

    return strtoul (aStr.c_str(), NULL, base);
#else
    return wcstoul (nptr, endptr, base);
#endif
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
xmlDoc& BeXmlDom::GetDocument ()
    {
    return *m_doc;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
static BeMutex s_xmlErrorResetMutex;
BeXmlDom::BeXmlDom (ParseOptions options) : m_doc (NULL), m_options (options)
    {
    BeMutexHolder lock(s_xmlErrorResetMutex);
    // reset the global error.
    xmlResetLastError();

    // clear our internal error.
    memset (&m_error, 0, sizeof(m_error));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlDom::~BeXmlDom ()
    {
    for (xmlXPathObjectPtr const& xPathObj: m_xpathObjects)
        xmlXPathFreeObject (xPathObj);

    for (xmlXPathContextPtr const& xPathContext: m_xpathContexts)
        xmlXPathFreeContext (xPathContext);

    // clear internals of the error we saved.
    xmlResetError (&m_error);

    if (NULL != m_doc)
        xmlFreeDoc (m_doc);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
static int getDefaultXmlParseOptions ()
    {
    int options = 0;

    // we used to set up XML_PARSE_NOERROR and XML_PARSE_NOWARNING here, but that was not a good idea.
    // Our error function - beXmlErrorFunction - routes the error message appropriately.

    // Should we consider setting XML_PARSE_RECOVER?

    return options;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void XMLCDECL beXmlErrorFunction (void* userArg, xmlErrorPtr xmlParseError)
    {
    BeXmlDomP   beXmlDom = (BeXmlDomP) userArg;

    if (NULL == xmlParseError)
        return;

    // we (mis?)use the schema name as the XML namespace, and that's never relative. We don't want to hear about it.
    if (XML_WAR_NS_URI_RELATIVE == xmlParseError->code)
        return;

    if ( (XML_ERR_ATTRIBUTE_REDEFINED == xmlParseError->code) && (0 != (beXmlDom->GetOptions() & BeXmlDom::XMLPARSE_OPTION_FixDuplicateAttributes)))
        {
        xmlParserCtxtPtr parserContext;
        if (NULL != (parserContext = (xmlParserCtxtPtr) xmlParseError->ctxt))
            parserContext->recovery = 1;
        }

    if (XML_ERR_WARNING == xmlParseError->level)
        return;

    // The default is not to assert
    if (0 != (beXmlDom->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
        BeAssert (NULL != xmlParseError);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlDomPtr BeXmlDom::CreateAndReadFromFile (BeXmlStatus& xmlStatus, WCharCP filePath, WStringP errorMsg, ParseOptions options)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    BeXmlDomP wrapper = new BeXmlDom (options);

    if (WString::IsNullOrEmpty (filePath))
        {
        BeAssert (false);
        xmlStatus = BEXML_FileNotFound;
        return wrapper;
        }

    // this prevents libxml from overwriting our error handler.
    xmlInitParser();
    xmlSetStructuredErrorFunc (wrapper, beXmlErrorFunction);

    Utf8String filePathUtf8;
    BeStringUtilities::WCharToUtf8 (filePathUtf8, filePath);

    wrapper->m_doc = xmlReadFile (filePathUtf8.c_str (), NULL, getDefaultXmlParseOptions ());
    if (NULL == wrapper->m_doc)
        {
        // Can we tell more than this?
        xmlStatus = BEXML_ParseError;
        xmlCopyError (xmlGetLastError(), &wrapper->m_error);
        FormatErrorMessage (errorMsg, &wrapper->m_error);
        }
    else
        {
        // we put a pointer to our BeXmlDom object in the "_private" data in the libxml xmlDoc, so we can find it from each BeXmlNode.
        wrapper->m_doc->_private = wrapper;
        }

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                            Jean-Francois.Cote     08/2015
//---------------------------------------------------------------------------------------
BeXmlDomPtr BeXmlDom::CreateAndReadFromFile (BeXmlStatus& xmlStatus, Utf8CP filePath, WStringP errorMsg, ParseOptions options)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers();
    BeXmlDomP wrapper = new BeXmlDom(options);

    if (Utf8String::IsNullOrEmpty(filePath))
        {
        BeAssert(false);
        xmlStatus = BEXML_FileNotFound;
        return wrapper;
        }

    // this prevents libxml from overwriting our error handler.
    xmlInitParser();
    xmlSetStructuredErrorFunc(wrapper, beXmlErrorFunction);

    wrapper->m_doc = xmlReadFile(filePath, NULL, getDefaultXmlParseOptions());
    if (NULL == wrapper->m_doc)
        {
        // Can we tell more than this?
        xmlStatus = BEXML_ParseError;
        xmlCopyError(xmlGetLastError(), &wrapper->m_error);
        FormatErrorMessage(errorMsg, &wrapper->m_error);
        }
    else
        {
        // we put a pointer to our BeXmlDom object in the "_private" data in the libxml xmlDoc, so we can find it from each BeXmlNode.
        wrapper->m_doc->_private = wrapper;
        }

    return wrapper;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlDomPtr BeXmlDom::CreateAndReadFromMemory (BeXmlStatus& xmlStatus, void const* xmlBuffer, size_t bufferSize, WStringP errorMsg, ParseOptions options)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    BeXmlDomP wrapper = new BeXmlDom (options);

    // this prevents libxml from overwriting our error handler.
    xmlInitParser();
    xmlSetStructuredErrorFunc (wrapper, beXmlErrorFunction);

    // libxml will automatically attempt to detect encoding using Byte-Order-Mark(BOM) or <?xml ...encoding="..."?>
    wrapper->m_doc = xmlReadMemory ((CharCP)xmlBuffer, (int)bufferSize, NULL, NULL, getDefaultXmlParseOptions ());
    if (NULL == wrapper->m_doc)
        {
        // Can we tell more than this?
        xmlStatus = BEXML_ParseError;
        xmlCopyError (xmlGetLastError(), &wrapper->m_error);
        FormatErrorMessage (errorMsg, &wrapper->m_error);
        }
    else
        {
        // we put a pointer to our BeXmlDom object in the "_private" data in the libxml xmlDoc, so we can find it from each BeXmlNode.
        wrapper->m_doc->_private = wrapper;
        }

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
BeXmlDomPtr BeXmlDom::CreateAndReadFromString (BeXmlStatus& xmlStatus, CharCP source, size_t characterCount, CharCP encoding, WStringP errorMsg, ParseOptions options)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    BeXmlDomP wrapper = new BeXmlDom (options);

    // this prevents libxml from overwriting our error handler.
    xmlInitParser();
    xmlSetStructuredErrorFunc (wrapper, beXmlErrorFunction);

    if (0 == characterCount)
        characterCount = strlen (source);

    // when passing bufferSize to xmlReadMemory, include the trailing 0.
    wrapper->m_doc = xmlReadMemory (source, (int)(characterCount + 1), NULL, encoding, getDefaultXmlParseOptions ());
    if (NULL == wrapper->m_doc)
        {
        // Can we tell more than this?
        xmlStatus = BEXML_ParseError;
        xmlCopyError (xmlGetLastError(), &wrapper->m_error);
        FormatErrorMessage (errorMsg, &wrapper->m_error);
        }
    else
        {
        // we put a pointer to our BeXmlDom object in the "_private" data in the libxml xmlDoc, so we can find it from each BeXmlNode.
        wrapper->m_doc->_private = wrapper;
        }

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
BeXmlDomPtr BeXmlDom::CreateAndReadFromString (BeXmlStatus& xmlStatus, Utf8CP source, size_t characterCount, WStringP errorMsg, ParseOptions options)
    {
    return BeXmlDom::CreateAndReadFromString (xmlStatus, source, characterCount, "UTF-8", errorMsg, options);
    }

#if !defined (NDEBUG) && defined (_WIN32)
static bool charCountCloseEnough (size_t characterCount, WCharCP source)
    {
    // its ok to include the null terminator.
    size_t sizeDifference = characterCount - wcslen ((WCharCP) source);
    return ((sizeDifference == 0) || (sizeDifference == 1));
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlDomPtr BeXmlDom::CreateAndReadFromString (BeXmlStatus& xmlStatus, Utf16CP source, size_t characterCount, WStringP errorMsg, ParseOptions options)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    BeXmlDomP wrapper = new BeXmlDom (options);

    // this prevents libxml from overwriting our error handler.
    xmlInitParser();
    xmlSetStructuredErrorFunc (wrapper, beXmlErrorFunction);

    if (NULL == source)
        {
        xmlStatus = BEXML_ParseError;
        return wrapper;
        }

    if (0 == characterCount)
        characterCount = BeStringUtilities::Utf16Len(source);

    // this is here because the semantics of this method was changed from taking "bufferSize" to "characterCount", and I want to make sure all the old callers get an Assert.
    // It's ok if characterCount includes the zero terminator.
#if defined (_WIN32)
    // This uses wcslen, which is invalid on UTF-16 strings on non-Win32.
    BeAssert (charCountCloseEnough (characterCount, (WCharCP) source));
#endif

    size_t numCharsToCopy = ((0 == characterCount) || (source[characterCount - 1])) ? characterCount + 1 : characterCount; // TFS 15586

    // when passing bufferSize to xmlReadMemory, include the null terminator, account for char size.
    wrapper->m_doc = xmlReadMemory ((CharCP)source, (int) (sizeof (Utf16Char) * numCharsToCopy), NULL, "UTF-16LE", getDefaultXmlParseOptions ());
    if (NULL == wrapper->m_doc)
        {
        // Can we tell more than this?
        xmlStatus = BEXML_ParseError;
        xmlCopyError (xmlGetLastError(), &wrapper->m_error);
        FormatErrorMessage (errorMsg, &wrapper->m_error);
        }
    else
        {
        // we put a pointer to our BeXmlDom object in the "_private" data in the libxml xmlDoc, so we can find it from each BeXmlNode.
        wrapper->m_doc->_private = wrapper;
        }

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlDomPtr BeXmlDom::CreateAndReadFromString (BeXmlStatus& xmlStatus, WCharCP source, size_t characterCount, WStringP errorMsg, ParseOptions options)
    {
#if defined (_WIN32)
    return CreateAndReadFromString (xmlStatus, (Utf16CP)source, characterCount, errorMsg, options);
#else
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    BeXmlDomP wrapper = new BeXmlDom (options);

    // this prevents libxml from overwriting our error handler.
    xmlInitParser();
    xmlSetStructuredErrorFunc (wrapper, beXmlErrorFunction);

    // libXml no longer supports UTF-32, so convert to UTF-8.
    Utf8String utf8Source;
    BeStringUtilities::WCharToUtf8(utf8Source, source, characterCount);
    wrapper->m_doc = xmlReadMemory ((CharCP)utf8Source.c_str(), (int) (sizeof (Utf8Char) * (utf8Source.size() + 1)), NULL, "UTF-8", getDefaultXmlParseOptions ());

    if (NULL == wrapper->m_doc)
        {
        xmlStatus = BEXML_ParseError;
        xmlCopyError (xmlGetLastError(), &wrapper->m_error);
        FormatErrorMessage (errorMsg, &wrapper->m_error);
        }
    else
        {
        // we put a pointer to our BeXmlDom object in the "_private" data in the libxml xmlDoc, so we can find it from each BeXmlNode.
        wrapper->m_doc->_private = wrapper;
        }

    return wrapper;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
void    BeXmlDom::GetLastErrorString (WStringR errorString)
    {
    errorString.clear();
    FormatErrorMessage (&errorString, xmlGetLastError());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void    BeXmlDom::FormatErrorMessage (WStringP errorMsg, xmlErrorPtr xmlError)
    {
    if ( (NULL == errorMsg) || (NULL == xmlError) )
        return;

    errorMsg->clear();

    WString xmlErrorMessage (xmlError->message, BentleyCharEncoding::Utf8);
    if (NULL != xmlError->file)
        {
        WString xmlErrorFile (xmlError->file, BentleyCharEncoding::Utf8);
        if ( (0 != xmlError->code) && (XML_FROM_PARSER == xmlError->domain) )
            errorMsg->Sprintf (L"XML Parsing error code %d at line %d, column %d, parsing file '%ls'. %ls", xmlError->code, xmlError->line, xmlError->int2, xmlErrorFile.c_str(), xmlErrorMessage.c_str());
        else
            errorMsg->Sprintf (L"XML error code %d at line %d, column %d, file '%ls'. %ls", xmlError->code, xmlError->line, xmlError->int2, xmlErrorFile.c_str(), xmlErrorMessage.c_str());
        }
    else
        {
        if ( (0 != xmlError->code) && (XML_FROM_PARSER == xmlError->domain) )
            errorMsg->Sprintf (L"XML Parsing error code %d, position %d, parsing string : %ls", xmlError->code, xmlError->int2, xmlErrorMessage.c_str());
        else
            errorMsg->Sprintf (L"XML error code %d, position %d, %ls", xmlError->code, xmlError->int2, xmlErrorMessage.c_str());
        }

    if (NULL != xmlError->str1)
        {
        errorMsg->append (L"    \n    ");
        errorMsg->AppendA (xmlError->str1);
        }

    if (NULL != xmlError->str2)
        {
        errorMsg->append (L"    \n    ");
        errorMsg->AppendA (xmlError->str2);
        }
    if (NULL != xmlError->str3)
        {
        errorMsg->append (L"    \n    ");
        errorMsg->AppendA (xmlError->str3);
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlDomPtr BeXmlDom::CreateEmpty ()
    {
    initializeExtraEncodingHandlers ();
    BeXmlDomP wrapper = new BeXmlDom (XMLPARSE_OPTION_None);
    wrapper->m_doc = xmlNewDoc ((xmlChar const*)"1.0");

    // we put a pointer to our BeXmlDom object in the "_private" data in the libxml xmlDoc, so we can find it from each BeXmlNode.
    wrapper->m_doc->_private = wrapper;

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   09/2011
//---------------------------------------------------------------------------------------
void        BeXmlDom::RegisterNamespace (Utf8CP prefix, Utf8CP xmlNamespace)
    {
    if ( (NULL == prefix) || (NULL == xmlNamespace) || (0 == *prefix) || (0 == *xmlNamespace) )
        {
        if (0 != (m_options & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);

        return;
        }
    m_namespaces[prefix] = xmlNamespace;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
xmlXPathContextPtr BeXmlDom::AcquireXPathContext (BeXmlNodeP root)
    {
    xmlXPathContextPtr context = xmlXPathNewContext (m_doc);

    if (NULL == context)
        {
        BeAssert (NULL != context);
        return NULL;
        }

    if (NULL != root)
        context->node = root;

    // we have to add our namespace map to the XPath Context.
    for (auto iNS = m_namespaces.begin(); iNS != m_namespaces.end(); ++iNS)
        {
        xmlXPathRegisterNs (context, (xmlChar const*) iNS->first.c_str(), (xmlChar const*) iNS->second.c_str());
        }

    m_xpathContexts.push_back (context);

    return context;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::FreeXPathContext (xmlXPathContext& context)
    {
    bvector<xmlXPathContextPtr>::iterator foundContext = std::find (m_xpathContexts.begin (), m_xpathContexts.end (), &context);

    if (m_xpathContexts.end () == foundContext)
        {
        if (0 != (m_options & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false && L"Asked to free an xmlXPathContextPtr that we don't own; not freeing.");

        return BEXML_UnownedPath;
        }

    xmlXPathFreeContext (*foundContext);
    m_xpathContexts.erase (foundContext);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
xmlXPathObjectPtr BeXmlDom::EvaluateXPathExpression (Utf8CP expression, xmlXPathContextPtr context)
    {
    if ( (NULL == expression) || (0 == *expression) )
        {
        if (0 != (m_options & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);

        return NULL;
        }

    if (NULL == context)
        {
        if (m_xpathContexts.empty ())
            this->AcquireXPathContext (NULL);

        context = m_xpathContexts.back ();
        }

    xmlXPathObjectPtr xPathObj = xmlXPathEvalExpression ((xmlChar const*)expression, context);

    if (NULL == xPathObj)
        {
        if (0 != (m_options & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);

        return NULL;
        }

    m_xpathObjects.push_back (xPathObj);

    return xPathObj;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::FreeXPathObject (xmlXPathObject& xPathObj)
    {
    bvector<xmlXPathObjectPtr>::iterator foundObj = std::find (m_xpathObjects.begin (), m_xpathObjects.end (), &xPathObj);

    if (m_xpathObjects.end () == foundObj)
        {
        if (0 != (m_options & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert ((false && L"Asked to free an xmlXPathObjectPtr that we don't own; not freeing."));
        return BEXML_UnownedPath;
        }


    xmlXPathFreeObject (*foundObj);
    m_xpathObjects.erase (foundObj);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP  BeXmlDom::GetRootElement ()
    {
    return static_cast <BeXmlNodeP> (xmlDocGetRootElement (m_doc));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP BeXmlDom::AddNewElement (Utf8CP elementName, WCharCP _content, BeXmlNodeP parent)
    {
    if ( (NULL == elementName) || (0 == *elementName) )
        return NULL;

    Utf8String content;
    if (!WString::IsNullOrEmpty (_content))
        BeStringUtilities::WCharToUtf8 (content, _content);

    xmlNodePtr newNode = xmlNewDocRawNode (m_doc, NULL, (xmlChar const*)elementName, (content.empty () ? NULL : (xmlChar const*)content.c_str ()));
    if (NULL == newNode)
        return NULL;

    if (NULL != parent)
        xmlAddChild (parent, newNode);
    else
        xmlDocSetRootElement (m_doc, newNode);

    return static_cast <BeXmlNodeP> (newNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
static xmlSaveOption    SetSaveOptions (BeXmlDom::ToStringOption options)
    {
    int currentOptions = 0;

    if (0 != (options & BeXmlDom::TO_STRING_OPTION_OmitXmlDeclaration))
        currentOptions |= XML_SAVE_NO_DECL;

    if (0 != (options & BeXmlDom::TO_STRING_OPTION_Formatted))
        currentOptions |= XML_SAVE_FORMAT;

    // if directed, indent 2 spaces at each level.
    if (0 != (options & BeXmlDom::TO_STRING_OPTION_Indent))
        {
        xmlIndentTreeOutput = 1;
        xmlTreeIndentString = "  ";
        }
    else
        {
        xmlIndentTreeOutput = 0;
        xmlTreeIndentString = NULL;
        }

    return (xmlSaveOption) currentOptions;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlDom::ToString (Utf16BufferR buffer, ToStringOption options) const
    {
    buffer.resize (1);
    buffer.back () = 0;

    xmlSaveOption   libxmlOptions = SetSaveOptions (options);

    xmlBufferPtr    xmlBuffer   = xmlBufferCreate ();
    xmlSaveCtxtPtr  saveContext = xmlSaveToBuffer (xmlBuffer, "UTF-16LE", libxmlOptions);

    xmlSaveDoc (saveContext, m_doc);

    xmlSaveClose (saveContext);

    xmlChar const*  effectiveContent    = xmlBuffer->content;
    size_t          effectiveBufferSize = xmlBuffer->use;

    // the last character in an XML string produced by libxml is always the 0x0a character, which we don't want.
    BeAssert (0x0a == effectiveContent[effectiveBufferSize-2]);
    BeAssert (0 == effectiveContent[effectiveBufferSize-1]);
    effectiveBufferSize -= 2;

    // Utf16Buffer does not add 1 for NULL terminator, that is why this is different than the WStringR overload of this method.
    buffer.resize ((effectiveBufferSize / sizeof (uint16_t)) + 1);
    memcpy (&buffer[0], effectiveContent, effectiveBufferSize);
    buffer.back () = 0;

    xmlBufferFree (xmlBuffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlDom::ToString (WStringR buffer, ToStringOption options) const
    {
    buffer.clear ();

    xmlSaveOption   libxmlOptions = SetSaveOptions (options);

    xmlBufferPtr    xmlBuffer   = xmlBufferCreate ();
    // Default allocation scheme is XML_BUFFER_ALLOC_EXACT - ends up being a bottleneck due to constant reallocs.
    xmlBufferSetAllocationScheme (xmlBuffer, XML_BUFFER_ALLOC_DOUBLEIT);

#if defined (_WIN32)
    xmlSaveCtxtPtr  saveContext = xmlSaveToBuffer (xmlBuffer, "UTF-16LE", libxmlOptions);
#else
    xmlSaveCtxtPtr  saveContext = xmlSaveToBuffer (xmlBuffer, "UTF-8", libxmlOptions);
#endif

    xmlSaveDoc (saveContext, m_doc);

    xmlSaveClose (saveContext);

    xmlChar const*  effectiveContent    = xmlBuffer->content;
#if defined (_WIN32)
    size_t          effectiveBufferSize = xmlBuffer->use;

    // WString resize adds 1 for NULL terminator; account for that.
    buffer.resize (effectiveBufferSize / sizeof (WChar));
    memcpy (&buffer[0], effectiveContent, effectiveBufferSize);

    // the resize should have already put 0 at the termination point.
    BeAssert (0 == buffer[effectiveBufferSize/sizeof(WChar)]);
#else
    // libXml no longer supports UTF-32, so convert from UTF-8.
    buffer.AssignUtf8((Utf8CP)effectiveContent);
#endif

    // libxml appends a trailing newline; remove it.
    buffer.Trim();

    xmlBufferFree (xmlBuffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlDom::ToString (Utf8StringR buffer, ToStringOption options) const
    {
    buffer.clear ();

    xmlSaveOption   libxmlOptions = SetSaveOptions (options);

    xmlBufferPtr    xmlBuffer   = xmlBufferCreate ();
    // Default allocation scheme is XML_BUFFER_ALLOC_EXACT - ends up being a bottleneck due to constant reallocs.
    xmlBufferSetAllocationScheme (xmlBuffer, XML_BUFFER_ALLOC_DOUBLEIT);

    xmlSaveCtxtPtr  saveContext = xmlSaveToBuffer (xmlBuffer, "UTF-8", libxmlOptions);

    xmlSaveDoc (saveContext, m_doc);

    xmlSaveClose (saveContext);

    xmlChar const*  effectiveContent    = xmlBuffer->content;
    size_t          effectiveBufferSize = xmlBuffer->use;

    // the last character in an XML string produced by libxml is always the 0x0a character, which we don't want.
    BeAssert (0x0a == effectiveContent[effectiveBufferSize-1]);
    effectiveBufferSize -= 1;

    // Utf8String resize adds 1 for NULL terminator; account for that.
    buffer.resize (effectiveBufferSize / sizeof (char));
    memcpy (&buffer[0], effectiveContent, effectiveBufferSize);

    // the resize should have already put 0 at the termination point.
    BeAssert (0 == buffer[effectiveBufferSize/sizeof(char)]);

    xmlBufferFree (xmlBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlStatus     BeXmlDom::ToFile (WStringCR fileName, ToStringOption options, FileEncodingOption encoding)
    {
    char    mbFileName[1024];
    BeStringUtilities::WCharToCurrentLocaleChar (mbFileName, fileName.c_str(), _countof (mbFileName));

    CharCP  encodingType = "UTF-16LE";
    if (FILE_ENCODING_Utf8 == encoding)
        encodingType = "UTF-8";
    else if (FILE_ENCODING_Utf16 != encoding)
        {
        BeAssert (false);
        return BEXML_InvalidEncoding;
        }

    xmlSaveOption   libxmlOptions = SetSaveOptions (options);
    BeXmlStatus     status = BEXML_Success;
    xmlSaveCtxtPtr  saveContext =  xmlSaveToFilename (mbFileName, encodingType, libxmlOptions);

    xmlSaveDoc (saveContext, m_doc);

    if (NULL == saveContext)
        status = BEXML_ErrorWritingToFile;

    xmlSaveClose (saveContext);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void BeXmlDom::GetErrorLocation (int& line, int& column)
    {
    line = m_error.line;
    column = m_error.int2;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void BeXmlDom::GetErrorMessage (WStringR message)
    {
    if (NULL == m_error.message)
        message.clear();

    FormatErrorMessage (&message, &m_error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   06/2013
//---------------------------------------------------------------------------------------
BeXmlDom::ParseOptions    BeXmlDom::GetOptions ()
    {
    return m_options;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SchemaValidate(WCharCP _xsdFile)
    {
    Utf8String xsdFile;
    BeStringUtilities::WCharToUtf8 (xsdFile, _xsdFile);

    // Parse and load schema.
    xmlSchemaParserCtxtPtr pParserCtxt = xmlSchemaNewParserCtxt(xsdFile.c_str());
    if(pParserCtxt == NULL)
        return BEXML_ParseError;

    xmlSchemaPtr pSchema = xmlSchemaParse(pParserCtxt);
    xmlSchemaFreeParserCtxt(pParserCtxt);
    if(pSchema == NULL)
        return BEXML_ParseError;

    int validateStatus = -1;    // 0 == SUCCESS
    xmlSchemaValidCtxtPtr pSchemaCtxt = xmlSchemaNewValidCtxt(pSchema);
    if(pSchemaCtxt != NULL)
        {
        validateStatus = xmlSchemaValidateDoc(pSchemaCtxt, m_doc);
        xmlSchemaFreeValidCtxt(pSchemaCtxt);
        }

    if (pSchema != NULL)
        xmlSchemaFree(pSchema);

    return 0 == validateStatus ? BEXML_Success : BEXML_ParseError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlDom::IterableNodeSetIter::IterableNodeSetIter (xmlNodeSetPtr nodeSet, size_t index) : m_nodeSet (nodeSet), m_index (index) { }
BeXmlDom::IterableNodeSetIter& BeXmlDom::IterableNodeSetIter::operator++ () { ++m_index; return *this; }
BeXmlNodeP& BeXmlDom::IterableNodeSetIter::operator* () { return (BeXmlNodeP&) m_nodeSet->nodeTab[m_index]; }
bool BeXmlDom::IterableNodeSetIter::operator== (BeXmlDom::IterableNodeSetIter const& rhs) const { return ((m_nodeSet == rhs.m_nodeSet) && (m_index == rhs.m_index)); }
bool BeXmlDom::IterableNodeSetIter::operator!= (BeXmlDom::IterableNodeSetIter const& rhs) const { return !(*this == rhs); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlDom::IterableNodeSet::IterableNodeSet () : m_dom (NULL), m_result (NULL), m_nodeSet (NULL) { }
void BeXmlDom::IterableNodeSet::Init (xmlNodeSetPtr nodeSet) { m_nodeSet = nodeSet; }

void BeXmlDom::IterableNodeSet::Init (BeXmlDomR dom, xmlXPathObjectPtr result)
    {
    // in case we are reusing the same IterableNodeSet, free the results from the previous usage.
    if (NULL != m_result)
        m_dom->FreeXPathObject (*m_result);

    m_dom       = &dom;
    m_result    = result;
    m_nodeSet   = (result ? result->nodesetval : NULL);
    }


BeXmlDom::IterableNodeSet::~IterableNodeSet () { if (m_result) { m_dom->FreeXPathObject (*m_result); } }
BeXmlDom::IterableNodeSet::iterator BeXmlDom::IterableNodeSet::begin () { return BeXmlDom::IterableNodeSet::iterator (m_nodeSet, 0); }
BeXmlDom::IterableNodeSet::iterator BeXmlDom::IterableNodeSet::end () { return BeXmlDom::IterableNodeSet::iterator (m_nodeSet, (m_nodeSet ? m_nodeSet->nodeNr : 0)); }
size_t BeXmlDom::IterableNodeSet::size () const { return m_nodeSet ? (size_t)m_nodeSet->nodeNr : 0; }
BeXmlNodeP& BeXmlDom::IterableNodeSet::front () { return (BeXmlNodeP&) m_nodeSet->nodeTab[0]; }
BeXmlNodeP& BeXmlDom::IterableNodeSet::back () { return (BeXmlNodeP&) m_nodeSet->nodeTab[m_nodeSet->nodeNr - 1]; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SelectNode (BeXmlNodeP& result, Utf8CP expression, xmlXPathContextPtr context, NodeBias bias)
    {
    result = NULL;

    xmlXPathObjectPtr xpathResult = this->EvaluateXPathExpression (expression, context);
    if (NULL == xpathResult)
        return BEXML_NodeNotFound;

    if (NULL == xpathResult->nodesetval || 0 == xpathResult->nodesetval->nodeNr)
        {
        this->FreeXPathObject (*xpathResult);
        return BEXML_NodeNotFound;
        }

    BeXmlStatus status = BEXML_Success;

    if (1 == xpathResult->nodesetval->nodeNr)
        {
        result = static_cast <BeXmlNodeP> (xpathResult->nodesetval->nodeTab[0]);
        }
    else
        {
        switch (bias)
            {
            case NODE_BIAS_First:
                result = static_cast <BeXmlNodeP> (xpathResult->nodesetval->nodeTab[0]);
                break;

            case NODE_BIAS_Last:
                result = static_cast <BeXmlNodeP> (xpathResult->nodesetval->nodeTab[xpathResult->nodesetval->nodeNr - 1]);
                break;

            case NODE_BIAS_FailIfMultiple:
                status = BEXML_MultipleNodes;
                break;

            default:
                if (0 != (m_options & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
                    BeAssert ((false && L"Unknown NodeBias."));

                status = BEXML_ArgumentError;
                break;
            }
        }

    this->FreeXPathObject (*xpathResult);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SelectNodeContent(Utf8StringR resultContent, Utf8CP expression, xmlXPathContextPtr context, BeXmlDom::NodeBias bias)
    {
    resultContent.clear();

    BeXmlNodeP  effectiveNode;
    BeXmlStatus status;
    if (BEXML_Success != (status = this->SelectNode(effectiveNode, expression, context, bias)))
        return status;

    if (XML_ELEMENT_NODE != effectiveNode->type)
        return BEXML_NotElementNode;

    effectiveNode->GetContent(resultContent);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SelectNodeContent (WStringR resultContent, Utf8CP expression, xmlXPathContextPtr context, BeXmlDom::NodeBias bias)
    {
    resultContent.clear();

    Utf8String resultContentUtf8;
    BeXmlStatus status = SelectNodeContent(resultContentUtf8, expression, context, bias);
    if (BEXML_Success != status)
        return status;

    resultContent.AssignUtf8(resultContentUtf8.c_str());

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SelectNodeContentAsUInt32 (uint32_t& resultContent, Utf8CP expression, xmlXPathContextPtr context, BeXmlDom::NodeBias bias)
    {
    resultContent = 0;

    WString     strValue;
    BeXmlStatus status;
    if ((BEXML_Success != (status = this->SelectNodeContent (strValue, expression, context, bias))) || strValue.empty ())
        return status;

    if (WString(L"0x").EqualsI (strValue.substr (0, 2)))
        resultContent = BeWcsToul (strValue.c_str () + 2, NULL, 16);
    else
        resultContent = BeWcsToul (strValue.c_str (), NULL, 10);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SelectNodeContentAsUInt16 (uint16_t& resultContent, Utf8CP expression, xmlXPathContextPtr context, BeXmlDom::NodeBias bias)
    {
    resultContent = 0;

    uint32_t    ulValue;
    BeXmlStatus status;
    if ((BEXML_Success != (status = this->SelectNodeContentAsUInt32 (ulValue, expression, context, bias))) || (ulValue > USHRT_MAX))
        return status;

    resultContent = (uint16_t)ulValue;

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SelectNodeContentAsBool (bool& resultContent, Utf8CP expression, xmlXPathContextPtr context, BeXmlDom::NodeBias bias)
    {
    resultContent = false;

    WString strValue;
    BeXmlStatus status;
    if ((BEXML_Success != (status = this->SelectNodeContent (strValue, expression, context, bias))) || strValue.empty ())
        return status;

    if ((0 == BeStringUtilities::Wcsicmp (L"true", strValue.c_str ())) || (0 == BeStringUtilities::Wcsicmp (L"1", strValue.c_str ())))
        resultContent = true;

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlDom::SelectNodes (IterableNodeSet& results, Utf8CP expression, xmlXPathContextPtr context)
    {
    results.Init (*this, this->EvaluateXPathExpression (expression, context));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlDom::SelectChildNodeByName (BeXmlNodeP& result, BeXmlNodeR parent, Utf8CP childName, NodeBias bias)
    {
    Utf8String expression = "./";

    if ( (NULL == childName) || (0 == *childName) )
        expression += "*";
    else
        expression += childName;

    xmlXPathContextPtr  parentNodeContext = this->AcquireXPathContext (&parent);
    BeXmlStatus       status              = this->SelectNode (result, expression.c_str (), parentNodeContext, bias);

    this->FreeXPathContext (*parentNodeContext);

    return status;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- BeXmlNode ------------------------------------------------------------------------------------------------- BeXmlNode --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
Utf8CP      BeXmlNode::GetName ()
    {
    return (Utf8CP) this->name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   10/2011
//---------------------------------------------------------------------------------------
BeXmlNodeType   BeXmlNode::GetType()
    {
    return (BeXmlNodeType) this->type;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   10/2011
//---------------------------------------------------------------------------------------
Utf8CP          BeXmlNode::GetNamespace()
    {
    xmlNsPtr    xmlns;
    if (NULL == (xmlns = this->ns))
        return NULL;
    return (Utf8CP) xmlns->href;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  3/2015
//----------------------------------------------------------------------------------------
void   BeXmlNode::AddNamespace (Utf8CP prefix, Utf8CP uri)
    {
    if (NULL == uri)
        return;

    xmlNewNs (this, (xmlChar const*) uri, (xmlChar const*) prefix);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   10/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::SetNamespace (Utf8CP prefix, Utf8CP uri)
    {
    if (NULL == uri)
        return;

    xmlSetNs (this, xmlNewNs (this, (xmlChar const*) uri, (xmlChar const*) prefix));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz     09/2011
//---------------------------------------------------------------------------------------
int         BeXmlNode::NameStricmp (Utf8CP name)
    {
    return BeStringUtilities::Stricmp (GetName (), name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz     09/2011
//---------------------------------------------------------------------------------------
bool        BeXmlNode::IsName (Utf8CP name)
    {
    return 0 == strcmp (GetName (), name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz     09/2011
//---------------------------------------------------------------------------------------
bool        BeXmlNode::IsIName (Utf8CP name)
    {
    return 0 == BeStringUtilities::Stricmp (GetName (), name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlDomP   BeXmlNode::GetDom ()
    {
    if (NULL == this->doc)
        {
        BeAssert (false);
        return NULL;
        }

    BeAssert (NULL != this->doc->_private);

    return static_cast <BeXmlDomP> (this->doc->_private);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlNode::GetXmlString (WString& xmlString)
    {
    xmlOutputBufferPtr  buf = xmlAllocOutputBuffer (NULL);
    xmlNodeDumpOutput (buf, this->doc, this, 0, 0, NULL);

    BeStringUtilities::Utf8ToWChar (xmlString, (Utf8CP)xmlBufContent(buf->buffer), xmlBufUse(buf->buffer));

    xmlOutputBufferClose (buf);
    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlNode::GetXmlString (Utf16BufferR xmlString)
    {
    xmlOutputBufferPtr  buf = xmlAllocOutputBuffer (NULL);
    xmlNodeDumpOutput (buf, this->doc, this, 0, 0, NULL);

    BeStringUtilities::Utf8ToUtf16 (xmlString, (Utf8CP)xmlBufContent(buf->buffer), xmlBufUse(buf->buffer));

    xmlOutputBufferClose (buf);
    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlNode::GetXmlString (Utf8StringR xmlString)
    {
    xmlOutputBufferPtr buf = xmlAllocOutputBuffer (NULL);

    xmlNodeDumpOutput (buf, this->doc, this, 0, 0, NULL);

    xmlString.assign ((Utf8CP)xmlBufContent(buf->buffer));

    xmlOutputBufferClose (buf);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP  BeXmlNode::GetChildElementAt (size_t index)
    {
    BeXmlNodeP  currentNode = NULL;

    // I haven't found this functionality in the libxml2 API... this should match the internal implementation details of functions like xmlChildElementCount, xmlFirstElementChild, and xmlLastElementChild.
    switch (this->type)
        {
        case XML_ELEMENT_NODE:
        case XML_ENTITY_NODE:
        case XML_DOCUMENT_NODE:
        case XML_HTML_DOCUMENT_NODE:
            {
            currentNode = static_cast <BeXmlNodeP> (this->children);
            break;
            }

        default:
            return NULL;
        }

    while (NULL != currentNode)
        {
        if (XML_ELEMENT_NODE != currentNode->type)
            {
            currentNode = static_cast <BeXmlNodeP> (currentNode->next);
            continue;
            }

        if (0 == index)
            return currentNode;

        currentNode = static_cast <BeXmlNodeP> (currentNode->next);
        --index;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlNodeP  BeXmlNode::GetParentNode ()
    {
    return static_cast <BeXmlNodeP> (this->parent);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP  BeXmlNode::GetFirstChild (BeXmlNodeType typeWanted)
    {
    BeXmlNodeP testNode;
    if (NULL == (testNode = static_cast <BeXmlNodeP> (this->children)))
        return NULL;

    if ( (BEXMLNODE_Any == typeWanted) || (typeWanted == (BeXmlNodeType) testNode->type) )
        return testNode;

    return testNode->GetNextSibling (typeWanted);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP  BeXmlNode::GetNextSibling (BeXmlNodeType typeWanted)
    {
    for (BeXmlNodeP testNode = static_cast <BeXmlNodeP> (this->next); NULL != testNode; testNode = static_cast <BeXmlNodeP> (testNode->next))
        {
        if ( (BEXMLNODE_Any == typeWanted) || (typeWanted == (BeXmlNodeType) testNode->type) )
            return testNode;
        }

    return NULL;
    }

enum BeXmlInternalStatus
    {
    BEXMLINT_Success                = 0,
    BEXMLINT_SuccessNeedsFree       = 1,
    BEXMLINT_EmptyNeedsFree         = 2,
    BEXMLINT_NodeNotFound           = BEXML_NodeNotFound,
    BEXMLINT_NullNodeValue          = BEXML_NullNodeValue,
    };


xmlChar* GetRawContent (BeXmlNodeP node, BeXmlInternalStatus& status, Utf8CP relativePath/* =NULL */)
    {
    BeXmlNodeP pathNode = node;
    xmlChar* rawValue = NULL;
    status = BEXMLINT_NullNodeValue;

    if (NULL != relativePath)
        pathNode = node->SelectSingleNode (relativePath);

    if (NULL == pathNode)
        {
        status = BEXMLINT_NodeNotFound;
        return rawValue;
        }

    // This method attempts to short-circuit early and avoid the call to xmlNodeGetContent to avoid the unecessary string allocation.

    // If this is a plain text node, simply use its content.
    if ((XML_TEXT_NODE == pathNode->type) || (XML_CDATA_SECTION_NODE == pathNode->type))
        {
        rawValue = pathNode->content;
        if (*rawValue != '\0')
            status = BEXMLINT_Success;
        return rawValue;
        }

    // If these is only one child, and it is a plain text node, use its content.
    if ((XML_ELEMENT_NODE == pathNode->type) && (NULL != pathNode->children) && (pathNode->children == pathNode->last) && ((XML_TEXT_NODE == pathNode->children->type) || (XML_CDATA_SECTION_NODE == pathNode->children->type)))
        {
        rawValue = pathNode->children->content;
        if (*rawValue != '\0')
            status = BEXMLINT_Success;
        return rawValue;
        }

    // Otherwise it's not trivial, use the library's utility function.
    rawValue = xmlNodeGetContent (pathNode);
    if (NULL != rawValue)
        {
        if (*rawValue == '\0')
            status = BEXMLINT_EmptyNeedsFree;
        else
            status = BEXMLINT_SuccessNeedsFree;
        }

    return rawValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlNode::GetContent(Utf8StringR content, Utf8CP relativePath)
    {
    content.clear();
    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent(this, internalStatus, relativePath);

    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;

    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;

    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree(rawContent);
        return BEXML_Success;
        }

    if (NULL == rawContent)
        return BEXML_Success;

    content = (Utf8CP)rawContent;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree(rawContent);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContent (WStringR content, Utf8CP relativePath)
    {
    content.clear();

    Utf8String contentUtf8;
    BeXmlStatus status = GetContent(contentUtf8, relativePath);

    if (!contentUtf8.empty())
        content.AssignUtf8(contentUtf8.c_str());

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentInt32Value (int32_t& value, Utf8CP relativePath)
    {
    value = 0;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    int ivalue = value;
    if (1 != sscanf ((CharCP)rawContent, "%d", &ivalue))
        return BEXML_ContentWrongType;

    value = ivalue;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentUInt32Value (uint32_t& value, Utf8CP relativePath)
    {
    value = 0;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    unsigned int uivalue;
    if (1 != sscanf ((CharCP)rawContent, "%u", &uivalue))
        return BEXML_ContentWrongType;

    value = uivalue;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Colin.Kerr      09/2012
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentInt64Value (int64_t& value, Utf8CP relativePath)
    {
    value = 0;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    if (1 != sscanf ((CharCP)rawContent, "%" PRId64, &value))
        return BEXML_ContentWrongType;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentUInt64Value (uint64_t& value, Utf8CP relativePath)
    {
    value = 0;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    if (1 != sscanf ((CharCP)rawContent, "%" PRIu64, &value))
        return BEXML_ContentWrongType;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentDoubleValue (double& value, Utf8CP relativePath)
    {
    value = 0.0;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    if (1 != sscanf ((CharCP)rawContent, "%lg", &value))
        return BEXML_ContentWrongType;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Colin.Kerr      09/2012
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentDPoint2dValue (double& x, double& y, Utf8CP relativePath)
    {
    x = 0;
    y = 0;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    if (2 != sscanf ((CharCP)rawContent, "%lg,%lg", &x, &y))
        return BEXML_ContentWrongType;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Colin.Kerr      09/2012
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentDPoint3dValue (double& x, double& y, double& z, Utf8CP relativePath)
    {
    x = 0;
    y = 0;
    z = 0;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    if (3 != sscanf ((CharCP)rawContent, "%lg,%lg,%lg", &x, &y, &z))
        return BEXML_ContentWrongType;

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

// Copy out of source into buffer.
// Finish at null or comma.
// readIndex advances to be ready for next call.
// Repeated call at terminator will return 0.
// returns number of nonnull characters copied to buffer.
int FillStringToFieldBreaker (CharP buffer, int bufferSize, CharCP source, int &readIndex)
    {
    for (int n = 0; n + 1 < bufferSize; readIndex++)
        {
        char c = source[readIndex];
        buffer[n++] = c;
        if (c == 0) // terminator, already copied to buffer
            return n - 1;
        if (c == ',')
            {
            buffer[n-1] = 0;
            readIndex++;    // next call starts post-comma.
            return n - 1;
            }
        }
    return 0;   // reject buffer overflow
    }

int FillStringToFieldBreaker (wchar_t *buffer, int bufferSize, WString & source, int &readIndex)
    {
    for (int n = 0; n + 1 < bufferSize; readIndex++)
        {
        wchar_t c = source[readIndex];
        buffer[n++] = c;
        if (c == 0) // terminator, already copied to buffer
            return n - 1;
        if (c == ',')
            {
            buffer[n-1] = 0;
            readIndex++;    // next call starts post-comma.
            return n - 1;
            }
        }
    return 0;
    }
#define FIELD_BUFFER_SIZE 1024
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentDoubleValues (bvector<double> &values, Utf8CP relativePath)
    {
    double value = 0.0;
    values.clear ();
    BeXmlNodeP pathNode = this;

    if (NULL != relativePath)
        pathNode = this->SelectSingleNode (relativePath);

    if (NULL == pathNode)
        return BEXML_NodeNotFound;

    // If this is a plain text node, simply use its content.
    if ((XML_TEXT_NODE == pathNode->type) || (XML_CDATA_SECTION_NODE == pathNode->type))
        {
        char buffer[FIELD_BUFFER_SIZE];
        int readIndex = 0;
        while (0 < FillStringToFieldBreaker (buffer, FIELD_BUFFER_SIZE, (CharCP) pathNode->content, readIndex))
            {
            if (1 != sscanf (buffer, "%lg", &value))
                return BEXML_ContentWrongType;
            values.push_back (value);
            }
        return BEXML_Success;
        }

    // else use the method (paying the performance penalty) and get the WString
    WString content;
    pathNode->GetContent (content, NULL);
    wchar_t buffer[FIELD_BUFFER_SIZE];
    int readIndex = 0;
    while (0 < FillStringToFieldBreaker (buffer, FIELD_BUFFER_SIZE, content, readIndex))
        {
        if (1 != BE_STRING_UTILITIES_SWSCANF (buffer, L"%lg", &value))
            return BEXML_ContentWrongType;
        values.push_back (value);
        }
    return BEXML_Success;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetContentBooleanValue (bool& value, Utf8CP relativePath)
    {
    value = false;

    BeXmlInternalStatus internalStatus;
    xmlChar* rawContent = GetRawContent (this, internalStatus, relativePath);
    if (BEXMLINT_NodeNotFound == internalStatus)
        return BEXML_NodeNotFound;
    if (BEXMLINT_NullNodeValue == internalStatus)
        return BEXML_NullNodeValue;
    if (BEXMLINT_EmptyNeedsFree == internalStatus)
        {
        xmlFree (rawContent);
        return BEXML_NullNodeValue;
        }

    BeXmlStatus status = BEXML_Success;
    if ((0 == BeStringUtilities::Stricmp ((CharCP) rawContent, "true")) || (0 == BeStringUtilities::Stricmp ((CharCP)rawContent, "1")))
        {
        value = true;
        return BEXML_Success;
        }

    if (BEXMLINT_SuccessNeedsFree == internalStatus)
        xmlFree (rawContent);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlNode::GetCDATASection (WStringR cDataContents)
    {
    BeXmlNodeP  cDataNode;
    if (NULL == (cDataNode = this->GetFirstChild (BEXMLNODE_Section)))
        return BEXML_CDATANotFound;

    BeStringUtilities::Utf8ToWChar (cDataContents, (Utf8CP) cDataNode->content);
    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlNode::SetContent (WCharCP _value)
    {
    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, _value);

    // translate any special characters or entity references.
    xmlChar*    tmpBuffer = xmlEncodeEntitiesReentrant (&GetDom()->GetDocument(), (xmlChar const*) value.c_str());
    xmlNodeSetContent (this, tmpBuffer);
    xmlFree (tmpBuffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlNode::SetContentFast (WCharCP _value)
    {
    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, _value);

    xmlNodeSetContent (this, (xmlChar const*)value.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlNode::SetContentFast (Utf8CP _value)
    {
    xmlNodeSetContent (this, (xmlChar const*)_value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddChildNode (BeXmlNodeR childNode)
    {
    return static_cast <BeXmlNodeP> (xmlAddChild (this, &childNode));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddEmptyElement (Utf8CP nodeName)
    {
    BeXmlDomP xmlDom = GetDom ();

    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, NULL));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeXmlNodeP      BeXmlNode::AddEmptyElement (WCharCP _nodeName)
    {
    Utf8String nodeName;
    if ( (NULL != _nodeName) && (0 != *_nodeName) )
        BeStringUtilities::WCharToUtf8 (nodeName, _nodeName);

    return AddEmptyElement (nodeName.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Jean-Francois.Cote   07/2015
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementStringValue(Utf8CP nodeName, Utf8CP content)
    {
    BeXmlDomP xmlDom = GetDom();

    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode(xmlDom->m_doc, NULL, (xmlChar const*) nodeName, (xmlChar const*) content));

    if (NULL == newNode)
        return NULL;

    return AddChildNode(*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementStringValue (Utf8CP nodeName, WCharCP _content)
    {
    BeXmlDomP xmlDom = GetDom ();

    Utf8String content;
    if ( (NULL != _content) && (0 != *_content) )
        BeStringUtilities::WCharToUtf8 (content, _content);

    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, (xmlChar const*)content.c_str()));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementUInt32Value (Utf8CP nodeName, uint32_t value)
    {
    BeXmlDomP xmlDom = GetDom ();

    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%u", value);
    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, (xmlChar const*)valueString));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementInt32Value (Utf8CP nodeName, int32_t value)
    {
    BeXmlDomP xmlDom = GetDom ();

    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%d", value);
    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, (xmlChar const*)valueString));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementUInt64Value (Utf8CP nodeName, uint64_t value)
    {
    BeXmlDomP xmlDom = GetDom ();

    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%llu", value);
    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, (xmlChar const*)valueString));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementInt64Value (Utf8CP nodeName, int64_t value)
    {
    BeXmlDomP xmlDom = GetDom ();

    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%lld", value);
    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, (xmlChar const*)valueString));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementBooleanValue (Utf8CP nodeName, bool value)
    {
    BeXmlDomP xmlDom = GetDom ();

    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, (xmlChar const*)(value ? "true" : "false")));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::AddElementDoubleValue (Utf8CP nodeName, double value)
    {
    BeXmlDomP xmlDom = GetDom ();

    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%.17g", value);
    BeXmlNodeP newNode = static_cast <BeXmlNodeP> (xmlNewDocRawNode (xmlDom->m_doc, NULL, (xmlChar const*)nodeName, (xmlChar const*)valueString));

    if (NULL == newNode)
        return NULL;

    return AddChildNode (*newNode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddCDATASection (WCharCP _cDataContents)
    {
    if ( (NULL == _cDataContents) || (0 == *_cDataContents) )
        return;

    Utf8String cDataContents;
    BeStringUtilities::WCharToUtf8 (cDataContents, _cDataContents);

    xmlNodePtr cDataBlock = xmlNewCDataBlock (this->doc, (xmlChar const *)cDataContents.c_str(), (int)cDataContents.length());
    xmlAddChild (this, cDataBlock);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetAttributeStringValue (WStringR resultValue, Utf8CP attributeName)
    {
    Utf8String  resultValueUtf8;
    BeXmlStatus status          = GetAttributeStringValue(resultValueUtf8, attributeName);

    resultValue.AssignUtf8(resultValueUtf8.c_str());

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetAttributeStringValue (Utf8StringR resultValue, Utf8CP attributeName)
    {
    resultValue.clear ();

    // This method attempts to short-circuit early and avoid the call to xmlGetProp to avoid the unecessary string allocation.
    xmlAttrPtr foundAttr = xmlHasProp (this, (xmlChar const*)attributeName);
    if (NULL == foundAttr)
        return BEXML_AttributeNotFound;

    if ((NULL != foundAttr->children) && (NULL == foundAttr->children->next) && ((XML_TEXT_NODE == foundAttr->children->type) || (XML_CDATA_SECTION_NODE == foundAttr->children->type)))
        {
        resultValue = (CharCP)foundAttr->children->content;
        return BEXML_Success;
        }

    xmlChar* attrValue;
    if (NULL == (attrValue = xmlGetProp (this, (xmlChar const*)attributeName)))
        return BEXML_AttributeNotFound;

    resultValue = (CharCP)attrValue;

    xmlFree (attrValue);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetAttributeBooleanValue (bool& value, Utf8CP attributeName)
    {
    value = false;

    // This method attempts to short-circuit early and avoid the call to xmlGetProp to avoid the unecessary string allocation.
    xmlAttrPtr foundAttr = xmlHasProp (this, (xmlChar const*)attributeName);
    if (NULL == foundAttr)
        return BEXML_AttributeNotFound;

    if ((NULL != foundAttr->children) && (NULL == foundAttr->children->next) && ((XML_TEXT_NODE == foundAttr->children->type) || (XML_CDATA_SECTION_NODE == foundAttr->children->type)))
        {
        if (0 == BeStringUtilities::Stricmp ((CharCP) foundAttr->children->content, "true"))
            {
            value = true;
            return BEXML_Success;
            }

        else if (0 == BeStringUtilities::Stricmp ((CharCP) foundAttr->children->content, "false"))
            {
            return BEXML_Success;
            }
        return BEXML_UnexpectedValue;
        }

    BeXmlStatus status = BEXML_UnexpectedValue;

    xmlChar* attrValue;
    if (NULL == (attrValue = xmlGetProp (this, (xmlChar const*)attributeName)))
        return BEXML_AttributeNotFound;

    if (0 == BeStringUtilities::Stricmp ((CharCP) attrValue, "true"))
        {
        value = true;
        status = BEXML_Success;
        }

    else if (0 == BeStringUtilities::Stricmp ((CharCP) attrValue, "false"))
        {
        status = BEXML_Success;
        }

    xmlFree (attrValue);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetAttributeDoubleValue (double& doubleValue, Utf8CP attributeName)
    {
    doubleValue = 0.0;

    // This method attempts to short-circuit early and avoid the call to xmlGetProp to avoid the unecessary string allocation.
    xmlAttrPtr foundAttr = xmlHasProp (this, (xmlChar const*)attributeName);
    if (NULL == foundAttr)
        return BEXML_AttributeNotFound;

    if ((NULL != foundAttr->children) && (NULL == foundAttr->children->next) && ((XML_TEXT_NODE == foundAttr->children->type) || (XML_CDATA_SECTION_NODE == foundAttr->children->type)))
        {
        if (1 == sscanf ((CharCP) foundAttr->children->content, "%lg", &doubleValue))
            return BEXML_Success;

        return BEXML_UnexpectedValue;
        }

    xmlChar* attrValue;
    if (NULL == (attrValue = xmlGetProp (this, (xmlChar const*)attributeName)))
        return BEXML_AttributeNotFound;

    int scanCount = sscanf ((CharCP) attrValue, "%lg", &doubleValue);

    xmlFree (attrValue);

    return (1 == scanCount) ? BEXML_Success : BEXML_UnexpectedValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetAttributeUInt32Value (uint32_t& uint32Value, Utf8CP attributeName)
    {
    uint32Value = 0;

    // This method attempts to short-circuit early and avoid the call to xmlGetProp to avoid the unecessary string allocation.
    xmlAttrPtr foundAttr = xmlHasProp (this, (xmlChar const*)attributeName);
    if (NULL == foundAttr)
        return BEXML_AttributeNotFound;

    if ((NULL != foundAttr->children) && (NULL == foundAttr->children->next) && ((XML_TEXT_NODE == foundAttr->children->type) || (XML_CDATA_SECTION_NODE == foundAttr->children->type)))
        {
        unsigned int uivalue;
        if (1 == sscanf ((CharCP) foundAttr->children->content, "%u", &uivalue))
            {
            uint32Value = uivalue;
            return BEXML_Success;
            }

        return BEXML_UnexpectedValue;
        }

    xmlChar* attrValue;
    if (NULL == (attrValue = xmlGetProp (this, (xmlChar const*)attributeName)))
        return BEXML_AttributeNotFound;

    unsigned int uivalue;
    int scanCount = sscanf ((CharCP) attrValue, "%u", &uivalue);
    uint32Value = uivalue;

    xmlFree (attrValue);

    return (1 == scanCount) ? BEXML_Success : BEXML_UnexpectedValue;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetAttributeInt32Value (int32_t& int32Value, Utf8CP attributeName)
    {
    int32Value = 0;

    // This method attempts to short-circuit early and avoid the call to xmlGetProp to avoid the unecessary string allocation.
    xmlAttrPtr foundAttr = xmlHasProp (this, (xmlChar const*)attributeName);
    if (NULL == foundAttr)
        return BEXML_AttributeNotFound;

    if ((NULL != foundAttr->children) && (NULL == foundAttr->children->next) && ((XML_TEXT_NODE == foundAttr->children->type) || (XML_CDATA_SECTION_NODE == foundAttr->children->type)))
        {
        int ivalue;
        if (1 == sscanf ((CharCP) foundAttr->children->content, "%d", &ivalue))
            {
            int32Value = ivalue;
            return BEXML_Success;
            }

        return BEXML_UnexpectedValue;
        }

    xmlChar* attrValue;
    if (NULL == (attrValue = xmlGetProp (this, (xmlChar const*)attributeName)))
        return BEXML_AttributeNotFound;

    int ivalue;
    int scanCount = sscanf ((CharCP) attrValue, "%d", &ivalue);
    int32Value = ivalue;

    xmlFree (attrValue);

    return (1 == scanCount) ? BEXML_Success : BEXML_UnexpectedValue;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus     BeXmlNode::GetAttributeUInt64Value (uint64_t& int64Value, Utf8CP attributeName)
    {
    int64Value = 0;

    // This method attempts to short-circuit early and avoid the call to xmlGetProp to avoid the unecessary string allocation.
    xmlAttrPtr foundAttr = xmlHasProp (this, (xmlChar const*)attributeName);
    if (NULL == foundAttr)
        return BEXML_AttributeNotFound;

    if ((NULL != foundAttr->children) && (NULL == foundAttr->children->next) && ((XML_TEXT_NODE == foundAttr->children->type) || (XML_CDATA_SECTION_NODE == foundAttr->children->type)))
        {
        if (1 == sscanf ((CharCP) foundAttr->children->content, "%" PRIu64, &int64Value))
            return BEXML_Success;

        return BEXML_UnexpectedValue;
        }

    xmlChar* attrValue;
    if (NULL == (attrValue = xmlGetProp (this, (xmlChar const*)attributeName)))
        return BEXML_AttributeNotFound;

    int scanCount = sscanf ((CharCP) attrValue, "%" PRIu64, &int64Value);

    xmlFree (attrValue);

    return (1 == scanCount) ? BEXML_Success : BEXML_UnexpectedValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddAttributeStringValue (Utf8CP attributeName, WCharCP _value)
    {
    if ((NULL == attributeName) || (0 == *attributeName) || (NULL == _value))
        return;

    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, _value);

    xmlNewProp (this, (xmlChar const*)attributeName, (xmlChar const*)value.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Carole.MacDonald 07/2015
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddAttributeStringValue (Utf8CP attributeName, Utf8CP _value)
    {
    if ((NULL == attributeName) || (0 == *attributeName) || (NULL == _value))
        return;

    xmlNewProp (this, (xmlChar const*)attributeName, (xmlChar const*)_value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddAttributeBooleanValue (Utf8CP attributeName, bool value)
    {
    if ((NULL == attributeName) || (0 == *attributeName))
        return;

    xmlNewProp (this, (xmlChar const*)attributeName, (xmlChar const*)(value ? "true" : "false"));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddAttributeUInt32Value (Utf8CP attributeName, uint32_t value)
    {
    if ((NULL == attributeName) || (0 == *attributeName))
        return;

    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%u", value);

    xmlNewProp (this, (xmlChar const*)attributeName, (xmlChar const*)valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddAttributeInt32Value (Utf8CP attributeName, int32_t value)
    {
    if ((NULL == attributeName) || (0 == *attributeName))
        return;

    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%d", value);

    xmlNewProp (this, (xmlChar const*)attributeName, (xmlChar const*)valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddAttributeDoubleValue (Utf8CP attributeName, double value)
    {
    if ((NULL == attributeName) || (0 == *attributeName))
        return;

    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%.17g", value);

    xmlNewProp (this, (xmlChar const*)attributeName, (xmlChar const*)valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::AddAttributeUInt64Value (Utf8CP attributeName, uint64_t value)
    {
    if ((NULL == attributeName) || (0 == *attributeName))
        return;

    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%llu", value);

    xmlNewProp (this, (xmlChar const*)attributeName, (xmlChar const*)valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::ImportNode (BeXmlNodeP nodeToImport)
    {
    BeXmlNodeP copiedNode = static_cast <BeXmlNodeP> (xmlDocCopyNode (nodeToImport, this->doc, 1));
    return AddChildNode (*copiedNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            BeXmlNode::RemoveChildNode (BeXmlNodeP childNode)
    {
    // verify that this is a child node.
    if (this != childNode->GetParentNode())
        {
        if (0 != (GetDom()->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);
        return;
        }
    xmlUnlinkNode (childNode);
    xmlFreeNode (childNode);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
BeXmlNodeP      BeXmlNode::SelectSingleNode (Utf8CP childPath)
    {
    BeXmlDomP   ourDom;
    if ( (NULL == childPath) || (0 == *childPath) || (NULL == (ourDom = GetDom())) )
        {
        if (0 != (GetDom()->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);
        return NULL;
        }

    // get an XPathContext with this as the root node and our namespaces in it.
    xmlXPathContextPtr pathContext;
    if (NULL == (pathContext = ourDom->AcquireXPathContext (this)))
        {
        if (0 != (GetDom()->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);
        return NULL;
        }

    xmlXPathObjectPtr xPathResult = xmlXPathEvalExpression ((xmlChar const*)childPath, pathContext);

    ourDom->FreeXPathContext (*pathContext);

    if (NULL == xPathResult)
        {
        if (0 != (GetDom()->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);
        return NULL;
        }

    BeXmlNodeP result = (xPathResult->nodesetval->nodeNr > 0) ? static_cast <BeXmlNodeP> ((xPathResult->nodesetval->nodeTab[0])) : NULL;

    // free the xPathResult now.
    xmlXPathFreeObject (xPathResult);

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Barry.Bentley   07/2011
//---------------------------------------------------------------------------------------
void            BeXmlNode::SelectChildNodes (BeXmlDom::IterableNodeSet& childNodes, Utf8CP childPath)
    {
    BeXmlDomP   ourDom;
    if ( (NULL == childPath) || (0 == *childPath) || (NULL == (ourDom = GetDom())) )
        {
        if ( (NULL == GetDom()) || (GetDom()->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);
        return;
        }

    // get an XPathContext with this as the root node and our namespaces in it.
    xmlXPathContextPtr pathContext;
    if (NULL == (pathContext = ourDom->AcquireXPathContext (this)))
        {
        if (0 != (GetDom()->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);
        return;
        }

    // when we use BeXmlDom::EvaluateXPathExpression, the result is cached for us in the BeXmlDom object, and freed when an IterableNodeSet based on it is freed.
    xmlXPathObjectPtr   xPathResult = ourDom->EvaluateXPathExpression (childPath, pathContext);

    // we don't need the context any more.
    ourDom->FreeXPathContext (*pathContext);

    if (NULL == xPathResult)
        {
        if (0 != (GetDom()->GetOptions() & BeXmlDom::XMLPARSE_OPTION_AssertOnParseError))
            BeAssert (false);
        return;
        }

    // set childNodes to hld the XPathResult. When childNodes is freed, the xPathResult is freed also.
    childNodes.Init (*ourDom, xPathResult);
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- BeXmlReader ------------------------------------------------------------------------------------------------- BeXmlReader --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
xmlTextReader& BeXmlReader::GetReader ()
    {
    return *m_reader;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::BeXmlReader () :
    m_reader (NULL)
    {
    // reset the global error.
    xmlResetLastError();

    // clear our internal error.
    memset (&m_error, 0, sizeof(m_error));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::~BeXmlReader ()
    {
    if (NULL != m_reader)
        xmlFreeTextReader (m_reader);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
static int getDefaultXmlReadOptions ()
    {
    int options = 0;
    // Should we consider setting XML_PARSE_RECOVER?

#if !defined (_WIN32)
#define LIMBXML2_PRINT_WARNINGS
#endif
#ifndef LIMBXML2_PRINT_WARNINGS
    // Windows: Don't let libxml2 print warnings or errors to stdout.
    // While these will typically be useful, it has been determined that printing to stdout is normally too disruptive, so rely on higher level code to detect error codes and re-emit in better ways.
    //  Unix: For now, do print warnings, as they are the only clues we get about portability problems.
    options |= XML_PARSE_NOERROR;
    options |= XML_PARSE_NOWARNING;
#endif

    return options;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReaderPtr BeXmlReader::CreateAndReadFromFile (BeXmlStatus& xmlStatus, WCharCP _filePath, WStringP errorMsg)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    BeXmlReaderP wrapper = new BeXmlReader ();

    if (WString::IsNullOrEmpty (_filePath))
        {
        BeAssert (false);
        xmlStatus = BEXML_FileNotFound;
        return wrapper;
        }

    Utf8String filePath;
    BeStringUtilities::WCharToUtf8 (filePath, _filePath);

    wrapper->m_reader = xmlReaderForFile (filePath.c_str (), NULL, getDefaultXmlReadOptions ());
    if (NULL == wrapper->m_reader)
        {
        xmlStatus = BEXML_ReadError;
        xmlCopyError (xmlGetLastError(), &wrapper->m_error);
        BeXmlDom::FormatErrorMessage (errorMsg, &wrapper->m_error);
        }

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReaderPtr BeXmlReader::CreateAndReadFromString (BeXmlStatus& xmlStatus, WCharCP source, size_t characterCount, WStringP errorMsg)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    BeXmlReaderP wrapper = new BeXmlReader ();

    // For some reason, the size argument to xmlReaderForMemory cannot include the null terminator. Guard against a caller passing in the size including the null terminator.
    if (0 == characterCount)
        characterCount = wcslen ((WCharCP) source);
    else if (0 == source[characterCount-1])
        characterCount--;

    BeAssert (characterCount == wcslen (source));

#if defined (_WIN32)
    wrapper->m_reader = xmlReaderForMemory ((CharCP)source, (int) (sizeof (Utf16Char) * characterCount), NULL, "UTF-16LE", getDefaultXmlReadOptions ());
#else
    // libXml no longer supports UTF-32, so convert to UTF-8.
    Utf8String utf8Source;
    BeStringUtilities::WCharToUtf8(utf8Source, source, characterCount);
    wrapper->m_reader = xmlReaderForMemory ((CharCP)utf8Source.c_str(), (int) (sizeof (Utf8Char) * (utf8Source.size() + 1)), NULL, "UTF-8", getDefaultXmlReadOptions ());
#endif

    if (NULL == wrapper->m_reader)
        {
        xmlStatus = BEXML_ReadError;
        xmlCopyError (xmlGetLastError(), &wrapper->m_error);
        BeXmlDom::FormatErrorMessage (errorMsg, &wrapper->m_error);
        }

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2013
//---------------------------------------------------------------------------------------
BeXmlReaderPtr BeXmlReader::CreateAndReadFromString(BeXmlStatus& xmlStatus, Utf8CP source, size_t characterCount, WStringP errorMsg)
    {
    xmlStatus = BEXML_Success;

    if (NULL != errorMsg)
        errorMsg->clear();

    initializeExtraEncodingHandlers ();
    // For some reason, the size argument to xmlReaderForMemory cannot include the null terminator. Guard against a caller passing in the size including the null terminator.
    if (0 == characterCount)
        characterCount = strlen (source);
    else if (0 == source[characterCount-1])
        characterCount--;

    BeXmlReaderP wrapper = new BeXmlReader ();
    wrapper->m_reader = xmlReaderForMemory (source, (int) characterCount, NULL, NULL, getDefaultXmlReadOptions());

    if (NULL == wrapper->m_reader)
        {
        xmlStatus = BEXML_ReadError;
        xmlCopyError(xmlGetLastError(), &wrapper->m_error);
        BeXmlDom::FormatErrorMessage (errorMsg, &wrapper->m_error);

        return NULL;
        }

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult BeXmlReader::_Read ()
    {
    return (ReadResult)xmlTextReaderRead (m_reader);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult BeXmlReader::_ReadTo (NodeType nodeType)
    {
    return this->_ReadTo (nodeType, NULL, false, NULL);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult BeXmlReader::_ReadTo (NodeType nodeType, Utf8CP name, bool stayInCurrentElement, WStringP value)
    {
    if (value)
        value->clear ();

    ReadResult  status;
    bool        checkName           = ((NULL != name) && (0 != *name));
    size_t      nestedElementCount  = 0;

    if (stayInCurrentElement && xmlTextReaderIsEmptyElement (m_reader))
        return READ_RESULT_Error;

    while (READ_RESULT_Success == (status = this->Read ()))
        {
        if (stayInCurrentElement)
            {
            if ((NODE_TYPE_Element == this->GetCurrentNodeType ()) && !xmlTextReaderIsEmptyElement (m_reader))
                {
                ++nestedElementCount;
                }
            else if (NODE_TYPE_EndElement == this->GetCurrentNodeType ())
                {
                if (0 == nestedElementCount)
                    return READ_RESULT_Error;

                --nestedElementCount;
                }
            }

        if (nodeType != this->GetCurrentNodeType ())
            continue;

        if (checkName)
            {
            Utf8String currName;
            this->GetCurrentNodeName (currName);

            if (0 == strcmp (name, currName.c_str ()))
                break;
            else
                continue;
            }

        break;
        }

    if ((READ_RESULT_Success == status) && (NULL != value))
        this->_GetCurrentNodeValue (*value);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult BeXmlReader::ReadToEndOfCurrentElement ()
    {
    ReadResult  status;
    size_t      nestedElementCount  = 0;

    while (READ_RESULT_Success == (status = this->Read ()))
        {
        switch (this->GetCurrentNodeType ())
            {
            case NODE_TYPE_Element:
                {
                ++nestedElementCount;
                break;
                }

            case NODE_TYPE_EndElement:
                {
                if (0 == nestedElementCount)
                    return status;

                -- nestedElementCount;
                }
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult BeXmlReader::ReadToStartOfNextSiblingElement()
    {
    int         originalDepth   = xmlTextReaderDepth(m_reader);
    ReadResult  moveStatus;

    while (1 == (moveStatus = this->_ReadTo(NODE_TYPE_Element)))
        {
        if (xmlTextReaderDepth(m_reader) <= originalDepth)
            break;
        }

    return moveStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult BeXmlReader::_ReadToEndOfElement()
    {
    int         originalDepth   = xmlTextReaderDepth(m_reader);
    ReadResult  moveStatus      = READ_RESULT_Error;

    if (!IsCurrentElementEmpty ())
        {
        while (1 == (moveStatus = this->_ReadTo(NODE_TYPE_EndElement)))
            {
            if (xmlTextReaderDepth(m_reader) <= originalDepth)
                break;
            }
        }

    return moveStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::_ReadToNextAttribute (Utf8StringP name, WStringP value)
    {
    if (name)
        name->clear ();

    if (value)
        value->clear ();

    if (1 != xmlTextReaderMoveToNextAttribute (m_reader))
        return BEXML_NoMoreAttributes;

    if (name)
        this->GetCurrentNodeName (*name);

    if (value)
        this->_GetCurrentNodeValue (*value);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::_ReadToNextAttribute (Utf8StringP name, Utf8StringP value)
    {
    if (name)
        name->clear ();

    if (value)
        value->clear ();

    if (1 != xmlTextReaderMoveToNextAttribute (m_reader))
        return BEXML_NoMoreAttributes;

    if (name)
        this->_GetCurrentNodeName (*name);

    if (value)
        this->_GetCurrentNodeValue (*value);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2013
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult BeXmlReader::Skip()
    {
    switch (xmlTextReaderNext(m_reader))
        {
        case 1:     return READ_RESULT_Success;
        case 0:     return READ_RESULT_Empty;
        case -1:    return READ_RESULT_Error;

        default:
            BeAssert(false);
            return READ_RESULT_Error;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
bool BeXmlReader::IsAtEnd ()
    {
    switch (xmlTextReaderReadState (m_reader))
        {
        case (XML_TEXTREADER_MODE_INITIAL):
        case (XML_TEXTREADER_MODE_INTERACTIVE):
        case (XML_TEXTREADER_MODE_READING):
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlReader::NodeType BeXmlReader::_GetCurrentNodeType ()
    {
    return (NodeType)xmlTextReaderNodeType (m_reader);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2012
//---------------------------------------------------------------------------------------
bool BeXmlReader::IsCurrentElementEmpty()
    {
    // Attribute nodes always report non-empty. This method checks "elements", not arbitrary nodes. As a convenience (as documented), check the attribute's element if possible.
    if (NODE_TYPE_Attribute == GetCurrentNodeType())
        {
        if (-1 == xmlTextReaderMoveToElement(m_reader))
            return true;
        }

    return (0 != xmlTextReaderIsEmptyElement(m_reader));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::_GetCurrentNodeName (Utf8StringR name)
    {
    xmlChar const* rawName = xmlTextReaderConstName (m_reader);
    if (NULL == rawName)
        {
        name.clear ();
        return BEXML_NullNodeName;
        }

    name.assign ((Utf8CP)rawName);
    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::_GetCurrentNodeValue (WStringR value)
    {
    xmlChar const* rawValue = xmlTextReaderConstValue (m_reader);
    if (NULL == rawValue)
        {
        value.clear ();
        return BEXML_NullNodeValue;
        }

    BeStringUtilities::Utf8ToWChar (value, (Utf8CP)rawValue);
    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::_GetCurrentNodeValue (Utf8StringR value)
    {
    xmlChar const* rawValue = xmlTextReaderConstValue (m_reader);
    if (NULL == rawValue)
        {
        value.clear ();
        return BEXML_NullNodeValue;
        }

    value = (Utf8CP)rawValue;

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::GetCurrentNodeString(WStringR value)
    {
    xmlChar* rawValue = xmlTextReaderReadString(m_reader);
    if (NULL == rawValue)
        {
        value.clear();
        return BEXML_NullInnerXml;
        }

    BeStringUtilities::Utf8ToWChar(value, (Utf8CP)rawValue);
    xmlFree(rawValue);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::GetCurrentNodeOuterXml (WStringR xmlString)
    {
    xmlChar* rawInnerXml = xmlTextReaderReadOuterXml (m_reader);
    if (NULL == rawInnerXml)
        {
        xmlString.clear ();
        return BEXML_NullOuterXml;
        }

    BeStringUtilities::Utf8ToWChar (xmlString, (Utf8CP)rawInnerXml);
    xmlFree (rawInnerXml);

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlReader::GetCurrentNodeInnerXml (WStringR xmlString)
    {
    xmlChar* rawInnerXml = xmlTextReaderReadInnerXml(m_reader);
    if (NULL == rawInnerXml)
        {
        xmlString.clear();
        return BEXML_NullInnerXml;
        }

    BeStringUtilities::Utf8ToWChar(xmlString, (Utf8CP)rawInnerXml);
    xmlFree(rawInnerXml);

    return BEXML_Success;
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>11/2014</date>
//---------------+---------------+---------------+---------------+---------------+-----
BeXmlReader::NodeType BeXmlReader::_MoveToContent()
    {
    ReadResult  status;

    BeXmlReader::NodeType nodeType;
    do {
        nodeType = this->_GetCurrentNodeType();
        switch (nodeType)
            {
            case BeXmlReader::NodeType::NODE_TYPE_Attribute:
                // MoveToElement();
                break;
            case BeXmlReader::NodeType::NODE_TYPE_Element:
            case BeXmlReader::NodeType::NODE_TYPE_EndElement:
            case BeXmlReader::NodeType::NODE_TYPE_CDATA:
            case BeXmlReader::NodeType::NODE_TYPE_Text:
            case BeXmlReader::NodeType::NODE_TYPE_EntityReference:
            case BeXmlReader::NodeType::NODE_TYPE_EndEntity:
                return nodeType;

            }
        } while (READ_RESULT_Success == (status = this->_Read ()));

    return this->_GetCurrentNodeType ();
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>07/2015</date>
//---------------+---------------+---------------+---------------+---------------+-----
bool BeXmlReader::_IsEmptyElement()
    {
    // Returns 1 if empty, 0 if not and -1 in case of error
    return xmlTextReaderIsEmptyElement(m_reader) == 1;
    }

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//-- BeXmlReader ------------------------------------------------------------------------------------------------- BeXmlReader --
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
xmlTextWriter& BeXmlWriter::GetWriter ()
    {
    return *m_writer;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlWriter::BeXmlWriter () :
    m_buffer    (NULL),
    m_writer    (NULL)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlWriter::~BeXmlWriter ()
    {
    if (NULL != m_writer)
        xmlFreeTextWriter (m_writer);

    if (NULL != m_buffer)
        xmlBufferFree (m_buffer);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlWriterPtr BeXmlWriter::Create ()
    {
    initializeExtraEncodingHandlers ();
    xmlBufferPtr buffer = xmlBufferCreate ();
    // Default allocation scheme is XML_BUFFER_ALLOC_EXACT - ends up being a bottleneck due to constant reallocs.
    xmlBufferSetAllocationScheme (buffer, XML_BUFFER_ALLOC_DOUBLEIT);

    if (NULL == buffer)
        return NULL;

    xmlTextWriterPtr writer = xmlNewTextWriterMemory (buffer, 0);
    if (NULL == writer)
        {
        xmlBufferFree (buffer);
        return NULL;
        }

    BeXmlWriterP wrapper = new BeXmlWriter ();
    wrapper->m_buffer   = buffer;
    wrapper->m_writer   = writer;

    return wrapper;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlWriterPtr BeXmlWriter::CreateFileWriter (WCharCP filePath)
    {
    initializeExtraEncodingHandlers ();
    Utf8String filePathUtf8;
    BeStringUtilities::WCharToUtf8 (filePathUtf8, filePath);
    xmlTextWriterPtr writer = xmlNewTextWriterFilename (filePathUtf8.c_str (), 0);
    if (NULL == writer)
        return NULL;

    BeXmlWriterP wrapper = new BeXmlWriter ();
    wrapper->m_writer = writer;

    return wrapper;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::AddFromReader (xmlTextReader& reader)
    {
    switch (xmlTextReaderNodeType (&reader))
        {
        case BeXmlReader::NODE_TYPE_Document:
            {
            xmlDocPtr doc = xmlTextReaderCurrentDoc (&reader);
            return (-1 != xmlTextWriterStartDocument (m_writer, (CharCP)doc->version, (CharCP)doc->encoding, doc->standalone ? "yes" : "no")) ? BEXML_Success : BEXML_CantWrite;
            }

        case BeXmlReader::NODE_TYPE_Element:
            return (-1 != xmlTextWriterStartElement (m_writer, xmlTextReaderConstName (&reader))) ? BEXML_Success : BEXML_CantWrite;

        case BeXmlReader::NODE_TYPE_EndElement:
            return (-1 != xmlTextWriterEndElement (m_writer)) ? BEXML_Success : BEXML_CantWrite;

        case BeXmlReader::NODE_TYPE_Attribute:
            return (-1 != xmlTextWriterWriteAttribute (m_writer, xmlTextReaderConstName (&reader), xmlTextReaderConstValue (&reader))) ? BEXML_Success : BEXML_CantWrite;

        case BeXmlReader::NODE_TYPE_Text:
        case BeXmlReader::NODE_TYPE_Whitespace:
        case BeXmlReader::NODE_TYPE_SignificantWhitespace:
            return (-1 != xmlTextWriterWriteString (m_writer, xmlTextReaderConstValue (&reader))) ? BEXML_Success : BEXML_CantWrite;

        case BeXmlReader::NODE_TYPE_CDATA:
            return (-1 != xmlTextWriterWriteCDATA (m_writer, xmlTextReaderConstValue (&reader))) ? BEXML_Success : BEXML_CantWrite;

        case BeXmlReader::NODE_TYPE_Comment:
            return (-1 != xmlTextWriterWriteComment (m_writer, xmlTextReaderConstValue (&reader))) ? BEXML_Success : BEXML_CantWrite;

        case BeXmlReader::NODE_TYPE_None:
        case BeXmlReader::NODE_TYPE_XmlDeclaration:
            return BEXML_Success;

        case BeXmlReader::NODE_TYPE_EntityReference:
        case BeXmlReader::NODE_TYPE_Entity:
        case BeXmlReader::NODE_TYPE_ProcessingInstruction:
        case BeXmlReader::NODE_TYPE_DocumentType:
        case BeXmlReader::NODE_TYPE_DocumentFragment:
        case BeXmlReader::NODE_TYPE_Notation:
        case BeXmlReader::NODE_TYPE_EndEntity:
        default:
            BeAssert (false && L"Not implemented.");
            return BEXML_UnimplementedType;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2013
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::WriteDocumentStart(xmlCharEncoding encoding)
    {
    return ((-1 != xmlTextWriterStartDocument(m_writer, NULL, xmlGetCharEncodingName(encoding), NULL)) ? BEXML_Success : BEXML_CantWrite);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::_WriteElementStart (Utf8CP elementName)
    {
    if ( (NULL == elementName) || (0 == *elementName) )
        return BEXML_ArgumentError;

    return (-1 != xmlTextWriterStartElement (m_writer, (xmlChar const*)elementName)) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::_WriteElementStart (Utf8CP elementName, Utf8CP namespaceURI)
    {
    if ( (NULL == elementName) || (0 == *elementName) )
        return BEXML_ArgumentError;

    return (-1 != xmlTextWriterStartElementNS (m_writer, NULL, (xmlChar const*)elementName, (xmlChar const*)namespaceURI)) ? BEXML_Success : BEXML_CantWrite;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::_WriteElementEnd ()
    {
    return (-1 != xmlTextWriterEndElement (m_writer)) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::_WriteAttribute (Utf8CP attributeName, WCharCP _value)
    {
    if ((NULL == attributeName) || (0 == *attributeName) || (NULL == _value) || (0 == *_value) )
        return BEXML_ArgumentError;

    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, _value);

    return (-1 != xmlTextWriterWriteAttribute (m_writer, (xmlChar const*)attributeName, (xmlChar const*)value.c_str ())) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2013
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::_WriteAttribute(Utf8CP name, Utf8CP value)
    {
    if ((NULL == name) || (0 == *name) || (NULL == value) || (0 == *value) )
        return BEXML_ArgumentError;

    return (-1 != xmlTextWriterWriteAttribute (m_writer, (xmlChar const*)name, (xmlChar const*)value)) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     12/2015
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::WriteEmptyAttribute(Utf8CP name)
    {
    return (-1 != xmlTextWriterWriteAttribute(m_writer, (xmlChar const*)name, (xmlChar const*)"")) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::_WriteText (WCharCP _value)
    {
    if (WString::IsNullOrEmpty (_value))
        return BEXML_ArgumentError;

    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, _value);

    return (-1 != xmlTextWriterWriteString (m_writer, (xmlChar const*)value.c_str ())) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2013
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::_WriteText(Utf8CP value)
    {
    if (Utf8String::IsNullOrEmpty (value))
        return BEXML_ArgumentError;

    return (-1 != xmlTextWriterWriteString (m_writer, (xmlChar const*)value)) ? BEXML_Success : BEXML_CantWrite;
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>07/2015</date>
//---------------+---------------+---------------+---------------+---------------+-----
BeXmlStatus IBeXmlWriter::WriteAttribute(Utf8CP name, int32_t value)
    {
    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%d", value);

    return _WriteAttribute(name, valueString);
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>07/2015</date>
//---------------+---------------+---------------+---------------+---------------+-----
BeXmlStatus IBeXmlWriter::WriteAttribute(Utf8CP name, uint32_t value)
    {
    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%u", value);

    return _WriteAttribute(name, valueString);
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>07/2015</date>
//---------------+---------------+---------------+---------------+---------------+-----
BeXmlStatus IBeXmlWriter::WriteAttribute(Utf8CP name, uint64_t value)
    {
    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%llu", value);

    return _WriteAttribute(name, valueString);
    }

//-------------------------------------------------------------------------------------
/// <author>Carole.MacDonald</author>                     <date>07/2015</date>
//---------------+---------------+---------------+---------------+---------------+-----
BeXmlStatus IBeXmlWriter::WriteAttribute(Utf8CP name, double value)
    {
    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%lg", value);

    return _WriteAttribute(name, valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::WriteRaw (WCharCP _value)
    {
    if (WString::IsNullOrEmpty (_value))
        return BEXML_ArgumentError;

    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, _value);

    return (-1 != xmlTextWriterWriteRaw (m_writer, (xmlChar const*)value.c_str ())) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Carole.MacDonald 07/2015
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::WriteRaw (Utf8CP _value)
    {
    if (Utf8String::IsNullOrEmpty (_value))
        return BEXML_ArgumentError;

    return (-1 != xmlTextWriterWriteRaw (m_writer, (xmlChar const*)_value)) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2013
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::WriteCDataSafe(Utf8CP _value)
    {
    Utf8String value(_value);
    value.ReplaceAll("]]>", "]]]]><![CDATA[>");

    return (-1 != xmlTextWriterWriteCDATA(m_writer, (xmlChar const*)value.c_str())) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::WriteComment(Utf8CP value)
    {
    return (-1 != xmlTextWriterWriteComment(m_writer, (xmlChar const*)value)) ? BEXML_Success : BEXML_CantWrite;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
BeXmlStatus BeXmlWriter::SetIndentation(int indent)
    {
    if (-1 == xmlTextWriterSetIndent(m_writer, (indent > 0)))
        return BEXML_CantWrite;

    if (indent <= 0)
        return BEXML_Success;

    Utf8String indentString(indent, ' ');

    if (-1 == xmlTextWriterSetIndentString(m_writer, (xmlChar const*)indentString.c_str()))
        return BEXML_CantWrite;

    return BEXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     06/2011
//---------------------------------------------------------------------------------------
void BeXmlWriter::ToString (WStringR buffer)
    {
    Utf8String utf8Buffer;
    ToString(utf8Buffer);

    buffer.AssignUtf8(utf8Buffer.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     04/2013
//---------------------------------------------------------------------------------------
void BeXmlWriter::ToString (Utf8StringR buffer)
    {
    if (NULL == m_buffer)
        {
        BeAssert (false); // Should not call ToString on a file-based writer; no string can be returned.
        buffer.clear ();
        return;
        }
    xmlTextWriterEndDocument (m_writer);
    xmlTextWriterFlush(m_writer);

    xmlChar const*  effectiveContent    = m_buffer->content;
    size_t          effectiveBufferSize = m_buffer->use;

    buffer.resize(effectiveBufferSize / sizeof (char));
    memcpy(&buffer[0], effectiveContent, effectiveBufferSize);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult IBeXmlReader::ReadTo(NodeType nodeType)
    {
    return _ReadTo(nodeType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult IBeXmlReader::ReadTo(NodeType nodeType, Utf8CP name, bool stayInCurrentElement, WStringP value)
    {
    return _ReadTo(nodeType, name, stayInCurrentElement, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
IBeXmlReader::NodeType IBeXmlReader::GetCurrentNodeType()
    {
    return _GetCurrentNodeType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlReader::GetCurrentNodeName(Utf8StringR nodeName)
    {
    return _GetCurrentNodeName(nodeName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlReader::GetCurrentNodeValue(WStringR nodeValue)
    {
    return _GetCurrentNodeValue(nodeValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlReader::GetCurrentNodeValue(Utf8StringR nodeValue)
    {
    return _GetCurrentNodeValue(nodeValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult IBeXmlReader::Read()
    {
    return _Read();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlReader::ReadToNextAttribute(Utf8StringP name, WStringP value)
    {
    return _ReadToNextAttribute(name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlReader::ReadToNextAttribute(Utf8StringP name, Utf8StringP value)
    {
    return _ReadToNextAttribute(name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
IBeXmlReader::NodeType IBeXmlReader::MoveToContent()
    {
    return _MoveToContent();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlReader::ReadResult IBeXmlReader::ReadToEndOfElement()
    {
    return _ReadToEndOfElement();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlReader::ReadContentAsString(Utf8StringR str)
    {
    return _ReadContentAsString(str);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
bool IBeXmlReader::IsEmptyElement()
    {
    return _IsEmptyElement();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlWriter::WriteElementStart(Utf8CP name)
    {
    return _WriteElementStart(name);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlWriter::WriteElementStart(Utf8CP name, Utf8CP namespaceURI)
    {
    return _WriteElementStart(name, namespaceURI);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlWriter::WriteElementEnd()
    {
    return _WriteElementEnd();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlWriter::WriteText(Utf8CP text)
    {
    return _WriteText(text);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlWriter::WriteText(WCharCP text)
    {
    return _WriteText(text);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlWriter::WriteAttribute(Utf8CP name, Utf8CP value)
    {
    return _WriteAttribute(name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                              Carole.MacDonald     09/2014
//---------------------------------------------------------------------------------------
BeXmlStatus IBeXmlWriter::WriteAttribute(Utf8CP name, WCharCP value)
    {
    return _WriteAttribute(name, value);
    }
POP_DISABLE_DEPRECATION_WARNINGS
