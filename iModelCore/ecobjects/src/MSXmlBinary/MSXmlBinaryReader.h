/*--------------------------------------------------------------------------------------+
|
|     $Source: src/MSXmlBinary/MSXmlBinaryReader.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <ECObjects/ECObjects.h>

/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
#include "MSXmlEnums.h"
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
struct MSXmlBinaryReader : public IBeXmlReader
    {
    private:
        enum class XmlNodeType
            {
            None,
            Element,
            Attribute,
            Text,
            CDATA,
            EntityReference,
            Entity,
            ProcessingInstruction,
            Comment,
            Document,
            DocumentType,
            DocumentFragment,
            Notation,
            Whitespace,
            SignificantWhitespace,
            EndElement,
            EndEntity,
            XmlDeclaration
            };

        struct ValueHandle
            {
            private:
                ValueHandleType m_valueType;
                int m_offset;
                byte* m_buffer;
                int m_length;
                Int32 GetInt8();
                Int32 GetInt16();
                Int32 GetInt32();
                Int64 GetInt64();
                UInt64 GetUInt64();
                float GetFloat();
                double GetDouble();
                Utf8String GetCharsText();

            public:
                ValueHandle(byte* buffer) : m_valueType(ValueHandleType::Empty), m_buffer(buffer) {};
                void SetValue(byte* buffer, ValueHandleType type, int length, int offset);
                Utf8String GetString();
                Int32 ToInt();
            };

        struct XmlNode
            {
            private:
                XmlNodeType m_nodeType;
                bool m_hasValue;
                bool m_canGetAttribute;
                bool m_canMoveToElement;
                bool m_hasContent;
                bool m_isEmptyElement;
                Utf8String m_localName;
                ValueHandle m_value;

            protected: enum class XmlNodeFlags
                    {
                    None = 0x00,
                    CanGetAttribute = 0x01,
                    CanMoveToElement = 0x02,
                    HasValue = 0x04,
                    AtomicValue = 0x08,
                    SkipValue = 0x10,
                    HasContent = 0x20
                    };

            public:
                XmlNode(XmlNodeType nodeType, byte* buffer, UInt32 flags);
                bool HasContent() { return m_hasContent;}
                XmlNodeType NodeType() { return m_nodeType;}
                void SetLocalName(Utf8String localName);
                Utf8CP GetLocalName() { return m_localName.c_str();}
                void SetValueHandle(byte* buffer, ValueHandleType type, int length, int offset);
                Utf8String ValueAsString() { return m_value.GetString(); }
            };

        struct XmlElementNode : XmlNode
            {
            public:
                XmlElementNode(byte* buffer) : XmlNode(XmlNodeType::Element, buffer, (UInt32) XmlNodeFlags::CanGetAttribute | (UInt32) XmlNodeFlags::HasContent) 
                    {
                    }
            };

        struct XmlAtomicTextNode : XmlNode
            {
            public :
                XmlAtomicTextNode(byte* buffer) : XmlNode(XmlNodeType::Text, buffer, (UInt32) XmlNodeFlags::HasValue | (UInt32) XmlNodeFlags::AtomicValue | (UInt32) XmlNodeFlags::SkipValue | (UInt32) XmlNodeFlags::HasContent) {}
            };

        struct XmlInitialNode : XmlNode
            {
            public :
                XmlInitialNode(byte* buffer) : XmlNode(XmlNodeType::None, buffer, 0) {}
            };

        struct XmlEndElementNode : XmlNode
            {
            public:
                XmlEndElementNode() : XmlNode(XmlNodeType::EndElement, nullptr, 0) {}
            };

        byte* m_bytes;
        int m_length;
        int m_depth;
        XmlNode* m_node;
        Utf8String m_localName;
        int m_offset;
        bool m_rootElement;
        bool m_isTextWithEndElement;
        bvector<XmlElementNode*> m_elementNodes;
        XmlAtomicTextNode* m_atomicTextNode;
        XmlInitialNode* m_initialNode;
        XmlEndElementNode* m_endElementNode;

        BeXmlReader::ReadResult ReadNode();
        byte GetByte();
        XmlBinaryNodeType GetBinaryNodeType();
        void SkipByte() {m_offset++;}
        XmlElementNode* EnterScope();
        void ExitScope();
        void MoveToNode(XmlNode* node);
        XmlAtomicTextNode* MoveToAtomicTextWithEndElement();
        XmlAtomicTextNode* MoveToAtomicText();
        void ReadText(XmlAtomicTextNode* textNode, ValueHandleType type, int length);
        void ReadAttributes();

        void ReadName(Utf8StringR name);
        void ReadName(XmlElementNode* node);
        int ReadMultiByteUInt31();

    public:
        ECOBJECTS_EXPORT MSXmlBinaryReader(byte* bytes, int length);
        ECOBJECTS_EXPORT ~MSXmlBinaryReader();

        XmlNodeType MoveToContent();

        bool IsStartElement();

        //! Advances (reads) to the next node.
        ECOBJECTS_EXPORT IBeXmlReader::ReadResult Read () override;

        //! Advances (reads) to the next node of the provided type.
        ECOBJECTS_EXPORT IBeXmlReader::ReadResult ReadTo (NodeType) override;

        //! Gets the type of the current node.
        ECOBJECTS_EXPORT IBeXmlReader::NodeType GetCurrentNodeType () override;

        //! Gets the name of the current node.
        ECOBJECTS_EXPORT BeXmlStatus GetCurrentNodeName (Utf8StringR) override;

        //! Gets the value of the current node.
        //! @note This only peeks at the value of the current node. This will not, for example, return the value of any child text nodes of an element node. If you want this utility, @see GetCurrentNodeValue.
        ECOBJECTS_EXPORT BeXmlStatus GetCurrentNodeValue(Utf8StringR) override;


    };

END_BENTLEY_ECOBJECT_NAMESPACE
