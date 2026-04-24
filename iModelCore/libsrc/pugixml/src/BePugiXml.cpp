/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <pugixml/src/BePugiXml.h>
#include <sstream>

BEGIN_BENTLEY_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlWriter::BePugiXmlWriter () :
    m_encoding          (BEPUGIXML_CHAR_ENCODING_Utf8),
    m_hasDocumentDecl   (false),
    m_indent            (false),
    m_isFileWriter      (false)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlWriterPtr BePugiXmlWriter::Create ()
    {
    return new BePugiXmlWriter ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlWriterPtr BePugiXmlWriter::CreateFileWriter (WCharCP filePath)
    {
    if (nullptr == filePath || 0 == *filePath)
        return nullptr;

    BePugiXmlWriterP writer = new BePugiXmlWriter ();
    BeStringUtilities::WCharToUtf8 (writer->m_filePath, filePath);
    writer->m_isFileWriter = true;
    return writer;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteDocumentStart (BePugiXmlCharEncoding encoding)
    {
    m_encoding = encoding;
    m_hasDocumentDecl = true;

    pugi::xml_node decl = m_doc.prepend_child (pugi::node_declaration);
    if (decl.empty ())
        return BEPUGIXML_CantWrite;

    decl.append_attribute ("version").set_value ("1.0");

    if (encoding == BEPUGIXML_CHAR_ENCODING_Utf16LE)
        decl.append_attribute ("encoding").set_value ("UTF-16");
    else
        decl.append_attribute ("encoding").set_value ("UTF-8");

    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteElementStart (Utf8CP name)
    {
    if (nullptr == name || 0 == *name)
        return BEPUGIXML_ArgumentError;

    pugi::xml_node parent = m_nodeStack.empty () ? m_doc : m_nodeStack.back ();
    pugi::xml_node child = parent.append_child (name);
    if (child.empty ())
        return BEPUGIXML_CantWrite;

    m_nodeStack.push_back (child);
    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteElementStart (Utf8CP name, Utf8CP namespaceURI)
    {
    BePugiXmlStatus status = WriteElementStart (name);
    if (BEPUGIXML_Success != status)
        return status;

    if (nullptr != namespaceURI && 0 != *namespaceURI)
        m_nodeStack.back ().append_attribute ("xmlns").set_value (namespaceURI);

    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteElementEnd ()
    {
    if (m_nodeStack.empty ())
        return BEPUGIXML_CantWrite;

    m_nodeStack.pop_back ();
    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteText (Utf8CP text)
    {
    if (Utf8String::IsNullOrEmpty (text))
        return BEPUGIXML_ArgumentError;

    if (m_nodeStack.empty ())
        return BEPUGIXML_CantWrite;

    pugi::xml_node textNode = m_nodeStack.back ().append_child (pugi::node_pcdata);
    if (textNode.empty ())
        return BEPUGIXML_CantWrite;

    textNode.set_value (text);
    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteText (WCharCP text)
    {
    if (WString::IsNullOrEmpty (text))
        return BEPUGIXML_ArgumentError;

    Utf8String utf8;
    BeStringUtilities::WCharToUtf8 (utf8, text);
    return WriteText (utf8.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::writeAttributeImpl (Utf8CP name, Utf8CP value)
    {
    if (nullptr == name || 0 == *name || nullptr == value)
        return BEPUGIXML_ArgumentError;

    if (m_nodeStack.empty ())
        return BEPUGIXML_CantWrite;

    m_nodeStack.back ().append_attribute (name).set_value (value);
    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteAttribute (Utf8CP name, Utf8CP value)
    {
    if (nullptr == name || 0 == *name || nullptr == value || 0 == *value)
        return BEPUGIXML_ArgumentError;

    return writeAttributeImpl (name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteAttribute (Utf8CP name, WCharCP value)
    {
    if (nullptr == name || 0 == *name || nullptr == value || 0 == *value)
        return BEPUGIXML_ArgumentError;

    Utf8String utf8;
    BeStringUtilities::WCharToUtf8 (utf8, value);
    return writeAttributeImpl (name, utf8.c_str ());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteAttribute (Utf8CP name, int32_t value)
    {
    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%d", value);
    return writeAttributeImpl (name, valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteAttribute (Utf8CP name, uint32_t value)
    {
    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%u", value);
    return writeAttributeImpl (name, valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteAttribute (Utf8CP name, uint64_t value)
    {
    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%llu", value);
    return writeAttributeImpl (name, valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteAttribute (Utf8CP name, double value)
    {
    char valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%.17g", value);
    return writeAttributeImpl (name, valueString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::SetIndentation (int indent)
    {
    if (indent <= 0)
        {
        m_indent = false;
        m_indentString.clear ();
        return BEPUGIXML_Success;
        }

    m_indent = true;
    m_indentString.assign ((size_t) indent, ' ');
    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BePugiXmlWriter::ToString (Utf8StringR buffer)
    {
    if (m_isFileWriter)
        {
        m_doc.save_file (m_filePath.c_str (),
            m_indent ? m_indentString.c_str () : "",
            m_indent ? pugi::format_indent : pugi::format_raw,
            pugi::encoding_utf8);
        buffer.clear ();
        return;
        }

    std::ostringstream stream;
    unsigned int flags = m_indent ? pugi::format_indent : pugi::format_raw;
    if (!m_hasDocumentDecl)
        flags |= pugi::format_no_declaration;

    m_doc.save (stream, m_indent ? m_indentString.c_str () : "", flags, pugi::encoding_utf8);
    buffer = stream.str ().c_str ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BePugiXmlWriter::ToString (WStringR buffer)
    {
    Utf8String utf8;
    ToString (utf8);
    buffer.AssignUtf8 (utf8.c_str ());
    }

END_BENTLEY_NAMESPACE
