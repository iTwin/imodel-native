/*--------------------------------------------------------------------------------------+
|
|     $Source: src/MSXmlBinary/MSXmlBinaryWriter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECObjectsPch.h"
#include "MSXmlBinaryWriter.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//static
Utf8CP const MSXmlBinaryWriter::s_defaultNamespace = "http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0";

MSXmlBinaryWriter::MSXmlBinaryWriter()
    {
    m_writeState = WriteState::Start;
    m_depth = 0;
    m_textNodeOffset = -1;
    }

MSXmlBinaryWriter::~MSXmlBinaryWriter()
    {
    for (auto element : m_elements)
        delete element;
    }

BeXmlStatus MSXmlBinaryWriter::WriteElementStart(Utf8CP name)
    {
    return WriteElementStart(name, m_depth == 0 ? s_defaultNamespace : nullptr);
    }

BeXmlStatus MSXmlBinaryWriter::WriteElementStart(Utf8CP name, Utf8CP nameSpace)
    {
    StartElement(name, nameSpace);
    WriteNode(XmlBinaryNodeType::ShortElement);
    WriteName(name);
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteElementEnd()
    {
    if (WriteState::Element == m_writeState)
        {
        WriteEndStartElement(true);
        }
    else
        {
        WriteEndElement();
        }
    ExitScope();
    m_writeState = WriteState::Content;
    return BEXML_Success;
    }

void MSXmlBinaryWriter::WriteEndElement()
    {
    if (m_textNodeOffset != -1)
        {
        XmlBinaryNodeType nodeType = (XmlBinaryNodeType) m_buffer[m_textNodeOffset];
        m_buffer[m_textNodeOffset] = (byte) ((int) nodeType + 1);
        m_textNodeOffset = -1;
        }
    else
        WriteNode(XmlBinaryNodeType::EndElement);
    }

void MSXmlBinaryWriter::GetBytes(bvector<byte>& bytes)
    {
    for (auto b : m_buffer)
        bytes.push_back(b);
    }

void MSXmlBinaryWriter::WriteNode(XmlBinaryNodeType nodeType)
    {
    WriteByte((byte) nodeType);
    m_textNodeOffset = -1;
    }

void MSXmlBinaryWriter::WriteByte(byte b)
    {
    m_buffer.push_back(b);
    }

void MSXmlBinaryWriter::WriteByte(char c)
    {
    BeAssert(c < 0x80);
    WriteByte((byte) c);
    }

void MSXmlBinaryWriter::WriteName(Utf8CP name)
    {
    for (int i = 0; i < strlen(name); i++)
        {
        WriteByte((byte) name[i]);
        }
    }

BeXmlStatus MSXmlBinaryWriter::WriteText(WCharCP text)
    {
    if (WString::IsNullOrEmpty (text))
        return BEXML_ArgumentError;

    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, text);

    return WriteText((Utf8Char const*) value.c_str());
    }

BeXmlStatus MSXmlBinaryWriter::WriteText(Utf8CP text)
    {
    StartContent(text);

    for (int i = 0; i < strlen(text); i++)
        {
        WriteByte((byte) text[i]);
        }
    WriteByte((byte) XmlBinaryNodeType::Chars8Text);
    m_textNodeOffset = (int) m_buffer.size();
    m_buffer.push_back((byte) m_buffer.size());
    EndContent();
    return BEXML_Success;
    }

void MSXmlBinaryWriter::StartContent(Utf8CP content)
    {
    FlushElement();
    }

void MSXmlBinaryWriter::EndContent()
    {

    }

void MSXmlBinaryWriter::FlushElement()
    {
    if (m_writeState == WriteState::Element)
        {
        AutoComplete(WriteState::Content);
        }
    }

void MSXmlBinaryWriter::AutoComplete(WriteState writeState)
    {
    if (m_writeState == WriteState::Element)
        {
        EndStartElement();
        }
    m_writeState = writeState;
    }

void MSXmlBinaryWriter::StartElement(Utf8StringCR localName, Utf8StringCR nameSpace)
    {
    AutoComplete(WriteState::Element);
    Element* element = EnterScope();
    element->LocalName = localName;

    }

void MSXmlBinaryWriter::EndStartElement()
    {
    WriteEndStartElement(false);
    }

void MSXmlBinaryWriter::WriteEndStartElement(bool isEmpty)
    {
    if (isEmpty)
        WriteElementEnd();
    }

MSXmlBinaryWriter::Element* MSXmlBinaryWriter::EnterScope()
    {
    m_depth++;
    if (m_depth > m_elements.size())
        {
        while (m_elements.size() < m_depth)
            m_elements.push_back(nullptr);
        m_elements[m_depth] = new MSXmlBinaryWriter::Element();
        }
    return m_elements[m_depth];
    }

void MSXmlBinaryWriter::ExitScope()
    {
    m_elements[m_depth]->Clear();
    m_depth--;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
