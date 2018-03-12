/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/ATPUtilities/Sources/ScalableMeshAtpUtilities.cpp $
|    $RCSfile: ScalableMeshQuery.cpp,v $
|   $Revision: 1.41 $
|       $Date: 2012/11/29 17:30:37 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <ScalableMeshAtpUtilities\IScalableMeshAtpUtilities.h>
#include <GeomSerialization\GeomLibsFlatBufferApi.h>


void IScalableMeshAtpUtilities::StoreVolumeTestCase(const Utf8String& volumeTestCaseFile, uint64_t elementId, bvector<PolyfaceHeaderPtr> const& candidateMeshes, double expectedCutTotal, double expectedFillTotal)
    {
    FILE* file = fopen(volumeTestCaseFile.c_str(), "a+b");

    fwrite(&elementId, sizeof(elementId), 1, file);
    fwrite(&expectedCutTotal, sizeof(expectedCutTotal), 1, file);
    fwrite(&expectedFillTotal, sizeof(expectedFillTotal), 1, file);

    uint64_t nbMeshes = candidateMeshes.size();


    fwrite(&nbMeshes, sizeof(nbMeshes), 1, file);

    for (auto& mesh : candidateMeshes)
        {
        IGeometryPtr geometry(IGeometry::Create(mesh));
        bvector<Byte> buffer;

        BentleyGeometryFlatBuffer::GeometryToBytes(*geometry, buffer);

        uint64_t nbBytes = buffer.size();

        fwrite(&nbBytes, sizeof(nbBytes), 1, file);
        fwrite(&buffer[0], nbBytes, 1, file);
        }

    fclose(file);
    }
