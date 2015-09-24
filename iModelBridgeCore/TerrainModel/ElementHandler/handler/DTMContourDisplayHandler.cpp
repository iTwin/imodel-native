/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/DTMContourDisplayHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <stdafx.h>

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
const double MAX_FIVE_POINT_SMOOTHING = 5.0;
const double MAX_BSPLINE_SMOOTHING = 0.5;
const int MAX_SMOOTH_VALUE = 10;

#define CLOSED true

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
inline bool IsDepression (DTMUserTag userTag)
    {
    return 1 == userTag || 2 == userTag;
    }

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
inline void Append (bvector<bvector<DPoint3d>> &arr, DPoint3dCP points, size_t nPoints)
    {
    arr.push_back (bvector<DPoint3d>());
    bvector<DPoint3d> &lastItem = *arr.rbegin ();
    lastItem.insert (lastItem.end(), points, points + nPoints);
    }

//=======================================================================================
// @bsiclass                                            Sylvain.Pucci      08/2005
//=======================================================================================
struct DTMStrokeForCacheContours : IStrokeForCache
{
friend struct ContourLabellingLoaderGuard;
private:
    TextDrawer* m_textDrawer;
    mutable bool m_inDepressionSymbology;
    IDTM* m_dtmElement;
    ViewContextP m_viewContext;
    double m_majorInterval;
    double m_minorInterval;
    double m_registrationElevation;
    DTMDrawingInfo& m_drawingInfo;
    ElementHandleCR m_elementHandle;
    DPoint3d TextOffset; 
    WString m_buffer;
    double m_elevation;
    DTMElementContoursHandler::DisplayParams &m_dp;
    mutable TextDrawer::TextItems m_labels;
    DgnModelRefP const m_contextModelRef;
    const uint32_t m_id;
    DPoint3d m_ptGO;
    bool m_isDepressionSymbologyDifferent;

public:

    //=======================================================================================
    // @bsimethod                                                   Daryl.Holmwood 07/08
    //=======================================================================================
    DTMStrokeForCacheContours
    (
    IDTMP DTMElement,
    DTMElementContoursHandler::DisplayParams& dp,
    DTMDrawingInfo& drawingInfo,
    ElementHandleCR elementHandle,
    DgnModelRefP contextModelRef,
    uint32_t id
    ) : m_drawingInfo(drawingInfo), m_elementHandle(elementHandle), m_dp (dp), m_elevation (DBL_MAX), \
        m_inDepressionSymbology (false), m_contextModelRef (contextModelRef), m_id(id), m_textDrawer (nullptr)
        {
        m_dtmElement = DTMElement;
        m_majorInterval = 1e100;

        if (SUCCESS != dgnModel_getGlobalOrigin (elementHandle.GetDgnModelP(), &m_ptGO))
            m_ptGO.x = m_ptGO.y = m_ptGO.z = 0.;
        m_registrationElevation = drawingInfo.GetUORToStorageTransformation ()->form3d[2][3] + drawingInfo.ScaleUorsToStorageZ(m_ptGO.z);

        m_isDepressionSymbologyDifferent = IsDepressionSymbologyIsDifferent (m_dp);

        // Fix up the symbology if it is using ByCell (Use normal contour symbology)
        if (m_dp.GetDepressionSymbology().color== COLOR_BYCELL)
            m_dp.GetDepressionSymbology().color = dp.GetSymbology().color;
        if (m_dp.GetDepressionSymbology().style == STYLE_BYCELL)
            m_dp.GetDepressionSymbology().style = dp.GetSymbology().style;
        if (m_dp.GetDepressionSymbology().weight == WEIGHT_BYCELL)
            m_dp.GetDepressionSymbology().weight = dp.GetSymbology().weight;

        if (!m_dp.GetIsMajor())
            {
            // Find Major
            DTMSubElementIter iter(drawingInfo.GetSymbologyElement());

            m_minorInterval = m_drawingInfo.ScaleUorsToStorageZ (m_dp.GetContourInterval());
            for (; iter.IsValid(); iter.ToNext())
                {
                if (DTMElementContoursHandler::GetInstance()->GetSubHandlerId () == iter.GetCurrentId().GetHandlerId())
                    {
                    DTMElementContoursHandler::DisplayParams dp (iter);
                    if (dp.GetIsMajor())
                        {
                        m_majorInterval = m_drawingInfo.ScaleUorsToStorageZ (dp.GetContourInterval());

                        if (dp.GetVisible() && IsMajorContour(dp.GetContourInterval(), m_dp.GetContourInterval()))
                            m_drawingInfo.SetHasMajorContourCache ();
                        m_isDepressionSymbologyDifferent |= IsDepressionSymbologyIsDifferent (dp);
                        }
                    }
                }
            }
        else
            {
            // Find Major
            DTMSubElementIter iter (drawingInfo.GetSymbologyElement());

            m_minorInterval = m_majorInterval = m_drawingInfo.ScaleUorsToStorageZ (m_dp.GetContourInterval());
            for (; iter.IsValid(); iter.ToNext())
                {
                if (DTMElementContoursHandler::GetInstance()->GetSubHandlerId () == iter.GetCurrentId().GetHandlerId())
                    {
                    DTMElementContoursHandler::DisplayParams dp (iter);
                    if (!dp.GetIsMajor())
                        {
                        m_minorInterval = m_drawingInfo.ScaleUorsToStorageZ (dp.GetContourInterval());

                        // If the major isnt a whole multiple of the minor or the minor isnt drawn. then use the major interval as the minor.
                        if (!(dp.GetVisible() && IsMajorContour(m_dp.GetContourInterval(), dp.GetContourInterval())))
                            {
                            m_minorInterval = m_majorInterval;
                            }
                        }
                    }
                }
            }
        }

