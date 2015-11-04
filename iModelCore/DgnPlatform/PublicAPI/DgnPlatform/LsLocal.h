/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/LsLocal.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   01/03
//=======================================================================================
struct  LineJoint
{
    DPoint3d    m_dir;          // Unit bvector in direction of joint
    double      m_scale;        // Ratio of joint length to line offset

public:
    // Calculates an ARRAY of LineJoints from the input array of points.
    static void  FromVertices (LineJoint*, DPoint3dCP points, int nPoints, DPoint3dCP normal, DPoint3dCP pStartTangent, DPoint3dCP pEndTangent);
};

//=======================================================================================
// @bsiclass                                                      Keith.Bentley   02/03
//=======================================================================================
struct          Centerline
{
private:
    bool                m_taper;
    bool                m_hasWidth;
    int                 m_count;
    double              m_lastLen;
    DPoint3dP            m_pts;
    double*             m_widths;
    double*             m_lengths;
    DPoint3dCP          m_segmentDirection;

public:
    Centerline (DPoint3dP  pts, double* widths, double* lengths, bool tapered)
            {
            m_taper     = tapered;
            m_hasWidth  = false;
            m_count     = 0;
            m_lastLen   = 0.0;
            m_pts       = pts;
            m_widths    = widths;
            m_lengths   = lengths;
            }

        void                Empty       () {m_count = 0;}
        int                 GetCount    ()          const  {return m_count;}
        DPoint3dCP          GetPointAt  (int index) const  {return ((index <0) || (index >= m_count)) ? NULL : &m_pts[index];}
        double const*       GetWidthAt  (int index) const  {return ((index <0) || (index >= m_count)) ? NULL : &m_widths[index];}
        double const*       GetLengthAt (int index) const  {return ((index <0) || (index >= m_count)) ? NULL : &m_lengths[index];}
        void                SetDirection (DPoint3dCP dir)  {m_segmentDirection = dir;}
        DPoint3dCP          GetDirection () const  {return m_segmentDirection;}
        void                GetDirectionVector (DPoint3dR segDir, DPoint3dCR org, DPoint3dCR end) const;

    void    AddPoint    (DPoint3dCP pt, double width, double length);
    void    Output      (ViewContextP, LsStrokeP, DPoint3dCP normal, DPoint3dCP startTangent, DPoint3dCP endTangent);
};

END_BENTLEY_DGN_NAMESPACE

