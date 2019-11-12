/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "serializationPCH.h"
#include <BeJsonCpp/BeJsonUtilities.h>
#include "BeCGWriter.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// static Utf8CP s_jasonValueWriterRootName = "_JsonValueWriter_root";

// Json::Value methods for ARRAY:
//  clear ()
//  resize
//  []   What is ArrayIndex type?
//  [] can act as lvalue?
//  append (value) appends value to the array?
//=======================================================================================
//! Interface of XmlWriter.  This allows users to pass in different types of writers, like the new binary XML writer in ecobjects
//
// @bsiclass                                                    Carole.MacDonald 09/2014
//=======================================================================================
struct BeCGJsonValueWriter : IBeStructuredDataWriter
    {
    private:
    size_t m_numErrors;
    enum class ElementType
        {
        Unknown,
        Set,
        Array      
        };
    struct StackFrame
        {
        ElementType m_type;
        Json::Value m_value;
        Utf8CP m_name;  // only for debugging?
        // A new stack frame starts with an intended type and a blank Json::Value
        StackFrame (ElementType type, Utf8CP name) : m_type (type), m_name(name) {}
        };
    bvector<StackFrame> m_stack;
    bvector<Json::Value> m_values;
    bool CheckTOSType (ElementType type)
        {
        if (m_stack.empty ())
            return false;
        if (m_stack.back ().m_type == type)
            return true;
        return false;
        }
        
    bool AddNamedValueToTOS (Utf8CP name, Json::Value &value, bool nameOptional)
        {
        if (m_stack.empty ())
            {
            Json::Value object;
            object[name] = value;
            m_values.push_back (object);
            return true;
            }
        if ((nullptr == name || nameOptional) && m_stack.back ().m_type == ElementType::Array)
            {
            m_stack.back ().m_value.append (value);
            return true;
            }
        else if (m_stack.back ().m_type == ElementType::Set)
            {
            m_stack.back ().m_value[name] = value;
            return true;            
            }           
        else if (m_stack.back ().m_type == ElementType::Array)
            {
            Json::Value namedValue; // put the named value in a singleton set to push as a positional array member.
            namedValue[name] = value;
            m_stack.back ().m_value.append (namedValue);
            } 
        return false;
        }
    
    public: BeCGJsonValueWriter ()
        {
        m_numErrors = 0;
        //WriteArrayElementStart (s_jasonValueWriterRootName, s_jasonValueWriterRootName);
        }

    public: size_t StackSize () {return m_stack.size ();}

    public: BeXmlStatus GrabValue (Json::Value &value)
        {
        if (m_stack.size () != 0)
            Error ("Stack not empty in GrabValue");
        if (m_values.size () == 0)
            {
            Error ("No results for GrabValue");
            }
        else if (m_values.size () == 1)
            {
            value.swap (m_values.back ());
            m_values.pop_back ();
            }
        else
            {
            value = Json::Value ();
            for (size_t i = 0; i < m_values.size (); i++)
                {
                value.append (m_values[i]);
                }
            }
        return Status ();
        }
    public: bool TryPopStack (Json::Value &value, Utf8CP name = nullptr)
        {
        if (m_stack.size () == 0)
            {
            Error ("Pop from EmptyStack");
            value = Json::Value ();
            return false;
            }
        if (name != nullptr && 0 != BeStringUtilities::Stricmp (m_stack.back ().m_name, name))
            {
            Error ("Stack start/end mismatch");
            value = Json::Value ();
            return false;
            }
        value.swap (m_stack.back ().m_value);
        m_stack.pop_back ();
        return true;
        }
    public: void PushStack (ElementType frameType, Utf8CP name)
        {
        m_stack.push_back (StackFrame (frameType, name));
        }
    public: void Error (Utf8CP description)
        {
        m_numErrors++;
        }
    public: BeXmlStatus Status (){ return m_numErrors == 0 ? BEXML_Success : BEXML_ContentWrongType;}
            

    //! Start construction of a set.
    // "The element" will be two levels: {"name":{content}}
    public: BeXmlStatus _WriteSetElementStart (Utf8CP name) override
        {
        PushStack (ElementType::Set, name);
        return Status ();
        }

    //! End construction of a (top level) CG-typed object. {"type":{properties}}
    // The content
    public: BeXmlStatus _WriteSetElementEnd (Utf8CP name) override
        {
        Json::Value topValue;
        if (TryPopStack (topValue, name))
            {
            AddNamedValueToTOS (name, topValue, false);
            }
        return Status ();
        }

    //! Start construction of named set element
    public: BeXmlStatus _WriteNamedSetStart (Utf8CP name) override
        {
        PushStack (ElementType::Set, name);
        return Status ();
        }

    //! Start construction of named set element
    public: BeXmlStatus _WriteNamedSetEnd (Utf8CP name) override
        {
        Json::Value topValue;
        size_t n = m_stack.size ();
        // uh oh ... unclear usage.
        // a) caller pushed placeholder set for name, then value atop.
        // a) caller pushed placeholder set for name, then added content values directly.
        if (n >= 3
            && m_stack[n-1].m_name != name
            && m_stack[n-2].m_name == name
            && m_stack[n-2].m_type == ElementType::Set
            && m_stack[n-2].m_value.isObject ()
            && m_stack[n-2].m_value.size () == 0
            )
            {
            TryPopStack (topValue);
            Json::Value (emptyObject);
            TryPopStack (emptyObject);
            AddNamedValueToTOS (name, topValue, false);
            }
        else if (n >= 1 && m_stack[n-1].m_name == name
            && TryPopStack (topValue, name)
            )
            {
            AddNamedValueToTOS (name, topValue, false);
            }
        else
            Error ("WriteNamedSet failed naming scenarios");
        return Status ();
        }

    //! Start construction of named set element
    public: BeXmlStatus _WritePositionalElementStart (Utf8CP name, bool nameOptional, bool doBreak) override
        {
        WriteSetElementStart (name);
        return Status ();
        }

    //! Start construction of named set element
    public: BeXmlStatus _WritePositionalElementEnd (Utf8CP name, bool nameOptional, bool doBreak) override
        {
        WriteSetElementEnd (name);
        return Status ();
        }


    public: BeXmlStatus _WriteSetElementStart (Utf8CP name, Utf8CP namespaceURI) override
            { return _WriteSetElementStart(name); }
    public: BeXmlStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName, Utf8CP namespaceURI) override
            { return _WriteArrayElementStart(longName, shortName); }
    public: BeXmlStatus _WriteNamedSetStart (Utf8CP name, Utf8CP namespaceURI) override
            { return _WriteNamedSetStart(name); }


    //---------------------------------------------------------------------------------------
    // @bsimethod                                                   Earlin.Lutz 10/2014
    //---------------------------------------------------------------------------------------
        //! Writes multiple doubles.
    BeXmlStatus _WriteArrayOfBlockedDoubles
    (
    Utf8CP longName,
    Utf8CP shortName,
    Utf8CP itemName,
    bool itemNameOptional,
    double const *data,
    size_t numPerBlock,
    size_t numBlock
    ) override
        {
        BeXmlStatus status = BEXML_Success;        
        GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementStart (longName, shortName))
        for (size_t i = 0; status == BEXML_Success && i < numBlock; i++)
            {
            WriteBlockedDoubles (itemName, itemNameOptional, data + i * numPerBlock, numPerBlock);
            }
        GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementEnd (longName, shortName))
        return status;
        }




    //! Start construction of a set.
    public: BeXmlStatus _WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) override
        {
        PushStack (ElementType::Array, longName);
        return Status ();
        }

    public: BeXmlStatus _WriteArrayMemberStart () override {return BEXML_Success;}
    public: BeXmlStatus _WriteArrayMemberEnd () override
        {
#ifdef PopArrayMemberEnd
        Json::Value topValue;
        if (TryPopStack (topValue))
            {
            AddNamedValueToTOS ("StackCapture", topValue, true);
            return Status ();
            }
#endif
        return BEXML_Success;
        }


    //! End construction of a set.
    public: BeXmlStatus _WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName) override
        {
        CheckTOSType (ElementType::Array);
        Json::Value topValue;
        if (TryPopStack (topValue, longName))
            {
            AddNamedValueToTOS (shortName, topValue, false);
            //AddNamedValueToTOS (longName, topValue, false);
            return Status ();
            }
        return Status ();
        }

    public: BeXmlStatus _WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) override
        {
        Json::Value itemValue;
        for (size_t i = 0; i < n; i++)
            itemValue.append (Json::Value(data[i]));
        AddNamedValueToTOS (itemName, itemValue, itemNameOptional);        
        return Status ();
        }

    public: BeXmlStatus _WriteNamedDouble (Utf8CP itemName, double data, bool optionalName) override
        {
        Json::Value itemValue (data);
        AddNamedValueToTOS (itemName, itemValue, optionalName);
        return Status ();
        }

    public: BeXmlStatus _WriteNamedText (Utf8CP itemName, Utf8CP data, bool optionalName) override
        {
        Json::Value itemValue (data);
        AddNamedValueToTOS (itemName, itemValue, optionalName);
        return Status ();
        }

    public: BeXmlStatus _WriteNamedBool (Utf8CP itemName, bool data, bool optionalName) override
        {
        Json::Value itemValue (data);
        AddNamedValueToTOS (itemName, itemValue, optionalName);
        return Status ();
        }

    public: BeXmlStatus _WriteNamedInt32 (Utf8CP itemName, int32_t data, bool optionalName) override
        {
        Json::Value itemValue (data);
        AddNamedValueToTOS (itemName, itemValue, optionalName);
        return Status ();
        }

    public: BeXmlStatus _WriteDoubleArray
    (
    Utf8CP longName,
    Utf8CP shortName,
    Utf8CP itemName,
    bool itemNameOptional,
    double const *data,
    size_t n
    ) override
        {
        if (n > 0)
            {
            Json::Value values;
            for (size_t i = 0; i < n; i++)
                values.append (Json::Value (data[i]));
            AddNamedValueToTOS (shortName, values, itemNameOptional);
            }
        return Status ();
        }

    public: BeXmlStatus _WriteIntArray
    (
    Utf8CP longName,
    Utf8CP shortName,
    Utf8CP itemName,
    bool itemNameOptional,
    int const *data,
    size_t n
    ) override
        {
        if (n > 0)
            {
            Json::Value values;
            for (size_t i = 0; i < n; i++)
                values.append (Json::Value (data[i]));
            AddNamedValueToTOS (shortName, values, itemNameOptional);
            }
        return Status ();
        }

    //! Writes an attribute node (name and value).
    BeXmlStatus _WriteAttribute(Utf8CP name, Utf8CP value) override
        {
        return BEXML_Success;
        }

    //! Writes content to an existing node
    BeXmlStatus _WriteContent(Utf8CP value) override
        {
        return BEXML_Success;
        }

};

