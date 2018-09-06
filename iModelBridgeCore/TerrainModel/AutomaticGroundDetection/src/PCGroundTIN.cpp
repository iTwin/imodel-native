/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/src/PCGroundTIN.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "AutomaticGroundDetectionPch.h"

#include <TerrainModel/AutomaticGroundDetection/GroundDetectionMacros.h>
#include "GroundDetectionManagerDc.h"
#include "BcDtmProvider.h"

#include "PCGroundTIN.h"
//#include "TriangleSearcher.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_TERRAINMODEL


BEGIN_GROUND_DETECTION_NAMESPACE



const size_t PCGroundTIN::CONTAINER_MAX_SIZE = 400000;
const double PCGroundTIN::HISTO_STEP_PRECISION_FACTOR = 0.010; //default precision in meters
const size_t PCGroundTIN::MAX_HISTO_STEP = 100000;
const size_t PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD = 1; //Number of point that will be add as seed points
const double PCGroundTIN::SEED_BORDER_FACTOR = 1.0; //the percentage of the max grid size we will use around the border -> help add seed points near the border
const double Triangle::TOLERANCE_FACTOR = 0.001; //One millimeter
const unsigned int PCGroundTINMT::MAX_NUMBER_THREAD = 8;

BeMutex PCGroundTINMT::s_newPointToAddCS;

static const uint32_t PROGESS_UPDATE_TIME = 2000;

/*=================================================================================**//**
* Wrapper class over a ProgressReport class to support multithread
* @bsiclass                                             Marc.Bedard        11/2014
+===============+===============+===============+===============+===============+======*/
class  ProgressMonitor
    {
    public:
        ProgressMonitor(ProgressReport& report, size_t processSize, bool refreshDisplayOnProgress, bool useMultiThread) :m_pReport(&report),m_processSize(processSize), m_useMutiThread(useMultiThread), m_refreshDisplay(refreshDisplayOnProgress)
            {
            s_process_canceled = false;
            s_errorOccured = false;

            s_workDone = 0;
            m_lastWorkDone=0;
            }

        ~ProgressMonitor()
            {
            }

        void  SetWorkToDo(size_t workToDo) 
            {
            m_processSize = workToDo; 
            s_workDone = 0;
            }

        //return true if process should continue (was not canceled)
        static bool UpdateProgressInThread()
            {
            //BeMutexHolder lk(s_CSProgress);
            bool shouldContinue = UpdateProgress();
            return shouldContinue;
            }
        static bool CheckContinueInThread()
            {
            //BeMutexHolder lk(s_CSProgress);
            return !s_process_canceled;
            }

        static void SignalErrorInThread()
            {
            //BeMutexHolder lk(s_CSProgress);
            SignalError();
            }

        //return true if process should continue (was not canceled)
        static bool UpdateProgress()
            {
            //Always increment
            s_workDone++;
            bool shouldContinue(!s_process_canceled);
            return shouldContinue;
            }

        static size_t GetWorkDone()
            {
            return s_workDone;
            }

        static size_t GetWorkDoneMT()
            {
            //BeMutexHolder lk(s_CSProgress);
            return s_workDone;
            }

        static void SignalError()
            {
            s_process_canceled = true;
            s_errorOccured = true;
            }

        void FinalStageProcessing()
            {
            m_pReport->SetWorkDone(1.0);
            s_process_canceled = !m_pReport->CheckContinueOnProgress();
            }


        //return true if process should continue (was not canceled)
        bool InProgress()
            {
            if (m_useMutiThread)
                {
                size_t actualWorkDone(GetWorkDoneMT());
                ShowProgressMT(actualWorkDone);
                if (m_refreshDisplay)
                    RefreshMSViewMT(true);
                return CheckContinueMT();
                }
            ShowProgress(GetWorkDone());

            if (m_refreshDisplay)
                RefreshMSView(true);

            return CheckContinue();
            }

        static bool WasCanceled()    {return s_process_canceled;}
        static bool WasError()       { return s_errorOccured; }

    private:
        /*GDZERO
        static BeCriticalSection        s_CSProgress;
        static BeCriticalSection        s_reportCS;
        */

        static size_t       s_workDone;
        static bool         s_process_canceled;
        static bool         s_errorOccured;

        bool        m_useMutiThread;
        bool        m_refreshDisplay;
        size_t      m_processSize;
        ProgressReportPtr  m_pReport;
        size_t      m_lastWorkDone;

        //return true if process should continue (was not canceled)
        bool ShowProgress(size_t workDone)
            {
            double accomplishmentRatio = (static_cast<double>(workDone) / static_cast<double>(m_processSize));
            m_pReport->SetWorkDone(accomplishmentRatio);

            if (s_errorOccured)
                m_pReport->OnSignalError();

            return (workDone < m_processSize && !s_process_canceled);
            }
        bool ShowProgressMT(size_t workDone)
            {
            double accomplishmentRatio = (static_cast<double>(workDone) / static_cast<double>(m_processSize));
            {
            //BeMutexHolder lk(s_reportCS);
            m_pReport->SetWorkDone(accomplishmentRatio);
            }

            if (s_errorOccured)
                {
                //BeMutexHolder lk(s_reportCS);
                m_pReport->OnSignalError();
                }

            return (workDone < m_processSize && !s_process_canceled);
            }

        //return true if process should continue (was not canceled)
        bool CheckContinue()
            {
            {
            s_process_canceled = !m_pReport->CheckContinueOnProgress();
            }

            return (s_workDone < m_processSize && !s_process_canceled);
            }
        bool CheckContinueMT()
            {
                {
                //BeMutexHolder lk(s_reportCS);
                s_process_canceled = !m_pReport->CheckContinueOnProgress();
                }

            return (s_workDone < m_processSize && !s_process_canceled);
            }

        void RefreshMSView(bool incremental)
            {
            m_pReport->RefreshMSView(incremental);
            }
        void RefreshMSViewMT(bool incremental)
            {
            //BeMutexHolder lk(s_reportCS);

            m_pReport->RefreshMSView(incremental);
            }


    }; // ProgressMonitor

