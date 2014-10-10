/*--------------------------------------------------------------------------------------+
|
|     $Source: src/MSXmlBinary/MSXmlBinaryReader.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECObjectsPch.h"
#include "MSXmlBinaryReader.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

void MSXmlBinaryReader::ValueHandle::SetValue(byte* buffer, ValueHandleType type, int length, int offset)
    {
    m_buffer = buffer;
    m_valueType = type;
    m_length = length;
    m_offset = offset;
    }

void MSXmlBinaryReader::ValueHandle::SetValue(ValueHandleType type)
    {
    m_valueType = type;
    }

Int32 MSXmlBinaryReader::ValueHandle::GetInt8()
    {
    return (char) (m_buffer[m_offset]);
    }

Int32 MSXmlBinaryReader::ValueHandle::GetInt16()
    {
    return (Int16)(m_buffer[m_offset] + (m_buffer[m_offset + 1] << 8));
    }

Int32 MSXmlBinaryReader::ValueHandle::GetInt32()
    {
    byte b1 = m_buffer[m_offset + 0];
    byte b2 = m_buffer[m_offset + 1];
    byte b3 = m_buffer[m_offset + 2];
    byte b4 = m_buffer[m_offset + 3];
    return (((((b4 << 8) + b3) << 8) + b2) << 8) + b1;
    }

Int64 MSXmlBinaryReader::ValueHandle::GetInt64()
    {
    byte b1, b2, b3, b4;
    b1 = m_buffer[m_offset + 0];
    b2 = m_buffer[m_offset + 1];
    b3 = m_buffer[m_offset + 2];
    b4 = m_buffer[m_offset + 3];
    Int64 lo = (UInt32)(((((b4 << 8) + b3) << 8) + b2) << 8) + b1;
    b1 = m_buffer[m_offset + 4];
    b2 = m_buffer[m_offset + 5];
    b3 = m_buffer[m_offset + 6];
    b4 = m_buffer[m_offset + 7];
    Int64 hi = (UInt32)(((((b4 << 8) + b3) << 8) + b2) << 8) + b1;
    return (hi << 32) + lo;
    }

UInt64 MSXmlBinaryReader::ValueHandle::GetUInt64()
    {
    return (UInt64)GetInt64();
    }

float MSXmlBinaryReader::ValueHandle::GetFloat()
    {
    float value;
    byte* pb = (byte*)&value;
    pb[0] = m_buffer[m_offset + 0];
    pb[1] = m_buffer[m_offset + 1];
    pb[2] = m_buffer[m_offset + 2];
    pb[3] = m_buffer[m_offset + 3];
    return value;
    }

double MSXmlBinaryReader::ValueHandle::GetDouble()
    {
    double value;
    byte* pb = (byte*)&value;
    pb[0] = m_buffer[m_offset + 0];
    pb[1] = m_buffer[m_offset + 1];
    pb[2] = m_buffer[m_offset + 2];
    pb[3] = m_buffer[m_offset + 3];
    pb[4] = m_buffer[m_offset + 4];
    pb[5] = m_buffer[m_offset + 5];
    pb[6] = m_buffer[m_offset + 6];
    pb[7] = m_buffer[m_offset + 7];
    return value;
    }

Int32 MSXmlBinaryReader::ValueHandle::ToInt()
    {
    if (m_valueType == ValueHandleType::Zero)
        return 0;
    if (m_valueType == ValueHandleType::One)
        return 1;
    if (m_valueType == ValueHandleType::Int8)
        return GetInt8();
    if (m_valueType == ValueHandleType::Int16)
        return GetInt16();
    if (m_valueType == ValueHandleType::Int32)
        return GetInt32();
    if (m_valueType == ValueHandleType::Int64)
        {
        long value = GetInt64();
        if (value >= std::numeric_limits<Int32>::min() && value <= std::numeric_limits<Int32>::max())
            {
            return (int)value;
            }
        }
    if (m_valueType == ValueHandleType::UInt64)
        {
        UInt64 value = GetUInt64();
        if (value <= std::numeric_limits<Int32>::max())
            {
            return (Int32)value;
            }
        }

    // We shouldn't ever get here
    return -1;
    //if (type == ValueHandleType.UTF8)
    //    return XmlConverter.ToInt32(bufferReader.Buffer, offset, length);
    //return XmlConverter.ToInt32(GetString());

    }

Utf8String MSXmlBinaryReader::ValueHandle::GetCharsText()
    {
    Utf8Char *chars = new Utf8Char[m_length + 1];
    for (int i = 0; i < m_length; i++)
        {
        chars[i] = (Utf8Char) m_buffer[m_offset + i];
        }
    chars[m_length] = '\0';
    return Utf8String(chars);
    }

Utf8String MSXmlBinaryReader::ValueHandle::GetString()
    {
    switch(m_valueType)
        {
        case ValueHandleType::False:
            return Utf8String("false");
        case ValueHandleType::True:
            return Utf8String("true");
        case ValueHandleType::Zero:
            return Utf8String("0");
        case ValueHandleType::One:
            return Utf8String("1");
        case ValueHandleType::Int8:
        case ValueHandleType::Int16:
        case ValueHandleType::Int32:
            return Utf8PrintfString("%d", ToInt());
        case ValueHandleType::Int64:
            return Utf8PrintfString("%ld", GetInt64());
        case ValueHandleType::UInt64:
            return Utf8PrintfString("%lu", GetInt64());
        case ValueHandleType::Single:
            return Utf8PrintfString("%f", GetFloat());
        case ValueHandleType::Double:
            return Utf8PrintfString("%.17G", GetDouble());
        case ValueHandleType::UTF8:
            return GetCharsText();
        }
    return "";
    }

MSXmlBinaryReader::XmlAtomicTextNode* MSXmlBinaryReader::MoveToAtomicTextWithEndElement()
    {
    m_isTextWithEndElement = true;
    return MoveToAtomicText();
    }

MSXmlBinaryReader::XmlAtomicTextNode* MSXmlBinaryReader::MoveToAtomicText()
    {
    MoveToNode(m_atomicTextNode);
    return m_atomicTextNode;
    }

void MSXmlBinaryReader::XmlNode::SetValueHandle(byte* buffer, ValueHandleType type, int length, int offset)
    {
    m_value.SetValue(buffer, type, length, offset);
    }

void MSXmlBinaryReader::XmlNode::SetValueHandle(ValueHandleType type)
    {
    m_value.SetValue(type);
    }

void MSXmlBinaryReader::ReadAttributes()
    {
    XmlBinaryNodeType nodeType = GetBinaryNodeType();
    if (nodeType < XmlBinaryNodeType::MinAttribute || nodeType > XmlBinaryNodeType::MaxAttribute)
        return;

    switch (nodeType)
        {
        case XmlBinaryNodeType::ShortXmlnsAttribute:
            SkipByte();
            Utf8String temp;
            ReadName(temp);
            break;
        }
    }

void MSXmlBinaryReader::ReadText(XmlAtomicTextNode* textNode, ValueHandleType type, int length)
    {
    textNode->SetValueHandle(m_bytes, type, length, m_offset);
    m_offset += length;
    }

IBeXmlReader::ReadResult MSXmlBinaryReader::ReadNode()
    {
    XmlBinaryNodeType nodeType = GetBinaryNodeType();
    SkipByte();

    if (m_offset >= m_length)
        {
        MoveToNode(m_endElementNode);
        return READ_RESULT_Error;
        }

    XmlElementNode* xmlNode = nullptr;
    switch(nodeType)
        {
        case XmlBinaryNodeType::ShortElement:
            {
            xmlNode = EnterScope();
            ReadName(xmlNode);
            ReadAttributes();
            return READ_RESULT_Success;
            }

        case XmlBinaryNodeType::EndElement:
            {
            // MoveToEndElement();
            return READ_RESULT_Success;
            }

        case XmlBinaryNodeType::ZeroTextWithEndElement:
            {
            MoveToAtomicTextWithEndElement()->SetValueHandle(ValueHandleType::Zero);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::OneTextWithEndElement:
            {
            MoveToAtomicTextWithEndElement()->SetValueHandle(ValueHandleType::One);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::TrueTextWithEndElement:
            {
            MoveToAtomicTextWithEndElement()->SetValueHandle(ValueHandleType::True);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::FalseTextWithEndElement:
            {
            MoveToAtomicTextWithEndElement()->SetValueHandle(ValueHandleType::False);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::Chars8TextWithEndElement:
            {
            int length = GetByte();
            SkipByte();
            ReadText(MoveToAtomicTextWithEndElement(), ValueHandleType::UTF8, length);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::Int8TextWithEndElement:
            {
            ReadText(MoveToAtomicTextWithEndElement(), ValueHandleType::Int8, 1);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::Int16TextWithEndElement:
            {
            ReadText(MoveToAtomicTextWithEndElement(), ValueHandleType::Int16, 2);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::FloatTextWithEndElement:
            {
            ReadText(MoveToAtomicTextWithEndElement(), ValueHandleType::Single, 4);
            return READ_RESULT_Success;
            }
        case XmlBinaryNodeType::DoubleTextWithEndElement:
            {
            ReadText(MoveToAtomicTextWithEndElement(), ValueHandleType::Double, 8);
            return READ_RESULT_Success;
            }
        }

    return BeXmlReader::READ_RESULT_Error;
    }

byte MSXmlBinaryReader::GetByte()
    {
    return m_bytes[m_offset];
    }

XmlBinaryNodeType MSXmlBinaryReader::GetBinaryNodeType()
    {
    return (XmlBinaryNodeType)GetByte();
    }

MSXmlBinaryReader::XmlElementNode* MSXmlBinaryReader::EnterScope()
    {
    if (0 == m_depth)
        m_rootElement = true;

    m_depth++;

    if (m_depth >= (int) m_elementNodes.size())
        {
        while ((int) m_elementNodes.size() < m_depth)
            m_elementNodes.push_back(nullptr);
        m_elementNodes.push_back(new MSXmlBinaryReader::XmlElementNode(m_bytes));
        }
    MoveToNode(m_elementNodes[m_depth]);
    return m_elementNodes[m_depth];
    }

void MSXmlBinaryReader::ExitScope()
    {
    m_depth--;
    }

void MSXmlBinaryReader::MoveToNode(XmlNode* node)
    {
    this->m_node = node;
    this->m_localName = nullptr;
    }

void MSXmlBinaryReader::ReadName(Utf8StringR name)
    {
    int length = ReadMultiByteUInt31();
    Utf8Char *chars = new Utf8Char[length+1];
    for (int i = 0; i < length; i++)
        {
        chars[i] = (Utf8Char) GetByte();
        SkipByte();
        }
    chars[length] = '\0';
    Utf8String utf(chars);
    name = utf;
    delete chars;
    }

void MSXmlBinaryReader::ReadName(XmlElementNode* node)
    {
    Utf8String name;
    ReadName(name);
    node->SetLocalName(name);
    }

int MSXmlBinaryReader::ReadMultiByteUInt31()
    {
    int i = GetByte();
    SkipByte();

    if ((i & 0x80) == 0)
        return i;
    i &= 0x7F;

    int j = GetByte();
    SkipByte();
    i |= ((j & 0x7F) << 7);
    if ((j & 0x80) == 0)
        return i;

    int k = GetByte();
    SkipByte();
    i |= ((k & 0x7F) << 14);
    if ((k & 0x80) == 0)
        return i;

    int l = GetByte();
    SkipByte();
    i |= ((l & 0x7F) << 21);
    if ((l & 0x80) == 0)
        return i;

    int m = GetByte();
    SkipByte();
    i |= (m << 28);
    if ((m & 0xF8) != 0)
        return -1;

    return i;

    }

MSXmlBinaryReader::XmlNode::XmlNode(XmlNodeType nodeType, byte* buffer, UInt32 nodeFlags) : m_value(buffer)
    {
    m_nodeType = nodeType;
    m_hasContent = ((nodeFlags & (UInt32) XmlNodeFlags::HasContent) != 0);
    m_hasValue = ((nodeFlags & (UInt32) XmlNodeFlags::HasValue) != 0);
    m_canGetAttribute = ((nodeFlags & (UInt32) XmlNodeFlags::CanGetAttribute) != 0);
    m_canMoveToElement = ((nodeFlags & (UInt32)XmlNodeFlags::CanMoveToElement) != 0);
    m_isEmptyElement = false;
    }

void MSXmlBinaryReader::XmlNode::SetLocalName(Utf8String localName)
    {
    m_localName = localName;
    }

MSXmlBinaryReader::MSXmlBinaryReader(byte* bytes, int length) : m_bytes(bytes), m_length(length)
    {
    m_depth = 0;
    m_localName = nullptr;
    m_offset = 0;
    m_rootElement = false;
    m_isTextWithEndElement = false;
    m_atomicTextNode = new XmlAtomicTextNode(bytes);
    m_initialNode = new XmlInitialNode(bytes);
    m_endElementNode = new XmlEndElementNode();

    m_node = m_initialNode;
    }

MSXmlBinaryReader::~MSXmlBinaryReader()
    {
    for (auto element : m_elementNodes)
        delete element;
    delete m_bytes;
    delete m_atomicTextNode;
    delete m_initialNode;
    delete m_endElementNode;
    }

MSXmlBinaryReader::XmlNodeType MSXmlBinaryReader::MoveToContent()
    {
    do 
        {
        if (m_node->HasContent())
            {
            if (m_node->NodeType() != XmlNodeType::Text && m_node->NodeType() != XmlNodeType::CDATA)
                break;
            }
        else
            {

            }
        } while (Read());

    return m_node->NodeType();
    }

bool MSXmlBinaryReader::IsStartElement()
    {
    if (XmlNodeType::Element == m_node->NodeType())
        return true;
    if (XmlNodeType::EndElement == m_node->NodeType())
        return false;
    // we should never get to this point.  In fact, every call to this method should result in XmlNodeType::Element
    return true;
    }

IBeXmlReader::ReadResult MSXmlBinaryReader::Read()
    {
    return ReadNode();
    }

IBeXmlReader::ReadResult MSXmlBinaryReader::ReadTo(IBeXmlReader::NodeType nodeType)
    {
    if (NODE_TYPE_Element == nodeType)
        {
        if (m_node->NodeType() == MSXmlBinaryReader::XmlNodeType::Element)
            ReadNode();

        return MoveToContent() == MSXmlBinaryReader::XmlNodeType::Element ? READ_RESULT_Success : READ_RESULT_Error;
        }
    else if (NODE_TYPE_EndElement == nodeType)
        {
        if (m_isTextWithEndElement)
            {
            m_isTextWithEndElement = false;
            m_depth--;
            m_node = m_endElementNode;
            }
        }
    else if (NODE_TYPE_Text == nodeType)
        {
        return ReadNode();
        }

    return (m_offset == m_length ? READ_RESULT_Error : READ_RESULT_Success);
    }

BeXmlStatus MSXmlBinaryReader::GetCurrentNodeName (Utf8StringR nodeName)
    {
    if (Utf8String::IsNullOrEmpty(m_localName.c_str()))
        {
        m_localName = Utf8String(m_node->GetLocalName());
        }
    nodeName = m_localName;
    return BeXmlStatus::BEXML_Success;
    }

IBeXmlReader::NodeType MSXmlBinaryReader::GetCurrentNodeType()
    {
    return (IBeXmlReader::NodeType) m_node->NodeType();
    }

BeXmlStatus MSXmlBinaryReader::GetCurrentNodeValue(Utf8StringR value)
    {
    value = m_node->ValueAsString();
    return BeXmlStatus::BEXML_Success;
    }
END_BENTLEY_ECOBJECT_NAMESPACE
