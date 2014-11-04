/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeJsonValueBuilder.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <D:/graphite06/src/libsrc/jsoncpp/include/json/value.h>
#include <D:/graphite06/src/libsrc/jsoncpp/include/json/writer.h>
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

 static Utf8CP s_jasonValueWriterRootName = "_JsonValueWriter_root";

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
struct BeCGJsonValueWriter : IBeXmlWriter
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
        // A new stack frame starts with an intended type and a blank Json::Value
        StackFrame (ElementType type) : m_type (type){}
        };
    bvector<StackFrame> m_stack;
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
            return false;
        if (nameOptional && m_stack.back ().m_type == ElementType::Array)
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
        WriteSetElementStart (s_jasonValueWriterRootName);
        }

    public: size_t StackSize () {return m_stack.size ();}
    public: bool TryPopStack (Json::Value &value)
        {
        if (m_stack.size () == 0)
            {
            value = Json::Value ();
            return false;
            }
        value.swap (m_stack.back ().m_value);
        m_stack.pop_back ();
        return true;
        }

    public: void Error (Utf8CP description)
        {
        m_numErrors++;
        }
    public: BeXmlStatus Status (){ return m_numErrors == 0 ? BEXML_Success : BEXML_ContentWrongType;}
            
    //! Writes the start of named
    public: BeXmlStatus virtual WriteElementStart (Utf8CP name) override
        {
        Error ("Usupported WriteElementStart text");
        return Status ();
        }

    //! Writes the end of an element node.
    public: BeXmlStatus virtual WriteElementEnd (Utf8CP name) override
        {
        Error ("Usupported WriteElementEnd text");
        return Status ();
        }

    //! Writes a text node (plain string as content).
    public: BeXmlStatus virtual WriteText (WCharCP) override
        {
        Error ("Usupported WCharCP text");
        return Status ();
        }
    //! Writes a text content 
    public: BeXmlStatus virtual WriteText(Utf8CP value) override
        {
        Error ("Usupported WriteText");        
        return Status ();
        }

    //! Start construction of a set.
    public: BeXmlStatus virtual WriteSetElementStart (Utf8CP name) override
        {        
        m_stack.push_back (StackFrame (ElementType::Set));
        return Status ();
        }

    //! End construction of a set.
    public: BeXmlStatus virtual WriteSetElementEnd (Utf8CP name) override
        {
        CheckTOSType (ElementType::Set);
        Json::Value topValue;
        if (TryPopStack (topValue))
            {
            AddNamedValueToTOS (name, topValue, false);
            return Status ();
            }
        return Status ();
        }

    //! Start construction of a set.
    public: BeXmlStatus virtual WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) override
        {        
        m_stack.push_back (StackFrame (ElementType::Array));
        return Status ();
        }

    //! End construction of a set.
    public: BeXmlStatus virtual WriteArrayElementEnd (Utf8CP longName, Utf8CP shortName) override
        {
        CheckTOSType (ElementType::Array);
        Json::Value topValue;
        if (TryPopStack (topValue))
            {
            AddNamedValueToTOS (shortName, topValue, false);
            return Status ();
            }
        return Status ();
        }

    public: BeXmlStatus WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n) override
        {
        Json::Value itemValue;
        for (size_t i = 0; i < n; i++)
            itemValue.append (Json::Value(data[i]));
        AddNamedValueToTOS (itemName, itemValue, itemNameOptional);        
        return Status ();
        }

    public: BeXmlStatus WriteNamedDouble (Utf8CP itemName, double data, bool optionalName) override
        {
        Json::Value itemValue (data);
        AddNamedValueToTOS (itemName, itemValue, optionalName);
        return Status ();
        }

    public: BeXmlStatus WriteNamedBool (Utf8CP itemName, bool data, bool optionalName) override
        {
        Json::Value itemValue (data);
        AddNamedValueToTOS (itemName, itemValue, optionalName);
        return Status ();
        }

    public: BeXmlStatus WriteNamedInt32 (Utf8CP itemName, Int32 data, bool optionalName) override
        {
        Json::Value itemValue (data);
        AddNamedValueToTOS (itemName, itemValue, optionalName);
        return Status ();
        }

    public: BeXmlStatus WriteDoubleArray
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

    public: BeXmlStatus WriteIntArray
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

};
END_BENTLEY_ECOBJECT_NAMESPACE
