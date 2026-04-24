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

BEGIN_BENTLEY_NAMESPACE

typedef RefCountedPtr<BePugiXmlWriter> BePugiXmlWriterPtr;

//! Possible status values returned from BePugiXml methods.
#define BEPUGIXML_ERROR_BASE    0x2000
enum BePugiXmlStatus
    {
    BEPUGIXML_Success           = 0,
    BEPUGIXML_ArgumentError     = BEPUGIXML_ERROR_BASE + 1,
    BEPUGIXML_CantWrite         = BEPUGIXML_ERROR_BASE + 2,
    };

//! Character encoding options for the XML document declaration.
enum BePugiXmlCharEncoding
    {
    BEPUGIXML_CHAR_ENCODING_Utf8,
    BEPUGIXML_CHAR_ENCODING_Utf16LE,
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

    //! Generates a WString representation of the XML.
    //! @note Returns an empty string for a file-based writer.
    BEPUGIXML_EXPORT void ToString (WStringR buffer);

    //! Generates a UTF-8 string representation of the XML.
    //! @note Returns an empty string for a file-based writer.
    BEPUGIXML_EXPORT void ToString (Utf8StringR buffer);
    };

END_BENTLEY_NAMESPACE
