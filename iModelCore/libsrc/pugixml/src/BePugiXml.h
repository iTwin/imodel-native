/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <pugixml/src/pugixml.hpp>
#include <Bentley/Bentley.h>
#include <Bentley/BeStringUtilities.h>
#include <Bentley/RefCounted.h>
#include <Bentley/NonCopyableClass.h>
#include <Bentley/WString.h>
#include <Bentley/bvector.h>

#if defined (__PUGIXML_BUILD__)
    #define BEPUGIXML_EXPORT EXPORT_ATTRIBUTE
#else
    #define BEPUGIXML_EXPORT IMPORT_ATTRIBUTE
#endif

BENTLEY_NAMESPACE_TYPEDEFS (BePugiXmlWriter)
BENTLEY_NAMESPACE_TYPEDEFS (BePugiXmlDom)

BEGIN_BENTLEY_NAMESPACE

typedef RefCountedPtr<BePugiXmlWriter> BePugiXmlWriterPtr;
typedef RefCountedPtr<BePugiXmlDom>    BePugiXmlDomPtr;

//! Possible status values returned from BePugiXml methods.
#define BEPUGIXML_ERROR_BASE    0x2000
enum BePugiXmlStatus
    {
    BEPUGIXML_Success           = 0,
    BEPUGIXML_ArgumentError     = BEPUGIXML_ERROR_BASE + 1,
    BEPUGIXML_CantWrite         = BEPUGIXML_ERROR_BASE + 2,
    BEPUGIXML_NodeNotFound      = BEPUGIXML_ERROR_BASE + 3,
    BEPUGIXML_ContentWrongType  = BEPUGIXML_ERROR_BASE + 4,
    BEPUGIXML_ParseError        = BEPUGIXML_ERROR_BASE + 5,
    BEPUGIXML_FileNotFound      = BEPUGIXML_ERROR_BASE + 6,
    };

//! Character encoding options for the XML document declaration.
enum BePugiXmlCharEncoding
    {
    BEPUGIXML_CHAR_ENCODING_Utf8,
    BEPUGIXML_CHAR_ENCODING_Utf16LE,
    };

//=======================================================================================
//! A value-type wrapper around pugi::xml_node that provides a BeXml-compatible API.
//! Supports nullptr comparison and operator-> so that consumer code using BeXmlNodeP
//! can migrate with minimal changes (replace BeXmlNodeP with BePugiXmlNode, and
//! BEXML_Success with BEPUGIXML_Success).
// @bsiclass
//=======================================================================================
struct BePugiXmlNode
    {
    pugi::xml_node m_node;

    BePugiXmlNode () = default;
    BePugiXmlNode (std::nullptr_t) {}
    BePugiXmlNode (pugi::xml_node node) : m_node (node) {}

    //! Allow -> syntax on value type so node->Method() works like BeXmlNodeP.
    BePugiXmlNode*       operator-> ()       { return this; }
    BePugiXmlNode const* operator-> () const { return this; }

    //! nullptr comparison so "nullptr == node" and "nullptr != node" patterns work.
    friend bool operator== (BePugiXmlNode const& n, std::nullptr_t) { return n.m_node.empty (); }
    friend bool operator== (std::nullptr_t, BePugiXmlNode const& n) { return n.m_node.empty (); }
    friend bool operator!= (BePugiXmlNode const& n, std::nullptr_t) { return !n.m_node.empty (); }
    friend bool operator!= (std::nullptr_t, BePugiXmlNode const& n) { return !n.m_node.empty (); }

    //! Gets the first child element node.
    BEPUGIXML_EXPORT BePugiXmlNode GetFirstChild ();

    //! Gets the next sibling element node.
    BEPUGIXML_EXPORT BePugiXmlNode GetNextSibling ();

    //! Gets the node name.
    Utf8CP GetName () { return m_node.name (); }

    //! Gets the text content of this node, or of a child found via relativePath.
    //! @returns BEPUGIXML_NodeNotFound if relativePath is non-null and not found, or BEPUGIXML_Success.
    BEPUGIXML_EXPORT BePugiXmlStatus GetContent (Utf8StringR result, Utf8CP relativePath = nullptr);

    //! Gets the text content as a double.
    //! @returns BEPUGIXML_ContentWrongType if the content cannot be parsed as a double.
    BEPUGIXML_EXPORT BePugiXmlStatus GetContentDoubleValue (double& result, Utf8CP relativePath = nullptr);

    //! Gets the text content as a uint32_t. Detects "0x"/"0X" prefix for hex values.
    //! @returns BEPUGIXML_ContentWrongType if the content cannot be parsed.
    BEPUGIXML_EXPORT BePugiXmlStatus GetContentUInt32Value (uint32_t& result, Utf8CP relativePath = nullptr);

    //! Gets the text content as an int32_t.
    //! @returns BEPUGIXML_ContentWrongType if the content cannot be parsed.
    BEPUGIXML_EXPORT BePugiXmlStatus GetContentInt32Value (int32_t& result, Utf8CP relativePath = nullptr);

private:
    static void collectText (pugi::xml_node node, Utf8StringR out);

public:
    //! Gets the string value of an attribute.
    //! @returns BEPUGIXML_NodeNotFound if the attribute does not exist.
    BEPUGIXML_EXPORT BePugiXmlStatus GetAttributeStringValue (Utf8StringR result, Utf8CP attributeName);
    };

