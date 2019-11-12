/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "serializationPCH.h"
#include "MSXmlBinaryWriter.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//static
Utf8CP const MSXmlBinaryWriter::s_defaultNamespace = "http://www.bentley.com/schemas/Bentley.Geometry.Common.1.0";


MSXmlBinaryWriter::MSXmlBinaryWriter()
    {
    m_writeState = WriteState::Start;
    m_depth = 0;
    m_textNodeOffset = -1;
    }

BeXmlStatus MSXmlBinaryWriter::_WriteElementStart(Utf8CP name)
    {
    return _WriteElementStart(name, m_depth == 0 ? s_defaultNamespace : nullptr);
    }
    
BeXmlStatus MSXmlBinaryWriter::_WriteElementStart(Utf8CP name, Utf8CP nameSpace)
    {
    StartElement(name, nameSpace);
    WriteNode(XmlBinaryNodeType::ShortElement);
    WriteName(name);
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::_WriteElementEnd()
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
        m_buffer[m_textNodeOffset] = (Byte) ((int) nodeType + 1);
        m_textNodeOffset = -1;
        }
    else
        WriteNode(XmlBinaryNodeType::EndElement);
    }

void MSXmlBinaryWriter::GetBytes(bvector<Byte>& bytes)
    {
    for (auto b : m_buffer)
        bytes.push_back(b);
    }

void MSXmlBinaryWriter::WriteNode(XmlBinaryNodeType nodeType)
    {
    WriteByte((Byte) nodeType);
    m_textNodeOffset = -1;
    }

void MSXmlBinaryWriter::WriteByte(Byte b)
    {
    m_buffer.push_back(b);
    }

void MSXmlBinaryWriter::WriteByte(char c)
    {
    // BeAssert(c < 0x80); // comparison of constant 128 with expression of type 'char' is always true
    WriteByte((Byte) c);
    }

void MSXmlBinaryWriter::WriteName(Utf8CP name)
    {
    WriteByte((Byte) strlen(name));
    for (int i = 0; i < (int) strlen(name); i++)
        {
        WriteByte((Byte) name[i]);
        }
    }

BeXmlStatus MSXmlBinaryWriter::_WriteText(WCharCP text)
    {
    if (WString::IsNullOrEmpty (text))
        return BEXML_ArgumentError;

    Utf8String value;
    BeStringUtilities::WCharToUtf8 (value, text);

    return WriteText((Utf8Char const*) value.c_str());
    }

BeXmlStatus MSXmlBinaryWriter::_WriteText(Utf8CP text)
    {
    StartContent();

    m_textNodeOffset = (int) m_buffer.size();
    WriteByte((Byte) XmlBinaryNodeType::Chars8Text);
    m_buffer.push_back((Byte) strlen(text));
    for (int i = 0; i < (int) strlen(text); i++)
        {
        WriteByte((Byte) text[i]);
        }
    EndContent();
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteDoubleToken(double data)
    {
    float f;
    float fMax = std::numeric_limits<float>::max();
    if (data >= -fMax && data <= fMax && (f = (float)data) == data)
        WriteFloatToken(f);
    else
        {
        StartContent();
        m_textNodeOffset = (int) m_buffer.size();
        WriteByte((Byte)XmlBinaryNodeType::DoubleText);
        Byte* bytes = reinterpret_cast<Byte*>(&data);
        for (size_t i = 0; i < sizeof(double); i++)
            WriteByte(bytes[i]);
        EndContent();
        }
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteFloatToken(float data)
    {
    int64_t l;
    if (data >= std::numeric_limits<int64_t>::min() && data <= std::numeric_limits<int64_t>::max() && (l = (int64_t)data) == data)
        WriteInt64Token(l);
    else
        {
        StartContent();
        m_textNodeOffset = (int) m_buffer.size();
        WriteByte((Byte)XmlBinaryNodeType::FloatText);
        Byte* bytes = reinterpret_cast<Byte*>(&data);
        for (int i = 0; i < sizeof(float); i++)
            WriteByte(bytes[i]);
        EndContent();
        }
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteInt64Token(int64_t data)
    {
    if (data >= std::numeric_limits<int32_t>::min() && data <= std::numeric_limits<int32_t>::max())
        return WriteInt32Token((int32_t) data);
    else
        {
        StartContent();
        m_textNodeOffset = (int) m_buffer.size();
        WriteByte((Byte)XmlBinaryNodeType::Int64Text);
        Byte* bytes = reinterpret_cast<Byte*>(&data);
        for (int i = 0; i < sizeof(int64_t); i++)
            WriteByte(bytes[i]);
        EndContent();
        }
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteInt32Token(int32_t data)
    {
    StartContent();
    m_textNodeOffset = (int) m_buffer.size();
    if (data >= -128 && data <= 128)
        {
        if (0 == data)
            WriteByte((Byte)XmlBinaryNodeType::ZeroText);
        else if (1 == data)
            WriteByte((Byte)XmlBinaryNodeType::OneText);
        else
            {
            WriteByte((Byte)XmlBinaryNodeType::Int8Text);
            WriteByte((Byte) data);
            }
        }
    else if (data >= -32768 && data < 32768)
        {
        WriteByte((Byte)XmlBinaryNodeType::Int16Text);
        WriteByte((Byte) (data & 0xFF));
        data >>= 8;
        WriteByte((Byte) data);
        }
    else
        {
        WriteByte((Byte)XmlBinaryNodeType::Int32Text);
        WriteByte((Byte) (data & 0xFF));
        data >>= 8;
        WriteByte((Byte) (data & 0xFF));
        data >>= 8;
        WriteByte((Byte) (data & 0xFF));
        data >>= 8;
        WriteByte((Byte) (data & 0xFF));
        }
    EndContent();
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::WriteBoolToken(bool value)
    {
    StartContent();
    m_textNodeOffset = (int) m_buffer.size();
    if (value)
        WriteByte((Byte)XmlBinaryNodeType::TrueText);
    else
        WriteByte((Byte)XmlBinaryNodeType::FalseText);
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
        WriteElementEnd();   // We know that our implementation does not use the name here
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

BeXmlStatus MSXmlBinaryWriter::_WriteAttribute(Utf8CP name, Utf8CP value)
    {
    WriteNode(XmlBinaryNodeType::ShortAttribute);
    WriteName(name);
    WriteText(value);
    m_textNodeOffset = -1;
    return BEXML_Success;
    }

BeXmlStatus MSXmlBinaryWriter::_WriteAttribute(Utf8CP name, WCharCP value)
    {
    return BEXML_Success;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
