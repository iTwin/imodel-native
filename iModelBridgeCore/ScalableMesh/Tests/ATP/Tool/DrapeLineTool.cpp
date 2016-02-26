/*#include <ScalableMeshATPPch.h>
#include "DrapeLineTool.h"
#include "..\Common\ATPUtils.h"
#include "..\TiledTriangulation\MrDTMUtil.h"
#include <ScalableMesh/IScalableMeshATP.h>

#define ABORT (ERROR + 1)

/*bool DPoint3dEqualityTest(const DPoint3d& point1, const DPoint3d& point2)
    {
    return LegacyMath::DEqual(point1.x, point2.x) && LegacyMath::DEqual(point1.y, point2.y);
    }*/

/*StatusInt DoBatchDrape(ElementAgenda* vectorAgenda, DTMPtr& dtmPtr, bvector<bvector<DPoint3d>>& drapeLines)
    {
    bool aborded = false;

    EditElementHandleP    curr = vectorAgenda->GetFirstP();
    EditElementHandleP end = curr + vectorAgenda->GetCount();

   // size_t numElement = vectorAgenda->GetCount();
    //int atElement = 1;
    int numElemDraped = 0;
    int numElemNOTDraped = 0;
    int numPartial = 0;
    int numPts = 0;
    double drapeLength = 0.0;

    clock_t timer = clock();
    for (; curr < end; curr++) //For each valid element we do the draping
        {
        ElementHandle elemHandle = *curr;

        //Start the draping process
        //atElement++;
        StatusInt status;
        std::vector<DPoint3d> origPoints;// , origCopy;
            //Get the number of points on the line depending on if it's a simple line or a poly line
        MSElementCP element = elemHandle.GetElementCP();
        //Validate if we have a valid element type, line WString or polyline
        switch (elemHandle.GetElementType())
            {
            case LINE_ELM:
                {
                origPoints.push_back(element->line_3d.start);
                origPoints.push_back(element->line_3d.end);
                break;
                }
            case LINE_STRING_ELM:
                {
                origPoints.resize(element->point_string_3d.numpts);
                memcpy(&origPoints[0], &element->point_string_3d.point[0], element->point_string_3d.numpts * sizeof(DPoint3d));
                break;
                }
            default:
                break;
            }

        Transform refToActiveTrf;
       // origCopy.insert(origCopy.end(), origPoints.begin(), origPoints.end());
        GetFromModelRefToActiveTransform(refToActiveTrf, elemHandle.GetModelRef());

        bsiTransform_multiplyDPoint3dArrayInPlace(&refToActiveTrf, &origPoints[0], (int)origPoints.size());

        if (origPoints.size() == 0)
            {
            //numElemNOTDraped++;
            continue;
            }

        std::vector<std::vector<DPoint3d>> drapedPoints(origPoints.size() - 1);
        status = DrapeOnScalableMesh (dtmPtr, drapedPoints, origPoints);
        if (status == ABORT)
            {
            aborded = true;
            break;
            }
        else if (status == ERROR)
            {
            numElemNOTDraped++;

            continue;
            }
        numElemDraped++;
        Transform uorToMeter;
        Transform meterToUor;
        bvector<DPoint3d> drapeLine;
        GetTransformForPoints(uorToMeter, meterToUor);
        // Sort draped points on line and remove duplicates...
        for (unsigned int line = 0; line < origPoints.size() - 1; ++line)
            {
            sort(drapedPoints[line].begin(), drapedPoints[line].end(), STMDrapeHelper::DPoint3dComparer(origPoints[line], origPoints[line + 1]));
            std::vector<DPoint3d>::iterator it = unique(drapedPoints[line].begin(), drapedPoints[line].end(), DPoint3dEqualityTest);
            drapedPoints[line].resize(std::distance(drapedPoints[line].begin(), it));
            }
        
        // Add to final draped points vector
        for (unsigned int line = 0; line < drapedPoints.size(); line++)
            for (unsigned int i = 0; i < drapedPoints[line].size(); i++)
                drapeLine.push_back(drapedPoints[line][i]);

        bvector<DPoint3d>::iterator it = unique(drapeLine.begin(), drapeLine.end(), DPoint3dEqualityTest);
        drapeLine.resize(std::distance(drapeLine.begin(), it));

        bsiTransform_multiplyDPoint3dArrayInPlace(&meterToUor, (DPoint3dP)&drapeLine[0], (int)drapeLine.size());

        DPoint3d ptGO;
        //if (SUCCESS != mdlModelRef_getGlobalOrigin(mdlModelRef_getActive(), &ptGO))
        ModelInfoCP modelInfo = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP();
        if (SUCCESS != modelInfo->GetGlobalOrigin(modelInfo, &ptGO))
            ptGO.x = ptGO.y = ptGO.z = 0.;

        Transform transTrf;
        transTrf.InitIdentity();
        transTrf.SetTranslation(ptGO);
        //mdlTMatrix_getIdentity(&transTrf);
        //mdlTMatrix_setTranslation(&transTrf, &ptGO);
        bsiTransform_multiplyDPoint3dArrayInPlace(&transTrf, (DPoint3dP)&drapeLine[0], (int)drapeLine.size());

        //reporting of partial drapes
        if (status == SUCCESS && drapeLine.size() == 0)
            {
            numPartial++;
            }
        else if (status == SUCCESS)
            {
            //DRay3d lastSegment = DRay3d::FromOriginAndVector(origPoints[origPoints.size() - 2], DVec3d::FromStartEnd(origPoints[origPoints.size() - 2], origPoints[origPoints.size() - 1]));
            //DPoint3d intersectPt1, intersectPt2;
            //DRay3d endOfDrapeLine = DRay3d::FromOriginAndVector(drapeLine.back(), DVec3d::From(0, 0, 1));
            //double p1, p2;
            //if (bsiDRay3d_closestApproach(&p1, &p2, &intersectPt1, &intersectPt2, &lastSegment, &endOfDrapeLine) && p1 > 0 && p1 <= 1 && bsiDPoint3d_pointEqualTolerance(&intersectPt1, &intersectPt2, 0.001) && bsiDPoint3d_pointEqualTolerance(&intersectPt1, &origPoints[origPoints.size() - 1], 0.001))
            DPoint3d pt = drapeLine.back();
            if (fabs(origPoints[origPoints.size() - 1].x - pt.x)<1 && fabs(origPoints[origPoints.size() - 1].y - pt.y)<1)
                {
                drapeLength += 1.0;
                }
            else
                {
                numPartial++;

               // if (bsiDPoint3d_pointEqualTolerance(&intersectPt1, &intersectPt2, 0.001))
                DPoint3d intersectPt1 = pt;
                intersectPt1.z = origPoints[origPoints.size() - 1].z;
                    drapeLength += fabs(DVec3d::FromStartEnd(origPoints[origPoints.size() - 2], intersectPt1).MagnitudeSquared() / DVec3d::FromStartEnd(origPoints[origPoints.size() - 2], origPoints[origPoints.size() - 1]).MagnitudeSquared());
                }
            }
        numPts += (int)drapeLine.size();
        drapeLines.push_back(drapeLine);
        }

    if (curr == end)
        {
        timer = clock() - timer;

        float secs;
        secs = ((float)timer) / CLOCKS_PER_SEC;
        IScalableMeshATP::StoreDouble(L"drapeTime", secs);
        }
    drapeLength /= numElemDraped;
    IScalableMeshATP::StoreInt(L"nOfLines", numElemDraped + numElemNOTDraped);
    IScalableMeshATP::StoreInt(L"nOfLinesNotDraped", numElemNOTDraped);
    IScalableMeshATP::StoreInt(L"nOfLinesDraped", numElemDraped);
    IScalableMeshATP::StoreInt(L"nOfLinesPartial", numPartial);
    IScalableMeshATP::StoreInt(L"nOfOutputPoints", numPts);
    IScalableMeshATP::StoreDouble(L"lengthOfLinesPartial", drapeLength);
    StatusInt status = aborded;

    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Elenie.Godzaridis                   12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
/*StatusInt DrapeOnScalableMesh (DTMPtr& smPtr, std::vector<std::vector<DPoint3d>>& drapedPoints, std::vector<DPoint3d> origPoints)
    {
    Transform uorToMeter;
    Transform meterToUor;

    GetTransformForPoints(uorToMeter, meterToUor);
    //coordinate system stuff
    DPoint3d ptGO;
    //if (SUCCESS != mdlModelRef_getGlobalOrigin(mdlModelRef_getActive(), &ptGO))
    ModelInfoCP modelInfo = ScalableMeshLib::GetHost().GetScalableMeshAdmin()._GetActiveModelRef()->GetModelInfoCP();
    if (SUCCESS != modelInfo->GetGlobalOrigin(modelInfo, &ptGO))
        ptGO.x = ptGO.y = ptGO.z = 0.;

    Transform transTrf;
    transTrf.InitIdentity();
    transTrf.SetTranslation(ptGO);
    transTrf.InverseOf(transTrf);
    //mdlTMatrix_getIdentity(&transTrf);
    //mdlTMatrix_setTranslation(&transTrf, &ptGO);
    //mdlTMatrix_getInverse(&transTrf, &transTrf);

    bsiTransform_multiplyDPoint3dArrayInPlace(&transTrf, (DPoint3dP)&origPoints[0], (int)origPoints.size());

    // Get the coordinates of the line in meter
    bsiTransform_multiplyDPoint3dArrayInPlace(&uorToMeter, (DPoint3dP)&origPoints[0], (int)origPoints.size());
    DTMDrapedLinePtr drapedLine;
    auto draping = smPtr->GetDTMDraping();
    draping->DrapeLinear(drapedLine, origPoints.data(), (int)origPoints.size());
    if (drapedLine.IsNull())
        return ERROR;
    delete draping;
    unsigned int numPoints = drapedLine->GetPointCount();
    drapedPoints.resize(1);
    for (unsigned int ptNum = 0; ptNum < numPoints; ptNum++)
        {
        DPoint3d pt;
        drapedLine->GetPointByIndex(&pt, nullptr, nullptr, ptNum);
        drapedPoints[0].push_back(pt);
        }
    
    return SUCCESS;
    }*/