     virtual ~DTMStrokeForCacheContours()
        {
        if (m_textDrawer)
            delete m_textDrawer;
        }

    bool IsMajorContour (double z, double interval)
        {
        static const double tol = 0.0001;
        double zrem = fmod (z, interval);

        if (zrem > tol)
            zrem -= interval;
        zrem = fabs (zrem);
        return zrem < tol;
        }

    void DrawContourForPicking (ElementHandleCR el, ViewContextR context, DPoint3d& startPt, DPoint3d& endPt, BcDTMP dtm);

    virtual void _StrokeForCache (ElementHandleCR el, ViewContextR context, double pixelSize) override;

protected:

    bool IsDepressionSymbologyIsDifferent(DTMElementContoursHandler::DisplayParams& dp)
        {
        // Need to check for ByCell
        return !(dp.GetDepressionSymbology().color == dp.GetSymbology().color || dp.GetDepressionSymbology().color == COLOR_BYCELL)
            || !(dp.GetDepressionSymbology().style == dp.GetSymbology().style || dp.GetDepressionSymbology().style == STYLE_BYCELL)
            || !(dp.GetDepressionSymbology().weight == dp.GetSymbology().weight || dp.GetDepressionSymbology().weight == WEIGHT_BYCELL);
        }

    bool CheckIgnoreInterval (bool isDepression, DPoint3dCP tPoint, int nPoint)
        {
        if (m_majorInterval != 0. && !m_dp.GetIsMajor())
            {
            double z = tPoint[0].z - m_registrationElevation;
            if (IsMajorContour (z, m_majorInterval))
                {
                if (m_drawingInfo.GetHasMajorContourCache ())
                    {
                    Append (isDepression ? m_drawingInfo.GetMajorContourDepressionCache() : m_drawingInfo.GetMajorContourCache(), tPoint, nPoint);
                    }
                return true;
                }
            }
        return false;
        }

    void ApplySymbology (bool depression) const
        {
        if (depression == m_inDepressionSymbology)
            return;

        m_inDepressionSymbology = depression;
        if (depression)
            {
            DTMElementSubDisplayHandler::SetSymbology (m_dp, m_drawingInfo, *m_viewContext, m_dp.GetDepressionSymbology());
            }
        else
            {
            DTMElementSubDisplayHandler::SetSymbology (m_dp, m_drawingInfo, *m_viewContext);
            }
        }

protected:

    bool GetPointContourLabelOffset
    (
    double      Offset,
    DPoint3d    *Cpl,
    DPoint3d    *Cph,
    DPoint3d    **Pv,
    DPoint3d    **Nv,
    DPoint3dR   p
    )
        {
        return GetPointContourLabelOffset (Offset, Cpl, Cph, Pv, Nv, &p.x, &p.y, &p.z);
        }

    /*----------------------------------------------------------------------+
    |                                                                       |
    |   GetPointContourLabelOffset                                                                |                                                                                    |
    |                                                                                                         |
    |     Counts the number of decimal points in a string until 0                      |
    |                                                                       |
    |   jov.04nov97  -  Created.                                                            |
    |                                                                       |
    +----------------------------------------------------------------------*/
    bool GetPointContourLabelOffset (double Offset,
                         DPoint3d *Cpl, DPoint3d *Cph,
                         DPoint3d **Pv, DPoint3d **Nv,
                         double *Xp, double *Yp, double *Zp )
    {
     double    dx = 0.0,dy = 0.0,ds = 0.0,ls = 0.0,ratio  = 0.0;
     DPoint3d  *pP ;
    /*
    ** Initialise Variables
    */
     *Xp = *Yp = *Zp = 0.0 ;
    /*
    ** Scan Offset Distance Aint Line
    */
     ls = ds = 0.0 ; pP = Cpl ;
     while ( ds < Offset && pP < Cph )
       {
        ++pP  ;  ls = ds ;
        dx = pP->x - (pP-1)->x ;
        dy = pP->y - (pP-1)->y ;
        ds = ds + sqrt(dx*dx+dy*dy) ;
       }
     if ( pP == Cph && ds < Offset ) return (false) ;
    /*
    ** Calculate Label Position
    */
     ratio = ( Offset - ls ) / ( ds - ls ) ;
     *Xp =  (pP-1)->x +  ratio * dx ;
     *Yp =  (pP-1)->y +  ratio * dy ;
     *Zp =  (pP-1)->z ;
     *Pv =  (pP-1) ;
     *Nv =  pP ;
    /*
    ** Label Position Found
    */
    /*
    ** Job Completed
    */
     return (true) ;
    }

