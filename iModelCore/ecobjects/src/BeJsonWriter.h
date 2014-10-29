/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeJsonWriter.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//=======================================================================================
//! Interface of XmlWriter.  This allows users to pass in different types of writers, like the new binary XML writer in ecobjects
//
// @bsiclass                                                    Carole.MacDonald 09/2014
//=======================================================================================
struct BeCGJsonWriter : IBeXmlWriter
    {
    Utf8String m_string;
    int m_indent;
    int m_indentSize;
    size_t m_numErrors;
    enum class ElementType {
        Null,   // empty output.
        Anonymous, // unnamed element.
        Named,   // simple element that should contain only a value
        Set,    // set name has been output, awaiting elements.
        Array   // array name has been output, awaiting elements.
        };
    // The stack records the state of the elements.
    // The state has two parts:
    //    ElementType -- defined when the element was entered
    //    Count -- number of children so far.
    struct ElementState
        {
        ElementType m_type;
        int m_count;
        ElementState (ElementType type) : m_type (type), m_count(0)
            {
            }
        void IncrementCount () {m_count++;}
        int Count (){return m_count;}
        bool IsEmpty (){return m_count == 0;}
        ElementType Type (){return m_type;}
        };

    bvector<ElementState> m_state;
    ElementType TOSType (){return m_state.empty () ? ElementType::Null : m_state.back ().Type ();}
    bool IsTOSCountZero (){return m_state.empty () ? true : m_state.back ().Count () == 0;}
    void IncrementTOSCount ()
        {
        if (!m_state.empty ())
            m_state.back ().IncrementCount ();
        }


    void PushState (ElementType type)
        {
        //m_indent += m_indentSize;
        m_state.push_back (ElementState (type));
        }
    void PopState ()
        {
        if (!m_state.empty ())
            {
            m_state.pop_back ();
            //m_indent -= m_indentSize;
            }
        }

    // Step = 1 ==> indent before
    // Step = -1 ==> undent after
    // step = 0 ==> no chnage
    void NewLine (int step = 0)
        {
        if (step == 1)
            m_indent += m_indentSize;
        m_string.push_back ('\n');
        if (m_indent > 0)
            for (int i = 0; i < m_indent; i++)
                m_string.push_back (' ');
        if (step == -1)
            m_indent -= m_indentSize;
        }
    void EmitSeparator (Utf8CP separator = ",")
        {
        if (!IsTOSCountZero ())
            Emit (separator);
        IncrementTOSCount ();
        }
    void EmitQuoted (Utf8CP text)
        {
        m_string.push_back ('\"');
        Emit (text);
        m_string.push_back ('\"');
        }

    void Emit (Utf8CP text)
        {
        for (size_t i = 0; text[i] != 0; i++)
            m_string.push_back (text[i]);
        }
    void Error (Utf8CP description)
        {
        NewLine ();
        Emit ("ERROR =");
        Emit (description);
        }        

    public: BeCGJsonWriter (int indentSize)
        {
        m_indent = 0;
        m_indentSize = 2;
        m_numErrors = 0;
        }

    public: BeXmlStatus Status (){ return m_numErrors == 0 ? BEXML_Success : BEXML_ContentWrongType;}
            
    public: void ToString (Utf8StringR dest)
        {
        dest.swap(m_string);
        }


    //! Writes the start of a list element node with the provided name.
    public: BeXmlStatus virtual WriteSetElementStart (Utf8CP name) override
        {
        EmitSeparator ();
        PushState (ElementType::Set);
        NewLine ();
        EmitQuoted (name);
        Emit (":");
        NewLine (1);
        Emit ("{");
        return Status ();
        }

    //! Writes the start of named
    public: BeXmlStatus virtual WriteElementStart (Utf8CP name) override
        {
        EmitSeparator ();
        PushState (ElementType::Named);
        NewLine ();
        EmitQuoted (name);
        Emit (":");
        return Status ();
        }

    //! Writes the start of an array element node with the provided name.
    public: BeXmlStatus virtual WriteArrayElementStart (Utf8CP longName, Utf8CP shortName) override
        {
        EmitSeparator ();
        PushState (ElementType::Array);
        NewLine ();
        EmitQuoted (shortName);
        Emit (":");
        NewLine (1);
        Emit ("[");
        return Status ();
        }

    //! Writes the start of an anonymous
    public: BeXmlStatus virtual WriteShortElementStart (Utf8CP name) override
        {
        EmitSeparator ();
        PushState (ElementType::Anonymous);
        NewLine ();
        return Status ();
        }


    //! Writes the end of an element node.
    public: BeXmlStatus virtual WriteElementEnd () override
        {
        ElementType type = TOSType ();
        switch (type)
            {
            case ElementType::Null:
                {
                Error ("WriteElementEnd in Null State");
                break;
                }
            case ElementType::Set:
                {
                NewLine (-1);
                Emit ("}");
                PopState ();
                break;
                }
            case ElementType::Array:
                {
                NewLine (-1);
                Emit ("]");
                PopState ();
                break;
                }
            case ElementType::Anonymous:
                {
                PopState ();
                break;
                }
            case ElementType::Named:
                {
                PopState ();
                break;
                }
            }
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
        EmitQuoted (value);
        return Status ();
        }

    //! Write text that has been preformated (comma separated)
    //! and hence (big assumption?) can be used as either (a) xml element content or
    //! (b) json array content.
    public: BeXmlStatus virtual WriteCommaSeparatedNumerics(Utf8CP value) override
        {
        bool hasCommas = false;
        for (size_t i = 0; value[i] != 0; i++)
            if (value[i] == ',')
                {
                hasCommas = true;
                }
        if (hasCommas)
            {
            Emit ("[");
            Emit (value);
            Emit ("]");
            }
        else
            Emit (value);
        return Status ();
        }
    };

END_BENTLEY_ECOBJECT_NAMESPACE
