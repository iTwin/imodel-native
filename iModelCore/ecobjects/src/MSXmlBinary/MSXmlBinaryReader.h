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
                void SetValue(ValueHandleType type);
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
                bool m_exitScope;

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
                Utf8StringR GetLocalNameR() {return m_localName; }
                Utf8CP GetLocalName() const { return m_localName.c_str();}
                void SetValueHandle(byte* buffer, ValueHandleType type, int length, int offset);
                void SetValueHandle(ValueHandleType type);
                Utf8String ValueAsString() { return m_value.GetString(); }
                bool ExitScope() { return m_exitScope; }
            };

        struct XmlElementNode : XmlNode
            {
            public:
                XmlElementNode(byte* buffer) : XmlNode(XmlNodeType::Element, buffer, (UInt32) XmlNodeFlags::CanGetAttribute | (UInt32) XmlNodeFlags::HasContent) 
                    {
                    }
            };

        struct XmlTextNode : XmlNode
            {
            public:
                XmlTextNode(XmlNodeType nodeType, byte* buffer, UInt32 flags) : XmlNode(nodeType, buffer, flags) {}
            };

        struct XmlAtomicTextNode : XmlTextNode
            {
            public :
                XmlAtomicTextNode(byte* buffer) : XmlTextNode(XmlNodeType::Text, buffer, (UInt32) XmlNodeFlags::HasValue | (UInt32) XmlNodeFlags::AtomicValue | (UInt32) XmlNodeFlags::SkipValue | (UInt32) XmlNodeFlags::HasContent) {}
            };

        struct XmlInitialNode : XmlNode
            {
            public :
                XmlInitialNode(byte* buffer) : XmlNode(XmlNodeType::None, buffer, 0) {}
            };

        struct XmlAttributeNode : XmlTextNode
            {
            public :
                XmlAttributeNode(byte* buffer) : XmlTextNode(XmlNodeType::Attribute, buffer, (UInt32) XmlNodeFlags::CanGetAttribute | (UInt32) XmlNodeFlags::CanMoveToElement | (UInt32) XmlNodeFlags::HasValue | (UInt32) XmlNodeFlags::AtomicValue) {}
            };

        struct XmlEndElementNode : XmlNode
            {
            public:
                XmlEndElementNode() : XmlNode(XmlNodeType::EndElement, nullptr, (UInt32) XmlNodeFlags::HasContent) {}
            };

        byte* m_bytes;
        int m_length;
        int m_depth;
        XmlNode* m_node;
        Utf8String m_localName;
        Utf8String m_value;
        int m_offset;
        bool m_rootElement;
        bool m_isTextWithEndElement;
        bvector<XmlElementNode> m_elementNodes;
        bvector<XmlAttributeNode> m_attributeNodes;
        int m_attributeCount;
        XmlAtomicTextNode m_atomicTextNode;
        XmlInitialNode m_initialNode;
        XmlEndElementNode m_endElementNode;

        BeXmlReader::ReadResult ReadNode();
        byte GetByte();
        XmlBinaryNodeType GetBinaryNodeType();
        void SkipByte() {m_offset++;}
        XmlElementNode& EnterScope();
        void ExitScope();
        void MoveToNode(XmlNode* node);
        XmlAtomicTextNode* MoveToAtomicTextWithEndElement();
        XmlAtomicTextNode* MoveToAtomicText();
        void ReadText(XmlTextNode* textNode, ValueHandleType type, int length);
        void ReadAttributes();
        void ReadAttributeText(XmlTextNode* textNode);
        MSXmlBinaryReader::XmlAttributeNode& AddAttributeNode();

        void ReadName(Utf8StringR name);
        void ReadName(XmlElementNode& node);
        int ReadMultiByteUInt31();

        int ReadUInt8();

    protected:
        //! Advances (reads) to the next node.
        IBeXmlReader::ReadResult _Read () override;

        //! Advances (reads) to the next node of the provided type.
        IBeXmlReader::ReadResult _ReadTo (NodeType) override;

        IBeXmlReader::ReadResult _ReadTo (NodeType, Utf8CP name, WStringP value) override { return IBeXmlReader::ReadResult::READ_RESULT_Error; } // WIP: Not implemented yet.

        //! Advances (reads) to the start the end of this element node.
        IBeXmlReader::ReadResult _ReadToEndOfElement() override;

        //! Gets the type of the current node.
        IBeXmlReader::NodeType _GetCurrentNodeType () override;

        //! Gets the name of the current node.
        BeXmlStatus _GetCurrentNodeName (Utf8StringR) override;

        //! Gets the value of the current node.
        //! @note This only peeks at the value of the current node. This will not, for example, return the value of any child text nodes of an element node. If you want this utility, @see GetCurrentNodeValue.
        BeXmlStatus _GetCurrentNodeValue(Utf8StringR) override;

        //! Gets the value of the current node.
        //! @note This only peeks at the value of the current node. This will not, for example, return the value of any child text nodes of an element node. If you want this utility, @see GetCurrentNodeValue.
        BeXmlStatus _GetCurrentNodeValue(WStringR) override;

        //! Advances (reads) to the next attribute in the current element node, optionally providing its name and value. If the current node is not an element, this automatically fails.
        BeXmlStatus _ReadToNextAttribute (Utf8StringP name, Utf8StringP value) override;

        //! Advances (reads) to the next attribute in the current element node, optionally providing its name and value. If the current node is not an element, this automatically fails.
        BeXmlStatus _ReadToNextAttribute (Utf8StringP name, WStringP value) override;

        IBeXmlReader::NodeType _MoveToContent() override;

        //! Reads the text content at the current position as a String object.
        BeXmlStatus _ReadContentAsString(Utf8StringR str) override;

    public:
        ECOBJECTS_EXPORT MSXmlBinaryReader(byte* bytes, int length);

        bool IsStartElement();


    };

END_BENTLEY_ECOBJECT_NAMESPACE
