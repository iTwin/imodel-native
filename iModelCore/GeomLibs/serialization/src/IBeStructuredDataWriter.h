/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

enum BeJsonStatus
    {
    BEJSON_Success = 0,
    BEJSON_ContentWrongType = 0x1000 + 17,
    };

#define GUARDED_STATUS_ASSIGNMENT(_status_,_expression_)\
    {\
    if (_status_ == BEJSON_Success)\
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
        GEOMAPI_VIRTUAL BeJsonStatus _WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak) = 0;

        //! Writes the start of an element node with long or short name at discretion of the writer.
        //! Expected usage is that xml will use longName, json will use shortName.
        GEOMAPI_VIRTUAL BeJsonStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName)  = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName, Utf8CP namespaceURI)  = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WriteArrayMemberStart ()  = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WriteArrayMemberEnd   ()  = 0;

        GEOMAPI_VIRTUAL BeJsonStatus _WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName)  = 0;

        //! Writes the start of an element expected to have named children.
        GEOMAPI_VIRTUAL BeJsonStatus _WriteSetElementStart (Utf8CP name)  = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WriteSetElementStart (Utf8CP name, Utf8CP namespaceURI)  = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WriteSetElementEnd (Utf8CP name)  = 0;

        //! Writes the start of a named element
        GEOMAPI_VIRTUAL BeJsonStatus _WriteNamedSetStart (Utf8CP name)  = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WriteNamedSetStart (Utf8CP name, Utf8CP namespaceURI)  = 0;
        GEOMAPI_VIRTUAL BeJsonStatus _WriteNamedSetEnd (Utf8CP name)  = 0;

        //! Writes an element with plain string as content. 
        GEOMAPI_VIRTUAL BeJsonStatus _WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional) = 0;

        //! Writes an element with a bool as content. 
        GEOMAPI_VIRTUAL BeJsonStatus _WriteNamedBool(Utf8CP name, bool value, bool nameOptional) = 0;

        //! Writes an element with Int32 as content. 
        GEOMAPI_VIRTUAL BeJsonStatus _WriteNamedInt32 (Utf8CP name, int value, bool nameOptional) = 0;

        //! Writes an element with a double as content.
        GEOMAPI_VIRTUAL BeJsonStatus _WriteNamedDouble (Utf8CP name, double value, bool nameOptional) = 0;

        //! Writes multiple doubles.
        GEOMAPI_VIRTUAL BeJsonStatus _WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) = 0;

        //! Writes multiple doubles.
        GEOMAPI_VIRTUAL BeJsonStatus _WriteArrayOfBlockedDoubles (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t numPerBlock, size_t numBlock) = 0;

        //! Writes multiple doubles.
        GEOMAPI_VIRTUAL BeJsonStatus _WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) = 0;

        //! Writes multiple ints.
        GEOMAPI_VIRTUAL BeJsonStatus _WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n) = 0;

        //! Writes an attribute node (name and value).
        GEOMAPI_VIRTUAL BeJsonStatus _WriteAttribute(Utf8CP name, Utf8CP value) = 0;

        GEOMAPI_VIRTUAL BeJsonStatus _WriteContent(Utf8CP value) = 0;

    public:  
        //! Writes the start of an element node with the provided name, but possibly omit the name at discretion of the writer.
        //! Expected usage is that an xml writer will use the name, json writer will omit.
        //! Xml is noop for separator
        BeJsonStatus WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) {return _WritePositionalElementStart(name, nameOptional, doBreak); }
        BeJsonStatus WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak) { return _WritePositionalElementEnd(name, nameOptional, doBreak); }

        //! Writes the start of an element node with long or short name at discretion of the writer.
        //! Expected usage is that xml will use longName, json will use shortName.
        BeJsonStatus WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) { return _WriteArrayElementStart(longName, shortName); }
        BeJsonStatus WriteArrayElementStart (Utf8CP longName, Utf8CP shortName, Utf8CP namespaceURI) { return _WriteArrayElementStart(longName, shortName, namespaceURI); }

        BeJsonStatus WriteArrayMemberStart (){return _WriteArrayMemberStart ();}
        BeJsonStatus WriteArrayMemberEnd (){return _WriteArrayMemberEnd ();}

        BeJsonStatus WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName)  { return _WriteArrayElementEnd(longName, shortName); }

        //! Writes the start of an element expected to have named children.
        BeJsonStatus WriteSetElementStart (Utf8CP name) { return _WriteSetElementStart(name); }
        BeJsonStatus WriteSetElementStart (Utf8CP name, Utf8CP namespaceURI) { return _WriteSetElementStart(name, namespaceURI); }
        BeJsonStatus WriteSetElementEnd (Utf8CP name) { return _WriteSetElementEnd(name); }

        //! Writes the start of a named element
        BeJsonStatus WriteNamedSetStart (Utf8CP name) { return _WriteNamedSetStart(name); }
        BeJsonStatus WriteNamedSetStart (Utf8CP name, Utf8CP namespaceURI) { return _WriteNamedSetStart(name, namespaceURI); }
        BeJsonStatus WriteNamedSetEnd (Utf8CP name) { return _WriteNamedSetEnd(name); }

        //! Writes an element with plain string as content. 
        BeJsonStatus WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional) { return _WriteNamedText(name, text, nameOptional); }

        //! Writes an element with a bool as content. 
        BeJsonStatus WriteNamedBool(Utf8CP name, bool value, bool nameOptional) { return _WriteNamedBool(name, value, nameOptional); }

        //! Writes an element with Int32 as content. 
        BeJsonStatus WriteNamedInt32 (Utf8CP name, int value, bool nameOptional) { return _WriteNamedInt32(name, value, nameOptional); }

        //! Writes an element with a double as content.
        BeJsonStatus WriteNamedDouble (Utf8CP name, double value, bool nameOptional) { return _WriteNamedDouble(name, value, nameOptional); }

        //! Writes multiple doubles.
        BeJsonStatus WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) { return _WriteBlockedDoubles(itemName, itemNameOptional, data, n); }

        //! Writes multiple doubles.
        BeJsonStatus WriteArrayOfBlockedDoubles (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t numPerBlock, size_t numBlock) 
            {
            return _WriteArrayOfBlockedDoubles(longName, shortName, itemName, itemNameOptional, data, numPerBlock, numBlock);
            }

        //! Writes multiple doubles.
        BeJsonStatus WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) 
            {
            return _WriteDoubleArray(longName, shortName, itemName, itemNameOptional, data, n);
            }

        //! Writes multiple ints.
        BeJsonStatus WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n)
            {
            return _WriteIntArray(longName, shortName, itemName, itemNameOptional, data, n);
            }

        //! Writes an attribute node (name and value).
        BeJsonStatus WriteAttribute(Utf8CP name, Utf8CP value) { return _WriteAttribute(name, value); }

        //! Writes content to an existing node
        BeJsonStatus WriteContent(Utf8CP content) { return _WriteContent(content); }
    };

END_BENTLEY_GEOMETRY_NAMESPACE
