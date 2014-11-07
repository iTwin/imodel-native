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

#define GUARDED_STATUS_ASSIGNMENT(_status_,_expression_)\
    {\
    if (_status_ == BEXML_Success)\
        _status_ = _expression_;\
    }

struct IBeStructuredDataWriter;
typedef IBeStructuredDataWriter &IBeStructuredDataWriterR;

struct IBeStructuredDataWriter
{
//! Writes the start of an element node with the provided name, but possibly omit the name at discretion of the writer.
//! Expected usage is that an xml writer will use the name, json writer will omit.
//! Xml is noop for separator
public:  BeXmlStatus virtual WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) = 0;
public:  BeXmlStatus virtual WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak) = 0;




//! Writes the start of an element node with long or short name at discretion of the writer.
//! Expected usage is that xml will use longName, json will use shortName.
public:  BeXmlStatus virtual WriteArrayElementStart (Utf8CP longName, Utf8CP shortName)  = 0;
public:  BeXmlStatus virtual WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName)  = 0;
    
//! Writes the start of an element expected to have named children.
public:  BeXmlStatus virtual WriteSetElementStart (Utf8CP name)  = 0;
public:  BeXmlStatus virtual WriteSetElementEnd (Utf8CP name)  = 0;

//! Writes the start of a named element
public:  BeXmlStatus virtual WriteNamedSetStart (Utf8CP name)  = 0;
public:  BeXmlStatus virtual WriteNamedSetEnd (Utf8CP name)  = 0;

//! Writes an element with plain string as content. 
public:  BeXmlStatus virtual WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional) = 0;
    
//! Writes an element with a bool as content. 
public:  BeXmlStatus virtual WriteNamedBool(Utf8CP name, bool value, bool nameOptional) = 0;
        
//! Writes an element with Int32 as content. 
public:  BeXmlStatus virtual WriteNamedInt32 (Utf8CP name, int value, bool nameOptional) = 0;

//! Writes an element with a double as content.
public:  BeXmlStatus virtual WriteNamedDouble (Utf8CP name, double value, bool nameOptional) = 0;

//! Writes multiple doubles.
public:  BeXmlStatus virtual WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) = 0;

//! Writes multiple doubles.
public:  BeXmlStatus virtual WriteArrayOfBlockedDoubles (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t numPerBlock, size_t numBlock) = 0;

//! Writes multiple doubles.
public:  BeXmlStatus virtual WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) = 0;
    
//! Writes multiple ints.
public:  BeXmlStatus virtual WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n) = 0;
};

//! Abstracton layer to express json-like structure in xml.
struct BeStructuredXmlWriter : IBeStructuredDataWriter
    {
    IBeXmlWriter *m_writer;
    BeStructuredXmlWriter (IBeXmlWriter *writer)
        {
        m_writer = writer;
        }
    // Mid level element start/end pairs:
    //  1) always convert to direct xml start end
    //  2) always use element names (ignore nameOptional params)
    //  3) always use "long" names for arrays

    //! Writes the start of an element node with the provided name, but possibly omit the name at discretion of the writer.
    //! Expected usage is that an xml writer will use the name, json writer will omit.
    //! Xml is noop for separator
    public:  BeXmlStatus virtual WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) override
          {
          return m_writer->WriteElementStart (name);
          }
    public:  BeXmlStatus virtual WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak)
          {
          return m_writer->WriteElementEnd ();
          }


    //! Writes the start of an element node with long or short name at discretion of the writer.
    //! Expected usage is that xml will use longName, json will use shortName.
    public:  BeXmlStatus virtual WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) {return m_writer->WriteElementStart (longName);}
    public:  BeXmlStatus virtual WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName) {return m_writer->WriteElementEnd ();}
    
    //! Writes the start of an element expected to have named children.
    public:  BeXmlStatus virtual WriteSetElementStart (Utf8CP name) {return m_writer->WriteElementStart (name);}
    public:  BeXmlStatus virtual WriteSetElementEnd (Utf8CP name) {return m_writer->WriteElementEnd ();}

    //! Writes the start of an element expected to have named children.
    public:  BeXmlStatus virtual WriteNamedSetStart (Utf8CP name) {return m_writer->WriteElementStart (name);}
    public:  BeXmlStatus virtual WriteNamedSetEnd (Utf8CP name) {return m_writer->WriteElementEnd ();}

    //! Writes an element with plain string as content. 
    public:  BeXmlStatus virtual WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional);
    
    //! Writes an element with a bool as content. 
    public:  BeXmlStatus virtual WriteNamedBool(Utf8CP name, bool value, bool nameOptional);
        
    //! Writes an element with Int32 as content. 
    public:  BeXmlStatus virtual WriteNamedInt32 (Utf8CP name, int value, bool nameOptional);

    //! Writes an element with a double as content.
    public:  BeXmlStatus virtual WriteNamedDouble (Utf8CP name, double value, bool nameOptional);

    //! Writes multiple doubles.
    public:  BeXmlStatus virtual WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n);

    //! Writes multiple doubles.
    public:  BeXmlStatus virtual WriteArrayOfBlockedDoubles (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t numPerBlock, size_t numBlock);

    //! Writes multiple doubles.
    public:  BeXmlStatus virtual WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n);
    
    //! Writes multiple ints.
    public:  BeXmlStatus virtual WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n);

    };


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

        bvector<Element> m_elements;
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

        Element& EnterScope();
        void ExitScope();

        void AutoComplete(WriteState writeState);

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

        BeXmlStatus WriteDoubleToken(double data);
        BeXmlStatus WriteFloatToken(float data);
        BeXmlStatus WriteInt64Token(Int64 data);
        BeXmlStatus WriteInt32Token(Int32 data);
        BeXmlStatus WriteBoolToken(bool value);


        void GetBytes(bvector<byte>& bytes);
        
    public:        
        
        
    };

struct MSStructuredXmlBinaryWriter : MSXmlBinaryWriter, BeStructuredXmlWriter
{
MSStructuredXmlBinaryWriter ()
    : BeStructuredXmlWriter (this)
    {
    }

//! Writes a node with name and bool value.
BeXmlStatus virtual WriteNamedBool(Utf8CP name, bool value, bool nameOptional) override
    {
    BeXmlStatus status = BEXML_Success;
    GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
    GUARDED_STATUS_ASSIGNMENT (status, WriteBoolToken (value))
    GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
    return status;
    }
    
//! Writes a node with name and Int32 value.
BeXmlStatus virtual WriteNamedInt32 (Utf8CP name, Int32 value, bool nameOptional) override
    {
    BeXmlStatus status = BEXML_Success;
    GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
    GUARDED_STATUS_ASSIGNMENT (status, WriteInt32Token (value))
    GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
    return status;
    }

//! Writes a node with name and double value.
BeXmlStatus virtual WriteNamedDouble (Utf8CP name, double value, bool nameOptional) override
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
BeXmlStatus WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n) override
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
BeXmlStatus WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n)
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


};

END_BENTLEY_ECOBJECT_NAMESPACE
