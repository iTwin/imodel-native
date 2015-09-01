//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DEllipse.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//----------------------------------------------------------------------------
// Methods for class HVE2DEllipse
//----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DEllipse.h>
#include <Imagepp/all/h/HGF2DPolygonOfSegments.h>

HPM_REGISTER_CLASS(HVE2DEllipse, HVE2DSimpleShape)

#include <Imagepp/all/h/HVE2DSegment.h>
#include <Imagepp/all/h/HGF2DTransfoModel.h>

#define NB_SAMPLINGS 50

//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear describing the path of the ellipse
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DEllipse::GetLinear() const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    // Allocate linear
    HVE2DComplexLinear MyComplexLinear(GetCoordSys());

    double Delta = m_SemiMajorAxis / ((NB_SAMPLINGS / 2) - 1);

    if (m_IsVertical == true)
        {
        double PosY = m_Center.GetY() - m_SemiMajorAxis / 2;
        double FirstPosY = m_Center.GetY() - m_SemiMajorAxis / 2;
        double LastPosY = m_Center.GetY() + m_SemiMajorAxis / 2;
        HGF2DLocation Pos1(GetCoordSys());
        HGF2DLocation Pos2(GetCoordSys());

        Pos1.SetX(ComputeX(PosY));
        Pos1.SetY(PosY);

        PosY += Delta;

        while (PosY <= LastPosY)
            {
            Pos2.SetX(ComputeX(PosY));
            Pos2.SetY(PosY);

            MyComplexLinear.AppendLinear(HVE2DSegment(Pos1, Pos2));

            Pos1 = Pos2;

            PosY += Delta;
            }

        //This is to fix the case where PosX would be slightly greater than LastPosX
        //and thus creating an ellipse with a missing segment at the opposite of the starting point
        if(PosY < (LastPosY + Delta/2))
            {
            PosY = LastPosY;

            Pos2.SetX(ComputeX(PosY));
            Pos2.SetY(PosY);

            MyComplexLinear.AppendLinear(HVE2DSegment(Pos1, Pos2));

            Pos1 = Pos2;

            PosY += Delta;
            }

        PosY = LastPosY - Delta;

        Pos1.SetX(ComputeX(PosY, -1));
        Pos1.SetY(PosY);

        PosY -= Delta;

        MyComplexLinear.AppendLinear(HVE2DSegment(Pos2, Pos1));

        while (PosY > FirstPosY)
            {
            Pos2.SetX(ComputeX(PosY, -1));
            Pos2.SetY(PosY);

            MyComplexLinear.AppendLinear(HVE2DSegment(Pos1, Pos2));

            Pos1 = Pos2;

            PosY -= Delta;
            }

        //Close the shape
        MyComplexLinear.AppendLinear(HVE2DSegment(Pos1,
                                                  MyComplexLinear.GetStartPoint()));
        }
    else
        {
        double PosX = m_Center.GetX() - m_SemiMajorAxis / 2;
        double FirstPosX = m_Center.GetX() - m_SemiMajorAxis / 2;
        double LastPosX = m_Center.GetX() + m_SemiMajorAxis / 2;
        HGF2DLocation Pos1(GetCoordSys());
        HGF2DLocation Pos2(GetCoordSys());

        Pos1.SetX(PosX);
        Pos1.SetY(ComputeY(PosX));

        PosX += Delta;

        while (PosX <= LastPosX)
            {
            Pos2.SetX(PosX);
            Pos2.SetY(ComputeY(PosX));

            MyComplexLinear.AppendLinear(HVE2DSegment(Pos1, Pos2));

            Pos1 = Pos2;

            PosX += Delta;
            }

        //This is to fix the case where PosX would be slightly greater than LastPosX
        //and thus creating an ellipse with a missing segment at the opposite of the starting point
        if(PosX < (LastPosX + Delta/2))
            {
            PosX = LastPosX;

            Pos2.SetX(PosX);
            Pos2.SetY(ComputeY(PosX));

            MyComplexLinear.AppendLinear(HVE2DSegment(Pos1, Pos2));

            Pos1 = Pos2;

            PosX += Delta;
            }

        PosX = LastPosX - Delta;

        Pos1.SetX(PosX);
        Pos1.SetY(ComputeY(PosX, -1));

        PosX -= Delta;

        MyComplexLinear.AppendLinear(HVE2DSegment(Pos2, Pos1));

        while (PosX > FirstPosX)
            {
            Pos2.SetX(PosX);
            Pos2.SetY(ComputeY(PosX, -1));

            MyComplexLinear.AppendLinear(HVE2DSegment(Pos1, Pos2));

            Pos1 = Pos2;

            PosX -= Delta;
            }

        //Close the shape
        MyComplexLinear.AppendLinear(HVE2DSegment(Pos1,
                                                  MyComplexLinear.GetStartPoint()));
        }

    return(MyComplexLinear);
    }


