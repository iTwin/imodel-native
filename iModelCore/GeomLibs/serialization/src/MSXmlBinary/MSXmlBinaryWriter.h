/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
#include "MSXmlEnums.h"
#include "../IBeStructuredDataWriter.h"
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

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
                        ns.AssignOrClear(nullptr);
                        }
                    };
                bvector<Namespace> m_namespaces;
                int m_depth;
                int m_nsCount;

            public:
                NamespaceManager()
                    {
                    m_depth = 0;
                    Namespace ns;
                    m_namespaces.push_back(ns);
                    m_nsCount = 1;
                    }

                void EnterScope()
                    {
                    m_depth++;
                    }

                void AddNamespace(Utf8StringCR ns)
                    {
                    if (m_depth >= m_nsCount)
                        {
                        while ((int) m_namespaces.size() <= m_nsCount)
                            {
                            Namespace ns;
                            m_namespaces.push_back(ns);
                            }
                        }
                    Namespace& nameSpace = m_namespaces[m_nsCount];
                    nameSpace.depth = m_depth;
                    nameSpace.ns = ns;
                    m_nsCount++;
                    }

                void DeclareNamespace(MSXmlBinaryWriter* dest)
                    {
                    int i = m_nsCount;
                    while (i > 0)
                        {
                        Namespace& ns = m_namespaces[i - 1];
                        if (ns.depth != m_depth)
                            break;
                        i--;
                        }

                    while (i < m_nsCount)
                        {
                        dest->WriteXmlnsAttribute(m_namespaces[i].ns.c_str());
                        i++;
                        }
                    }
            };

        bvector<Byte> m_buffer;
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

        bvector<Element> m_elements;
        NamespaceManager m_nsMgr;

    private:
        void WriteXmlnsAttribute(Utf8CP ns);
        void WriteNode(XmlBinaryNodeType nodeType);
        void WriteByte(Byte b);
        void WriteByte(char c);
        void WriteName(Utf8CP name);
        void WriteEndStartElement(bool isEmpty);
        void WriteEndElement();
        void FlushElement();

        void StartElement(Utf8StringCR localName, Utf8StringCR nameSpace/*, XmlDictionaryString xNs*/);
        void EndStartElement();

        void StartContent();
        void EndContent();

        Element& EnterScope();
        void ExitScope();

        void AutoComplete(WriteState writeState);

    protected:
        //! Writes the start of an element node with the provided name.
        BeXmlStatus virtual _WriteElementStart (Utf8CP name) override;
        //! Writes the start of an element node with the provided name and namespace
        BeXmlStatus virtual _WriteElementStart (Utf8CP name, Utf8CP nameSpace) override;

        //! Writes the end of an element node.
        BeXmlStatus virtual _WriteElementEnd () override;

        BeXmlStatus virtual _WriteText(Utf8CP text) override;
        BeXmlStatus virtual _WriteText(WCharCP text) override;

        //! Writes an attribute node (name and value).
        BeXmlStatus virtual _WriteAttribute (Utf8CP name, WCharCP value) override;
        BeXmlStatus virtual _WriteAttribute (Utf8CP name, Utf8CP value) override;


    public:
        //! Default constructor
        GEOMLIBS_SERIALIZATION_EXPORT MSXmlBinaryWriter();

        BeXmlStatus WriteDoubleToken(double data);
        BeXmlStatus WriteFloatToken(float data);
        BeXmlStatus WriteInt64Token(int64_t data);
        BeXmlStatus WriteInt32Token(int32_t data);
        BeXmlStatus WriteBoolToken(bool value);

        void GetBytes(bvector<Byte>& bytes);

    };

struct MSStructuredXmlBinaryWriter : MSXmlBinaryWriter, BeStructuredXmlWriter
    {
    protected:
        //! Writes a node with name and bool value.
        BeXmlStatus virtual _WriteNamedBool(Utf8CP name, bool value, bool nameOptional) override
            {
            BeXmlStatus status = BEXML_Success;
            GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
                GUARDED_STATUS_ASSIGNMENT (status, WriteBoolToken (value))
                GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
                return status;
            }

        //! Writes a node with name and Int32 value.
        BeXmlStatus virtual _WriteNamedInt32 (Utf8CP name, int32_t value, bool nameOptional) override
            {
            BeXmlStatus status = BEXML_Success;
            GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
                GUARDED_STATUS_ASSIGNMENT (status, WriteInt32Token (value))
                GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
                return status;
            }

        //! Writes a node with name and double value.
        BeXmlStatus virtual _WriteNamedDouble (Utf8CP name, double value, bool nameOptional) override
            {
            BeXmlStatus status = BEXML_Success;
            GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
                GUARDED_STATUS_ASSIGNMENT (status, WriteDoubleToken (value))
                GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
                return status;
            } 

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Earlin.Lutz 10/2014
        //---------------------------------------------------------------------------------------
        //! Writes multiple ints.
        BeXmlStatus _WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n) override
            {
            BeXmlStatus status = BEXML_Success;
            GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementStart (longName, shortName))
                static int s_breakCount = 10;
            for (size_t i = 0; status == BEXML_Success && i < n; i++)
                {
                bool doBreak = ((i % s_breakCount) == 0);
                GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (itemName, itemNameOptional, doBreak))
                    GUARDED_STATUS_ASSIGNMENT (status, WriteInt32Token (data[i]))
                    GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (itemName, itemNameOptional, doBreak))
                }
            GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementEnd (longName, shortName))
                return status;
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod                                                   Earlin.Lutz 10/2014
        //---------------------------------------------------------------------------------------
        //! Writes multiple doubles.
        BeXmlStatus _WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) override
            {
            BeXmlStatus status = BEXML_Success;
            GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementStart (longName, shortName))
                static int s_breakCount = 5;
            for (size_t i = 0; status == BEXML_Success && i < n; i++)
                {
                bool doBreak = ((i % s_breakCount) == 0);
                GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (itemName, itemNameOptional, doBreak))
                    GUARDED_STATUS_ASSIGNMENT (status, WriteDoubleToken (data[i]))
                    GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (itemName, itemNameOptional, doBreak))
                }
            GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementEnd (longName, shortName))
                return status;
            }

    public:
        MSStructuredXmlBinaryWriter ()
            : BeStructuredXmlWriter (this)
            {
            }

        virtual ~MSStructuredXmlBinaryWriter() {}
    };

END_BENTLEY_GEOMETRY_NAMESPACE