bool BentleyGeometryJson::TryGeometryToJsonValue(Json::Value &value, bvector<IGeometryPtr> const &data, bool preferNativeDgnTypes)
    {
    BeCGJsonValueWriter builder;
    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
    static bool s_compactCurvePrimitives = false;
    bool preferCGSweeps = !preferNativeDgnTypes;
    bool preferMostCompactPrimitivesInCGCurveVectors = !preferNativeDgnTypes;
    Json::Value values;
    BeCGWriter writer (builder, &extendedData, false, s_compactCurvePrimitives, preferCGSweeps, preferMostCompactPrimitivesInCGCurveVectors);
    writer.Write (data);
    return builder.GrabValue(value) == BEXML_Success;
    }

bool BentleyGeometryJson::TryGeometryToJsonValue (Json::Value &value, IGeometryCR data, bool preferNativeDgnTypes)
    {
    BeCGJsonValueWriter builder;
    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
    static bool s_compactCurvePrimitives = false;
    bool preferCGSweeps = !preferNativeDgnTypes;
    bool preferMostCompactPrimitivesInCGCurveVectors = !preferNativeDgnTypes;
    BeCGWriter (builder, &extendedData, false, s_compactCurvePrimitives, preferCGSweeps, preferMostCompactPrimitivesInCGCurveVectors).Write (data);
    return builder.GrabValue (value) == BEXML_Success;
    }

bool BentleyGeometryJson::TryGeometryToJsonString (Utf8StringR string, IGeometryCR data, bool preferNativeDgnTypes)
    {
    Json::Value value;
    if (TryGeometryToJsonValue (value, data, preferNativeDgnTypes))
        {
        Json::FastWriter fastWriter;
        string = fastWriter.write (value);
        return true;
        }
    return false;
    }

bool BentleyGeometryJson::TryGeometryToJsonString(Utf8StringR string, bvector<IGeometryPtr> const &data, bool preferNativeDgnTypes)
    {
    Json::Value value;
    if (TryGeometryToJsonValue(value, data, preferNativeDgnTypes))
        {
        Json::FastWriter fastWriter;
        string = fastWriter.write(value);
        return true;
        }
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