size_t      ProgressMonitor::s_workDone = 0;
bool        ProgressMonitor::s_process_canceled = (false);
bool        ProgressMonitor::s_errorOccured     = (false);
/*GDZERO
BeCriticalSection   ProgressMonitor::s_CSProgress;
BeCriticalSection   ProgressMonitor::s_reportCS;
*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool CheckProcessNotAborted()
    {
    return ProgressMonitor::CheckContinueInThread();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawingFacility::DrawTriangle(DPoint3d const& pt1, DPoint3d const& pt2, DPoint3d const& pt3, Transform const& metersToUors)
    {
    /*
    MSElement lineElm;
    PointCollection pointArray;

    pointArray.push_back(pt1);
    pointArray.push_back(pt2);
    pointArray.push_back(pt3);
    pointArray.push_back(pt1);

    metersToUors.multiply(&pointArray[0], (int)pointArray.size());

    MSElementDescr* pElmDscr;
    mdlElmdscr_createFromVertices(&pElmDscr, NULL, &pointArray[0], pointArray.size(), TRUE, 0);
    EditElementHandle eeh(pElmDscr, true, false, ACTIVEMODEL);
    eeh.AddToModel();*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawingFacility::DrawPoint(DPoint3d const& pt, Transform const& metersToUors)
    {
    /*
    double uorPerMetersScale = mdlModelRef_getUorPerMeter(ACTIVEMODEL);
    double AxisLengthUor = 1 * uorPerMetersScale;

    MSElement       ellipseElm;
    DPoint3d PointInUors(pt);
    metersToUors.Multiply(PointInUors);

    int status = mdlEllipse_create(&ellipseElm, 0, &PointInUors, AxisLengthUor, AxisLengthUor, NULL, 1);

    EditElementHandle ellipseElement(&ellipseElm, ACTIVEMODEL);
    ellipseElement.AddToModel();*/
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawingFacility::DrawBoundingBox(DRange3d const& boundingBox, Transform const& metersToUors)
    {
    // Create a new US element from the vertices.
    /*
    PointCollection pointArray;
    DPoint3d point;
    point.x = boundingBox.low.x;
    point.y = boundingBox.low.y;
    point.z = boundingBox.high.z;
    pointArray.push_back(point);
    point.x = boundingBox.low.x;
    point.y = boundingBox.high.y;
    point.z = boundingBox.high.z;
    pointArray.push_back(point);
    point.x = boundingBox.high.x;
    point.y = boundingBox.high.y;
    point.z = boundingBox.high.z;
    pointArray.push_back(point);
    point.x = boundingBox.high.x;
    point.y = boundingBox.low.y;
    point.z = boundingBox.high.z;
    pointArray.push_back(point);

    //Close top of the box
    point.x = boundingBox.low.x;
    point.y = boundingBox.low.y;
    point.z = boundingBox.high.z;
    pointArray.push_back(point);

    //Then go with bottom
    point.x = boundingBox.low.x;
    point.y = boundingBox.low.y;
    point.z = boundingBox.low.z;
    pointArray.push_back(point);
    point.x = boundingBox.low.x;
    point.y = boundingBox.high.y;
    point.z = boundingBox.low.z;
    pointArray.push_back(point);
    point.x = boundingBox.high.x;
    point.y = boundingBox.high.y;
    point.z = boundingBox.low.z;
    pointArray.push_back(point);
    point.x = boundingBox.high.x;
    point.y = boundingBox.low.y;
    point.z = boundingBox.low.z;
    pointArray.push_back(point);

    //Close bottom
    point.x = boundingBox.low.x;
    point.y = boundingBox.low.y;
    point.z = boundingBox.low.z;
    pointArray.push_back(point);

    metersToUors.Multiply(&pointArray[0], (int)pointArray.size());


    MSElementDescr* pElmDscr;
    mdlElmdscr_createFromVertices(&pElmDscr, NULL, &pointArray[0], pointArray.size(), TRUE, 0);
    EditElementHandle eeh2(pElmDscr,true,false,ACTIVEMODEL);
    eeh2.AddToModel();
    */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TINPointContainerPtr TINPointContainer::Create()
    {
    return new TINPointContainer();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TINPointContainer::TINPointContainer()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TINPointContainer::~TINPointContainer()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void TINPointContainer::AddPoint(DPoint3d& ptIndex, PCGroundTriangle& pcGroundTriangle)
    {
    BeMutexHolder lock(m_pointContainerMutex);

    if (PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD > 1)
        {
        push_back(ptIndex);
        if (size() > PCGroundTIN::CONTAINER_MAX_SIZE)
            {
            //Sort and keep only the MAX_NB_SEEDPOINTS_TO_ADD first entries
            PCGroundTrianglePointAcceptedPredicat fPredicat(pcGroundTriangle);
            //Get only accepted point in a temporary collection
            PointCollection tempCollection(size());
            PointCollection::iterator lastCopied = copy_if(begin(), end(), tempCollection.begin(), fPredicat);
            size_t nbElementCopied = lastCopied - tempCollection.begin();
            if (nbElementCopied == 0)
                {
                //if no point, just clear our container
                clear();
                }
            else
                {
                //update our collection with first MAX_NB_SEEDPOINTS_TO_ADD accepted points
                tempCollection.resize(nbElementCopied);
                ZValueEntryCompare fZOrder;
                nth_element(tempCollection.begin(), tempCollection.begin() + (PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD-1), tempCollection.end(), fZOrder);
                resize(PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD);
                copy_n(tempCollection.begin(), PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD, end());
                }
            }
        }
    else
        {
        PCGroundTrianglePointAcceptedPredicat fPredicatIsPointAccepted(pcGroundTriangle);
        if (fPredicatIsPointAccepted(ptIndex))
            {
            //ZValueEntryCompare fPredicatIsZSmaller;
            PCGroundTrianglePointSorterPredicat fPredicatIsTinPointBetter(pcGroundTriangle);
            if (empty())
                push_back(ptIndex);
            else if (fPredicatIsTinPointBetter(ptIndex, *begin()))
                *begin() = ptIndex;
            }
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTrianglePtr PCGroundTriangle::Create(PCGroundTIN& pcGroundTIN, DPoint3d const& pt1, DPoint3d const& pt2, DPoint3d const& pt3)
    {
    return new PCGroundTriangle(pcGroundTIN, Triangle(pt1,pt2,pt3));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTrianglePtr PCGroundTriangle::Create(PCGroundTIN& pcGroundTIN, Triangle const& triangle)
    {
    return new PCGroundTriangle(pcGroundTIN, triangle);
    }


 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTriangle::PCGroundTriangle(PCGroundTIN& pcGroundTIN, Triangle const& triangle)
:m_PCGroundTin(pcGroundTIN),
m_triangle(triangle),
m_memorySize(0),
m_pAcceptedPointCollection(TINPointContainer::Create())
    {
    DRange3d boundingBoxMeters = ComputeBoundingBox(pcGroundTIN.GetParamR());
    pcGroundTIN.GetParam().GetMetersToUors().Multiply(m_boundingBoxUors, boundingBoxMeters);
    //GDZERO
    IPointsProviderCreatorPtr ptsProviderCreator(pcGroundTIN.GetParam().GetPointsProviderCreator());
    m_pPointsProvider = IPointsProvider::CreateFrom(ptsProviderCreator, &m_boundingBoxUors);    
    m_pPointsProvider->SetUseMultiThread(pcGroundTIN.GetParam().GetUseMultiThread());
    m_pPointsProvider->SetUseMeterUnit(true);//We want to work in meters, faster for pointCloud...           
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTriangle::~PCGroundTriangle()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsPointOnPlaneInside(DPoint3d pointOnPlane, bool strictlyInside) const
    {
    return m_triangle.IsPointOnPlaneInside(pointOnPlane, strictlyInside);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d PCGroundTriangle::ComputeBoundingBox(GroundDetectionParameters const& params) const
    {
    DPlane3d planeFromTriangle(m_triangle.GetPlane());

    DVec3d normalScaled;
    normalScaled.ScaleToLength(planeFromTriangle.normal,params.GetHeightThreshold());

    DRange3d range(m_triangle.GetDRange3d());

    range.ExtendBySweep(normalScaled);
    normalScaled.Negate();
    range.ExtendBySweep(normalScaled);

    DRange3d tinBoundingBox(m_PCGroundTin.GetBoundingBox());
    DPoint3d lowerPoint(DPoint3d::From(range.low.x, range.low.y, tinBoundingBox.low.z));
    range.Extend(lowerPoint);

    return range;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTriangle::DrawBoundingBox() const
    {
    Transform ident(Transform::FromIdentity());
     DrawingFacility::DrawBoundingBox(m_boundingBoxUors, ident);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTriangle::DrawTriangle() const
    {
    DrawingFacility::DrawTriangle(m_triangle.GetPoint(0), m_triangle.GetPoint(1), m_triangle.GetPoint(2), m_PCGroundTin.GetParam().GetMetersToUors());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsEqualToOneCoordinate(DPoint3d const& point) const
    {
    return m_triangle.IsEqualToOneCoordinate(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsDensificationRequired() const
    {
    // check every edges to see if below threshold
    bool isEdge1BelowT(false);
    bool isEdge2BelowT(false);
    bool isEdge3BelowT(false);
    DVec3d v0(DVec3d::FromStartEnd(m_triangle.GetPoint(0), m_triangle.GetPoint(2)));
    if (v0.Magnitude() < m_PCGroundTin.GetParam().GetTriangleEdgeThreshold())
        isEdge1BelowT = true;
    DVec3d v1(DVec3d::FromStartEnd(m_triangle.GetPoint(0), m_triangle.GetPoint(1)));
    if (v1.Magnitude() < m_PCGroundTin.GetParam().GetTriangleEdgeThreshold())
        isEdge2BelowT = true;
    DVec3d v2(DVec3d::FromStartEnd(m_triangle.GetPoint(1), m_triangle.GetPoint(2)));
    if (v2.Magnitude() < m_PCGroundTin.GetParam().GetTriangleEdgeThreshold())
        isEdge3BelowT = true;
//     if (isEdge1BelowT && isEdge2BelowT && isEdge3BelowT)
//         return false;
    //Stop densification if two edges on 3 is below threshold
    if ((isEdge1BelowT && isEdge2BelowT) || (isEdge1BelowT && isEdge3BelowT) || (isEdge2BelowT && isEdge3BelowT))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsAcceptedForTINDensification(DPoint3d const& point) const
    {
    //If the point is one of our coordinate , don't accept the point -> 
    //we dont want to add point we already know!
    if (IsEqualToOneCoordinate(point))
        return false;

    return IsAccepted(point,false);
    }
 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsAcceptedForClassification(DPoint3d const& point) const
    {
    return IsAccepted(point,true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsAcceptedFromCuttingOffEdgeCriteria(DPoint3d const& point, double distanceThreshold) const
    {
    //Find closest node point
    DVec3d v0(DVec3d::FromStartEnd(point, m_triangle.GetPoint(0)));
    DVec3d v1(DVec3d::FromStartEnd(point, m_triangle.GetPoint(1)));
    DVec3d v2(DVec3d::FromStartEnd(point, m_triangle.GetPoint(2)));
    double v0M = v0.Magnitude();
    double v1M = v1.Magnitude();
    double v2M = v2.Magnitude();

    DPoint3d closesNodePt[3];
    short    closesNodePtCount = 0;

    if ((v0M == v1M) && (v0M == v2M))
        {
        //All three point equidistant
        closesNodePt[0] = m_triangle.GetPoint(0);
        closesNodePt[1] = m_triangle.GetPoint(1);
        closesNodePt[2] = m_triangle.GetPoint(2);
        closesNodePtCount = 3;
        }
    else if (v0M == v1M)
        {
        //Special case!
        if (v2M < v0M)
            {
            closesNodePt[0] = m_triangle.GetPoint(2);
            closesNodePtCount=1;
            }
        else //v0M and v1M
            {
            //special case with two point
            closesNodePt[0] = m_triangle.GetPoint(0);
            closesNodePt[1] = m_triangle.GetPoint(1);
            closesNodePtCount = 2;
            }
        }
    else if (v0M == v2M)
        {
        //Special case!
        if (v1M < v0M)
            {
            closesNodePt[0] = m_triangle.GetPoint(1);
            closesNodePtCount = 1;
            }
        else //v0M and v2M
            {
            //special case with two point
            closesNodePt[0] = m_triangle.GetPoint(0);
            closesNodePt[1] = m_triangle.GetPoint(2);
            closesNodePtCount = 2;
            }
        }
    else if (v1M == v2M)
        {
        //Special case!
        if (v0M < v1M)
            {
            closesNodePt[0] = m_triangle.GetPoint(0);
            closesNodePtCount = 1;
            }
        else //v1M and v2M
            {
            //special case with two point
            closesNodePt[0] = m_triangle.GetPoint(1);
            closesNodePt[1] = m_triangle.GetPoint(2);
            closesNodePtCount = 2;
            }
        }
    else if (v0M < v1M)
        { 
        if (v0M < v2M)
            {
            closesNodePt[0] = m_triangle.GetPoint(0);
            closesNodePtCount = 1;
            }
        else if (v2M < v0M)
            {
            closesNodePt[0] = m_triangle.GetPoint(2);
            closesNodePtCount = 1;
            }
        }
    else if (v0M < v2M)
        {
        //v0M < v1M already check above
        if (v1M < v0M)
            {
            closesNodePt[0] = m_triangle.GetPoint(1);
            closesNodePtCount = 1;
            }
        }
    else if (v1M < v2M)
        {
        //v0M > v1M and v0M > v2M
        if (v1M < v0M)
            {
            closesNodePt[0] = m_triangle.GetPoint(1);
            closesNodePtCount = 1;
            }
        }
    else if (v2M < v1M)
        {
        //v0M < v2M already check above
        if (v2M < v0M)
            {
            closesNodePt[0] = m_triangle.GetPoint(2);
            closesNodePtCount = 1;
            }
        }

    //First find the mirrored point to the closest node point point
    for (short i = 0; i < closesNodePtCount; i++)
        {
        //Find mirrored point
        DPoint3d mirroredPt;
        double   distance;
        bool isFound  = m_PCGroundTin.FindMirroredPointAndTriangle(distance, mirroredPt, point, closesNodePt[i]);

        //No triangle found, continue...
        if (!isFound)
            continue;

        if (distance <= distanceThreshold)
            return true;
        }


    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsAcceptedFromDistanceCriteria(double& distanceFound, DPoint3d const& point, DPlane3d& planeFromTriangle, double distanceThreshold) const
    {
    planeFromTriangle = m_triangle.GetPlane();
    if (planeFromTriangle.normal.z < 0)
        planeFromTriangle.normal.Negate();
    if (planeFromTriangle.Normalize()) //true if normal vector has nonzero length
        {
        //If the plane normal is a unit vector, this is the true distance from the
        //plane to the point.  If not, it is a scaled distance.
        //The plane is normalized, so it is the true distance.
        distanceFound = planeFromTriangle.Evaluate(point);
        if (distanceFound <= 0.0)
            return true;//always accept point below the plane...
        if ((distanceFound > distanceThreshold))
            return false;
        }
    else
        {
        //Plane normal has zero length
        return false;
        }
    return true;//Accepted!
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::IsAccepted(DPoint3d const& point, bool isClassification) const
    {
    double distanceThreshold = isClassification ? m_PCGroundTin.GetParam().GetClassificationTolerance()
        : m_PCGroundTin.GetParam().GetHeightThreshold();
    bool isProjectedPointInsideTriangle(false);    
    DPlane3d planeFromTriangle;
    double distanceFound;
    if (!IsAcceptedFromDistanceCriteria(distanceFound,point, planeFromTriangle, distanceThreshold))
        {
        //Don't check cutting edges during classification
        if (isClassification)
            return false;
        else if (!IsAcceptedFromCuttingOffEdgeCriteria(point, distanceThreshold))
            return false;
        }

    bool IsBelowSurface(distanceFound<=0);


    //Don't check angle during classification
    //Also don't check angle if point below surface...always accept point below actual surface.
    if (!isClassification || !IsBelowSurface)
        {
        DPoint3d projectedPoint;
        planeFromTriangle.ProjectPoint(projectedPoint, point);
        isProjectedPointInsideTriangle = IsPointOnPlaneInside(projectedPoint,false);

        //Don't want point that are not inside our triangle
        if (!isProjectedPointInsideTriangle)
            return false;

        //Now check angle threshold
        DVec3d v0p(DVec3d::FromStartEnd(m_triangle.GetPoint(0), point));//Vector from coordinate 0 to input point p.
        DVec3d v0Pp(DVec3d::FromStartEnd(m_triangle.GetPoint(0), projectedPoint));//Vector from coordinate 0 to projected input point Pp.
        double alpha = v0Pp.AngleTo(v0p); //Angle in Radian between 0 and pi
        if (alpha > m_PCGroundTin.GetParam().GetAngleThreshold().Radians())
            {
            return false;
            }

        DVec3d v1p(DVec3d::FromStartEnd(m_triangle.GetPoint(1), point));//Vector from coordinate 1 to input point p.
        DVec3d v1Pp(DVec3d::FromStartEnd(m_triangle.GetPoint(1), projectedPoint));//Vector from coordinate 1 to projected input point Pp.
        double beta  = v1Pp.AngleTo(v1p); //Angle in Radian between 0 and pi
        if (beta > m_PCGroundTin.GetParam().GetAngleThreshold().Radians())
            {
            return false;
            }

        DVec3d v2p(DVec3d::FromStartEnd(m_triangle.GetPoint(2), point));//Vector from coordinate 2 to input point p.
        DVec3d v2Pp(DVec3d::FromStartEnd(m_triangle.GetPoint(2), projectedPoint));//Vector from coordinate 2 to projected input point Pp.
        double gamma = v2Pp.AngleTo(v2p); //Angle in Radian between 0 and pi
        if (gamma > m_PCGroundTin.GetParam().GetAngleThreshold().Radians())
            {
            return false;
            }
        }

    //All threshold pass with success, point accepted!
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTrianglePointSorterPredicat::operator()(const DPoint3d& a, const DPoint3d& b)
    {
    //return true if a is nearest to centroid than b, and if a is lowest than b if equal distance to centroid
    DVec3d vac(DVec3d::FromStartEnd(a, m_PCGroundTriangle.GetCentroid()));
    DVec3d vbc(DVec3d::FromStartEnd(b, m_PCGroundTriangle.GetCentroid()));
    double aLength = vac.Magnitude();
    double bLength = vbc.Magnitude();
    if (DOUBLE_EQUAL_TOL(aLength, bLength, Triangle::TOLERANCE_FACTOR))
        return a.z < b.z;
    return (aLength < bLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTrianglePointAcceptedPredicat::operator()(const DPoint3d& a)
    {
    return m_PCGroundTriangle.IsAcceptedForTINDensification(a);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTriangle::PrefetchPoints()
    {
    m_pPointsProvider->PrefetchPoints();
    m_memorySize = m_pPointsProvider->GetMemorySize();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::QueryPointToAddToTin()
    {          
    m_queryPointMutex.lock();

    for (auto itr = m_pPointsProvider->begin(); itr != m_pPointsProvider->end(); ++itr)
        {
        DPoint3d ptIndex(*itr);
        //Point will only be added if  PCGroundTrianglePointAcceptedPredicat is true
        //Point will be sorted by PCGroundTrianglePointSorterPredicat
        //Only PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD will be keep in the container
        m_pAcceptedPointCollection->AddPoint(ptIndex,*this);
        }
    //Free our memory, we don't need it anymore for now
    m_pPointsProvider->ClearPrefetchedPoints();

    m_queryPointMutex.unlock();
                          
    //if no point, nothing to Add
    if (m_pAcceptedPointCollection->size() == 0)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTriangle::TryPointToAddToTin(const DPoint3d& pt)
    {
    DPoint3d ptToAdd(pt);

    m_pAcceptedPointCollection->AddPoint(ptToAdd, *this);

    //if no point, nothing to Add
    if (m_pAcceptedPointCollection->size() == 0)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  PCGroundTriangle::GetMemorySize() const
    {
    return m_memorySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryAllPointsForTriangleWork::_DoWork()
    {
    IGroundPointsAccumulatorPtr ptsAccumPtr(m_PCGroundTin.GetParamR().GetGroundPointsAccumulator());

    if (!ptsAccumPtr->ShouldContinue())
        return;

    try
        {       
        m_PCGroundTriangle->PrefetchPoints();
        //Start densify work in a new queue
        DensifyTriangleWorkPtr pWork(DensifyTriangleWork::Create(m_PCGroundTin, *m_PCGroundTriangle));
        pWork->DoWork();
        //m_PCGroundTin.GetThreadPool().QueueWork(*pWork);
        }
    catch (...)
        {
        ProgressMonitor::SignalErrorInThread();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  QueryAllPointsForTriangleWork::_GetMemorySize()
    {
    return m_PCGroundTriangle->GetMemorySize();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void DensifyTriangleWork::_DoWork()
    {
    IGroundPointsAccumulatorPtr ptsAccumPtr(m_PCGroundTin.GetParamR().GetGroundPointsAccumulator());    

    if (!ptsAccumPtr->ShouldContinue())
        return;

    try
        {        
        m_PCGroundTriangle->QueryPointToAddToTin();
        size_t nbSeedPointsToAdded(0);
        for (auto itr = m_PCGroundTriangle->GetPointToAdd().begin();
             itr != m_PCGroundTriangle->GetPointToAdd().end() && nbSeedPointsToAdded < PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD;
             ++itr, nbSeedPointsToAdded++)            
            m_PCGroundTin.AddPoint(*itr);            

        m_PCGroundTin.IncrementWorkDone();
        }
    catch (...)
        {
        ProgressMonitor::SignalErrorInThread();
        }
    }
 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  DensifyTriangleWork::_GetMemorySize()
    {
    return m_PCGroundTriangle->GetMemorySize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryAllPointsForFirstSeedPointWork::_DoWork()
    {
    try
        {
        m_pGridCellEntry->PrefetchPoints();
        FindFirstSeedPointWorkPtr pWork(FindFirstSeedPointWork::Create(m_PCGroundTin, *m_pGridCellEntry));
        pWork->DoWork();
        //m_PCGroundTin.GetThreadPool().QueueWork(*pWork);
        }
    catch (...)
        {
        ProgressMonitor::SignalErrorInThread();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t  QueryAllPointsForFirstSeedPointWork::_GetMemorySize()  
    {
    return m_pGridCellEntry->GetMemorySize();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void FindFirstSeedPointWork::_DoWork()
    {
    try
        {
        m_pGridCellEntry->QueryFirstSeedPointAndAddToTin(m_PCGroundTin);
        m_PCGroundTin.IncrementWorkDone();
        }
    catch (...)
        {
        ProgressMonitor::SignalErrorInThread();
        }
    }


#if 0 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassifyPointCloudWork::_DoWork()
    {
    try
        {
        BeAssert(NULL != m_PCGroundTin.GetParam().GetElementHandleToClassifyCP());
        m_pGridCellEntry->Classify(*m_PCGroundTin.GetParam().GetElementHandleToClassifyCP(), m_PCGroundTin);
        m_PCGroundTin.IncrementWorkDone();
        }
    catch (...)
        {
        ProgressMonitor::SignalErrorInThread();
        }
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTINPtr PCGroundTIN::Create(GroundDetectionParameters& params, ProgressReport& report)
    {
    return new PCGroundTIN(params, report);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTIN::PCGroundTIN(GroundDetectionParameters& params, ProgressReport& report)
:m_pBcDtm(BcDtmProvider::Create()),
m_pParams(&params),
m_pReport(&report),
m_pGDGrid(GroundDetectionGrid::Create(params)),
m_shouldStopIteration(false),
m_strikeToStopComputeParameters(0),
m_oldAllowedAngle(0.0),
m_oldAllowedHeight(0.0),
m_isFirstIteration(true), 
m_lastOutputPreviewTime(0)
    {
    m_boundingBoxMeter = m_pGDGrid->GetBoundingBox();
    double minValue = m_boundingBoxMeter.low.z;
    double maxValue = m_boundingBoxMeter.high.z;
    double histoStepPrecisionFactor(PCGroundTIN::HISTO_STEP_PRECISION_FACTOR);
    double histoSteps = (maxValue - minValue) / histoStepPrecisionFactor;
    //Limit size of our histogram array to a reasonable size... 
    while (histoSteps > PCGroundTIN::MAX_HISTO_STEP)
        {
        histoStepPrecisionFactor *=2.0;
        histoSteps = (maxValue - minValue) / histoStepPrecisionFactor;
        }
    m_pHeightDeltaHisto = DiscreetHistogram::Create(histoStepPrecisionFactor, (maxValue - minValue), static_cast<size_t>(histoSteps));
    m_pAnglesHisto = DiscreetHistogram::Create(0, PI / 2.0, 10000);
    /*
    if (GroundDetectionParameters::USE_EXISTING_DTM == params.GetCreateDtmFile() && !WString::IsNullOrEmpty(params.GetDtmFilename().c_str()))
        {
        m_pBcDtm = BcDtmProvider::CreateFrom(params.GetDtmFilename().c_str());
        _ComputeTrianglesToProcess();

        //If we don't want to densify or classify, at least compute our parameters once.
        if (!params.GetDensifyTin() && !params.GetClassifyPointCloud())
            ComputeParameterFromTINPoints();
        }
        */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTIN::~PCGroundTIN()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::SetNewSeedPoints(const bvector<DPoint3d>& newPoints)
    {
    _SetNewSeedPoints(newPoints);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_SetNewSeedPoints(const bvector<DPoint3d>& newPoints)
{
    m_pBcDtm = BcDtmProvider::Create();

    m_newPointToAdd.clear(); //clear the list
    
    for (size_t i = 0; i<newPoints.size(); i++)
        m_newPointToAdd.insert(newPoints[i]);

    _ComputeTriangulation();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::AddPoint(DPoint3d const& point)
    {
    _AddPoint(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_AddPoint(DPoint3d const& point)
    { 
    m_newPointToAdd.insert(point); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTIN::PrepareFirstIteration()
    {
    m_strikeToStopComputeParameters=0;
    m_shouldStopIteration=false;
    m_isFirstIteration=true;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTIN::PrepareNextIteration()
    {
    if (m_shouldStopIteration)
        return false;

    size_t previousTrianglesCount(m_pBcDtm->GetTriangleCount());
    size_t newTrianglesCount = _ComputeTriangulation();

    if (newTrianglesCount==0)
        return false; //No triangle

    //If we don't have new points or number of triangles stay constant, stop to densify, but iter at least once
    if (!m_isFirstIteration && ((newTrianglesCount == previousTrianglesCount) || (m_newPointToAdd.size()==0)))
        return false; //Stop densification if triangle count stay constant...
    m_isFirstIteration=false;

    ComputeParameterFromTINPoints();

    _ComputeTrianglesToProcessFromPointToAdd();
    BeAssert(m_newPointToAdd.size()==0);//Point to add has been processed

    return (m_trianglesToProcess.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PCGroundTIN::CreateInitialTIN()
    {
    StatusInt status = _CreateInitialTIN();

#if defined(DEBUG_GROUND_DETECTION)
    DrawGrid();
    DrawSeedPoints();
#endif

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.McKenzie                 04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::GetDTMPoints(bvector<DPoint3d>& points)
    {
    _GetDTMPoints(points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PCGroundTIN::DensifyTIN()
    {    
    StatusInt status = _DensifyTIN();
    DrawTriangles();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PCGroundTIN::Classify()
    {    
    StatusInt status = _Classify();
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::IncrementWorkDone()
    {
    return _IncrementWorkDone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PCGroundTIN::_CreateInitialTIN()
    {
    m_pReport->StartPhase(1, L"START - Find Initial Seed Points");
    if (!m_pReport->CheckContinueOnProgress())
        return ERROR;//User abort

    ProgressMonitor progressMonitor(*m_pReport, m_pGDGrid->GetSize(), false, m_pParams->GetUseMultiThread());

    for (size_t i = 0; i < m_pGDGrid->GetSize() && progressMonitor.InProgress(); i++, IncrementWorkDone())
        {
        GridCellEntryPtr pGridCellEntry = m_pGDGrid->GetGridCellEntry(i);

        pGridCellEntry->QueryFirstSeedPointAndAddToTin(*this);
        }
    progressMonitor.FinalStageProcessing();

    if (progressMonitor.WasCanceled() || progressMonitor.WasError())
        {
        m_pReport->EndPhase(L"ABORT - Find Initial Seed Points");

        return ERROR;
        }

    //If we don't want to densify or classify, at least compute our parameters once.
    if (!m_pParams->GetDensifyTin())
        {
        size_t trianglesCount = _ComputeTriangulation();
        if (trianglesCount==0)
            return ERROR;

        ComputeParameterFromTINPoints();
        }

    m_pReport->EndPhase(L"END - Find Initial Seed Points");

    bvector<DPoint3d> additionalSeedPoints;

    m_pParams->GetAdditionalSeedPoints(additionalSeedPoints);

    for (auto& seedPoint : additionalSeedPoints)
        {
        AddPoint(seedPoint);
        }
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static bool s_testOneQuery = false;
static bool s_outputPreview = true;

static clock_t outputDelay = 10 * CLOCKS_PER_SEC;


void PCGroundTIN::OutputDtmPreview(bool noDelay, BeMutex* newPointToAddMutex)
    {            
    if (s_outputPreview && 
        (((clock() - m_lastOutputPreviewTime) > outputDelay) || noDelay))
        {
        IGroundPointsAccumulatorPtr ptsAccumPtr(GetParamR().GetGroundPointsAccumulator());
        assert(ptsAccumPtr.IsValid());

        Transform transform;
        ptsAccumPtr->GetPreviewTransform(transform);

        BENTLEY_NAMESPACE_NAME::TerrainModel::BcDTMPtr bcDtmPtr(((BcDtmProvider*)m_pBcDtm.get())->GetBcDTM());

        size_t newPointToAddSize;

        if (newPointToAddMutex != nullptr)
            {
            newPointToAddMutex->lock();
            newPointToAddSize = m_newPointToAdd.size();
            newPointToAddMutex->unlock();
            }
        else
            {
            newPointToAddSize = m_newPointToAdd.size();
            }

        if (newPointToAddSize > 0)
            {
            bcDtmPtr = bcDtmPtr->Clone();

            if (newPointToAddMutex != nullptr)
                {
                newPointToAddMutex->lock();
                }

            for (auto& point : m_newPointToAdd)
                {
                bcDtmPtr->AddPoint(point);
                }

            if (newPointToAddMutex != nullptr)
                {
                newPointToAddMutex->unlock();
                }            

            if (!transform.IsIdentity())
                {
                DTMStatusInt status = bcDtmPtr->Transform(transform);
                assert(status == SUCCESS);
                }
            
            DTMStatusInt status = bcDtmPtr->Triangulate();
            assert(status == SUCCESS);
            }
        else
        if (!transform.IsIdentity())
            {
            bcDtmPtr = bcDtmPtr->Clone();
            DTMStatusInt status = bcDtmPtr->Transform(transform);
            assert(status == SUCCESS);
            }
        
        DTMMeshEnumeratorPtr en = DTMMeshEnumerator::Create(*bcDtmPtr);
    
        en->SetExcludeAllRegions();
        en->SetMaxTriangles(((BcDtmProvider*)m_pBcDtm.get())->GetBcDTM()->GetTrianglesCount() * 2);        

        for (PolyfaceQueryP pf : *en)
            {
            // Polyface returned.        
            ptsAccumPtr->OutputPreview(*pf);
            }

        m_lastOutputPreviewTime = clock();        
        }
    }

StatusInt  PCGroundTIN::_DensifyTIN()
    {       
    if (s_testOneQuery)
        {      
#if 0 
        int currentIteration = 1;
        for (PrepareFirstIteration(); PrepareNextIteration(); currentIteration++)
            {            
            m_pReport->StartCurrentIteration(currentIteration);
                        
            if (!m_pReport->CheckContinueOnProgress())
                {
                StopIteration();
                m_pReport->EndCurrentIteration();
                m_pReport->EndPhase(L"ABORT -  Densification of TIN");
                return ERROR;//User abort
                }            

            IPointsProviderCreatorPtr ptsProviderCreator(GetParam().GetPointsProviderCreator());
            IPointsProviderPtr pPointsProvider = IPointsProvider::CreateFrom(ptsProviderCreator, &m_triangleToProcessExtent);    
            pPointsProvider->SetUseMultiThread(GetParam().GetUseMultiThread());
            pPointsProvider->SetUseMeterUnit(true);//We want to work in meters, faster for pointCloud...  
/*
            for (auto itr = pPointsProvider->begin(); itr != pPointsProvider->end(); ++itr)
                {
                DPoint3d ptIndex(*itr);
                //Point will only be added if  PCGroundTrianglePointAcceptedPredicat is true
                //Point will be sorted by PCGroundTrianglePointSorterPredicat
                //Only PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD will be keep in the container
            //   m_pAcceptedPointCollection->AddPoint(ptIndex,*this);
                }
*/

            TriangleSearcherPtr triSearcher(TriangleSearcher::Create());
            
            for (PCGroundTriangleCollection::iterator triItr = m_trianglesToProcess.begin(); triItr != m_trianglesToProcess.end() /*&& progressMonitor.InProgress()*/; ++triItr/*, ++nbTriangleProcessed, IncrementWorkDone()*/)
                {                
                CPoint a((*triItr)->GetPoint(0).x, (*triItr)->GetPoint(0).y, (*triItr)->GetPoint(0).z);
                CPoint b((*triItr)->GetPoint(1).x, (*triItr)->GetPoint(1).y, (*triItr)->GetPoint(1).z);
                CPoint c((*triItr)->GetPoint(2).x, (*triItr)->GetPoint(2).y, (*triItr)->GetPoint(2).z);
                CTriangle triangle(a, b, c);

                triSearcher->AddTriangle(triangle);                                
                }

            for (auto itr = pPointsProvider->begin(); itr != pPointsProvider->end(); ++itr)
                {
                DPoint3d ptIndex(*itr);                

                double distance;
                CTriangle nearestTriangle;
                triSearcher->SearchNearestTri(nearestTriangle, distance, ptIndex);

                DPoint3d a(DPoint3d::From(nearestTriangle.vertex(0).x(), nearestTriangle.vertex(0).y(), nearestTriangle.vertex(0).z()));
                DPoint3d b(DPoint3d::From(nearestTriangle.vertex(1).x(), nearestTriangle.vertex(1).y(), nearestTriangle.vertex(1).z()));
                DPoint3d c(DPoint3d::From(nearestTriangle.vertex(2).x(), nearestTriangle.vertex(2).y(), nearestTriangle.vertex(2).z()));

                PCGroundTrianglePtr groundTriPtr(PCGroundTriangle::Create(*this, a, b, c));

                groundTriPtr->TryPointToAddToTin(ptIndex);

                size_t nbSeedPointsToAdded(0);
                for (auto itr = groundTriPtr->GetPointToAdd().begin();
                    itr != groundTriPtr->GetPointToAdd().end() && nbSeedPointsToAdded < PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD;
                    ++itr, nbSeedPointsToAdded++)
                    AddPoint(*itr);
                }

#if 0 
            ProgressMonitor progressMonitor(*m_pReport, m_trianglesToProcess.size(), false, m_pParams->GetUseMultiThread());
            size_t nbTriangleProcessed(1);
            for (PCGroundTriangleCollection::iterator triItr = m_trianglesToProcess.begin(); triItr != m_trianglesToProcess.end() && progressMonitor.InProgress(); ++triItr, ++nbTriangleProcessed, IncrementWorkDone())
                {
                PCGroundTrianglePtr pTriangle = *triItr;
                BeAssert(pTriangle.IsValid());

                pTriangle->QueryPointToAddToTin();

                size_t nbSeedPointsToAdded(0);
                for (auto itr = pTriangle->GetPointToAdd().begin(); 
                    itr != pTriangle->GetPointToAdd().end() && nbSeedPointsToAdded < PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD; 
                    ++itr, nbSeedPointsToAdded++)
                    AddPoint(*itr);                
                }

            progressMonitor.FinalStageProcessing();
            if (progressMonitor.WasCanceled() || progressMonitor.WasError())
                {
                m_pReport->EndCurrentIteration();
                m_pReport->EndPhase(L"ABORT -  Densification of TIN");
                return ERROR;//User abort
                }
#endif
            m_pReport->EndCurrentIteration();
            }
#endif
        }
    else
        {    
        //Then we find new seed points from parameter threshold values and update our TIN
        m_pReport->StartPhase(2, L"START - Densification of TIN");
        if (!m_pReport->CheckContinueOnProgress())
            return ERROR;//User abort

        IGroundPointsAccumulatorPtr ptsAccumPtr(GetParamR().GetGroundPointsAccumulator());

        int currentIteration = 1;
        for (PrepareFirstIteration(); ptsAccumPtr->ShouldContinue() && PrepareNextIteration(); currentIteration++)
            {            
            OutputDtmPreview(true);

            m_pReport->StartCurrentIteration(currentIteration);

            //Special case for first iteration        
            //Assume every triangle to process will add one point -> and thus create three new triangles
            //thus this triangle will be replaced by three -> -1 + 3 = 2
                        
            if (!m_pReport->CheckContinueOnProgress())
                {
                StopIteration();
                m_pReport->EndCurrentIteration();
                m_pReport->EndPhase(L"ABORT -  Densification of TIN");
                return ERROR;//User abort
                }

            ProgressMonitor progressMonitor(*m_pReport, m_trianglesToProcess.size(), false, m_pParams->GetUseMultiThread());
            size_t nbTriangleProcessed(1);
            for (PCGroundTriangleCollection::iterator triItr = m_trianglesToProcess.begin(); triItr != m_trianglesToProcess.end() && progressMonitor.InProgress(); ++triItr, ++nbTriangleProcessed, IncrementWorkDone())
                {
                PCGroundTrianglePtr pTriangle = *triItr;
                BeAssert(pTriangle.IsValid());

                pTriangle->QueryPointToAddToTin();

                size_t nbSeedPointsToAdded(0);
                for (auto itr = pTriangle->GetPointToAdd().begin(); 
                    itr != pTriangle->GetPointToAdd().end() && nbSeedPointsToAdded < PCGroundTIN::MAX_NB_SEEDPOINTS_TO_ADD; 
                    ++itr, nbSeedPointsToAdded++)
                    AddPoint(*itr);

                if (!ptsAccumPtr->ShouldContinue())
                    break;

                OutputDtmPreview();
                

                //Display some stat while processing to help debug
    /*
                WChar tmpMessage[200];
                WString formatStr;
                double accomplishmentRatioItr = (static_cast<double>(actualTriangleCount) / static_cast<double>(TotalTrianglesExpected));

    #if !defined (NDEBUG)
                GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatisticDebug, STRINGID_DCPCMISC_Messages);
                BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), actualTriangleCount, trianglesToProcess, accomplishmentRatioItr * 100.0, m_pParams->GetHeightThreshold(), m_pParams->GetAngleThreshold().Degrees());
    #else
                GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatistic, STRINGID_DCPCMISC_Messages);
                BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), accomplishmentRatioItr * 100.0);
    #endif

                m_pReport->OutputMessage(tmpMessage);
    */
                }

            progressMonitor.FinalStageProcessing();
            if (progressMonitor.WasCanceled() || progressMonitor.WasError())
                {
                m_pReport->EndCurrentIteration();
                m_pReport->EndPhase(L"ABORT -  Densification of TIN");
                return ERROR;//User abort
                }                                                            

            m_pReport->EndCurrentIteration();
            }
        //Display some stat while processing to help debug
        /*
        double elapsedSecond = m_pReport->GetTimerR().GetCurrentSeconds();
        long hours = (long) (elapsedSecond / 60 / 60);
        long minutes = (long) (elapsedSecond / 60) % 60;
        long seconds = (long) elapsedSecond % 60;
        WChar tmpMessage[200];
        WString formatStr;
    
    #if !defined (NDEBUG)
        GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatisticFinalDebug, STRINGID_DCPCMISC_Messages);
        BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), m_pBcDtm->GetTriangleCount(), hours, minutes, seconds, currentIteration, m_pParams->GetHeightThreshold(), m_pParams->GetAngleThreshold().Degrees());
    #else
        GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatisticFinal, STRINGID_DCPCMISC_Messages);
        BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), m_pBcDtm->GetTriangleCount(), hours, minutes, seconds, currentIteration);
    #endif
    
        m_pReport->OutputMessage(tmpMessage);
        */
        m_pReport->EndPhase(L"END - Densification of TIN");
        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PCGroundTIN::_Classify()
    {
    m_pReport->StartPhase(3, L"START - Classification");
    if (!m_pReport->CheckContinueOnProgress())
        return ERROR;//User abort


    ProgressMonitor progressMonitor(*m_pReport, m_pGDGrid->GetSize(), false, m_pParams->GetUseMultiThread());

    for (size_t i = 0; i < m_pGDGrid->GetSize() && progressMonitor.InProgress(); i++, IncrementWorkDone())
        {
        GridCellEntryPtr pGridCellEntry = m_pGDGrid->GetGridCellEntry(i);
        }
    progressMonitor.FinalStageProcessing();

    if (progressMonitor.WasCanceled() || progressMonitor.WasError())
        {
        m_pReport->EndPhase(L"ABORT - Classification");
        return ERROR;//User abort
        }

    m_pReport->EndPhase(L"END - Classification");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_IncrementWorkDone()
    {
    ProgressMonitor::UpdateProgress();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PCGroundTIN::_ComputeTriangulation()
    {    
    //bool needToSave(m_newPointToAdd.size() > 0);
    
    if (m_newPointToAdd.size() > 0)
        {
        for (auto itr = m_newPointToAdd.begin(); itr != m_newPointToAdd.end(); ++itr)
            m_pBcDtm->AddPoint(*itr);
        }

    if (m_pBcDtm->GetPointCount()<3)
        return 0; //not enough point

    long newTrianglesCount(m_pBcDtm->ComputeTriangulation());

    //Will create/save dtm TIN file
    /*GDZERO
    if (needToSave)
        SaveDtmFile();
        */

    return newTrianglesCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elenie.Godzaridis               04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats)
    {
    m_pBcDtm->ComputeStatisticsFromDTM(angleStats, heightStats);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::ComputeParameterFromTINPoints()
    {
    //OPTIMIZATION: After three strikes where the parameters have not changed, 
    //              we will not compute them anymore
    if (m_strikeToStopComputeParameters>=2)
        return;
        
    if (m_strikeToStopComputeParameters == 0)
        {
        m_oldAllowedAngle  = m_pParams->GetAngleThreshold().Radians();
        m_oldAllowedHeight = m_pParams->GetHeightThreshold();
        }


    GetAngleDiscreetHistogram().Reset();
    GetHeightDeltaDiscreetHistogram().Reset();
    _ComputeStatisticsFromDTM(GetAngleDiscreetHistogram(), GetHeightDeltaDiscreetHistogram());
    //Height and Angle threshold factor are interpreted as percentile [0..100]
    double anglePercentileFactor = m_pParams->GetAnglePercentileFactor();
    double heightPercentileFactor = m_pParams->GetHeightPercentileFactor();
    double allowedAngle = GetAngleDiscreetHistogram().ComputePercentile(anglePercentileFactor);
    //Limit allowed angle between 0 and Pi/2
    if (allowedAngle < 0.0)
        allowedAngle=0.0;
    if (allowedAngle > Angle::PiOver2())
        allowedAngle = Angle::PiOver2();
    double allowedHeight = GetHeightDeltaDiscreetHistogram().ComputePercentile(heightPercentileFactor);

    //OPTIMIZATION: After three strikes where the parameters have not changed, 
    //              we will not compute them anymore
    if ((m_oldAllowedAngle == allowedAngle) && (m_oldAllowedHeight == allowedHeight))
        m_strikeToStopComputeParameters++;
    else
        m_strikeToStopComputeParameters=0; //Parameters have changed, restart counting strike

    //Set back in param
    m_pParams->SetAngleThreshold(Angle::FromRadians(allowedAngle));
    m_pParams->SetHeightThreshold(allowedHeight);
    if (m_pParams->GetClassificationTolEstimateState())
        m_pParams->SetClassificationTolerance(allowedHeight); 
    
    return; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::AddTriangleToProcess(PCGroundTriangle& triangle1)
    {
    return _AddTriangleToProcess(triangle1);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_AddTriangleToProcess(PCGroundTriangle& triangle)
    {
    PCGroundTrianglePtr pTriangle = &triangle;
    m_trianglesToProcess.push_back(pTriangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_ComputeTrianglesToProcessFromPointToAdd()
    {
    if (m_newPointToAdd.size() <= 0)
        return;
    //Process all faces of the mesh and add corresponding triangle when there is at least one point 
    //to add that is part of this face.
    m_trianglesToProcess.clear();
    m_triangleToProcessExtent = DRange3d::NullRange();

    for (auto itr = m_pBcDtm->begin(); itr!= m_pBcDtm->end(); ++itr)
        {
        Triangle myFace(*itr);
        //If at least one point of this face is found among our list of point to add
        //add this face to triangle to process
        if ((m_newPointToAdd.end() != m_newPointToAdd.find(myFace.GetPoint(0))) ||
            (m_newPointToAdd.end() != m_newPointToAdd.find(myFace.GetPoint(1))) ||
            (m_newPointToAdd.end() != m_newPointToAdd.find(myFace.GetPoint(2))))
            {            
            if (s_testOneQuery)
                {
                m_triangleToProcessExtent.Extend(myFace.GetPoint(0));
                m_triangleToProcessExtent.Extend(myFace.GetPoint(1));
                m_triangleToProcessExtent.Extend(myFace.GetPoint(2));
                }

            PCGroundTrianglePtr pTriangle(PCGroundTriangle::Create(*this, myFace));
            if (pTriangle->IsDensificationRequired())
                AddTriangleToProcess(*pTriangle);
            }
        }
    //new points has been processed
    m_newPointToAdd.clear();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_ComputeTrianglesToProcess()
    {
    size_t trianglesCount = _ComputeTriangulation();
    if (trianglesCount == 0)
        return; //ERROR, cannot process

    //Process all faces of the mesh and add corresponding triangle 
    m_trianglesToProcess.clear();
    for (auto itr = m_pBcDtm->begin(); itr != m_pBcDtm->end(); ++itr)
        {
        PCGroundTrianglePtr pTriangle(PCGroundTriangle::Create(*this, *itr));
        if (pTriangle->IsDensificationRequired())
            AddTriangleToProcess(*pTriangle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::DrawTriangles() const
    {
    //Draw result if ask to
    /*
    if (m_pParams->GetDrawTriangles())
        {
        for (auto itr = m_pBcDtm->begin(); itr != m_pBcDtm->end(); ++itr)
            {
            Triangle myFace(*itr);

            DPoint3d pt1 = myFace.GetPoint(0);
            DPoint3d pt2 = myFace.GetPoint(1);
            DPoint3d pt3 = myFace.GetPoint(2);
            DrawingFacility::DrawTriangle(pt1, pt2, pt3, m_pParams->GetMetersToUors());
            }
        m_pReport->RefreshMSView(true);
        }
        */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  PCGroundTIN::DrawSeedPoints() const
    {
    /*
    if (m_pParams->GetDrawSeeds())
        {
        // Create a new US element from the vertices.
        for (T_DPoint3dPointContainer::const_iterator Itr = GetNewPointToAdd().begin(); Itr != GetNewPointToAdd().end(); ++Itr)
            {
            DPoint3d pt(*Itr);
            DrawingFacility::DrawPoint(pt, GetParam().GetMetersToUors());
            }
        m_pReport->RefreshMSView(true);
        }
        */
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void  PCGroundTIN::DrawGrid() const
    {
    /*
    if (m_pParams->GetDrawGrid())
        {
        m_pGDGrid->Draw();
        m_pReport->RefreshMSView(true);
        }
        */
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GridCellEntryPtr PCGroundTIN::CreateGridCellEntry(DRange3d const& boundingBoxUors) 
    {
    return _CreateGridCellEntry(boundingBoxUors);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GridCellEntryPtr PCGroundTIN::_CreateGridCellEntry(DRange3d const& boundingBoxUors) 
    {
    return GridCellEntry::Create(boundingBoxUors, GetParamR());
    }


 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTIN::FindMirroredPointAndTriangle(double& distance, DPoint3d& mirroredPoint, DPoint3d const& point, DPoint3d const& nodePt) const
    {
    DVec3d ptToNode(DVec3d::FromStartEnd(point, nodePt));
    mirroredPoint = nodePt;
    mirroredPoint.Add(ptToNode);

    //Don't change z value for mirrored point
    mirroredPoint.z = point.z;

    //Now find corresponding triangle for this point
    return _GetDistanceToTriangleFromPoint(distance, mirroredPoint);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTIN::_GetDistanceToTriangleFromPoint(double& distance, DPoint3d const& point) const
    {
    bool isFound = m_pBcDtm->FindNearestTriangleDistanceFromPoint(nullptr, distance, point);
    return isFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
// IDPoint3dCriteria implementation
bool PCGroundTIN::_IsAccepted(DPoint3d const& point) const
    {
    //Used by CLASSIFICATION
    double distanceThreshold = GetParam().GetClassificationTolerance();

    //Find corresponding triangle for this point
    double   distance;
    bool isFound = _GetDistanceToTriangleFromPoint(distance, point);

    //No triangle found, continue...
    if (!isFound)
        return false;

    if (distance <= distanceThreshold)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Daniel.Mckenzie                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTIN::_GetDTMPoints(bvector<DPoint3d>& points)
    {   
    
    if (m_pBcDtm->GetPointCount() == 0)
        _ComputeTrianglesToProcess();

    points.resize(m_pBcDtm->GetPointCount());
    StatusInt status = m_pBcDtm->GetDTMPoints(&points[0]);
    BeAssert(status==SUCCESS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTINPtr PCGroundTINMT::Create(GroundDetectionParameters& params, ProgressReport& report)
    {
    return new PCGroundTINMT(params, report);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTINMT::PCGroundTINMT(GroundDetectionParameters& params, ProgressReport& report)
:PCGroundTIN(params, report), m_mainThreadID(BeThreadUtilities::GetCurrentThreadId())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PCGroundTINMT::~PCGroundTINMT()
    {
    //GDZERO
    FlushThreadPoolWork();
    }

//GDZERO
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudThreadPool& PCGroundTINMT::GetThreadPool()
    {
    if (m_pThreadPool==NULL)
        {
        //how many  threads can be created, zero means is meaningless for this platform
        unsigned int num_threads = std::thread::hardware_concurrency();
        if (num_threads == 0 || num_threads > PCGroundTINMT::MAX_NUMBER_THREAD)
            num_threads = PCGroundTINMT::MAX_NUMBER_THREAD;  //Arbitrary number

        //Use a stack for optimization - we hope to have last work buffer ready in memory
        m_pThreadPool = PointCloudThreadPool::Create(num_threads, num_threads, SchedulingMethod::LIFO);
        }
    return *m_pThreadPool;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GroundDetectionThreadPoolPtr PCGroundTINMT::GetWorkThreadPool()
    {
    if (m_newThreadPool == NULL)
        {
        unsigned int num_threads = std::thread::hardware_concurrency();
        m_newThreadPool = GroundDetectionThreadPool::Create(num_threads);
        }

    return m_newThreadPool;
    } 

PointCloudThreadPool& PCGroundTINMT::GetQueryThreadPool()
    {
    if (m_pQueryThreadPool == NULL)
        {
        unsigned int num_threads = std::thread::hardware_concurrency();
        //unsigned int num_threads = 1;
        m_pQueryThreadPool = PointCloudThreadPool::Create(num_threads, num_threads, SchedulingMethod::FIFO);
        }
    return *m_pQueryThreadPool;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::OutputDtmPreview(bool noDelay, BeMutex* newPointToAddMutex)
    {
    __super::OutputDtmPreview(noDelay, &s_newPointToAddCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::FlushThreadPoolWork()
    {
    if (m_pQueryThreadPool != NULL)
        m_pQueryThreadPool->Terminate();
    if (m_pThreadPool != NULL)
        m_pThreadPool->Terminate();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::_IncrementWorkDone()
    {
    ProgressMonitor::UpdateProgressInThread();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
GridCellEntryPtr PCGroundTINMT::_CreateGridCellEntry(DRange3d const& boundingBoxUors) 
    {    
    return T_Super::_CreateGridCellEntry(boundingBoxUors);
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PCGroundTINMT::_GetDistanceToTriangleFromPoint(double& distance, DPoint3d const& point) const
    {
    //BeMutexHolder lock(s_dtmLibCS);
    return T_Super::_GetDistanceToTriangleFromPoint(distance, point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t PCGroundTINMT::_ComputeTriangulation()
    {
    //BeMutexHolder lock(s_dtmLibCS);
    return T_Super::_ComputeTriangulation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::_ComputeStatisticsFromDTM(DiscreetHistogram& angleStats, DiscreetHistogram& heightStats)
    {
    //BeMutexHolder lock(s_dtmLibCS);
    return T_Super::_ComputeStatisticsFromDTM(angleStats, heightStats);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::_AddTriangleToProcess(PCGroundTriangle& triangle)
    {
    //No need to lock critical section -> never called by more than one thread at a time
    //But just assert this..
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_mainThreadID);
    return T_Super::_AddTriangleToProcess(triangle);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::_ComputeTrianglesToProcessFromPointToAdd()
    {
    //No need to lock critical section -> never called by more than one thread at a time
    //But just assert this..
    BeAssert(BeThreadUtilities::GetCurrentThreadId() == m_mainThreadID);
    T_Super::_ComputeTrianglesToProcessFromPointToAdd();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::_AddPoint(DPoint3d const& point)
    {
    BeMutexHolder lock(s_newPointToAddCS);
    T_Super::_AddPoint(point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PCGroundTINMT::_SetNewSeedPoints(const bvector<DPoint3d>& newpoints)
    {
    BeMutexHolder lock(s_newPointToAddCS);
    T_Super::_SetNewSeedPoints(newpoints);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PCGroundTINMT::_CreateInitialTIN()
    {    
    m_pReport->StartPhase(1, L"START - Find Initial Seed Points");
    if (!m_pReport->CheckContinueOnProgress())
        return ERROR;//User abort

    ProgressMonitor progressMonitor(*m_pReport, m_pGDGrid->GetSize(), false, m_pParams->GetUseMultiThread());
    for (size_t i = 0; i < m_pGDGrid->GetSize(); i++)
        {
        GridCellEntryPtr pGridCellEntry = m_pGDGrid->GetGridCellEntry(i);

        //QueryAllPointsForFirstSeedPointWorkPtr pWork(QueryAllPointsForFirstSeedPointWork::Create(*this, *pGridCellEntry));
        GroundDetectionWorkPtr pWork(QueryAllPointsForFirstSeedPointWork::Create(*this, *pGridCellEntry));
        //GetQueryThreadPool().QueueWork(*pWork);
        GetWorkThreadPool()->QueueWork(pWork);
        progressMonitor.InProgress();
        }

    GetWorkThreadPool()->Start();
    GetWorkThreadPool()->WaitAndStop();
    GetWorkThreadPool()->ClearQueueWork();    
    /*
    while (!GetQueryThreadPool().WaitUntilWorkDone(PROGESS_UPDATE_TIME) || !GetThreadPool().WaitUntilWorkDone(PROGESS_UPDATE_TIME))
        {
        progressMonitor.InProgress();
        if (progressMonitor.WasCanceled() || progressMonitor.WasError())
            {
            FlushThreadPoolWork();//process was cancelled; terminate thread pool...
            }
        }
        
    progressMonitor.FinalStageProcessing();
    if (progressMonitor.WasCanceled() || progressMonitor.WasError())
        {
        FlushThreadPoolWork();//process was cancelled; terminate thread pool...
        m_pReport->EndPhase(L"ABORT - Find Initial Seed Points");
        return ERROR;//User abort
        }
        */

    //If we don't want to densify or classify, at least compute our parameters once.
    if (!m_pParams->GetDensifyTin())
        {
        size_t trianglesCount = _ComputeTriangulation();
        if (trianglesCount == 0)
            return ERROR; //ERROR, cannot process

        ComputeParameterFromTINPoints();
        }

    m_pReport->EndPhase(L"END - Find Initial Seed Points");

    bvector<DPoint3d> additionalSeedPoints;

    m_pParams->GetAdditionalSeedPoints(additionalSeedPoints);

    for (auto& seedPoint : additionalSeedPoints)
        {
        AddPoint(seedPoint);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  PCGroundTINMT::_DensifyTIN()
    {
    //Then we find new seed points from parameter threshold values and update our TIN
    m_pReport->StartPhase(2, L"START - Densification of TIN");
    if (!m_pReport->CheckContinueOnProgress())
        return ERROR;//User abort

    IGroundPointsAccumulatorPtr ptsAccumPtr(GetParamR().GetGroundPointsAccumulator());

    int currentIteration = 1;
    for (PrepareFirstIteration(); PrepareNextIteration(); currentIteration++)
        {
        OutputDtmPreview(true);

        m_pReport->StartCurrentIteration(currentIteration);
        /*
        //Special case for first iteration
        size_t trianglesToProcess(m_trianglesToProcess.size());
        //Assume every triangle to process will add one point -> and thus create three new triangles
        //thus this triangle will be replaced by three -> -1 + 3 = 2
        size_t expectedNewTriangles = trianglesToProcess * 2;
        size_t actualTriangleCount = m_pBcDtm->GetTriangleCount();
        size_t TotalTrianglesExpected = actualTriangleCount + expectedNewTriangles;
        //double accomplishmentRatio = ((static_cast<double>(actualTriangleCount) / static_cast<double>(TotalTrianglesExpected)));
        */
        if (!m_pReport->CheckContinueOnProgress())
            {
            StopIteration();
            m_pReport->EndCurrentIteration();
            m_pReport->EndPhase(L"ABORT -  Densification of TIN");
            return ERROR;//User abort
            }

        //Display some stat while processing to help debug
/*        
        WChar tmpMessage[200];
        WString formatStr;
        double accomplishmentRatioItr = (static_cast<double>(actualTriangleCount) / static_cast<double>(TotalTrianglesExpected));

#if !defined (NDEBUG)
        GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatisticDebug, STRINGID_DCPCMISC_Messages);
        BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), actualTriangleCount, trianglesToProcess, accomplishmentRatioItr * 100.0, m_pParams->GetHeightThreshold(), m_pParams->GetAngleThreshold().Degrees());
#else
        GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatistic, STRINGID_DCPCMISC_Messages);
        BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), accomplishmentRatioItr * 100.0);
#endif
        m_pReport->OutputMessage(tmpMessage);
        */

        ProgressMonitor progressMonitor(*m_pReport, m_trianglesToProcess.size(), false, m_pParams->GetUseMultiThread());

        size_t nbTriangleProcessed(1);
        for (PCGroundTriangleCollection::iterator triItr = m_trianglesToProcess.begin(); triItr != m_trianglesToProcess.end() && progressMonitor.InProgress(); ++triItr, ++nbTriangleProcessed)
            {
            PCGroundTrianglePtr pTriangle = *triItr;
            BeAssert(pTriangle.IsValid());
            //QueryAllPointsForTriangleWorkPtr pWork(QueryAllPointsForTriangleWork::Create(*this, *pTriangle));
            GroundDetectionWorkPtr pWork(QueryAllPointsForTriangleWork::Create(*this, *pTriangle));             

            //GetQueryThreadPool().QueueWork(*pWork);
            GetWorkThreadPool()->QueueWork(pWork);
            }
       
        struct OutputPreviewProgress : GroundDetectionThreadPool::IActiveWait
            {
            OutputPreviewProgress(PCGroundTINMT* groundTin)
                {
                m_groundTin = groundTin;
                }
            
            virtual void Progress() override
                {
                m_groundTin->OutputDtmPreview(false, &m_groundTin->s_newPointToAddCS);
                }

            PCGroundTINMT* m_groundTin; 
            };

        OutputPreviewProgress previewProgress(this);

        GetWorkThreadPool()->Start(&previewProgress);
        GetWorkThreadPool()->WaitAndStop();
        GetWorkThreadPool()->ClearQueueWork();    

        /*
        while (!GetQueryThreadPool().WaitUntilWorkDone(PROGESS_UPDATE_TIME) || !GetThreadPool().WaitUntilWorkDone(PROGESS_UPDATE_TIME))
            {
            progressMonitor.InProgress();
            if (progressMonitor.WasCanceled() || progressMonitor.WasError())
                {
                FlushThreadPoolWork();//process was cancelled; terminate thread pool...
                }
            }        
        progressMonitor.FinalStageProcessing();
        if (progressMonitor.WasCanceled() || progressMonitor.WasError())
            {
            FlushThreadPoolWork();//process was cancelled; terminate thread pool...
            m_pReport->EndCurrentIteration();
            m_pReport->EndPhase(L"ABORT -  Densification of TIN");
            return ERROR;//User abort
            }
        m_pReport->EndCurrentIteration();

            */
        if (!ptsAccumPtr->ShouldContinue())
            break;
        }

    //Display some stat while processing to help debug
/*
    double elapsedSecond = m_pReport->GetTimerR().GetCurrentSeconds();
    long hours = (long) (elapsedSecond / 60 / 60);
    long minutes = (long) (elapsedSecond / 60) % 60;
    long seconds = (long) elapsedSecond % 60;


    WChar tmpMessage[200];
    WString formatStr;
#if !defined (NDEBUG)
    GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatisticFinalDebug, STRINGID_DCPCMISC_Messages);
    BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), m_pBcDtm->GetTriangleCount(), currentIteration, hours, minutes, seconds, m_pParams->GetHeightThreshold(), m_pParams->GetAngleThreshold().Degrees());
#else
    GetResourceHandle()->GetString(formatStr, DCPCMISC_PointCloudGroundDetectionStatisticFinal, STRINGID_DCPCMISC_Messages);
    BeStringUtilities::Snwprintf(&tmpMessage[0], 200, formatStr.c_str(), m_pBcDtm->GetTriangleCount(), currentIteration, hours, minutes, seconds);
#endif
    m_pReport->OutputMessage(tmpMessage);
*/
    m_pReport->EndPhase(L"END - Densification of TIN");
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt PCGroundTINMT::_Classify()
    {
    //GDZERO
    assert(!"Should not be called");
#if 0 
    m_pReport->StartPhase(3, L"START - Classification");
    if (!m_pReport->CheckContinueOnProgress())
        return ERROR;//User abort

    ProgressMonitor progressMonitor(*m_pReport, m_pGDGrid->GetSize(), false, m_pParams->GetUseMultiThread());

    for (size_t i = 0; i < m_pGDGrid->GetSize(); i++)
        {
        GridCellEntryPtr pGridCellEntry = m_pGDGrid->GetGridCellEntry(i);

        ClassifyPointCloudWorkPtr pWork(ClassifyPointCloudWork::Create(*pGridCellEntry,*this));
        GetQueryThreadPool().QueueWork(*pWork);
        progressMonitor.InProgress();
        }

    while (!GetQueryThreadPool().WaitUntilWorkDone(PROGESS_UPDATE_TIME) || !GetThreadPool().WaitUntilWorkDone(PROGESS_UPDATE_TIME))
        {
        progressMonitor.InProgress();
        if (progressMonitor.WasCanceled() || progressMonitor.WasError())
            {
            FlushThreadPoolWork();//process was cancelled; terminate thread pool...
            }
        }
    progressMonitor.FinalStageProcessing();
    if (progressMonitor.WasCanceled() || progressMonitor.WasError())
        {
        FlushThreadPoolWork();//process was cancelled; terminate thread pool...
        m_pReport->EndPhase(L"ABORT - Classification");
        return ERROR;//User abort
        }

    m_pReport->EndPhase(L"END - Classification");
#endif
    return SUCCESS;
    }

END_GROUND_DETECTION_NAMESPACE
