//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE3DPolyLine.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <ImagePP/all/h/HGF2DLocation.h>
#include <ImagePP/all/h/HVE2DPolySegment.h>
#include <ImagePP/all/h/HVE3DPolyLine.h>


//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the polyline
//-----------------------------------------------------------------------------
const HGF3DExtent<double>& HVE3DPolyLine::GetExtent() const
    {
    HINVARIANTS;

    // Check if 2D polyline is already present
    if (!m_ExtentUpToDate)
        {
        UpdateInternalExtent();
        }

    HASSERT(m_ExtentUpToDate);

    return m_Extent;
    }


//-----------------------------------------------------------------------------
// Intersect2D
// Returns a list of intersection points with polyline
//-----------------------------------------------------------------------------
size_t HVE3DPolyLine::Intersect2D(const HVE3DPolyLine& i_rPolyLine,
                                  HGF3DPointCollection* io_pCrossPoints) const
    {
    HINVARIANTS;

    // Recipient
    HPRECONDITION(io_pCrossPoints != NULL);


    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    // Do likewise for given polyline
    if (!i_rPolyLine.m_2DPolyLineUpToDate)
        {
        i_rPolyLine.Create2DPolyLine();
        }

    HASSERT(i_rPolyLine.m_2DPolyLineUpToDate);

    // Intersect 2d polylines
    HGF2DLocationCollection My2DCrossPoints;

    // Obtain intersection
    if (m_p2DPolyLine->Intersect(*(i_rPolyLine.m_p2DPolyLine), &My2DCrossPoints) > 0)
        {
        // There are cross points
        // We transform cross points into 3D (elevation is 0)
        HGF2DLocationCollection::iterator Pts2DItr;
        for (Pts2DItr = My2DCrossPoints.begin() ; Pts2DItr != My2DCrossPoints.end() ; ++Pts2DItr)
            {
            io_pCrossPoints->push_back(HGF3DPoint(Pts2DItr->GetX(), Pts2DItr->GetY(), 0.0));
            }
        }

    // Return number of new points
    return(My2DCrossPoints.size());
    }


//-----------------------------------------------------------------------------
// AutoCrosses2D
// Returns true if the linear crosses itself
//-----------------------------------------------------------------------------
bool HVE3DPolyLine::AutoCrosses2D() const
    {
    HINVARIANTS;

    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    // Obtain intersection
    return(m_p2DPolyLine->AutoCrosses());
    }


//-----------------------------------------------------------------------------
// Crosses2D
// Returns true if the two linears cross in 2D
//-----------------------------------------------------------------------------
bool HVE3DPolyLine::Crosses2D(const HVE3DPolyLine& i_rPolyLine) const
    {
    HINVARIANTS;

    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    // Do likewise for given polyline
    if (!i_rPolyLine.m_2DPolyLineUpToDate)
        {
        i_rPolyLine.Create2DPolyLine();
        }

    HASSERT(i_rPolyLine.m_2DPolyLineUpToDate);

    // Obtain intersection
    return(m_p2DPolyLine->Crosses(*(i_rPolyLine.m_p2DPolyLine)));
    }


//-----------------------------------------------------------------------------
// Intersect2D
// Returns a list of intersection points with polyline
//-----------------------------------------------------------------------------
size_t HVE3DPolyLine::GetContiguousPointsTo2D(const HVE3DPolyLine& i_rPolyLine,
                                              HGF3DPointCollection* io_pContiguousPoints) const
    {
    HINVARIANTS;

    // Recipient
    HPRECONDITION(io_pContiguousPoints != NULL);


    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    // Do likewise for given polyline
    if (!i_rPolyLine.m_2DPolyLineUpToDate)
        {
        i_rPolyLine.Create2DPolyLine();
        }

    HASSERT(i_rPolyLine.m_2DPolyLineUpToDate);

    // Intersect 2d polylines
    HGF2DLocationCollection MyContPoints;

    // Obtain intersection
    if (m_p2DPolyLine->ObtainContiguousnessPoints(*(i_rPolyLine.m_p2DPolyLine), &MyContPoints) > 0)
        {
        // There are cross points
        // We transform cross points into 3D (elevation is 0)
        HGF2DLocationCollection::iterator Pts2DItr;
        for (Pts2DItr = MyContPoints.begin() ; Pts2DItr != MyContPoints.end() ; ++Pts2DItr)
            {
            io_pContiguousPoints->push_back(HGF3DPoint(Pts2DItr->GetX(), Pts2DItr->GetY(), 0.0));
            }
        }

    // Return number of new points
    return(MyContPoints.size());
    }





//-----------------------------------------------------------------------------
// ConnectsTo2D
//-----------------------------------------------------------------------------
bool HVE3DPolyLine::ConnectsTo2D(const HVE3DPolyLine& i_rPolyLine) const
    {
    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    // Do likewise for given polyline
    if (!i_rPolyLine.m_2DPolyLineUpToDate)
        {
        i_rPolyLine.Create2DPolyLine();
        }

    HASSERT(i_rPolyLine.m_2DPolyLineUpToDate);

    return(m_p2DPolyLine->ConnectsTo(*(i_rPolyLine.m_p2DPolyLine)));

    }

