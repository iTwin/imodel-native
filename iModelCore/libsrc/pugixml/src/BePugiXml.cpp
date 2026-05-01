/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <pugixml/src/BePugiXml.h>
#include <Bentley/BeFile.h>
#include <sstream>

BEGIN_BENTLEY_NAMESPACE

//======================================================================================
// BePugiXmlNode
//======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlNode BePugiXmlNode::GetFirstChild ()
    {
    for (pugi::xml_node child = m_node.first_child (); child; child = child.next_sibling ())
        {
        if (child.type () == pugi::node_element)
            return BePugiXmlNode (child);
        }
    return BePugiXmlNode ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlNode BePugiXmlNode::GetNextSibling ()
    {
    for (pugi::xml_node sibling = m_node.next_sibling (); sibling; sibling = sibling.next_sibling ())
        {
        if (sibling.type () == pugi::node_element)
            return BePugiXmlNode (sibling);
        }
    return BePugiXmlNode ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BePugiXmlNode::collectText (pugi::xml_node node, Utf8StringR out)
    {
    for (pugi::xml_node child = node.first_child (); child; child = child.next_sibling ())
        {
        if (child.type () == pugi::node_pcdata || child.type () == pugi::node_cdata)
            out.append (child.value ());
        else if (child.type () == pugi::node_element)
            collectText (child, out);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlNode::GetContent (Utf8StringR result, Utf8CP relativePath)
    {
    result.clear ();
    pugi::xml_node targetNode = m_node;
    if (nullptr != relativePath)
        {
        targetNode = m_node.child (relativePath);
        if (targetNode.empty ())
            return BEPUGIXML_NodeNotFound;
        }

    collectText (targetNode, result);
    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlNode::GetContentDoubleValue (double& result, Utf8CP relativePath)
    {
    result = 0.0;
    Utf8String content;
    BePugiXmlStatus status = GetContent (content, relativePath);
    if (BEPUGIXML_Success != status)
        return status;

    if (content.empty ())
        return BEPUGIXML_ContentWrongType;

    if (1 != Utf8String::Sscanf_safe (content.c_str (), "%lg", &result))
        return BEPUGIXML_ContentWrongType;

    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlNode::GetContentUInt32Value (uint32_t& result, Utf8CP relativePath)
    {
    result = 0;
    Utf8String content;
    BePugiXmlStatus status = GetContent (content, relativePath);
    if (BEPUGIXML_Success != status)
        return status;

    if (content.empty ())
        return BEPUGIXML_ContentWrongType;

    Utf8CP str = content.c_str ();

    // Check for hex prefix
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        {
        if (1 != Utf8String::Sscanf_safe (str, "%x", &result))
            return BEPUGIXML_ContentWrongType;
        }
    else
        {
        if (1 != Utf8String::Sscanf_safe (str, "%u", &result))
            return BEPUGIXML_ContentWrongType;
        }

    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlNode::GetContentInt32Value (int32_t& result, Utf8CP relativePath)
    {
    result = 0;
    Utf8String content;
    BePugiXmlStatus status = GetContent (content, relativePath);
    if (BEPUGIXML_Success != status)
        return status;

    if (content.empty ())
        return BEPUGIXML_ContentWrongType;

    if (1 != Utf8String::Sscanf_safe (content.c_str (), "%d", &result))
        return BEPUGIXML_ContentWrongType;

    return BEPUGIXML_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlNode::GetAttributeStringValue (Utf8StringR result, Utf8CP attributeName)
    {
    result.clear ();
    pugi::xml_attribute attr = m_node.attribute (attributeName);
    if (attr.empty ())
        return BEPUGIXML_NodeNotFound;

    result = attr.value ();
    return BEPUGIXML_Success;
    }

//======================================================================================
// BePugiXmlDom
//======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlDomPtr BePugiXmlDom::CreateAndReadFromString (BePugiXmlStatus& status, Utf8CP source, size_t characterCount)
    {
    BePugiXmlDomP dom = new BePugiXmlDom ();
    pugi::xml_parse_result result = (0 != characterCount)
        ? dom->m_doc.load_buffer (source, characterCount)
        : dom->m_doc.load_string (source);
    if (!result)
        {
        status = BEPUGIXML_ParseError;
        return nullptr;
        }

    status = BEPUGIXML_Success;
    return dom;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlDomPtr BePugiXmlDom::CreateAndReadFromFile (BePugiXmlStatus& status, Utf8CP fileName)
    {
    BePugiXmlDomP dom = new BePugiXmlDom ();
    pugi::xml_parse_result result = dom->m_doc.load_file (fileName);
    if (!result)
        {
        status = (result.status == pugi::status_file_not_found || result.status == pugi::status_io_error)
            ? BEPUGIXML_FileNotFound : BEPUGIXML_ParseError;
        return nullptr;
        }

    status = BEPUGIXML_Success;
    return dom;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlNode BePugiXmlDom::GetRootElement ()
    {
    return BePugiXmlNode (m_doc.document_element ());
    }

//======================================================================================
// BePugiXmlWriter
//======================================================================================

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
BePugiXmlWriter::~BePugiXmlWriter ()
    {
    if (m_isFileWriter)
        flushToFile ();
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

    // Move xmlns to last attribute position to match libxml2 attribute ordering.
    pugi::xml_node node = m_nodeStack.back ();
    pugi::xml_attribute xmlnsAttr = node.attribute ("xmlns");
    if (!xmlnsAttr.empty ())
        {
        // Save value, remove, re-append at end.
        Utf8String value (xmlnsAttr.value ());
        node.remove_attribute (xmlnsAttr);
        node.append_attribute ("xmlns").set_value (value.c_str ());
        }

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
static void removeSpaceBeforeSelfClose (Utf8StringR xml)
    {
    // pugixml with format_indent writes " />" for self-closing tags.
    // libxml2 wrote "/>". Match libxml2 output so checksums stay stable.
    // Track quote state so we never alter attribute values that happen to contain " />".
    Utf8String result;
    result.reserve (xml.size ());

    bool inQuote = false;
    char quoteChar = 0;
    size_t len = xml.size ();

    for (size_t i = 0; i < len; ++i)
        {
        char c = xml[i];
        if (inQuote)
            {
            if (c == quoteChar)
                inQuote = false;
            result.push_back (c);
            }
        else
            {
            if (c == '"' || c == '\'')
                {
                inQuote = true;
                quoteChar = c;
                result.push_back (c);
                }
            else if (c == ' ' && i + 2 < len && xml[i + 1] == '/' && xml[i + 2] == '>')
                {
                // Skip the space; the '/' and '>' will be copied on subsequent iterations.
                }
            else
                {
                result.push_back (c);
                }
            }
        }

    xml = result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BePugiXmlWriter::flushToFile ()
    {
    if (m_filePath.empty ())
        return;

    // Serialize to string, post-process, then write to file.
    std::ostringstream stream;
    unsigned int flags = m_indent ? pugi::format_indent : pugi::format_raw;
    m_doc.save (stream, m_indent ? m_indentString.c_str () : "", flags, pugiEncoding ());

    Utf8String content (stream.str ().c_str ());
    removeSpaceBeforeSelfClose (content);

    BeFile file;
    if (BeFileStatus::Success == file.Create (m_filePath.c_str (), true))
        {
        uint32_t bytesWritten;
        file.Write (&bytesWritten, content.c_str (), (uint32_t) content.size ());
        file.Close ();
        }

    // Clear path so we don't write again in the destructor.
    m_filePath.clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void BePugiXmlWriter::ToString (Utf8StringR buffer)
    {
    if (m_isFileWriter)
        {
        flushToFile ();
        buffer.clear ();
        return;
        }

    std::ostringstream stream;
    unsigned int flags = m_indent ? pugi::format_indent : pugi::format_raw;
    if (!m_hasDocumentDecl)
        flags |= pugi::format_no_declaration;

    m_doc.save (stream, m_indent ? m_indentString.c_str () : "", flags, pugiEncoding ());
    buffer = stream.str ().c_str ();
    removeSpaceBeforeSelfClose (buffer);
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BePugiXmlStatus BePugiXmlWriter::WriteRaw (Utf8CP rawXml)
    {
    if (Utf8String::IsNullOrEmpty (rawXml))
        return BEPUGIXML_ArgumentError;

    if (m_nodeStack.empty ())
        return BEPUGIXML_CantWrite;

    // Parse the raw XML by wrapping in a temporary root element.
    pugi::xml_document tempDoc;
    Utf8String wrapped = Utf8String ("<_r>") + rawXml + "</_r>";
    pugi::xml_parse_result result = tempDoc.load_string (wrapped.c_str ());
    if (!result)
        return BEPUGIXML_ArgumentError;

    // Copy parsed children into the current node.
    for (pugi::xml_node child = tempDoc.document_element ().first_child (); child; child = child.next_sibling ())
        m_nodeStack.back ().append_copy (child);

    return BEPUGIXML_Success;
    }

END_BENTLEY_NAMESPACE