    /*=========================== HTML doc =============================*//**
    // @memo     Draw and contour and label
    // @doc      
    // @notes    NONE
    // @author   Jay Vose --  2003/07/08 -- jay.vose@geopak.com
    // @param                     => 
    // @param    ,                <=                                       (NULL allowed)
    // @return   SUCCESS
    *           ERROR if thisP == NULL
    // @see      
    *======================================================================*/
    int DisplayContour
    (
    DPoint3d    *verticesP,                  /* => Array of vertices - last give direction */
    int               numberOfVertices,       /* => Number of vertices */
    //int               contourDirection,       /* => Contour direction */ 
    double            textLength,
    double            contourInterval        /* => Contour interval */
    );

    /// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
protected: void SmartPreLabelling (DPoint3dCR pt)
        {
        if (m_elevation == pt.z)
            return;

        // Elevation has chagned draw what we have.
        DrawLabels ();
        DPoint3d uorPt = pt;
        m_drawingInfo.FullStorageToUors (uorPt);

        DistanceFormatterPtr formatter = DistanceFormatter::Create (*m_viewContext->GetCurrentModel ()->GetDgnModelP ());
        formatter->SetPrecision (DoubleFormatter::ToPrecisionEnum (PrecisionType::Decimal, (byte)m_dp.GetContourLabelPrecision ()));
        m_buffer = formatter->ToString (uorPt.z - m_ptGO.z);        // includeUnits ?? 

        m_elevation = pt.z;
        }

    /// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
protected: void AddLabel (DPoint3dCR origin, double angle)
        {
        if (!m_viewContext->IsLocalPointVisible (origin, false)) 
            return;
//        if (!m_viewContext->IsFrustumPointVisible (origin, false)) // ToDo Vancovuer..
//            return;

        m_labels.push_back (TextDrawer::TextItem (origin, angle));
        }

protected:
    void DrawLabels (void) const;

}; // End DTMStrokeForCacheContours struct

/// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
void DTMStrokeForCacheContours::DrawLabels (void) const
    {
    if (m_textDrawer)
        m_textDrawer->DrawMultipleUsingCache (m_buffer, m_labels, false);
    m_labels.clear ();
    }

/// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
struct ContourLabellingLoaderGuard
{
private: typedef int (ContourLabellingLoaderGuard::*MyLoader) (DPoint3dP tPoint, bool isDepression, int nPoint);
private: DTMStrokeForCacheContours &m_stroker;
private: MyLoader       m_myLoader;
private: double         m_contourInterval;

public: ContourLabellingLoaderGuard (DTMStrokeForCacheContours &stroker) : m_stroker (stroker), m_myLoader (&ContourLabellingLoaderGuard::LabelNothing)
        {}

    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
public: ContourLabellingLoaderGuard (DTMStrokeForCacheContours &stroker, DrawPurpose drawPurpose, double pixelSize) : m_stroker (stroker)
        {
        if (drawPurpose == DrawPurpose::CaptureGeometry || stroker.m_textDrawer == nullptr)
            m_myLoader = &ContourLabellingLoaderGuard::LabelNothing;
        else if (pixelSize > 0.)
            {
            bool isTextVisible = m_stroker.m_textDrawer->IsTextVisible (pixelSize);

            m_contourInterval = m_stroker.m_drawingInfo.ScaleUorsToStorage (m_stroker.m_dp.GetTextInterval());
            if (!isTextVisible || m_contourInterval <= 0.)
                m_myLoader = &ContourLabellingLoaderGuard::LabelNothing;
            else if (m_stroker.m_dp.GetDrawTextOption() == DTMElementContoursHandler::DRTXT_Always)
                m_myLoader = &ContourLabellingLoaderGuard::LabelAll;
            else
                m_myLoader = &ContourLabellingLoaderGuard::LabelNothing;
            }
        else
            m_myLoader = &ContourLabellingLoaderGuard::LabelDepressionsOnly;
        }

    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
private: int LabelAll (DPoint3dP tPoint, bool isDepression, int nPoint)
        {
       // Check To Ignore Major Contours
        if (m_stroker.CheckIgnoreInterval (isDepression, tPoint, nPoint))
            return SUCCESS;

        m_stroker.ApplySymbology (isDepression);
        m_stroker.SmartPreLabelling (tPoint[0]);
        BeAssert (m_contourInterval != 0. && "Unexpected contourInterval value");
        m_stroker.DisplayContour (tPoint, nPoint, GetCurrlabelLength(), m_contourInterval);
        return SUCCESS;
        }

    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
private: int LabelNothing (DPoint3dP tPoint, bool isDepression, int nPoint)
        {
        // Check To Ignore Major Contours
        if (m_stroker.CheckIgnoreInterval (isDepression, tPoint, nPoint))
            return SUCCESS;

        m_stroker.ApplySymbology (isDepression);

        m_stroker.m_viewContext->DrawStyledLineString3d (nPoint, tPoint, NULL, !CLOSED);
        return SUCCESS;
        }

