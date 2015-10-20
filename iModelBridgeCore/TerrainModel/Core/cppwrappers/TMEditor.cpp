/*--------------------------------------------------------------------------------------+
|
|     $Source: Core/cppwrappers/TMEditor.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

//#define CHECKDTM
#include "bcDTMBaseDef.h"
#include "dtmevars.h"
#include "bcdtminlines.h"
#include "BcDTMEdit.h"
#include "TerrainModel\Core\TMTransformHelper.h"
#include "TerrainModel\Core\TMEditor.h"
#include <stack>
using namespace std;


/*-------------------------------------------------------------------+
|                                                                    |
|                                                                    |
|                                                                    |
+-------------------------------------------------------------------*/
int bcdtmList_testForNonGroupSpotDtmFeatureLineDtmObject (BC_DTM_OBJ *dtmP, long P1, long P2)
    {
    /*
    ** This Function Tests If Line P1-P2 Is A Dtm Feature Line
    */
    long clPtr;
    /*
    ** Scan P1 and See If it Connects To P2
    */
    clPtr = nodeAddrP (dtmP, P1)->fPtr;
    while (clPtr != dtmP->nullPtr)
        {
        if (P2 == flistAddrP (dtmP, clPtr)->nextPnt)
            {
            if (ftableAddrP (dtmP, flistAddrP (dtmP, clPtr)->dtmFeature)->dtmFeatureType != DTMFeatureType::GroupSpots)
                return(1);
            }
        clPtr = flistAddrP (dtmP, clPtr)->nextPtr;
        }
    /*
    ** Scan P2 and See If it Connects To P1
    */
    clPtr = nodeAddrP (dtmP, P2)->fPtr;
    while (clPtr != dtmP->nullPtr)
        {
        if (P1 == flistAddrP (dtmP, clPtr)->nextPnt)
            {
            if (ftableAddrP (dtmP, flistAddrP (dtmP, clPtr)->dtmFeature)->dtmFeatureType != DTMFeatureType::GroupSpots)
                return(1);
            }

        clPtr = flistAddrP (dtmP, clPtr)->nextPtr;
        }
    /*
    ** Job Completed
    */
    return(0);
    }

