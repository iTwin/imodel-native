/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Bentley/BeTimeUtilities.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

// Use repeated undercut to construct upper and lower surfaces in both orders.
void PolyfaceHeader::ComputeSingleSheetCutFill
(
PolyfaceHeaderCR meshA,
PolyfaceHeaderCR meshB,
DVec3dCR viewVector,
PolyfaceHeaderPtr &cutMesh,
PolyfaceHeaderPtr &fillMesh
)
    {
    PolyfaceHeaderPtr ATop, AUnder, BTop, BUnder;

    PolyfaceQuery::ComputeOverAndUnderXY (meshA, nullptr, meshB, nullptr, ATop, AUnder);
    ATop = PolyfaceHeader::CloneWithTVertexFixup (bvector<PolyfaceHeaderPtr> {ATop, AUnder});
    cutMesh = PolyfaceHeader::CloneWithSidePanelsInserted (bvector<PolyfaceHeaderPtr>{ATop}, viewVector);


    PolyfaceQuery::ComputeOverAndUnderXY (meshB, nullptr, meshA, nullptr, BTop, BUnder);
    BTop = PolyfaceHeader::CloneWithTVertexFixup (bvector<PolyfaceHeaderPtr> {BTop, BUnder});
    fillMesh = PolyfaceHeader::CloneWithSidePanelsInserted (bvector<PolyfaceHeaderPtr> {BTop}, viewVector);

    }

END_BENTLEY_GEOMETRY_NAMESPACE