    /// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
private: double GetCurrlabelLength (void) const
        {
        return m_stroker.m_buffer.length() * m_stroker.m_textDrawer->FontSize().x;
        }

    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
private: int LabelDepressionsOnly (DPoint3dP tPoint, bool isDepression, int nPoint)
        {
        // Check To Ignore Major Contours
        if (m_stroker.CheckIgnoreInterval (isDepression, tPoint, nPoint))
            return SUCCESS;
        m_stroker.ApplySymbology (isDepression);
        if (isDepression)
            {
            m_stroker.SmartPreLabelling (tPoint[0]);
            m_stroker.DisplayContour (tPoint, nPoint, GetCurrlabelLength(), m_contourInterval);
            }
        else
            m_stroker.m_viewContext->DrawStyledLineString3d (nPoint, tPoint, NULL, !CLOSED);
        return SUCCESS;
        }

    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
public: static int LoadFuncBcDTM (DTMFeatureType featureType, DTMUserTag userTag, int64_t eltId, /*DTMFeatureId id,*/ DPoint3dP tPoint, size_t nPoint, void *userArgP)
        {
        ContourLabellingLoaderGuard *contourLabellingLoaderGuard = reinterpret_cast<ContourLabellingLoaderGuard*>(userArgP);
        return (*contourLabellingLoaderGuard) (tPoint, eltId, nPoint);
        }

    /// <author>Piotr.Slowinski</author>                            <date>6/2011</date>
public: int operator()(DPoint3dP tPoint, int64_t userTag, size_t nPoint)
        {
        if (m_stroker.m_viewContext->CheckStop ())
            return ERROR;

        if (0 == nPoint)
            return DTM_SUCCESS;

        bool isDepression = IsDepression (userTag);
        BeAssert (m_myLoader != NULL && "m_myLoader is NULL");
        return (this->*m_myLoader)(tPoint, isDepression, (int)nPoint);
        }

    /// <author>Piotr.Slowinski</author>                            <date>7/2011</date>
public: int operator()(bvector<DPoint3d> &points, DTMUserTag userTag)
        {
        return (*this)(&points[0], userTag, points.size());
        }

}; // End ContourLabellingLoaderGuard struct