int  bcdtmEdit_checkForLineOnHullLineDtmObject(BC_DTM_OBJ *dtmP,long P1,long P2,long *hullLineP)
/*
** This Function Tests If The Line P1P2 or P2P1 is On A dtmP,Void,Hole Or Island Hull
*/
{
 int  dbg=DTM_TRACE_VALUE(0) ;
 long clc ;
/*
** Write Entry Message
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Hull Line %9ld %9ld",P1,P2) ;
/*
** Initiliase
*/
 *hullLineP = 0 ;
/*
** Test For Tin Hull
*/
 if( nodeAddrP(dtmP,P1)->hPtr == P2 || nodeAddrP(dtmP,P2)->hPtr == P1 ) *hullLineP = 1 ; 
/*
** Test For P1-P2 on a Void,Hole or Island Hull
*/
 clc = nodeAddrP(dtmP,P1)->fPtr ;
 while ( clc != dtmP->nullPtr && ! *hullLineP  )
   {
     if(flistAddrP(dtmP,clc)->nextPnt == P2 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      )  *hullLineP = 1 ; 
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
   } 
/*
** Test For P2-P1 on a Void,Hole or Island Hull
*/
 clc = nodeAddrP(dtmP,P2)->fPtr ;
 while ( clc != dtmP->nullPtr  && ! *hullLineP  )
   {
    if(flistAddrP(dtmP,clc)->nextPnt == P1 )
      {
       if( ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Void   ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Island ||
           ftableAddrP(dtmP,flistAddrP(dtmP,clc)->dtmFeature)->dtmFeatureType == DTMFeatureType::Hole      )  *hullLineP = 1 ; 
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;  
   } 
/*
** Job Completed
*/
 if( dbg ) bcdtmWrite_message(0,0,0,"Testing For Hull Line %9ld %9ld Completed",P1,P2) ;
 return(DTM_SUCCESS) ;
}

int  bcdtmEdit_checkForPointOnHullLineDtmObject(BC_DTM_OBJ *dtmP,long Point,long *HullLine)
/*
** This Function Tests For A Point On A DTM Hull
*/
{
 long clc,feat ;
 BC_DTM_FEATURE *dtmFeatureP ;
/*
** Initialise
*/
 *HullLine = 0 ;
 if( nodeAddrP(dtmP,Point)->hPtr != dtmP->nullPnt ) { *HullLine = 1 ; return(0) ; }
/*
** Scan Feature List For Point
*/
 clc = nodeAddrP(dtmP,Point)->fPtr ;
 while ( clc != dtmP->nullPtr )
   {
    feat = flistAddrP(dtmP,clc)->dtmFeature ;
    dtmFeatureP = ftableAddrP(dtmP,feat) ;
    if( dtmFeatureP->dtmFeatureState == DTMFeatureState::Tin )
      {
       if( ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Void  ||
           ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Hole  ||
           ftableAddrP(dtmP,feat)->dtmFeatureType == DTMFeatureType::Island   )
         { *HullLine = 1 ; return(0) ; }
      }
    clc = flistAddrP(dtmP,clc)->nextPtr ;
   }
/*
** Job Completed
*/
 return(0) ;
}

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

enum class TMPointType : long
{
    ExternalToDtm = 0,            // **  == 0   Point External To Dtm
    CoincidentWithPoint = 1,      // **  == 1   Point Coincident with Point pnt1P
    OnLine = 2,                   // **  == 2   Point On Line pnt1-Ppnt2P
    OnHull = 3,                   // **  == 3   Point On Hull Line pnt1P-pnt2P
    InTriangle = 4,               // **  == 4   Point In Triangle pnt1P-pnt2P-pnt3P
    PointInternalToVoidHull = 5,
    PointExternalToTinHull = 6,
};

struct BcDTMEdit : RefCounted <ITMEditor>
{
private:
    RefCountedPtr<BcDTM> m_dtm;
    bool m_inDynamics;
    TMTransformHelperP helper;
    SelectionState m_selectionState;
    long m_pointNumber;
    long m_linePointNumber[2];
    long m_trianglePointNumber[3];
    bvector<DPoint3d> m_selectedPoints;
    bool m_canDeleteInternalTriangles;
    double m_contourInterval;
    bvector<DPoint3d> m_byLineSelection;
    bvector<long> m_trianglePointNumbersByLineSelection;

    // Added RobC - 04/04/2012

    SelectedLinearFeatures m_selectedFeatures;
    SelectedLinearFeatures::iterator m_selectedFeaturesIter;

    struct DrawUserP
        {
        TMTransformHelperP helper;
        ITMEditorDrawer* drawer;
        };

    static int dtmLoad_bcToGeopakLoadFunction(DTMFeatureType dtmFeatureType,DTMUserTag userTag,DTMFeatureId featureId,DPoint3d* featurePtsP,size_t numFeaturePts,void *userP)
    {
    DrawUserP* drawInfo = (DrawUserP*)userP;
    if (dtmFeatureType == DTMFeatureType::Theme)
        return DTM_SUCCESS;
    if (drawInfo->helper)
        return drawInfo->drawer->Draw (dtmFeatureType, userTag, drawInfo->helper->copyPointsFromDTM ((DPoint3dCP)featurePtsP, (int)numFeaturePts), (int)numFeaturePts);
    return drawInfo->drawer->Draw (dtmFeatureType, userTag, featurePtsP, (int)numFeaturePts);
    }

    // May need to store the XYZ
    struct dynamicMod
        {
        virtual StatusInt Revert (RefCountedPtr<BcDTM> dtm) abstract;
        virtual StatusInt DrawDynamics (BcDTMEdit* edit, RefCountedPtr<BcDTM> dtm, DrawUserP* drawInfo) abstract;
        struct AddVertex;
        struct DelTriangle;
        struct MoveVertex;
        };

    stack<dynamicMod*> m_dynamic_changes;

    BcDTMEdit(BcDTMP dtm) : m_dtm (dtm), helper (m_dtm->GetTransformHelper())
        {
        m_inDynamics = false;
        m_canDeleteInternalTriangles = false;
        m_selectionState = None;
        m_contourInterval = 1;
        }

public:
    static BcDTMEdit* Create (BcDTMP dtm)
        {
        return new BcDTMEdit (dtm);
        }

    // Added RobC - 04/04/2012

    virtual ~BcDTMEdit()
        {
        }

    double GetContourInterval ()
        {
        return m_contourInterval;
        }

private:
    void SetSelectedPoints (DPoint3d const points[], long numPoints, bool close = false)
        {
        m_selectedPoints.resize (close ? numPoints + 1 : numPoints);
        memcpy (&m_selectedPoints[0], points, numPoints * sizeof (DPoint3d));
        if (close)
            m_selectedPoints.back() = m_selectedPoints.front();
        }

    void AppendSelectedPoints (DPoint3d const points[], long numPoints, bool close = false)
        {
        int curSize = (int)m_selectedPoints.size();
        m_selectedPoints.resize (curSize + (close ? numPoints + 1 : numPoints));
        memcpy (&m_selectedPoints[curSize], points, numPoints * sizeof (DPoint3d));
        if (close)
            m_selectedPoints.back() = m_selectedPoints[curSize];
        }

    void RevertChanges (void)
        {
        while (m_dynamic_changes.size() != 0)
            {
            dynamicMod* operation = m_dynamic_changes.top();
            operation->Revert (m_dtm);
            delete operation;
            m_dynamic_changes.pop();
            }
        }

protected:
    virtual bool _CanDeleteInternalTriangles()
        {
        return m_canDeleteInternalTriangles;
        }
    virtual void _SetCanDeleteInternalTriangles (bool value)
        {
        m_canDeleteInternalTriangles = value;
        }

    virtual StatusInt _StartDynamics() override
        {
        m_inDynamics = true;
        if (m_dtm->SetMemoryAccess (DTMAccessMode::Temporary) != DTM_SUCCESS)
            {
            bcdtmWrite_message(1,0,0, "DTM is readonly");
            return DTM_ERROR;
            }
#ifdef CHECKDTM
        if (bcdtmCheck_tinComponentDtmObject(m_dtm->GetTinHandle()) != DTM_SUCCESS)
            return DTM_SUCCESS;
#endif

        return DTM_SUCCESS;
        }

    virtual StatusInt _EndDynamics() override
        {
        m_inDynamics = false;

        RevertChanges ();
#ifdef CHECKDTM
        if (bcdtmCheck_tinComponentDtmObject(m_dtm->GetTinHandle()) != DTM_SUCCESS)
            return DTM_SUCCESS;
#endif
        return DTM_SUCCESS;
        }

    virtual StatusInt _DrawDynamics (ITMEditorDrawer* drawer) override
        {
        DrawUserP drawInfo = {helper, drawer};
        stack<dynamicMod*> dynamic_changes = m_dynamic_changes;

        for (size_t i = 0; i < dynamic_changes.size(); i++)
            {
            dynamicMod* operation = dynamic_changes.top();
            operation->DrawDynamics (this, m_dtm, &drawInfo);
            dynamic_changes.pop();
            }
        return DTM_SUCCESS;
        }

    virtual StatusInt _DrawSelection (ITMEditorDrawer* drawer) override
        {
        if (m_selectionState == None)
            return DTM_SUCCESS;

        if (false && m_selectionState == TrianglesByLine)
            {
            for (size_t i = 0; i < m_selectedPoints.size (); i += 4)
                {
                if (helper)
                    drawer->Draw (/*ITMEditorDrawer::SELECTION_PURPOSE, */DTMFeatureType::None, -1, helper->copyPointsFromDTM (&m_selectedPoints[i], 4), 4);
                else
                    drawer->Draw (/*ITMEditorDrawer::SELECTION_PURPOSE, */DTMFeatureType::None, -1, &m_selectedPoints[i], 4);
                }
            }
        else
            {
            if (helper)
                drawer->Draw (/*ITMEditorDrawer::SELECTION_PURPOSE, */DTMFeatureType::None, -1, helper->copyPointsFromDTM (&m_selectedPoints[0], (int)m_selectedPoints.size ()), (int)m_selectedPoints.size ());
            else
                drawer->Draw (/*ITMEditorDrawer::SELECTION_PURPOSE, */DTMFeatureType::None, -1, &m_selectedPoints[0], (int)m_selectedPoints.size ());
            }

        return DTM_SUCCESS;
        }

    virtual void _ClearSelection () override
        {
        m_selectionState = None;
        }

    virtual StatusInt _SelectVertex (const DPoint3d& pt) override;
    virtual StatusInt _SelectLine (const DPoint3d& pt) override;
    virtual StatusInt _SelectTriangle (DPoint3dCR pt) override;
    virtual StatusInt _SelectFeature (const DPoint3d& pt, bool first) override;
    virtual DTMFeatureType BcDTMEdit::_GetSelectedFeatureType () override;
    virtual StatusInt _SelectTrianglesByLine (const DPoint3d pts[], int numPts, bool stopAtFeatures) override;

    virtual SelectionState _GetSelectionState() override
        {
        return m_selectionState;
        }

    virtual StatusInt _AddVertex (DPoint3dCR pt, bool useZ = true) override;
    virtual StatusInt _MoveVertex (const DPoint3d& pt, bool updateZ) override;
    virtual StatusInt _DeleteVertex () override;
    virtual StatusInt _SwapLine () override;
    virtual StatusInt _DeleteLine () override;
    virtual StatusInt _DeleteFeature () override;
    virtual StatusInt _DeleteTriangle (void) override;
    virtual StatusInt _DeleteTrianglesByLine () override;

    virtual StatusInt _InsertFeature ()
        {
//        bcdtmEdit_insertDtmFeatureIntoDtmObject(dtmP,DTMFeatureType::Breakline,dtmP->nullUserTag,stringPtsP,numstringPtsP,startPntP);
        return DTM_ERROR;
        }

    virtual bool _CanDeleteVertex ()
        {
        return true;
        }

    virtual bool _CanDeleteLine ()
        {
        if (m_selectionState != Line)
            return false;

        if (!m_canDeleteInternalTriangles)
            {
            long hullLine = 0;

            // Check that the edge is on the boundary and it isn't a break line.
            if (bcdtmEdit_checkForLineOnHullLineDtmObject (m_dtm->GetTinHandle(), m_linePointNumber[0], m_linePointNumber[1], &hullLine) != DTM_SUCCESS)
                return false;

            // We are deleting the triangle on the edge
            bool isRight = nodeAddrP (m_dtm->GetTinHandle(), m_linePointNumber[0])->hPtr == m_linePointNumber[1];

            m_selectionState = Triangle;
            m_trianglePointNumber[0] = m_linePointNumber[0];
            m_trianglePointNumber[1] = m_linePointNumber[1];
            if (isRight)
                m_trianglePointNumber[2] = bcdtmList_nextAntDtmObject (m_dtm->GetTinHandle(), m_linePointNumber[0], m_linePointNumber[1]);
            else
                m_trianglePointNumber[2] = bcdtmList_nextAntDtmObject (m_dtm->GetTinHandle(), m_linePointNumber[1], m_linePointNumber[0]);

            bool ret = _CanDeleteTriangle ();
            m_selectionState = Line;

            return ret;


            //if (hullLine)
            //    return bcdtmList_testForNonGroupSpotDtmFeatureLineDtmObject (m_dtm->GetTinHandle(), m_linePointNumber[0], m_linePointNumber[1]) == 0;
            //return false;
            }
        return true;
        }

    virtual bool _CanDeleteTrianglesByLine()
        {
        if (m_selectionState != TrianglesByLine)
            return false;

        bool ret = false;
        m_selectionState = Triangle;
        for (size_t i = 0; i < m_trianglePointNumbersByLineSelection.size(); i+= 3)
            {
            m_trianglePointNumber[0] = m_trianglePointNumbersByLineSelection[i];
            m_trianglePointNumber[1] = m_trianglePointNumbersByLineSelection[i + 1];
            m_trianglePointNumber[2] = m_trianglePointNumbersByLineSelection[i + 2];
            ret = _CanDeleteTriangle();
            if (ret) break;
            }
        m_selectionState = TrianglesByLine;
        return ret;
        }

    virtual bool _CanDeleteTriangle ()
        {
        if (m_selectionState != Triangle)
            return false;

        if (!m_canDeleteInternalTriangles)
            {
            long hullLine = 0;
            int hullPointCount = 0;
            int hullLineCount = 0;
            int hullFeaturesCount = 0;

            // See if all 3 points are on the boundary, if so this is either the last point or is invalid.
            for (int i = 0; i < 3; i++)
                {
                if (bcdtmEdit_checkForPointOnHullLineDtmObject(m_dtm->GetTinHandle(), m_trianglePointNumber[i], &hullLine) != DTM_SUCCESS)
                    return false;

                if (hullLine)
                    hullPointCount++;
                }

            // See if the triangle edge is on the boundary, and that it isn't a break line.
            if (bcdtmEdit_checkForLineOnHullLineDtmObject (m_dtm->GetTinHandle(), m_trianglePointNumber[0], m_trianglePointNumber[1], &hullLine) != DTM_SUCCESS)
                return false;
            if (hullLine)
                {
                if (bcdtmList_testForNonGroupSpotDtmFeatureLineDtmObject (m_dtm->GetTinHandle(), m_trianglePointNumber[0], m_trianglePointNumber[1]))
                    hullFeaturesCount++;
                else
                    hullLineCount++;
                }

            if (bcdtmEdit_checkForLineOnHullLineDtmObject (m_dtm->GetTinHandle(), m_trianglePointNumber[1], m_trianglePointNumber[2], &hullLine) != DTM_SUCCESS)
                return false;
            if (hullLine)
                {
                if (bcdtmList_testForNonGroupSpotDtmFeatureLineDtmObject (m_dtm->GetTinHandle(), m_trianglePointNumber[1], m_trianglePointNumber[2]))
                    hullFeaturesCount++;
                else
                    hullLineCount++;
                }

            if (bcdtmEdit_checkForLineOnHullLineDtmObject (m_dtm->GetTinHandle(), m_trianglePointNumber[2], m_trianglePointNumber[0], &hullLine) != DTM_SUCCESS)
                return false;
            if (hullLine)
                {
                if (bcdtmList_testForNonGroupSpotDtmFeatureLineDtmObject (m_dtm->GetTinHandle(), m_trianglePointNumber[2], m_trianglePointNumber[0]))
                    hullFeaturesCount++;
                else
                    hullLineCount++;
                }

            // If there are any hull lines with a feature on dont allow them to delete it.
            if (hullFeaturesCount != 0)
                return false;

            // If all points are on the hull then only delete it if we have 2 hull lines.
            if (hullPointCount == 3)
                return hullLineCount == 2;

            return hullLineCount != 0;
            }
        return true;
        }

    virtual bool _CanSwapLine ()
        {
        if (m_selectionState != Line)
            return false;

        return bcdtmList_testForNonGroupSpotDtmFeatureLineDtmObject (m_dtm->GetTinHandle(), m_linePointNumber[0], m_linePointNumber[1]) == 0;
        }
    };


/// Selection functions.

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>

StatusInt BcDTMEdit::_SelectVertex (const DPoint3d& pt)
    {
    int ret = DTM_SUCCESS;

    m_selectionState = None;

    DPoint3d cPt = pt;
    if (helper) helper->convertPointToDTM (cPt);

    bool found;
    long editPnt2, editPnt3;
    bvector<DPoint3d> points;

    if( !bcdtmEdit_selectDtmEditFeatureDtmObject( m_dtm->GetTinHandle(), DTMFeatureType::TinPoint, cPt.x, cPt.y, found, points))
        {
        SetSelectedPoints(points.data(), 1);
        bcdtmEdit_getDtmEditFeaturePoints(&m_pointNumber,&editPnt2,&editPnt3) ;
        m_selectionState = Point;
        }
    else
        ret = DTM_ERROR;

    return(ret) ;
    }

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>

StatusInt BcDTMEdit::_SelectLine (const DPoint3d& pt)
        {
        int ret = DTM_SUCCESS;
        long P3;
        bool found;
        bvector<DPoint3d> points;
        DPoint3d cPt = pt;
        if (helper) helper->convertPointToDTM (cPt);

        m_selectionState = None;

        if (!bcdtmEdit_selectDtmEditFeatureDtmObject( m_dtm->GetTinHandle(), DTMFeatureType::TinLine, cPt.x, cPt.y, found, points))
            {
            SetSelectedPoints(points.data(), (int)points.size());
            bcdtmEdit_getDtmEditFeaturePoints(&m_linePointNumber[0], &m_linePointNumber[1], &P3) ;
            m_selectionState = Line;
            }
        else
            ret = DTM_ERROR;

        return(ret) ;
        }


/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>

StatusInt BcDTMEdit::_SelectTriangle (DPoint3dCR pt)
    {
    int ret = DTM_SUCCESS;
    bool found;
    bvector<DPoint3d> points;
    DPoint3d cPt = pt;
    if (helper) helper->convertPointToDTM (cPt);

    m_selectionState = None;

    if (bcdtmEdit_selectDtmEditFeatureDtmObject (m_dtm->GetTinHandle(),DTMFeatureType::Triangle, cPt.x, cPt.y, found, points))
        {
        return DTM_ERROR;
        }

    SetSelectedPoints(points.data(), (int)points.size(), points.size() == 3);
    bcdtmEdit_getDtmEditFeaturePoints (&m_trianglePointNumber[0], &m_trianglePointNumber[1], &m_trianglePointNumber[2]);
    m_selectionState = Triangle;

    return(ret) ;
    }

/// <author>Rob.Cormack</author>                            <date>4/2012</date>

StatusInt BcDTMEdit::_SelectFeature (const DPoint3d& pt, bool first)
    {
    int ret = DTM_SUCCESS;
    double  snapTolerance=0.0 ;

    DPoint3d cPt = pt;
    if (helper) helper->convertPointToDTM (cPt);

    m_selectionState = None;

    if(first)
        {
        m_selectedFeatures.clear();
        m_selectedFeaturesIter = m_selectedFeatures.begin();
        ret = bcdtmEdit_selectDtmEditLinearFeatureDtmObject(m_dtm->GetTinHandle(),cPt.x,cPt.y,snapTolerance,m_selectedFeatures) ;
        if( ret ) return(ret) ;
        m_selectedFeaturesIter = m_selectedFeatures.begin();
        }
    else if (m_selectedFeaturesIter == m_selectedFeatures.end())
        m_selectedFeaturesIter = m_selectedFeatures.begin();
    else
        m_selectedFeaturesIter++;

    if (m_selectedFeaturesIter == m_selectedFeatures.end())
        return DTM_ERROR;
    else
        {
        SetSelectedPoints(( DPoint3d * )m_selectedFeaturesIter->featurePtsP,m_selectedFeaturesIter->numFeaturePts) ;
        m_selectionState = Feature;
        }

    return(ret) ;
    }

DTMFeatureType BcDTMEdit::_GetSelectedFeatureType ()
    {
    if (m_selectionState != Feature)
        return DTMFeatureType::None;
    return m_selectedFeaturesIter->featureType;
    }

StatusInt BcDTMEdit::_SelectTrianglesByLine (const DPoint3d pts[], int numPts, bool stopAtFeatures)
    {
    // Need to drape the line and get the boundary of the triangles around it.
    m_selectionState = None;
    bvector<DTM_DRAPE_POINT> drapePts;
    BC_DTM_OBJ* dtmP = m_dtm->GetTinHandle();
    TMTransformHelper::DPoint3dCopy transformedPoints = helper ? helper->copyPointsToDTM (pts, numPts) : TMTransformHelper::DPoint3dCopy (pts, true);
    bcdtmDrape_stringDtmObject(dtmP, (DPoint3d*)(DPoint3d*)transformedPoints, numPts, stopAtFeatures, drapePts);

    m_byLineSelection.resize(numPts);
    memcpy (&m_byLineSelection[0], (DPoint3d*)transformedPoints, sizeof (DPoint3d) * numPts);

    m_selectedPoints.clear();
    m_trianglePointNumbersByLineSelection.clear();
    // Now go for each point and get the triangle.
    for (long i = 0; i < (long)drapePts.size(); i++)
        {
        long fndType;
        long p1,p2,p3;
        double x = drapePts[i].drapePt.x;
        double y = drapePts[i].drapePt.y;

        if (drapePts[i].drapeFeatures.empty())
            break;

        if (i != (long)drapePts.size() - 1)
            {
            x += (drapePts[i + 1].drapePt.x - drapePts[i].drapePt.x) / 2;
            y += (drapePts[i + 1].drapePt.y - drapePts[i].drapePt.y) / 2;
            }
        if( bcdtmFind_triangleDtmObject (dtmP, x, y, &fndType, &p1, &p2, &p3) == DTM_SUCCESS && fndType >= 2)
            {
            DPoint3d points[4];
            int size = (int)m_trianglePointNumbersByLineSelection.size();
            m_trianglePointNumbersByLineSelection.resize(size + 3);
            m_trianglePointNumbersByLineSelection[size++] = p1;
            m_trianglePointNumbersByLineSelection[size++] = p2;
            m_trianglePointNumbersByLineSelection[size++] = p3;

            points[0] = *(DPoint3d*)pointAddrP(dtmP,p1) ;
            points[1] = *(DPoint3d*)pointAddrP(dtmP,p2) ;
            points[2] = *(DPoint3d*)pointAddrP(dtmP,p3) ;
            points[3] = *(DPoint3d*)pointAddrP(dtmP,p1) ;

            AppendSelectedPoints (points, 3, true);
            }
        }
    m_selectionState = TrianglesByLine;

    if (m_selectedPoints.size())
        {
        BcDTMPtr boundaryDTM = BcDTM::Create();
        DTMFeatureId id;

        for (size_t i = 0; i < m_selectedPoints.size(); i += 4)
            boundaryDTM->AddLinearFeature (DTMFeatureType::Breakline, &m_selectedPoints[i], 4, &id);
        boundaryDTM->SetTriangulationParameters (0.1, 0.1, 3, 0.1);
        boundaryDTM->Triangulate ();

        DTMPointArray points;
        boundaryDTM->GetBoundary (points);
        SetSelectedPoints (points.data(), (int)points.size());
        }
    return DTM_ERROR;
    }

/// Dynamic Modifcations classes.
/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>
struct BcDTMEdit::dynamicMod::AddVertex : dynamicMod
{
TMPointType m_pointType;
long m_feature;
long P1, P2, P3;
long Np;

virtual StatusInt Revert (RefCountedPtr<BcDTM> dtm) override
    {
    bcdtmEdit_removePointDtmObject (dtm->GetTinHandle(), Np, (long)m_pointType, m_feature, P1, P2, P3);
    bcdtmTin_compactPointAndNodeTablesDtmObject (dtm->GetTinHandle());
    bcdtmList_setConvexHullDtmObject(dtm->GetTinHandle());
    return DTM_SUCCESS;
    }
virtual StatusInt DrawDynamics (BcDTMEdit* edit, RefCountedPtr<BcDTM> dtm, DrawUserP* drawInfo) override
    {
    bcdtmEdit_drawPointFeaturesDtmObject(dtm->GetTinHandle(), dtmLoad_bcToGeopakLoadFunction, 3, Np, edit->GetContourInterval(), drawInfo);
    return DTM_SUCCESS;
    }
}; // End BcDTMEdit::dynamicMod::AddVertex struct

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>
struct BcDTMEdit::dynamicMod::MoveVertex : dynamicMod
{
long pointNumber;
DPoint3d originalPoint;

virtual StatusInt Revert (RefCountedPtr<BcDTM> dtm) override
    {
        bcdtmEdit_tempMoveVertexXYZDtmObject (dtm->GetTinHandle(), pointNumber, originalPoint.x, originalPoint.y, originalPoint.z) ;
    return DTM_SUCCESS;
    }

virtual StatusInt DrawDynamics (BcDTMEdit* edit, RefCountedPtr<BcDTM> dtm, DrawUserP* drawInfo) override
    {
    bcdtmEdit_drawPointFeaturesDtmObject(dtm->GetTinHandle(), dtmLoad_bcToGeopakLoadFunction, 3, pointNumber, edit->GetContourInterval(), drawInfo);
    return DTM_SUCCESS;
    }
};

/// <author>Piotr.Slowinski</author>                            <date>2/2012</date>
struct BcDTMEdit::dynamicMod::DelTriangle : dynamicMod
{
    StatusInt Revert (RefCountedPtr<BcDTM> dtm) override
        { return DTM_SUCCESS; }
    StatusInt DrawDynamics (BcDTMEdit* edit, RefCountedPtr<BcDTM> dtm, DrawUserP* drawInfo) override
        { return DTM_ERROR; }
}; // End BcDTMEdit::dynamicMod::DelTriangle struct


/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>
StatusInt BcDTMEdit::_AddVertex (DPoint3dCR pt, bool useZ)
    {
    if (!m_inDynamics && m_dtm->SetMemoryAccess (DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    dynamicMod::AddVertex tmp;
    double z;
    DPoint3d cPt = pt;
    if (helper) helper->convertPointToDTM (cPt);

    BC_DTM_OBJ* dtmP = m_dtm->GetTinHandle();

    if (bcdtmEdit_dataPointDtmObject(dtmP, cPt.x, cPt.y, &z, reinterpret_cast<long*>(&tmp.m_pointType), &tmp.m_feature, &tmp.P1, &tmp.P2, &tmp.P3))
        return DTM_ERROR;

    if (useZ)
        cPt.z = z;

    // ToDo: tmp.m_pointType == TMPointType::OnLine => tmp.P3 == dtmP->nullPnt
    // probably tmp.m_pointType == TMPointType::CoincidentWithPoint => tmp.P2 == dtmP->nullPnt
    if (tmp.m_pointType == TMPointType::CoincidentWithPoint)
        return DTM_ERROR;
    if (bcdtmMath_distance(cPt.x,cPt.y,pointAddrP(dtmP, tmp.P1)->x,pointAddrP(dtmP, tmp.P1)->y) < dtmP->ppTol)
        return DTM_ERROR;
    if (tmp.P2 != dtmP->nullPnt && bcdtmMath_distance(cPt.x,cPt.y,pointAddrP(dtmP, tmp.P2)->x,pointAddrP(dtmP, tmp.P2)->y) < dtmP->ppTol)
        return DTM_ERROR;
    if (tmp.P3 != dtmP->nullPnt && bcdtmMath_distance(cPt.x,cPt.y,pointAddrP(dtmP, tmp.P3)->x,pointAddrP(dtmP, tmp.P3)->y) < dtmP->ppTol)
        return DTM_ERROR;
    if (bcdtmEdit_insertPointDtmObject (dtmP, (long)tmp.m_pointType, tmp.m_feature, m_inDynamics ? 0 : 1, tmp.P1, tmp.P2, tmp.P3, cPt.x, cPt.y,  cPt.z, &tmp.Np))
        return DTM_ERROR;

    if (m_inDynamics)
        m_dynamic_changes.push (new dynamicMod::AddVertex(tmp));

    return DTM_SUCCESS;
    }

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>
StatusInt BcDTMEdit::_MoveVertex (const DPoint3d& pt, bool updateZ)
    {
    if (m_selectionState != Point)
        return DTM_ERROR;

    if (!m_inDynamics && m_dtm->SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    DPoint3d cPt = pt;
    if (helper) helper->convertPointToDTM (cPt);
    long moveFlag ;

    if (!updateZ)
        cPt.z = m_selectedPoints[0].z;

    bcdtmEdit_checkPointXYCanBeMovedDtmObject (m_dtm->GetTinHandle(), m_pointNumber,cPt.x,cPt.y,moveFlag) ;

    if (!moveFlag)
        return DTM_ERROR;

    if (!m_inDynamics)
        {

//       Move The Vertex To A New Point

         if (bcdtmEdit_moveVertexXYZDtmObject (m_dtm->GetTinHandle(),0,m_pointNumber,cPt.x,cPt.y,cPt.z))
            {
             bcdtmEdit_tempMoveVertexXYZDtmObject (m_dtm->GetTinHandle(), m_pointNumber, m_selectedPoints[0].x, m_selectedPoints[0].y, m_selectedPoints[0].z) ;
             return DTM_ERROR;
            }

//       Clean DTM - To Account For Memory Model Problems

         if( bcdtmList_cleanDtmObject(m_dtm->GetTinHandle()))
           {
            return DTM_ERROR;
           }

        }
    else
        bcdtmEdit_tempMoveVertexXYZDtmObject (m_dtm->GetTinHandle(), m_pointNumber, cPt.x, cPt.y, cPt.z) ;



    if (m_inDynamics)
        {
        dynamicMod::MoveVertex* dynamicOp = new dynamicMod::MoveVertex();
        dynamicOp->pointNumber = m_pointNumber;
        dynamicOp->originalPoint = m_selectedPoints[0];
        m_dynamic_changes.push (dynamicOp);
        }

    return DTM_SUCCESS;
    }

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>
StatusInt BcDTMEdit::_DeleteTriangle (void)
    {
    if (m_selectionState != Triangle)
        return DTM_ERROR;

    if (!m_inDynamics && m_dtm->SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    if (m_inDynamics)
        {
        if (4 == m_selectedPoints.size())
            m_dynamic_changes.push (new dynamicMod::DelTriangle());
        }
    else
        {
        m_selectionState = None;
        if (bcdtmEdit_deleteTriangleDtmObject(m_dtm->GetTinHandle(), m_trianglePointNumber[0], m_trianglePointNumber[1], m_trianglePointNumber[2]))
            return DTM_ERROR;
        }
    return DTM_SUCCESS;
    }

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>
StatusInt BcDTMEdit::_DeleteVertex ()
    {
    if ( m_inDynamics)
        return DTM_ERROR;

    if (m_selectionState != Point)
        return DTM_ERROR;

    if (m_dtm->SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    m_selectionState = None;

    if( bcdtmEdit_deletePointDtmObject (m_dtm->GetTinHandle(), m_pointNumber, 1))
        return DTM_ERROR;

    return DTM_SUCCESS;
    }

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>
StatusInt BcDTMEdit::_SwapLine ()
    {
    if (m_inDynamics)
        return DTM_ERROR;

    if (m_selectionState != Line)
        return DTM_ERROR;

    if (m_dtm->SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    long nP1, nP2;
    m_selectionState = None;
    if (bcdtmEdit_deleteLineDtmObject (m_dtm->GetTinHandle(), 2 /*SwapLine*/, m_linePointNumber[0], m_linePointNumber[1], &nP1, &nP2))
        return DTM_ERROR;
    return DTM_SUCCESS;
    }

/// <author>Daryl.Holmwood</author>                            <date>2/2012</date>

StatusInt BcDTMEdit::_DeleteLine ()
    {
    if (m_inDynamics)
        return DTM_ERROR;

    if (m_selectionState != Line)
        return DTM_ERROR;

    if (m_dtm->SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    long nP1, nP2;
    m_selectionState = None;
    if (bcdtmEdit_deleteLineDtmObject (m_dtm->GetTinHandle(), 1 /*DeleteLine*/, m_linePointNumber[0], m_linePointNumber[1], &nP1, &nP2))
        return DTM_ERROR;
    return DTM_SUCCESS;
    }

/// <author>Rob.Cormack</author>                            <date>4/2012</date>

StatusInt BcDTMEdit::_DeleteFeature ()
    {
    if (m_inDynamics)
        return DTM_ERROR;

    if (m_selectionState != Feature )
        return DTM_ERROR;

    if (m_dtm->SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    m_selectionState = None;
    m_selectedFeatures.clear();
    m_selectedFeaturesIter = m_selectedFeatures.begin();
    if (bcdtmEdit_deleteLinearFeatureDtmObject (m_dtm->GetTinHandle(),m_selectedFeaturesIter->featureNumber, m_selectedFeaturesIter->featureType))
        return DTM_ERROR;
    return DTM_SUCCESS;
    }

StatusInt BcDTMEdit::_DeleteTrianglesByLine()
    {
    if (m_inDynamics)
        return DTM_ERROR;

    if (m_dtm->SetMemoryAccess(DTMAccessMode::Write) != DTM_SUCCESS)
        {
        bcdtmWrite_message(1,0,0, "DTM is readonly");
        return DTM_ERROR;
        }

    // This is the old method we is much quicker but doesn't work as well and we need to stop at features.
    //if (bcdtmEdit_deleteTrianglesOnDeleteLineDtmObject (m_dtm->GetTinHandle(), (DPoint3d*)(DPoint3d*)helper->copyPointsToDTM (pts, numPts), numPts))
    //    return DTM_ERROR;
    //m_selectionState = None;
    //if (m_selectionState != TrianglesByLine)
    //    return false;

    bool doReverse = false;
    for (size_t i = 0; i < m_trianglePointNumbersByLineSelection.size(); i+= 3)
        {
        m_trianglePointNumber[0] = m_trianglePointNumbersByLineSelection[i];
        m_trianglePointNumber[1] = m_trianglePointNumbersByLineSelection[i + 1];
        m_trianglePointNumber[2] = m_trianglePointNumbersByLineSelection[i + 2];
        m_selectionState = Triangle;
        if (_CanDeleteTriangle())
            _DeleteTriangle();
        else
            {
            doReverse = true;
            break;
            }
        }
    if (doReverse)
        {
        for (size_t i = m_trianglePointNumbersByLineSelection.size() - 3; i >=3 ; i-= 3)
            {
            m_trianglePointNumber[0] = m_trianglePointNumbersByLineSelection[i];
            m_trianglePointNumber[1] = m_trianglePointNumbersByLineSelection[i + 1];
            m_trianglePointNumber[2] = m_trianglePointNumbersByLineSelection[i + 2];
            m_selectionState = Triangle;
            if (_CanDeleteTriangle())
                _DeleteTriangle();
            else
                {
                break;
                }
            }
        }
    m_selectionState = None;

    return DTM_SUCCESS;
    }


BENTLEYDTM_EXPORT TMEditorPtr ITMEditor::Make (BcDTMP dtm)
    {
    return BcDTMEdit::Create(dtm);
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::StartDynamics()
    {
    return _StartDynamics();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::EndDynamics()
    {
    return _EndDynamics();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::DrawDynamics (ITMEditorDrawer* drawer)
    {
    return _DrawDynamics (drawer);
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::DrawSelection (ITMEditorDrawer* drawer)
    {
    return _DrawSelection (drawer);
    }

BENTLEYDTM_EXPORT void ITMEditor::ClearSelection ()
    {
    return _ClearSelection();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::SelectVertex (const DPoint3d& pt)
    {
    return _SelectVertex (pt);
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::SelectLine (const DPoint3d& pt)
    {
    return _SelectLine (pt);
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::SelectFeature (const DPoint3d& pt,bool first)
    {
    return _SelectFeature (pt,first);
    }

BENTLEYDTM_EXPORT DTMFeatureType ITMEditor::GetSelectedFeatureType ()
    {
    return _GetSelectedFeatureType ();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::SelectTriangle (DPoint3dCR pt)
    {
    return _SelectTriangle (pt);
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::SelectTrianglesByLine (const DPoint3d pts[], int numPts, bool stopAtFeatures)
    {
    return _SelectTrianglesByLine (pts, numPts, stopAtFeatures);
    }

BENTLEYDTM_EXPORT ITMEditor::SelectionState ITMEditor::GetSelectionState()
    {
    return _GetSelectionState();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::AddVertex (const DPoint3d& pt, bool useZ)
    {
    return _AddVertex (pt, useZ);
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::MoveVertex (const DPoint3d& pt, bool updateZ)
    {
    return _MoveVertex (pt, updateZ);
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::DeleteVertex ()
    {
    return _DeleteVertex ();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::SwapLine ()
    {
    return _SwapLine ();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::DeleteLine ()
    {
    return _DeleteLine ();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::DeleteFeature ()
    {
    return _DeleteFeature ();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::DeleteTriangle()
    {
    return _DeleteTriangle();
    }

BENTLEYDTM_EXPORT StatusInt ITMEditor::DeleteTrianglesByLine ()
    {
    return _DeleteTrianglesByLine ();
    }

BENTLEYDTM_EXPORT bool ITMEditor::CanDeleteVertex ()
    {
    return _CanDeleteVertex ();
    }

BENTLEYDTM_EXPORT bool ITMEditor::CanDeleteLine ()
    {
    return _CanDeleteLine ();
    }

BENTLEYDTM_EXPORT bool ITMEditor::CanDeleteTriangle ()
    {
    return _CanDeleteTriangle();
    }

BENTLEYDTM_EXPORT bool ITMEditor::CanSwapLine ()
    {
    return _CanSwapLine ();
    }

BENTLEYDTM_EXPORT bool ITMEditor::CanDeleteTrianglesByLine ()
    {
    return _CanDeleteTrianglesByLine ();
    }


BENTLEYDTM_EXPORT bool ITMEditor::CanDeleteInternalTriangles()
    {
    return _CanDeleteInternalTriangles ();
    }

BENTLEYDTM_EXPORT void ITMEditor::SetCanDeleteInternalTriangles (bool value)
    {
    _SetCanDeleteInternalTriangles (value);
    }


END_BENTLEY_TERRAINMODEL_NAMESPACE
