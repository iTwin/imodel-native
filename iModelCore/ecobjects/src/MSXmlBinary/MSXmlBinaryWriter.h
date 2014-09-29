/*--------------------------------------------------------------------------------------+
|
|     $Source: src/MSXmlBinary/MSXmlBinaryWriter.h $
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

struct MSXmlBinaryWriter : IBeXmlWriter
    {
    private:
        bvector<byte> m_buffer;
        WriteState m_writeState;
        static Utf8CP const s_defaultNamespace;
        int m_depth;
        int m_textNodeOffset;

        struct Element
            {
            Utf8String Prefix;
            Utf8String LocalName;
            int PrefixId;

            void Clear()
                {
                Prefix = nullptr;
                LocalName = nullptr;
                PrefixId = 0;
                }
            };

        bvector<Element*> m_elements;

    private:
        void WriteNode(XmlBinaryNodeType nodeType);
        void WriteByte(byte b);
        void WriteByte(char c);
        void WriteName(Utf8CP name);
        void WriteEndStartElement(bool isEmpty);
        void WriteEndElement();
        void FlushElement();

        void StartElement(Utf8StringCR localName, Utf8StringCR nameSpace/*, XmlDictionaryString xNs*/);
        void EndStartElement();

        void StartContent(Utf8CP content);
        void EndContent();

        Element* EnterScope();
        void ExitScope();

        void AutoComplete(WriteState writeState);

        ~MSXmlBinaryWriter();
    public:
        //! Default constructor
        ECOBJECTS_EXPORT MSXmlBinaryWriter();

        //! Writes the start of an element node with the provided name.
        ECOBJECTS_EXPORT BeXmlStatus WriteElementStart (Utf8CP name) override;

        //! Writes the start of an element node with the provided name and namespace
        ECOBJECTS_EXPORT BeXmlStatus WriteElementStart (Utf8CP name, Utf8CP nameSpace);

        //! Writes the end of an element node.
        ECOBJECTS_EXPORT BeXmlStatus WriteElementEnd () override;

        BeXmlStatus WriteText(Utf8CP text) override;
        BeXmlStatus WriteText(WCharCP text) override;

        void GetBytes(bvector<byte>& bytes);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