//=======================================================================================
//! A RefCounted wrapper around pugi::xml_document that provides a BeXmlDom-compatible API.
//! Use BePugiXmlDomPtr (RefCountedPtr<BePugiXmlDom>) as the smart pointer type.
// @bsiclass
//=======================================================================================
struct BePugiXmlDom : RefCountedBase, NonCopyableClass
    {
private:
    pugi::xml_document m_doc;
    BePugiXmlDom () {}

public:
    //! Parses a UTF-8 XML string and returns a DOM wrapper.
    //! @param[out] status  BEPUGIXML_Success or BEPUGIXML_ParseError.
    //! @param[in]  source  The UTF-8 string to parse.
    //! @return nullptr if parsing failed; a valid DOM wrapper otherwise.
    BEPUGIXML_EXPORT static BePugiXmlDomPtr CreateAndReadFromString (BePugiXmlStatus& status, Utf8CP source, size_t characterCount = 0);

    //! Parses an XML file and returns a DOM wrapper.
    //! @param[out] status  BEPUGIXML_Success, BEPUGIXML_FileNotFound, or BEPUGIXML_ParseError.
    //! @param[in]  fileName  The UTF-8 file path.
    //! @return nullptr if parsing failed; a valid DOM wrapper otherwise.
    BEPUGIXML_EXPORT static BePugiXmlDomPtr CreateAndReadFromFile (BePugiXmlStatus& status, Utf8CP fileName);

    //! Gets the root element of the document. Returns an empty BePugiXmlNode if there is no root.
    BEPUGIXML_EXPORT BePugiXmlNode GetRootElement ();
    };

//=======================================================================================
//! A streaming-style XML writer backed by pugixml. Builds a DOM internally and serializes
//! to string or file on demand.
// @bsiclass
//=======================================================================================
struct BePugiXmlWriter : RefCountedBase, NonCopyableClass
    {
private:
    pugi::xml_document          m_doc;
    bvector<pugi::xml_node>     m_nodeStack;
    BePugiXmlCharEncoding       m_encoding;
    bool                        m_hasDocumentDecl;
    Utf8String                  m_indentString;
    bool                        m_indent;
    Utf8String                  m_filePath;
    bool                        m_isFileWriter;

    BePugiXmlWriter ();

    BePugiXmlStatus writeAttributeImpl (Utf8CP name, Utf8CP value);

public:
    //! Creates a writer that writes to an in-memory buffer.
    BEPUGIXML_EXPORT static BePugiXmlWriterPtr Create ();

    //! Creates a writer that writes to a file.
    BEPUGIXML_EXPORT static BePugiXmlWriterPtr CreateFileWriter (WCharCP fileName);

    //! Writes the XML declaration with the specified encoding.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteDocumentStart (BePugiXmlCharEncoding encoding);

    //! Writes the start of an element node.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteElementStart (Utf8CP name);

    //! Writes the start of an element node with a namespace URI (xmlns attribute).
    BEPUGIXML_EXPORT BePugiXmlStatus WriteElementStart (Utf8CP name, Utf8CP namespaceURI);

    //! Writes the end of the current element node.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteElementEnd ();

    //! Writes a text node.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteText (WCharCP text);

    //! Writes a text node.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteText (Utf8CP text);

    //! Writes an attribute with a string value.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteAttribute (Utf8CP name, WCharCP value);

    //! Writes an attribute with a string value.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteAttribute (Utf8CP name, Utf8CP value);

    //! Writes an attribute with a boolean value ("true" or "false").
    BePugiXmlStatus WriteAttribute (Utf8CP name, bool value) { return writeAttributeImpl (name, value ? "true" : "false"); }

    //! Writes an attribute with an int32_t value.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteAttribute (Utf8CP name, int32_t value);

    //! Writes an attribute with a uint32_t value.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteAttribute (Utf8CP name, uint32_t value);

    //! Writes an attribute with a uint64_t value.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteAttribute (Utf8CP name, uint64_t value);

    //! Writes an attribute with a double value.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteAttribute (Utf8CP name, double value);

    //! Sets the indentation. 0 disables indentation and line breaks.
    BEPUGIXML_EXPORT BePugiXmlStatus SetIndentation (int indent);

    //! Writes raw XML content into the current element.
    BEPUGIXML_EXPORT BePugiXmlStatus WriteRaw (Utf8CP rawXml);

    //! Writes an attribute with an empty value.
    BePugiXmlStatus WriteEmptyAttribute (Utf8CP name) { return writeAttributeImpl (name, ""); }

    //! Generates a WString representation of the XML.
    //! @note Returns an empty string for a file-based writer.
    BEPUGIXML_EXPORT void ToString (WStringR buffer);

    //! Generates a UTF-8 string representation of the XML.
    //! @note Returns an empty string for a file-based writer.
    BEPUGIXML_EXPORT void ToString (Utf8StringR buffer);
    };

END_BENTLEY_NAMESPACE