//-----------------------------------------------------------------------------
// GetLinear
// This method returns a linear descibing the path of the ellipse
//-----------------------------------------------------------------------------
HVE2DComplexLinear HVE2DEllipse::GetLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    HVE2DComplexLinear Linear = GetLinear();

    if ((m_IsVertical == true) && (pi_DirectionDesired == HVE2DSimpleShape::CW))
        {
        Linear.Reverse();
        }
    else if (pi_DirectionDesired == HVE2DSimpleShape::CCW)
        {
        Linear.Reverse();
        }

    return Linear;
    }


//-----------------------------------------------------------------------------
// AllocateLinear
// This method returns a linear descibing the path of the ellipse
//-----------------------------------------------------------------------------
HVE2DComplexLinear* HVE2DEllipse::AllocateLinear(HVE2DSimpleShape::RotationDirection pi_DirectionDesired) const
    {
    // The ellipse must not be empty
    HPRECONDITION(!IsEmpty());

    // Allocate linear
    HVE2DComplexLinear* pMyComplexLinear = new HVE2DComplexLinear(GetLinear(pi_DirectionDesired));

    return(pMyComplexLinear);
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DEllipse::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    HPRECONDITION(GetCoordSys() == pi_rpCoordSys);

    HVE2DVector* pCopyInCoordSys = 0;

    if (GetCoordSys() == pi_rpCoordSys)
        {
        pCopyInCoordSys = (HVE2DVector*)Clone();
        }
    else
        {
        // Obtain the transformation model to the given coordinate system
        HFCPtr<HGF2DTransfoModel> pMyModel = GetCoordSys()->GetTransfoModelTo(pi_rpCoordSys);

        // Check if this model preserves shape
        if (pMyModel->PreservesShape())
            {
            HVE2DRectangle           CircumscriptRect(GetCoordSys());
            HAutoPtr<HVE2DVector> pCircumscriptRectInReqCS;

            if (m_IsVertical == true)
                {
                CircumscriptRect.SetRectangle(m_Center.GetX() - (m_SemiMinorAxis / 2),
                                              m_Center.GetY() - (m_SemiMajorAxis / 2),
                                              m_Center.GetX() + (m_SemiMinorAxis / 2),
                                              m_Center.GetY() + (m_SemiMajorAxis / 2));
                }
            else
                {
                CircumscriptRect.SetRectangle(m_Center.GetX() - (m_SemiMajorAxis / 2),
                                              m_Center.GetY() - (m_SemiMinorAxis / 2),
                                              m_Center.GetX() + (m_SemiMajorAxis / 2),
                                              m_Center.GetY() + (m_SemiMinorAxis / 2));
                }

            pCircumscriptRectInReqCS = CircumscriptRect.AllocateCopyInCoordSys(pi_rpCoordSys);

            HASSERT(pCircumscriptRectInReqCS->IsCompatibleWith(HVE2DRectangle::CLASS_ID) == true);

            // The model preserves shape ... we can transform the points directly
            pCopyInCoordSys = new HVE2DEllipse(*((HVE2DRectangle*)pCircumscriptRectInReqCS.get()));
            }
        else
            {
            // The model either does not preserve the shape
            // We transform the ellipse into a polygon, and pass and tell the polygon to
            // process itself
            HVE2DPolygon    ThePolygon(GetLinear());
            pCopyInCoordSys = ThePolygon.AllocateCopyInCoordSys(pi_rpCoordSys);
            }
        }

    return pCopyInCoordSys;
    }