/*=========================== HTML doc =============================*//**
// @memo     Draw and contour and label
// @doc      
// @notes    NONE
// @author   Jay Vose --  2003/07/08 -- jay.vose@geopak.com
// @param                     => 
// @param    ,                <=                                       (NULL allowed)
// @return   SUCCESS
*           ERROR if thisP == NULL
// @see      
*======================================================================*/
int DTMStrokeForCacheContours::DisplayContour
(
DPoint3d    *verticesP,                  /* => Array of vertices - last give direction */
int               numberOfVertices,       /* => Number of vertices */
//int               contourDirection,       /* => Contour direction */ 
double            textLength,
double            contourInterval        /* => Contour interval */
)
    {
    /* Local varibables */
    int         nP = 0;
    DPoint3d *lowContoutPointP = NULL, *highContoutPointP = NULL, 
        *pP = NULL, *Lsp = NULL, *Lsn = NULL, *Ljp = NULL, 
        *Ljn = NULL, *Lep = NULL, *Len = NULL, origin = {0., 0., 0.}, start = {0., 0., 0.}, end = {0., 0., 0.}, temp;
    double      angle = 0.;

    /* Validate data */
    if (!verticesP || (numberOfVertices <= 0)) return SUCCESS;

    /* Write the contour and label */
    if (contourInterval <= 0.) return(ERROR);

    lowContoutPointP  = verticesP; 
    highContoutPointP = verticesP + numberOfVertices - 1 ;

    /* Test if there is room for the label */
    for (;;)
        {
        if (GetPointContourLabelOffset (contourInterval, lowContoutPointP, highContoutPointP, &Lsp, &Lsn, start) &&
            GetPointContourLabelOffset (contourInterval+textLength/2., lowContoutPointP, highContoutPointP, &Ljp, &Ljn, origin) &&
            GetPointContourLabelOffset (contourInterval+textLength, lowContoutPointP, highContoutPointP, &Lep, &Len, end) )
            {
            /* Write the first part of the contour line */
            pP = lowContoutPointP ;
            nP = (int)(Lsp - lowContoutPointP + 2) ; 
            temp = Lsp[1];
            Lsp[1] = start;

            m_viewContext->DrawStyledLineString3d (nP, pP, NULL);

            Lsp[1] = temp;

            /* Write Contour Label */
            if ((end.x == start.x) && (end.y == start.y))
                angle = 0.;
            else
                angle = atan2 (end.y - start.y, end.x - start.x);

            /* Rotate text to be readible */
            if (( angle > M_PI/2. ) && (angle <= 3*M_PI/2.))
                angle += M_PI;
            else if (( angle < M_PI/-2. ) && (angle > 3*M_PI/-2.))
                angle += M_PI;

            /*                      if ( contourDirection) 
            { 
            angle += M_PI ; 
            if ( angle > dc_2pi ) angle -= dc_2pi ; 
            }*/
            if (m_viewContext->GetViewport ())
                AddLabel (origin, angle);

            /* Adjust Pointers To Coordinate Array */
            lowContoutPointP = Len - 1 ; 
            *lowContoutPointP = end;
            }
        else
            {
            /* Ignore Label and Write Complete Contour Line */
            pP = lowContoutPointP ;
            nP = (int)(highContoutPointP - lowContoutPointP + 1) ;
            m_viewContext->DrawStyledLineString3d (nP, pP, NULL);
            break;
            }
        }
    return(SUCCESS);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//
// Strokes the DTM for the cache
// At the moment, everything is handled by this stroke method. In the future, each
// part (triangle, features, ...), will have its own stroking method.
//
//=======================================================================================
void DTMStrokeForCacheContours::_StrokeForCache (ElementHandleCR el, ViewContextR context, double pixelSize)
    {   
    if (context.CheckStop() || m_dtmElement == nullptr)
        return;
    DrawPurpose const drawPurpose = context.GetDrawPurpose ();

    // Get the unmanaged handle.... everything must be very fast here.
    IDTM* bcDTM = m_dtmElement;

    m_viewContext = &context;

    if (drawPurpose != DrawPurpose::CaptureGeometry)
        {
        m_textDrawer = new TextDrawer (m_drawingInfo, context, m_id);
        if (!m_textDrawer->IsValid () || !m_textDrawer->IsTextVisible (pixelSize))
            m_dp.SetDrawTextOption ( DTMElementContoursHandler::DRTXT_None);
        else
            {
            m_textDrawer->FixBG();
            m_textDrawer->SetJustification (TextElementJustification::CenterMiddle);
            }
        }

    BcDTMP dtm = bcDTM->GetBcDTM();

    // Push the transformation matrix to transform the coordinates to UORS.
    int maxSlopeOption = m_dp.GetMaxSlopeOption();
    double maxSlopeValue = m_dp.GetMaxSlopeValue();
    double highLowContourInterval = m_drawingInfo.ScaleUorsToStorageZ(m_dp.GetContourInterval());
    int highLowOption = highLowContourInterval != 0 && m_isDepressionSymbologyDifferent;
    double contourInterval = m_drawingInfo.ScaleUorsToStorageZ(m_dp.GetContourInterval());
    double smoothingFactor = m_dp.GetSmoothingFactor();
    DTMContourSmoothing smoothing = (DTMContourSmoothing)m_dp.GetContourSmoothing ();
    if (smoothing == DTMContourSmoothing::Vertex)
        smoothingFactor = (m_dp.GetSmoothingFactor() / MAX_SMOOTH_VALUE) * MAX_FIVE_POINT_SMOOTHING;
    else if (smoothing == DTMContourSmoothing::Spline || smoothing == DTMContourSmoothing::SplineWithoutOverLapDetection)
        smoothingFactor = ((m_dp.GetSmoothingFactor() - 1) / MAX_SMOOTH_VALUE) * MAX_BSPLINE_SMOOTHING;

    if (drawPurpose == DrawPurpose::Hilite || drawPurpose == DrawPurpose::CaptureGeometry || drawPurpose == DrawPurpose::Flash || drawPurpose == DrawPurpose::Pick || drawPurpose == DrawPurpose::Hilite)
        highLowOption = 0;

    DrawSentinel    sentinel (context, m_drawingInfo);

    if (DrawPurpose::Flash == drawPurpose)
        {
        // Need to do something else here as they may not be anything visible if we just flash the hull.
        DisplayPathCP path = context.GetSourceDisplayPath();
        HitPathCP hitPath = dynamic_cast<HitPathCP>(path);

        if (hitPath)
            {
            DPoint3d pt;

            hitPath->GetHitPoint (pt);
            m_drawingInfo.RootToStorage (pt);
            ContourLabellingLoaderGuard ContourLabellingLoaderGuard (*this);
            dtm->ContourAtPoint (pt.x, pt.y, m_minorInterval, (DTMContourSmoothing)m_dp.GetContourSmoothing(), smoothingFactor, m_dp.GetSplineDensification(), m_drawingInfo.GetFence() /* DTM_Overlapped */, &ContourLabellingLoaderGuard, &ContourLabellingLoaderGuard::LoadFuncBcDTM);
            DrawLabels ();
            }
        }
    else
        {
        DTMContourSmoothing smoothing = (DrawPurpose::UpdateDynamic == drawPurpose) ? DTMContourSmoothing::None : (DTMContourSmoothing)m_dp.GetContourSmoothing();
        ContourLabellingLoaderGuard ContourLabellingLoaderGuard (*this, drawPurpose, pixelSize);

        if (m_dp.GetIsMajor() && m_drawingInfo.GetHasMajorContourCache ())
            {
            int const DEPRESSION = 1, NO_DEPRESSION = 0;
            typedef bvector<bvector<DPoint3d>> ContourCache;

            for (ContourCache::iterator iter = m_drawingInfo.GetMajorContourCache().begin(); iter != m_drawingInfo.GetMajorContourCache().end(); iter++)
                {
                //TRACE ("Pos: %d\n", iter - m_drawingInfo.GetMajorContourCache().begin ());
                if (ContourLabellingLoaderGuard (*iter, NO_DEPRESSION) != SUCCESS)
                    break;
                }
            for (ContourCache::iterator iter = m_drawingInfo.GetMajorContourDepressionCache().begin(); iter != m_drawingInfo.GetMajorContourDepressionCache().end(); iter++)
                {
                if (ContourLabellingLoaderGuard (*iter, DEPRESSION) != SUCCESS)
                    break;
                }
            }
        else if (DrawPurpose::Pick == drawPurpose)
            {
            IPickGeom*  pick = context.GetIPickGeom ();
            DPoint3d point = pick->GetPickPointRoot();
            DPoint3d pts[5];

            context.LocalToView (&pts[0], &point, 1);
            //                context->ViewToNpc (&pts[0], &pts[0], 1);
            pts[1] = pts[0];
            pts[2] = pts[0];
            pts[3] = pts[0];
            pts[4] = pts[0];

            double s = 25;
            pts[0].x -= s;
            pts[0].y -= s;

            pts[1].x -= s;
            pts[1].y += s;
            pts[2].x += s;
            pts[2].y += s;
            pts[3].x += s;
            pts[3].y -= s;
            pts[4] = pts[0];

            //                context->NpcToView (pts, pts, 5);
            context.ViewToLocal (pts, pts, _countof(pts));
            m_drawingInfo.RootToStorage (pts);
            DTMContourParams contourParams (contourInterval, m_registrationElevation, false, 0, 0, nullptr, 0, smoothing, smoothingFactor, m_dp.GetSplineDensification (), 0 /* smooth Length, true, */ , highLowOption != 0, maxSlopeOption, maxSlopeValue);
            dtm->BrowseContours (contourParams, DTMFenceParams(m_drawingInfo.GetFence().fenceType, DTMFenceOption::Overlap, (DPoint3d*)pts, 5), &ContourLabellingLoaderGuard, &ContourLabellingLoaderGuard::LoadFuncBcDTM);
            }
        else
            {
            DTMContourParams contourParams (contourInterval, m_registrationElevation, true, 0, 0, nullptr, 0, smoothing, smoothingFactor, m_dp.GetSplineDensification (), 0 /* smooth Length, true, */, highLowOption != 0, maxSlopeOption, maxSlopeValue);
            dtm->BrowseContours (contourParams, DTMFenceParams (m_drawingInfo.GetFence ().fenceType, DTMFenceOption::Overlap, (DPoint3d*)m_drawingInfo.GetFence ().points, m_drawingInfo.GetFence ().numPoints), &ContourLabellingLoaderGuard, &ContourLabellingLoaderGuard::LoadFuncBcDTM);
            }
        DrawLabels ();
        }
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/08
//=======================================================================================
void DTMStrokeForCacheContours::DrawContourForPicking (ElementHandleCR el, ViewContextR context, DPoint3d& startPt, DPoint3d& endPt, BcDTMP dtm)
    {   
    // Non Top View
    BC_DTM_OBJ* bcDTM = dtm->GetTinHandle();
    DPoint3d trianglePts[4];
    long drapedType;
    long voidFlag;
    DPoint3d point;
    double snapTol = 1;

    // Find the point on the surface to get the tolerance;
    if (bcdtmDrape_intersectTriangleDtmObject (bcDTM, ((DPoint3d*)&startPt), ((DPoint3d*)&endPt), &drapedType, (DPoint3d*)&point, (DPoint3d*)&trianglePts, &voidFlag) == DTM_SUCCESS && drapedType != 0 && voidFlag == 0)
        {
        startPt = point;
        endPt = point;
        m_drawingInfo.GetStorageToUORTransformation()->multiply (&point);
//        m_drawingInfo.GetCurrLocalToRootTrans().multiplyScaleAndTranslate (&point, &point, 1);
        snapTol = context.GetPixelSizeAtPoint (&point);
        point.x += snapTol;
//        m_drawingInfo.GetRootToCurrLocalTrans().multiplyScaleAndTranslate (&point, &point, 1);
        m_drawingInfo.GetUORToStorageTransformation()->multiply (&point);
        snapTol = point.distance (&startPt);
        startPt.z += 1;
        endPt.z -= 1;
        }

    m_dp.SetDrawTextOption (DTMElementContoursHandler::DRTXT_None);
//ToDo    int maxSlopeOption = m_dp.maxSlopeOption;
//ToDo    double maxSlopeValue = m_dp.maxSlopeValue;
//    double highLowContourInterval = m_drawingInfo.ScaleUorsToStorageZ(m_dp.GetContourInterval());
//    int highLowOption = highLowContourInterval != 0;
//  double contourInterval = m_drawingInfo.ScaleUorsToStorageZ(m_dp.GetContourInterval());
    double smoothingFactor = m_dp.GetSmoothingFactor();
    DTMContourSmoothing smoothing = (DTMContourSmoothing)m_dp.GetContourSmoothing ();
    
    if (smoothing == DTMContourSmoothing::Vertex)
        smoothingFactor = (m_dp.GetSmoothingFactor() / MAX_SMOOTH_VALUE) * MAX_FIVE_POINT_SMOOTHING;
    else if (smoothing == DTMContourSmoothing::Spline || smoothing == DTMContourSmoothing::SplineWithoutOverLapDetection)
        smoothingFactor = (m_dp.GetSmoothingFactor() / MAX_SMOOTH_VALUE) * MAX_BSPLINE_SMOOTHING;

    long contourFound = 0;
    DPoint3d* points = nullptr;
    long numConPts = 0;
    if (bcdtmDrape_intersectContourDtmObject (bcDTM, \
                                              ((DPoint3d*)&startPt), \
                                              ((DPoint3d*)&endPt), \
                                              m_minorInterval, \
                                              m_registrationElevation, \
                                              (DTMContourSmoothing)m_dp.GetContourSmoothing(), \
                                              smoothingFactor, \
                                              m_dp.GetSplineDensification(), \
                                              snapTol * 5., \
                                              &contourFound, \
                                              (DPoint3d**)&points, \
                                              &numConPts) == SUCCESS && contourFound)
        {
        if (m_dp.GetIsMajor() == IsMajorContour (points[0].z - m_registrationElevation, m_majorInterval))
            {
            m_viewContext = &context;

            DrawSentinel   sentinel (context, m_drawingInfo);
            bool const IS_DEPRESSION = false;

            if (!CheckIgnoreInterval (IS_DEPRESSION, points, numConPts))
                {
                ApplySymbology (IS_DEPRESSION);
                context.DrawStyledLineString3d (numConPts, points, NULL, !CLOSED);
                }
            }
        bcdtmUtl_freeMemory ((void**)&points);
        }
    }

//=======================================================================================
// @bsimethod                                                   Sylvain.Pucci
//=======================================================================================
bool DTMElementContoursDisplayHandler::_Draw (ElementHandleCR el, const ElementHandle::XAttributeIter& xAttr, DTMDrawingInfo& drawingInfo, ViewContextR context)
    {
    DrawPurpose const drawPurpose = context.GetDrawPurpose ();

    switch (drawPurpose)
        {
        case DrawPurpose::ChangedPre:
            // Draw is called with DrawPurpose::ChangedPre to erase a previously drawn object
            // So, we should call draw with the previous state of the element, but we do not have this
            // previous state, so we just redraw the range and it works.
            context.DrawElementRange (el.GetElementCP());
            return true;

        case DrawPurpose::FitView:
            DrawScanRange (context, el, drawingInfo.GetDTMDataRef());
            return false;

        case DrawPurpose::RangeCalculation:
            return true;

        default:
            // Create a DTM element from the XAttributes (this is is a very lightweight operation that
            // just assigns the dtm internal arrays to their addresses inside the XAttributes).
            RefCountedPtr<DTMDataRef> DTMDataRef = drawingInfo.GetDTMDataRef (); 
            BENTLEY_NAMESPACE_NAME::TerrainModel::DTMPtr dtmPtr (DTMDataRef->GetDTMStorage (DrawFeatures, context));
            BcDTMP dtm = dtmPtr != 0 ? dtm = dtmPtr->GetBcDTM() : 0;

            if (!dtm || dtm->GetDTMState() != DTMState::Tin)
                return false;
            if (!DTMElementSubDisplayHandler::CanDoPickFlash (DTMDataRef, drawPurpose) && \
                (DrawPurpose::Pick == drawPurpose || DrawPurpose::Flash == drawPurpose))
                {
                return false;
                }

            DTMElementContoursHandler::DisplayParams displayParams (xAttr);

            if (!SetSymbology (displayParams, drawingInfo, context))
                return false;

            DgnModelRefP contextModelRef = context.GetViewport() ? context.GetViewport()->GetRootModel() : GetModelRef(el);;
            DTMStrokeForCacheContours contoursStroker (dtmPtr.get(), displayParams, drawingInfo, el, contextModelRef, xAttr.GetId());

            if (DrawPurpose::Pick == drawPurpose)
                {
                DPoint3d startPt, endPt;

                GetViewVectorPoints (drawingInfo, context, dtmPtr, startPt, endPt);
                contoursStroker.DrawContourForPicking (el, context, startPt, endPt, dtm);
                }
            else
                contoursStroker._StrokeForCache (el, context, context.GetViewport() ? context.GetViewport()->GetPixelSizeAtPoint (NULL) : 0.);
        }

    return true;
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 07/10
//=======================================================================================
void DTMElementContoursDisplayHandler::_GetPathDescription
(
ElementHandleCR                        elem,
ElementHandle::XAttributeIter const&   xAttr,
LazyDTMDrawingInfoProvider&         ldip,
WStringR                            string,
HitPathCR                           path,
WCharCP                           levelStr,
WCharCP                           modelStr,
WCharCP                           groupStr,
WCharCP                           delimiterStr
)
    {
//ToDo #define MODEL_TYPE_SENSITIVE_TOOLTIP //This is probably wrong 
    uint32_t const DESIRED_LENGTH_IS_IGNORED = 0;
    DPoint3d pt, globalOrigin;
    WString wElevString;
#ifdef MODEL_TYPE_SENSITIVE_TOOLTIP
    DgnModelRefP targetModelRef;
#endif

    DTMElementContoursHandler::GetInstance()->_GetDescription (elem, xAttr, string, DESIRED_LENGTH_IS_IGNORED);
    path.GetHitPoint (pt);

    ldip.Get ().GetRootToCurrLocalTrans ().multiplyAndRenormalize (&pt, &pt, 1);

    dgnModel_getGlobalOrigin (path.GetRoot()->GetDgnModelP(), &globalOrigin);

#ifdef MODEL_TYPE_SENSITIVE_TOOLTIP
    BeAssert (path.GetViewport() && "HitPath without Viewport");
    targetModelRef = path.GetViewport() ? path.GetViewport()->GetTargetModel() : ACTIVEMODEL;
    if (mdlModelRef_isSheet(targetModelRef) || !mdlModelRef_is3D(targetModelRef))
        {
        char elevString[MAX_UNIT_LABEL_BYTES * 2];
        StatusInt   result;

        //ldip.Get().RootToStorage (pt);
        //Above does not work because DTM transformation matrix
        ldip.Get().GetRootToCurrLocalTrans().multiplyAndRenormalize (&pt, &pt, 1);
        pt.z *= mdlModelRef_getUorPerMaster (targetModelRef) / mdlModelRef_getUorPerMaster (path.GetRoot());
        mdlString_fromUors (elevString, pt.z - globalOrigin.z);
        mdlCnv_convertMultibyteToUnicode (elevString, -1, wElevString, _countof(wElevString));
        result =  mdlModelRef_getMasterUnitLabel (path.GetRoot(), wElevString + wcslen (wElevString));
        BeAssert (SUCCESS == result);
        }
    else
#endif
        {
        DTMElementContoursHandler::DisplayParams displayParams (xAttr);
        DistanceFormatterPtr formatter = DistanceFormatter::Create (*path.GetRoot ()->GetDgnModelP ());
        formatter->SetPrecision (DoubleFormatter::ToPrecisionEnum (PrecisionType::Decimal, (byte)displayParams.GetContourLabelPrecision ()));
        wElevString = formatter->ToString (pt.z - globalOrigin.z);        // includeUnits ?? 
        }
    WString elevationString = TerrainModelElementResources::GetString (MSG_TERRAINMODEL_Elevation);
    elevationString.ReplaceAll (L"{1}", wElevString.GetWCharCP());
    string.append (delimiterStr + elevationString);
    }

//=======================================================================================
// @bsimethod                                                   Daryl.Holmwood 09/11
//=======================================================================================
void DTMElementContoursDisplayHandler::_EditProperties (EditElementHandleR element, ElementHandle::XAttributeIter xAttr, DTMSubElementId const &sid, PropertyContextR context)
    {
    T_Super::_EditProperties (element, xAttr, sid, context);

    DTMElementContoursHandler::DisplayParams displayParams (element, sid);
    PropsCallbackFlags propsFlag = displayParams.GetVisible() ?  PROPSCALLBACK_FLAGS_NoFlagsSet : PROPSCALLBACK_FLAGS_UndisplayedID;
    bool changed = false;
    if (0 != (ELEMENT_PROPERTY_TextStyle & context.GetElementPropertiesMask ()))
        {
        uint32_t textStyle = displayParams.GetTextStyleID ();
        EachTextStyleArg arg (textStyle, propsFlag, context);
        changed |= context.DoTextStyleCallback (&textStyle, arg);
        displayParams.SetTextStyleID (textStyle);
        }

    if (changed)
        displayParams.SetElement (element, sid);

    // If purpose is just a simple id remap we don't need to regenerate note...
    if ((EditPropertyPurpose::Change == context.GetIEditPropertiesP ()->_GetEditPropertiesPurpose () || !context.GetElementChanged ()) && displayParams.GetTextStyleID() != 0)
        {
        // If properties being edited don't affect layout we don't beed to regenerate note...
        if (0 != ((ELEMENT_PROPERTY_Font | ELEMENT_PROPERTY_TextStyle | ELEMENT_PROPERTY_DimStyle | ELEMENT_PROPERTY_ElementTemplate) & context.GetElementPropertiesMask ()))
            {
            if (AddDTMTextStyle (element, displayParams.GetTextStyleID(), sid.GetId()))
                {
                element.GetElementDescrP ()->h.isValid = false;
                context.SetElementChanged ();
                }
            }
        }
    }

SUBDISPLAYHANDLER_DEFINE_MEMBERS (DTMElementContoursDisplayHandler);

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE
