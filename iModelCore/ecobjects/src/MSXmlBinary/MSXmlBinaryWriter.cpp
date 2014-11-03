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

BeXmlStatus MSXmlBinaryWriter::WriteElementStart(Utf8CP name)
    {
    return WriteElementStart(name, m_depth == 0 ? s_defaultNamespace : nullptr);
    }
    
BeXmlStatus MSXmlBinaryWriter::WriteArrayElementStart(Utf8CP longName, Utf8CP shortName)
    {
    return WriteElementStart (longName);
    }

BeXmlStatus MSXmlBinaryWriter::WriteSetElementStart(Utf8CP name)
    {
    return WriteElementStart (name);
    }        

BeXmlStatus MSXmlBinaryWriter::WriteShortElementStart(Utf8CP name)
    {
    return WriteElementStart (name);
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
    WriteByte((byte) strlen(name));
    for (int i = 0; i < (int) strlen(name); i++)
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
    StartContent();

    m_textNodeOffset = (int) m_buffer.size();
    WriteByte((byte) XmlBinaryNodeType::Chars8Text);
    m_buffer.push_back((byte) strlen(text));
    for (int i = 0; i < (int) strlen(text); i++)
        {
        WriteByte((byte) text[i]);
        }
    EndContent();
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteCommaSeparatedNumerics(Utf8CP text)
    {
    return WriteText (text);
    }


BeXmlStatus MSXmlBinaryWriter::WriteDoubleText(double data)
    {
    float f;
    float fMax = std::numeric_limits<float>::max();
    if (data >= -fMax && data <= fMax && (f = (float)data) == data)
        WriteFloatText(f);
    else
        {
        StartContent();
        m_textNodeOffset = (int) m_buffer.size();
        WriteByte((byte)XmlBinaryNodeType::DoubleText);
        byte* bytes = reinterpret_cast<byte*>(&data);
        for (size_t i = 0; i < sizeof(double); i++)
            WriteByte(bytes[i]);
        EndContent();
        }
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteFloatText(float data)
    {
    Int64 l;
    if (data >= std::numeric_limits<Int64>::min() && data <= std::numeric_limits<Int64>::max() && (l = (Int64)data) == data)
        WriteInt64Text(l);
    else
        {
        StartContent();
        m_textNodeOffset = (int) m_buffer.size();
        WriteByte((byte)XmlBinaryNodeType::FloatText);
        byte* bytes = reinterpret_cast<byte*>(&data);
        for (int i = 0; i < sizeof(float); i++)
            WriteByte(bytes[i]);
        EndContent();
        }
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteInt64Text(Int64 data)
    {
    if (data >= std::numeric_limits<Int32>::min() && data <= std::numeric_limits<Int32>::max())
        return WriteInt32Text((Int32) data);
    else
        {
        StartContent();
        m_textNodeOffset = (int) m_buffer.size();
        WriteByte((byte)XmlBinaryNodeType::Int64Text);
        byte* bytes = reinterpret_cast<byte*>(&data);
        for (int i = 0; i < sizeof(Int64); i++)
            WriteByte(bytes[i]);
        EndContent();
        }
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteInt32Text(Int32 data)
    {
    StartContent();
    m_textNodeOffset = (int) m_buffer.size();
    if (data >= -128 && data <= 128)
        {
        if (0 == data)
            WriteByte((byte)XmlBinaryNodeType::ZeroText);
        else if (1 == data)
            WriteByte((byte)XmlBinaryNodeType::OneText);
        else
            {
            WriteByte((byte)XmlBinaryNodeType::Int8Text);
            WriteByte((byte) data);
            }
        }
    else if (data >= -32768 && data < 32768)
        {
        WriteByte((byte)XmlBinaryNodeType::Int16Text);
        WriteByte((byte) data);
        data >>= 8;
        WriteByte((byte) data);
        }
    else
        {
        WriteByte((byte)XmlBinaryNodeType::Int32Text);
        WriteByte((byte) data);
        data >>= 8;
        WriteByte((byte) data);
        data >>= 8;
        WriteByte((byte) data);
        data >>= 8;
        WriteByte((byte) data);
        }
    EndContent();
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteBoolText(bool value)
    {
    StartContent();
    m_textNodeOffset = (int) m_buffer.size();
    if (value)
        WriteByte((byte)XmlBinaryNodeType::TrueText);
    else
        WriteByte((byte)XmlBinaryNodeType::FalseText);
    EndContent();
    return BEXML_Success;
    }

void MSXmlBinaryWriter::StartContent()
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
    Element& element = EnterScope();
    if (!Utf8String::IsNullOrEmpty(nameSpace.c_str()))
        m_nsMgr.AddNamespace(nameSpace);
    element.LocalName = localName;

    }

void MSXmlBinaryWriter::EndStartElement()
    {
    m_nsMgr.DeclareNamespace(this);
    WriteEndStartElement(false);
    }

void MSXmlBinaryWriter::WriteEndStartElement(bool isEmpty)
    {
    if (isEmpty)
        WriteElementEnd();
    }

MSXmlBinaryWriter::Element& MSXmlBinaryWriter::EnterScope()
    {
    m_nsMgr.EnterScope();
    m_depth++;
    if (m_depth >= (int) m_elements.size())
        {
        while ((int) m_elements.size() <= m_depth)
            {
            Element el;
            m_elements.push_back(el);
            }
        }
    return m_elements[m_depth];
    }

void MSXmlBinaryWriter::ExitScope()
    {
    m_elements[m_depth].Clear();
    m_depth--;
    }

void MSXmlBinaryWriter::WriteXmlnsAttribute(Utf8CP ns)
    {
    WriteNode(XmlBinaryNodeType::ShortXmlnsAttribute);
    WriteName(ns);
    }
END_BENTLEY_ECOBJECT_NAMESPACE
