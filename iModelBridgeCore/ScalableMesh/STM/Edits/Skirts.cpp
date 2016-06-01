#include "ScalableMeshPCH.h" 
#include "../ImagePPHeaders.h"
#include "Skirts.h"
#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/ScalableMeshUtilityFunctions.h>
#include <Vu/VuApi.h>
#include <TerrainModel\TerrainModel.h>
#include <TerrainModel/Core/DTMIterators.h>


USING_NAMESPACE_BENTLEY_TERRAINMODEL
USING_NAMESPACE_BENTLEY_SCALABLEMESH

//NEEDS_WORK_SM: actual skirt building code should probably remain in ConceptStation (and we need to figure out a way to get BuildSkirts() to call it)

#define DBL_EQUAL_TOLERANCE 0.00000001
void ReprojectPtOnSegmentInZdir(DPoint3d& reprojectedPt, const DPoint3d& ptToReproject, const DPoint3d& segPt1, const DPoint3d& segPt2)
    {
    //Base on the 3D line equation outline here : https:/answers.yahoo.com/question/index?qid=20080830195656AA3aEBr
    double delta;
    double t;

    if (fabs(delta = segPt2.x - segPt1.x) > DBL_EQUAL_TOLERANCE)
        {
        t = (ptToReproject.x - segPt1.x) / delta;
        }
    else
        if (fabs(delta = segPt2.y - segPt1.y) > DBL_EQUAL_TOLERANCE)
            {
            t = (ptToReproject.z - segPt1.z) / delta;
            }
        else
            {
            assert(!"Unexpected");
            t = 0;
            }

    double deltaZ = segPt2.z - segPt1.z;
    double zOnLine = segPt1.z + deltaZ * t;

    reprojectedPt = DPoint3d::From(ptToReproject.x, ptToReproject.y, zOnLine);
    }


struct HoleBoundary
    {
    bool     m_isHoleBeginning;
    DPoint3d m_reprojPointOnEndConditionSegment;
    };

void FindSegmentHoleBoundaries(bvector<HoleBoundary>&            holeBoundaries,
                               bool                              isInHole,
                               TerrainModel::DTMDrapedLinePtr& drapedLinePtr,
                               bvector<DPoint3d>&                skirtEndConditionSegments,
                               size_t                            drapedSegmentInd)
    {
    for (size_t sampleInd = 0; sampleInd < drapedLinePtr->GetPointCount(); sampleInd++)
        {
        DPoint3d          pt;
        double            distance;
        DTMDrapedLineCode code;

        DTMStatusInt status = drapedLinePtr->GetPointByIndex(&pt, &distance, &code, (int)sampleInd);
        assert(status == DTM_SUCCESS);

        switch (code)
            {
            case DTMDrapedLineCode::External:
            case DTMDrapedLineCode::InVoid:
                if (isInHole == false)
                    {
                    isInHole = true;

                    if (sampleInd - 1 >= 0)
                        {
                        DPoint3d lastValidDrapedPt;
                        DTMStatusInt status = drapedLinePtr->GetPointByIndex(&lastValidDrapedPt, 0, 0, (int)sampleInd - 1);
                        assert(status == DTM_SUCCESS);

                        DPoint3d reprojectedPt;

                        ReprojectPtOnSegmentInZdir(reprojectedPt, lastValidDrapedPt, skirtEndConditionSegments[drapedSegmentInd], skirtEndConditionSegments[drapedSegmentInd + 1]);

                        HoleBoundary holeBoundary;
                        holeBoundary.m_isHoleBeginning = true;
                        holeBoundary.m_reprojPointOnEndConditionSegment = reprojectedPt;
                        holeBoundaries.push_back(holeBoundary);
                        }
                    }

                break;
            case DTMDrapedLineCode::Tin:
            case DTMDrapedLineCode::Breakline:
            case DTMDrapedLineCode::BetweenBreaklines:
            case DTMDrapedLineCode::OnPoint:
            case DTMDrapedLineCode::Edge:
                if (isInHole == true)
                    {
                    isInHole = false;

                    DPoint3d reprojectedPt;

                    ReprojectPtOnSegmentInZdir(reprojectedPt, pt, skirtEndConditionSegments[drapedSegmentInd], skirtEndConditionSegments[drapedSegmentInd + 1]);

                    HoleBoundary holeBoundary;
                    holeBoundary.m_isHoleBeginning = false;
                    holeBoundary.m_reprojPointOnEndConditionSegment = reprojectedPt;
                    holeBoundaries.push_back(holeBoundary);
                    }

                break;
            }
        }
    }

