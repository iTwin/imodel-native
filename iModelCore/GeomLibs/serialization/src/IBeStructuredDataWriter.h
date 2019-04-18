/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/


/*__PUBLISH_SECTION_END__*/
#include <BeXml/BeXml.h>
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define GUARDED_STATUS_ASSIGNMENT(_status_,_expression_)\
    {\
    if (_status_ == BEXML_Success)\
    _status_ = _expression_;\
    }

struct IBeStructuredDataWriter;
typedef IBeStructuredDataWriter &IBeStructuredDataWriterR;

struct IBeStructuredDataWriter
    {
    protected:
        //! Writes the start of an element node with the provided name, but possibly omit the name at discretion of the writer.
        //! Expected usage is that an xml writer will use the name, json writer will omit.
        //! Xml is noop for separator
        GEOMAPI_VIRTUAL BeXmlStatus _WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak) = 0;

        //! Writes the start of an element node with long or short name at discretion of the writer.
        //! Expected usage is that xml will use longName, json will use shortName.
        GEOMAPI_VIRTUAL BeXmlStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName)  = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName, Utf8CP namespaceURI)  = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WriteArrayMemberStart ()  = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WriteArrayMemberEnd   ()  = 0;

        GEOMAPI_VIRTUAL BeXmlStatus _WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName)  = 0;

        //! Writes the start of an element expected to have named children.
        GEOMAPI_VIRTUAL BeXmlStatus _WriteSetElementStart (Utf8CP name)  = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WriteSetElementStart (Utf8CP name, Utf8CP namespaceURI)  = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WriteSetElementEnd (Utf8CP name)  = 0;

        //! Writes the start of a named element
        GEOMAPI_VIRTUAL BeXmlStatus _WriteNamedSetStart (Utf8CP name)  = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WriteNamedSetStart (Utf8CP name, Utf8CP namespaceURI)  = 0;
        GEOMAPI_VIRTUAL BeXmlStatus _WriteNamedSetEnd (Utf8CP name)  = 0;

        //! Writes an element with plain string as content. 
        GEOMAPI_VIRTUAL BeXmlStatus _WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional) = 0;

        //! Writes an element with a bool as content. 
        GEOMAPI_VIRTUAL BeXmlStatus _WriteNamedBool(Utf8CP name, bool value, bool nameOptional) = 0;

        //! Writes an element with Int32 as content. 
        GEOMAPI_VIRTUAL BeXmlStatus _WriteNamedInt32 (Utf8CP name, int value, bool nameOptional) = 0;

        //! Writes an element with a double as content.
        GEOMAPI_VIRTUAL BeXmlStatus _WriteNamedDouble (Utf8CP name, double value, bool nameOptional) = 0;

        //! Writes multiple doubles.
        GEOMAPI_VIRTUAL BeXmlStatus _WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) = 0;

        //! Writes multiple doubles.
        GEOMAPI_VIRTUAL BeXmlStatus _WriteArrayOfBlockedDoubles (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t numPerBlock, size_t numBlock) = 0;

        //! Writes multiple doubles.
        GEOMAPI_VIRTUAL BeXmlStatus _WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) = 0;

        //! Writes multiple ints.
        GEOMAPI_VIRTUAL BeXmlStatus _WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n) = 0;

        //! Writes an attribute node (name and value).
        GEOMAPI_VIRTUAL BeXmlStatus _WriteAttribute(Utf8CP name, Utf8CP value) = 0;

        GEOMAPI_VIRTUAL BeXmlStatus _WriteContent(Utf8CP value) = 0;

    public:  
        //! Writes the start of an element node with the provided name, but possibly omit the name at discretion of the writer.
        //! Expected usage is that an xml writer will use the name, json writer will omit.
        //! Xml is noop for separator
        BeXmlStatus WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) {return _WritePositionalElementStart(name, nameOptional, doBreak); }
        BeXmlStatus WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak) { return _WritePositionalElementEnd(name, nameOptional, doBreak); }

        //! Writes the start of an element node with long or short name at discretion of the writer.
        //! Expected usage is that xml will use longName, json will use shortName.
        BeXmlStatus WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) { return _WriteArrayElementStart(longName, shortName); }
        BeXmlStatus WriteArrayElementStart (Utf8CP longName, Utf8CP shortName, Utf8CP namespaceURI) { return _WriteArrayElementStart(longName, shortName, namespaceURI); }

        BeXmlStatus WriteArrayMemberStart (){return _WriteArrayMemberStart ();}
        BeXmlStatus WriteArrayMemberEnd (){return _WriteArrayMemberEnd ();}

        BeXmlStatus WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName)  { return _WriteArrayElementEnd(longName, shortName); }

        //! Writes the start of an element expected to have named children.
        BeXmlStatus WriteSetElementStart (Utf8CP name) { return _WriteSetElementStart(name); }
        BeXmlStatus WriteSetElementStart (Utf8CP name, Utf8CP namespaceURI) { return _WriteSetElementStart(name, namespaceURI); }
        BeXmlStatus WriteSetElementEnd (Utf8CP name) { return _WriteSetElementEnd(name); }

        //! Writes the start of a named element
        BeXmlStatus WriteNamedSetStart (Utf8CP name) { return _WriteNamedSetStart(name); }
        BeXmlStatus WriteNamedSetStart (Utf8CP name, Utf8CP namespaceURI) { return _WriteNamedSetStart(name, namespaceURI); }
        BeXmlStatus WriteNamedSetEnd (Utf8CP name) { return _WriteNamedSetEnd(name); }

        //! Writes an element with plain string as content. 
        BeXmlStatus WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional) { return _WriteNamedText(name, text, nameOptional); }

        //! Writes an element with a bool as content. 
        BeXmlStatus WriteNamedBool(Utf8CP name, bool value, bool nameOptional) { return _WriteNamedBool(name, value, nameOptional); }

        //! Writes an element with Int32 as content. 
        BeXmlStatus WriteNamedInt32 (Utf8CP name, int value, bool nameOptional) { return _WriteNamedInt32(name, value, nameOptional); }

        //! Writes an element with a double as content.
        BeXmlStatus WriteNamedDouble (Utf8CP name, double value, bool nameOptional) { return _WriteNamedDouble(name, value, nameOptional); }

        //! Writes multiple doubles.
        BeXmlStatus WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) { return _WriteBlockedDoubles(itemName, itemNameOptional, data, n); }

        //! Writes multiple doubles.
        BeXmlStatus WriteArrayOfBlockedDoubles (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t numPerBlock, size_t numBlock) 
            {
            return _WriteArrayOfBlockedDoubles(longName, shortName, itemName, itemNameOptional, data, numPerBlock, numBlock);
            }

        //! Writes multiple doubles.
        BeXmlStatus WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) 
            {
            return _WriteDoubleArray(longName, shortName, itemName, itemNameOptional, data, n);
            }

        //! Writes multiple ints.
        BeXmlStatus WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n)
            {
            return _WriteIntArray(longName, shortName, itemName, itemNameOptional, data, n);
            }

        //! Writes an attribute node (name and value).
        BeXmlStatus WriteAttribute(Utf8CP name, Utf8CP value) { return _WriteAttribute(name, value); }

        //! Writes content to an existing node
        BeXmlStatus WriteContent(Utf8CP content) { return _WriteContent(content); }
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

    protected:
        //! Writes the start of an element node with the provided name, but possibly omit the name at discretion of the writer.
        //! Expected usage is that an xml writer will use the name, json writer will omit.
        //! Xml is noop for separator
        BeXmlStatus _WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) override
            {
            return m_writer->WriteElementStart (name);
            }
        BeXmlStatus _WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak) override
            {
            return m_writer->WriteElementEnd ();
            }


        //! Writes the start of an element node with long or short name at discretion of the writer.
        //! Expected usage is that xml will use longName, json will use shortName.
        BeXmlStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) override {return m_writer->WriteElementStart (longName);}
        BeXmlStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName, Utf8CP namespaceURI) override {return m_writer->WriteElementStart (longName, namespaceURI);}

        BeXmlStatus _WriteArrayMemberStart () override {return BEXML_Success;}
        BeXmlStatus _WriteArrayMemberEnd () override {return BEXML_Success;}


        BeXmlStatus _WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName) override {return m_writer->WriteElementEnd ();}

        //! Writes the start of an element expected to have named children.
        BeXmlStatus _WriteSetElementStart (Utf8CP name) override {return m_writer->WriteElementStart (name);}
        BeXmlStatus _WriteSetElementStart (Utf8CP name, Utf8CP namespaceURI) override {return m_writer->WriteElementStart (name, namespaceURI);}
        BeXmlStatus _WriteSetElementEnd (Utf8CP name) override {return m_writer->WriteElementEnd ();}

        //! Writes the start of an element expected to have named children.
        BeXmlStatus _WriteNamedSetStart (Utf8CP name) override {return m_writer->WriteElementStart (name);}
        BeXmlStatus _WriteNamedSetStart (Utf8CP name, Utf8CP namespaceURI) override {return m_writer->WriteElementStart (name, namespaceURI);}
        BeXmlStatus _WriteNamedSetEnd (Utf8CP name) override {return m_writer->WriteElementEnd ();}

        //! Writes an element with plain string as content. 
        BeXmlStatus _WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional) override;

        //! Writes an element with a bool as content. 
        BeXmlStatus _WriteNamedBool(Utf8CP name, bool value, bool nameOptional) override;

        //! Writes an element with Int32 as content. 
        BeXmlStatus _WriteNamedInt32 (Utf8CP name, int value, bool nameOptional) override;

        //! Writes an element with a double as content.
        BeXmlStatus _WriteNamedDouble (Utf8CP name, double value, bool nameOptional) override;

        //! Writes multiple doubles.
        BeXmlStatus _WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) override;

        //! Writes multiple doubles.
        BeXmlStatus _WriteArrayOfBlockedDoubles (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t numPerBlock, size_t numBlock) override;

        //! Writes multiple doubles.
        BeXmlStatus _WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) override;

        //! Writes multiple ints.
        BeXmlStatus _WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n) override;

        //! Writes an attribute node (name and value).
        BeXmlStatus _WriteAttribute(Utf8CP name, Utf8CP value) override { return m_writer->WriteAttribute(name, value); }

        BeXmlStatus _WriteContent(Utf8CP content) override { return m_writer->WriteText(content); }
    };

END_BENTLEY_GEOMETRY_NAMESPACE
