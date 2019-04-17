/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "serializationPCH.h"
#include "IBeStructuredDataWriter.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
// DEFAULT IMPLEMENTATIONS --  verbose xml.
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes a text node (plain string as content). 
BeXmlStatus BeStructuredXmlWriter::_WriteNamedText(Utf8CP name, Utf8CP text, bool nameOptional)
        {
        BeXmlStatus status = BEXML_Success;
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
        GUARDED_STATUS_ASSIGNMENT (status, m_writer->WriteText (text))
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
        return status;
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes a text node (plain string as content). 
BeXmlStatus BeStructuredXmlWriter::_WriteNamedBool(Utf8CP name, bool value, bool nameOptional)
        {
        BeXmlStatus status = BEXML_Success;
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
        if (value)
            GUARDED_STATUS_ASSIGNMENT (status, m_writer->WriteText ("true"))
        else
            GUARDED_STATUS_ASSIGNMENT (status, m_writer->WriteText ("false"))
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
        return status;
        }
        
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes a text node (plain string as content). 
BeXmlStatus BeStructuredXmlWriter::_WriteNamedInt32 (Utf8CP name, int value, bool nameOptional)
        {
        char buffer[256];
        BeXmlStatus status = BEXML_Success;
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
        BeStringUtilities::Snprintf (buffer, _countof (buffer), "%d", value);
        GUARDED_STATUS_ASSIGNMENT (status, m_writer->WriteText (buffer))
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
        return status;
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes a text node (plain string as content). 
BeXmlStatus BeStructuredXmlWriter::_WriteNamedDouble (Utf8CP name, double value, bool nameOptional)
        {
        char buffer[256];
        BeXmlStatus status = BEXML_Success;
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (name, nameOptional, true))
        BeStringUtilities::Snprintf (buffer, _countof (buffer), "%.17G", value);
        GUARDED_STATUS_ASSIGNMENT (status, m_writer->WriteText (buffer))
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (name, nameOptional, true))
        return status;
        }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes multiple doubles.
BeXmlStatus BeStructuredXmlWriter::_WriteBlockedDoubles (Utf8CP itemName, bool itemNameOptional, double const *data, size_t n)
        {
        BeXmlStatus status = BEXML_Success;        
        char buffer[256];
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (itemName, itemNameOptional, true))
        for (size_t i = 0; status == BEXML_Success && i < n; i++)
            {
            BeStringUtilities::Snprintf (buffer, _countof (buffer), "%.17G", data[i]);
            if (i > 0)
                GUARDED_STATUS_ASSIGNMENT (status, m_writer->WriteText (","))
            GUARDED_STATUS_ASSIGNMENT (status, m_writer->WriteText (buffer))
            }
        GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (itemName, itemNameOptional, true))
        return status;
        }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes multiple doubles.
BeXmlStatus BeStructuredXmlWriter::_WriteArrayOfBlockedDoubles
(
Utf8CP longName,
Utf8CP shortName,
Utf8CP itemName,
bool itemNameOptional,
double const *data,
size_t numPerBlock,
size_t numBlock
)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes multiple doubles.
BeXmlStatus BeStructuredXmlWriter::_WriteDoubleArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, double const *data, size_t n)
        {
        BeXmlStatus status = BEXML_Success;
        char buffer[1024];
        GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementStart (longName, shortName))
        static int s_breakCount = 5;
        for (size_t i = 0; status == BEXML_Success && i < n; i++)
            {
            BeStringUtilities::Snprintf (buffer, _countof (buffer), "%.17G", data[i]);
            bool doBreak = ((i % s_breakCount) == 0);
            GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (itemName, itemNameOptional, doBreak))
            m_writer->WriteText (buffer);
            GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (itemName, itemNameOptional, doBreak))
            }
        GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementEnd (longName, shortName))
        return status;
        }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Earlin.Lutz 10/2014
//---------------------------------------------------------------------------------------
    //! Writes multiple ints.
BeXmlStatus BeStructuredXmlWriter::_WriteIntArray (Utf8CP longName, Utf8CP shortName, Utf8CP itemName, bool itemNameOptional, int const *data, size_t n)
        {
        BeXmlStatus status = BEXML_Success;
        char buffer[1024];
        GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementStart (longName, shortName))
        static int s_breakCount = 10;
        for (size_t i = 0; status == BEXML_Success && i < n; i++)
            {
            BeStringUtilities::Snprintf (buffer, _countof (buffer), "%d", data[i]);
            bool doBreak = ((i % s_breakCount) == 0);
            GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementStart (itemName, itemNameOptional, doBreak))
            m_writer->WriteText (buffer);
            GUARDED_STATUS_ASSIGNMENT (status, WritePositionalElementEnd (itemName, itemNameOptional, doBreak))
            }
        GUARDED_STATUS_ASSIGNMENT (status, WriteArrayElementEnd (longName, shortName))
        return status;
        }

END_BENTLEY_GEOMETRY_NAMESPACE
