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
        struct NamespaceManager
            {
            private:

                struct Namespace
                    {
                    Utf8String ns;
                    int depth = 0;

                    Namespace() {}

                    void Clear()
                        {
                        depth = 0;
                        ns = nullptr;
                        }
                    };
                bvector<Namespace*> m_namespaces;
                int m_depth;
                int m_nsCount;

            public:
                NamespaceManager()
                    {
                    m_depth = 0;
                    m_namespaces.push_back(new Namespace());
                    m_nsCount = 1;
                    }

                ~NamespaceManager()
                    {
                    for (auto ns : m_namespaces)
                        delete ns;
                    }

                void EnterScope()
                    {
                    m_depth++;
                    }

                void AddNamespace(Utf8StringCR ns)
                    {
                    if (m_depth >= m_nsCount)
                        {
                        while ((int) m_namespaces.size() < m_nsCount)
                            m_namespaces.push_back(new Namespace());
                        m_namespaces.push_back(new Namespace());
                        }
                    Namespace* nameSpace = m_namespaces[m_nsCount];
                    nameSpace->depth = m_depth;
                    nameSpace->ns = ns;
                    m_nsCount++;
                    }

                void DeclareNamespace(MSXmlBinaryWriter* dest)
                    {
                    int i = m_nsCount;
                    while (i > 0)
                        {
                        Namespace* ns = m_namespaces[i - 1];
                        if (ns->depth != m_depth)
                            break;
                        i--;
                        }

                    while (i < m_nsCount)
                        {
                        dest->WriteXmlnsAttribute(m_namespaces[i]->ns.c_str());
                        i++;
                        }
                    }

                //Utf8StringCR LookupNamespace()
                //    {
                //    int nsCount = m_nsCount;
                //    for (int i = nsCount - 1; i >= 1; i--)
                //        {
                //        Namespace* ns = m_namespaces[i];
                //        if (nullptr != ns)
                //            return ns->ns;
                //        }
                //    return "";
                //    }
            };

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
        NamespaceManager m_nsMgr;

    private:
        void WriteXmlnsAttribute(Utf8CP ns);
        void WriteNode(XmlBinaryNodeType nodeType);
        void WriteByte(byte b);
        void WriteByte(char c);
        void WriteName(Utf8CP name);
        void WriteEndStartElement(bool isEmpty);
        void WriteEndElement();
        void FlushElement();

        void StartElement(Utf8StringCR localName, Utf8StringCR nameSpace/*, XmlDictionaryString xNs*/);
        void EndStartElement();

        void StartContent();
        void EndContent();

        Element* EnterScope();
        void ExitScope();

        void AutoComplete(WriteState writeState);

    public:
        //! Default constructor
        ECOBJECTS_EXPORT MSXmlBinaryWriter();
        ~MSXmlBinaryWriter();

        //! Writes the start of an element node with the provided name.
        ECOBJECTS_EXPORT BeXmlStatus WriteElementStart (Utf8CP name) override;

        //! Writes the start of an element node with the provided name and namespace
        ECOBJECTS_EXPORT BeXmlStatus WriteElementStart (Utf8CP name, Utf8CP nameSpace);

        //! Writes the end of an element node.
        ECOBJECTS_EXPORT BeXmlStatus WriteElementEnd () override;

        BeXmlStatus WriteText(Utf8CP text) override;
        BeXmlStatus WriteText(WCharCP text) override;

        BeXmlStatus WriteDoubleText(double data);
        BeXmlStatus WriteFloatText(float data);
        BeXmlStatus WriteInt64Text(Int64 data);
        BeXmlStatus WriteInt32Text(Int32 data);
        BeXmlStatus WriteBoolText(bool value);


        void GetBytes(bvector<byte>& bytes);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