//-----------------------------------------------------------------------------
// LinksTo2D
//-----------------------------------------------------------------------------
bool HVE3DPolyLine::LinksTo2D(const HVE3DPolyLine& i_rPolyLine) const
    {
    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    // Do likewise for given polyline
    if (!i_rPolyLine.m_2DPolyLineUpToDate)
        {
        i_rPolyLine.Create2DPolyLine();
        }

    HASSERT(i_rPolyLine.m_2DPolyLineUpToDate);

    return(m_p2DPolyLine->LinksTo(*(i_rPolyLine.m_p2DPolyLine)));

    }


//-----------------------------------------------------------------------------
// IsPointOn2D
//-----------------------------------------------------------------------------
bool HVE3DPolyLine::IsPointOn2D(const HGF3DPoint& i_rPoint) const
    {
    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    return(m_p2DPolyLine->IsPointOn(HGF2DLocation(i_rPoint.GetX(), i_rPoint.GetY(), m_pCoordSys)));
    }


//-----------------------------------------------------------------------------
// IsContiguousTo2D
//-----------------------------------------------------------------------------
bool HVE3DPolyLine::IsContiguousTo2D(const HVE3DPolyLine& i_rPolyLine) const
    {
    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    // Do likewise for given polyline
    if (!i_rPolyLine.m_2DPolyLineUpToDate)
        {
        i_rPolyLine.Create2DPolyLine();
        }

    HASSERT(i_rPolyLine.m_2DPolyLineUpToDate);

    return(m_p2DPolyLine->AreContiguous(*(i_rPolyLine.m_p2DPolyLine)));
    }


//-----------------------------------------------------------------------------
// CalculateLength2D
//-----------------------------------------------------------------------------
double HVE3DPolyLine::CalculateLength2D() const
    {
    // Check if 2D polyline is already present
    if (!m_2DPolyLineUpToDate)
        {
        Create2DPolyLine();
        }

    HASSERT(m_2DPolyLineUpToDate);

    return(m_p2DPolyLine->CalculateLength());
    }


//-----------------------------------------------------------------------------
// Create2DPolyLine
//-----------------------------------------------------------------------------
void HVE3DPolyLine::Create2DPolyLine() const
    {
    // Create 2d polyline
    m_p2DPolyLine = new HVE2DPolySegment(m_pCoordSys);


    // Copy points
    for (size_t i = 0; i < m_Points.size(); ++i )
        {
        // Add point
        m_p2DPolyLine->AppendPosition(HGF2DPosition(m_Points[i].GetX(), m_Points[i].GetY()));
        }

    m_p2DPolyLine->SetAutoToleranceActive(false);
    m_p2DPolyLine->SetTolerance(m_Tolerance);

    // Indicate the polyline is up to date
    m_2DPolyLineUpToDate = true;
    }

//-----------------------------------------------------------------------------
// PRIVATE
// ResetTolerance() - resets the tolerance is auto tolerance is active
//-----------------------------------------------------------------------------
void HVE3DPolyLine::ResetTolerance()
    {
    // Check if auto tolerance is active
    if (m_AutoToleranceActive)
        {
        m_Tolerance = HNumeric<double>::GLOBAL_EPSILON();

        // Auto tolerance is active ... we scan all points if any
        if(m_Points.size() > 0)
            {
            // There are some points
            for (size_t i = 0 ; i < m_Points.size() ; ++i)
                {
                m_Tolerance = MAX(m_Tolerance, m_Points[i].GetX() * HNumeric<double>::EPSILON_MULTIPLICATOR());
                m_Tolerance = MAX(m_Tolerance, m_Points[i].GetY() * HNumeric<double>::EPSILON_MULTIPLICATOR());
                m_Tolerance = MAX(m_Tolerance, m_Points[i].GetZ() * HNumeric<double>::EPSILON_MULTIPLICATOR());
                }
            }
        }
    }

//-----------------------------------------------------------------------------
// PRIVATE
// ResetTolerance() - resets the tolerance is auto tolerance is active
//-----------------------------------------------------------------------------
void HVE3DPolyLine::UpdateInternalExtent() const
    {
    // Check if auto tolerance is active
    if (!m_ExtentUpToDate)
        {
        // Clear current extent
        m_Extent = HGF3DExtent<double>();

        if (m_Points.size() > 0)
            {
            m_Extent.Set(m_Points[0].GetX(), m_Points[0].GetY(), m_Points[0].GetZ(),
                         m_Points[0].GetX(), m_Points[0].GetY(), m_Points[0].GetZ());
            }

        for (size_t indexPoints = 0 ; indexPoints < m_Points.size() ; indexPoints++)
            {
            m_Extent.Add(m_Points[indexPoints]);
            }

        m_ExtentUpToDate = true;
        }
    }
