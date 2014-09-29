/*--------------------------------------------------------------------------------------+
|
|     $Source: test/NonPublished/CGBinarySerializationTests.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsTestPCH.h"
#include "TestFixture.h"

#include "../../src/MSXmlBinary/MSXmlBinaryWriter.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

TEST(CGBinarySerializationTests, WriterTest)
    {
    DEllipse3d ellipseData = DEllipse3d::From (0.0, 0.0, 0.0, 
        1.0, 0.0, 0.0, 
        0.0, 1.0, 0.0, 
        0.0, Angle::TwoPi ());

    ICurvePrimitivePtr originalArc = ICurvePrimitive::CreateArc (ellipseData);
    ASSERT_EQ(true, originalArc->GetArcCP()->IsCircular());

    bvector<byte> bytes;
    BeXmlCGWriter::WriteBytes(bytes, *(originalArc.get()));
    }

END_BENTLEY_ECOBJECT_NAMESPACE
