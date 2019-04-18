/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "serializationPCH.h"
#include "MSXmlBinary/MSXmlBinaryWriter.h"
#include "BeCGWriter.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeXmlCGWriter::Write(Utf8StringR cgBeXml, IGeometryCR data, bmap<OrderedIGeometryPtr, BeExtendedData> *extendedData, bool preferCGSweeps)
    {
    cgBeXml.clear();
    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();
    BeStructuredXmlWriter structuredWriter (xmlWriter.get ());
    BeCGWriter (structuredWriter, extendedData, false, false, preferCGSweeps).Write(data);
    xmlWriter->ToString(cgBeXml);
    }



void BeXmlCGWriter::WriteBytes(bvector<::Byte>& bytes, IGeometryCR data, bmap<OrderedIGeometryPtr, BeExtendedData> *extendedData, bool preferCGSweeps)
    {
#if defined (_WIN32) && _MSC_VER < 1900 /* VC14; TWO_DIGIT_EXPONENT is now the default and only option */
    unsigned int oldFormat = _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
    MSStructuredXmlBinaryWriter* writer = new MSStructuredXmlBinaryWriter();
    BeCGWriter(*writer, extendedData, true, false, preferCGSweeps).Write (data);
#if defined (_WIN32) && _MSC_VER < 1900 /* VC14 */
    _set_output_format(oldFormat);
#endif

    writer->GetBytes(bytes);
    delete writer;
    }
    


END_BENTLEY_GEOMETRY_NAMESPACE