void BuildSkirt(PolyfaceHeaderPtr& skirt, bvector<DPoint3d> polylineToSkirt, TerrainModel::DTMPtr& dtmPtr)
    {
    if (dtmPtr->GetBcDTM()->GetTinHandle() == nullptr) return;
    IFacetOptionsPtr facetOptions = IFacetOptions::Create();
    bvector<TerrainModel::DTMDrapedLinePtr> drapedSegments;
    TerrainModel::IDTMDraping * draping = dtmPtr->GetDTMDraping();
    for (size_t ptInd = 0; ptInd < polylineToSkirt.size() - 1; ptInd++)
        {
        TerrainModel::DTMDrapedLinePtr drapedLinePP;
        DTMStatusInt status = draping->DrapeLinear(drapedLinePP, &polylineToSkirt[ptInd], 2);        
        assert(status == DTM_SUCCESS);
        if (drapedLinePP.IsValid())
            drapedSegments.push_back(drapedLinePP);
        
        }

    bool wasInExternalHole = false;

    if ((drapedSegments.size() > 0) && (drapedSegments[0]->GetPointCount() > 0))
        {
        DPoint3d          pt;
        double            distance;
        DTMDrapedLineCode code;

        DTMStatusInt status = drapedSegments[0]->GetPointByIndex(&pt, &distance, &code, 0);
        assert(status == DTM_SUCCESS);

        wasInExternalHole = (code == DTMDrapedLineCode::External || code == DTMDrapedLineCode::InVoid) ? true : false;
        }

    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::New(*facetOptions);

    size_t nextTri[3];

    for (size_t drapedSegmentInd = 0; drapedSegmentInd < drapedSegments.size(); drapedSegmentInd++)
        {
        TerrainModel::DTMDrapedLinePtr drapedLineP(drapedSegments[drapedSegmentInd]);

        size_t holeBoundaryCurrentInd = 0;
        bvector<HoleBoundary> holeBoundaries;
        FindSegmentHoleBoundaries(holeBoundaries, wasInExternalHole, drapedLineP, polylineToSkirt, drapedSegmentInd);

        if (holeBoundaries.size() == 0)
            {
            if (wasInExternalHole) continue;

            nextTri[0] = builder->FindOrAddPoint(polylineToSkirt[drapedSegmentInd]);
            nextTri[1] = builder->FindOrAddPoint(polylineToSkirt[drapedSegmentInd + 1]);
            }
        else
            if (!wasInExternalHole)
                {
                nextTri[0] = builder->FindOrAddPoint(polylineToSkirt[drapedSegmentInd]);
                assert(holeBoundaries[0].m_isHoleBeginning == true);
                nextTri[1] = builder->FindOrAddPoint(holeBoundaries[0].m_reprojPointOnEndConditionSegment);
                holeBoundaryCurrentInd++;
                }

        for (size_t sampleInd = 0; sampleInd < drapedLineP->GetPointCount(); sampleInd++)
            {
            DPoint3d          pt;
            double            distance;
            DTMDrapedLineCode code;

            DTMStatusInt status = drapedLineP->GetPointByIndex(&pt, &distance, &code, (int)sampleInd);
            assert(status == DTM_SUCCESS);

            switch (code)
                {
                case DTMDrapedLineCode::External:
                case DTMDrapedLineCode::InVoid:
                    if (wasInExternalHole == false)
                        {
                        wasInExternalHole = true;

                        assert(holeBoundaries.size() >= holeBoundaryCurrentInd);
                        }
                    break;
                case DTMDrapedLineCode::Tin:
                case DTMDrapedLineCode::Breakline:
                case DTMDrapedLineCode::BetweenBreaklines:
                case DTMDrapedLineCode::OnPoint:
                case DTMDrapedLineCode::Edge:
                    if (wasInExternalHole == true)
                        {
                        assert(holeBoundaries[holeBoundaryCurrentInd].m_isHoleBeginning == false);
                        nextTri[0] = builder->FindOrAddPoint(holeBoundaries[holeBoundaryCurrentInd].m_reprojPointOnEndConditionSegment);
                        holeBoundaryCurrentInd++;

                        if (holeBoundaryCurrentInd < holeBoundaries.size())
                            {
                            assert(holeBoundaries[holeBoundaryCurrentInd].m_isHoleBeginning == true);
                            nextTri[1] = builder->FindOrAddPoint(holeBoundaries[holeBoundaryCurrentInd].m_reprojPointOnEndConditionSegment);
                            holeBoundaryCurrentInd++;
                            }
                        else
                            {
                            nextTri[1] = builder->FindOrAddPoint(polylineToSkirt[drapedSegmentInd + 1]);
                            }

                        wasInExternalHole = false;
                        }

                    nextTri[2] = builder->FindOrAddPoint(pt);
                    builder->AddPointIndexTriangle(nextTri[0], true, nextTri[1], true, nextTri[2], true);
                    nextTri[0] = nextTri[2];
                    break;
                }
            }
        }

    if (builder->GetClientMeshPtr()->GetPointIndexCount() > 0)
        {
        assert(builder->GetClientMeshPtr()->GetPointIndexCount() >= 3);
        skirt = builder->GetClientMeshPtr();
        }
    }

SkirtBuilder::SkirtBuilder(BcDTMPtr& dtmP)
    {
    m_dtm = dtmP;
    }

void SkirtBuilder::BuildSkirtMesh(bvector<PolyfaceHeaderPtr>& meshParts, bvector<bvector<DPoint3d>>& targetLines)
    {
    for (auto& line : targetLines)
        {
        PolyfaceHeaderPtr mesh;
        BuildSkirt(mesh, line, m_dtm);
        meshParts.push_back(mesh);
        }
    }