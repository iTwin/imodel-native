/*--------------------------------------------------------------------------------------+
|
|  $Source: src/BeXmlCGWriter.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include "MSXmlBinary\MSXmlBinaryWriter.h"
#include "BeJsonValueBuilder.h"
#include "BeJsonWriter.h"
#include "BeCGWriter.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCGWriter::Write(Utf8StringR cgBeXml, IGeometryPtr data)
    {
    cgBeXml.clear();
    BeXmlWriterPtr xmlDom = BeXmlWriter::Create();
    BeCGWriter (*xmlDom.get(), false).Write(data);
    xmlDom->ToString(cgBeXml);
    }

void BeXmlCGWriter::WriteJson(Utf8StringR cgBeXml, IGeometryPtr data)
    {
    cgBeXml.clear();
    
    BeCGJsonWriter writer (2);
    writer.Emit ("{\n");
    BeCGWriter (writer, false).Write(data);
    writer.Emit ("\n}");
    writer.ToString(cgBeXml);
    }

void BeXmlCGWriter::WriteBytes(bvector<byte>& bytes, IGeometryPtr data)
    {
#if defined (_WIN32)
    unsigned int oldFormat = _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
    MSXmlBinaryWriter* writer = new MSXmlBinaryWriter();
    BeCGWriter(*writer, true).Write (data);
#if defined (_WIN32)
    _set_output_format(oldFormat);
#endif

    writer->GetBytes(bytes);
    delete writer;
    }
    
bool BeXmlCGWriter::TryWriteJsonValue (Json::Value &value, IGeometryPtr data)
    {
    BeCGJsonValueWriter builder;
    BeCGWriter (builder, false).Write (data);
    return builder.TryPopStack (value);
    }
    
END_BENTLEY_ECOBJECT_NAMESPACE