//-----------------------------------------------------------------------------
// ResetTolerance
// PRIVATE
// This method recalculates the tolerance of ellipse if automatic tolerance is
// active
//-----------------------------------------------------------------------------
void HVE2DEllipse::ResetTolerance()
    {
    // Check if auto tolerance is active
    if (IsAutoToleranceActive())
        {
        // Autotolerance active ... update tolerance

        // Set tolerance to minimum value accepted
        double Tolerance = HGLOBAL_EPSILON;

        // Check if a greater tolerance may be applicable
        if (m_IsVertical == true)
            {
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetX() - m_SemiMinorAxis));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetX() + m_SemiMinorAxis));
            }
        else
            {
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetY() - m_SemiMinorAxis));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(m_Center.GetY() + m_SemiMinorAxis));
            }

        // Set tolerance
        SetTolerance(Tolerance);
        }
    }

//-----------------------------------------------------------------------------
// ComputeY
// PRIVATE
// This method returns the Y coordinate given an X coordinate.
// The parameter pi_SignFactor is used to return the Y coordinate below
// the centered coordinate system's X axis or above
//-----------------------------------------------------------------------------
double HVE2DEllipse::ComputeY(double pi_X,
                               short pi_SignFactor) const
    {
    HPRECONDITION(m_IsVertical == false);
    HPRECONDITION((pi_SignFactor == -1) || (pi_SignFactor == 1));

    return pi_SignFactor *
           sqrt(pow((m_SemiMinorAxis / 2.0), 2.0) *
                (1 - pow(pi_X - m_Center.GetX(), 2.0) /
                 pow(m_SemiMajorAxis / 2.0, 2.0))) +
           m_Center.GetY();
    }


//-----------------------------------------------------------------------------
// ComputeX
// PRIVATE
// This method returns the X coordinate given an Y coordinate.
// The parameter pi_SignFactor is used to return the X coordinate below
// the centered coordinate system's Y axis or above
//-----------------------------------------------------------------------------
double HVE2DEllipse::ComputeX(double pi_Y,
                               short pi_SignFactor) const
    {
    HPRECONDITION(m_IsVertical == true);
    HPRECONDITION((pi_SignFactor == -1) || (pi_SignFactor == 1));

    return pi_SignFactor *
           sqrt(pow((m_SemiMinorAxis / 2.0), 2.0) *
                (1 - pow(pi_Y - m_Center.GetY(), 2.0) /
                 pow(m_SemiMajorAxis / 2.0, 2.0))) +
           m_Center.GetX();
    }



//-----------------------------------------------------------------------------
// GetLightShape
// Returns the description of ellipse in the form of a light shape
//-----------------------------------------------------------------------------
HGF2DShape* HVE2DEllipse::GetLightShape() const
{
    HGF2DLocationCollection           Locations;
    HGF2DLocationCollection::iterator LocationIter;
    HGF2DLocationCollection::iterator LocationIterEnd;
    HGF2DPositionCollection           Coords;

    GetLinear().Drop(&Locations, 1.0);
    
    while (LocationIter != LocationIterEnd)
        {
        Coords.push_back(HGF2DPosition(LocationIter->GetX(),
                                       LocationIter->GetY()));
        LocationIter++;
        }
         
    return (new HGF2DPolygonOfSegments(Coords));
}
