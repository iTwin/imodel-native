//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGF3DCameraTransfoModel.cpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : KGI3DCameraTransfoModel (inline methods)
//-----------------------------------------------------------------------------

#include "hstdcpp.h"
#include "HGF3DCameraTransfoModel.h"

/** -----------------------------------------------------------------------------
    Creates a model based on the camera transformation model (camera settings)

    @param pi_rpCameraData
                       The camera data that contain the transformation model
                       that defines the relation between
                       orthorectified image and real photo. This transformation is simply
                       the transformation expressing the orientation of the camera
                       relative to ground at the time the picture was taken.

    -----------------------------------------------------------------------------
*/
HGF3DCameraTransfoModel::HGF3DCameraTransfoModel(const HGFCameraData& pi_rCameraData)
    : m_CameraData(pi_rCameraData)
    {
    // Extract rotation matrix
    m_RotationMatrix = m_CameraData.Get3DRotationMatrix();

    m_InvRotationMatrix = InverseMatrix(m_RotationMatrix);
    }


/** -----------------------------------------------------------------------------
    Copy constructor

    @param pi_rObj     The camera transformation model to duplicate
    -----------------------------------------------------------------------------
*/
HGF3DCameraTransfoModel::HGF3DCameraTransfoModel(const HGF3DCameraTransfoModel& pi_rObj)
    : m_CameraData(pi_rObj.m_CameraData),
      m_RotationMatrix(pi_rObj.m_RotationMatrix),
      m_InvRotationMatrix(pi_rObj.m_InvRotationMatrix)
    {
    // Extract rotation matrix
    m_RotationMatrix = m_CameraData.Get3DRotationMatrix();

    m_InvRotationMatrix = InverseMatrix(m_RotationMatrix);
    }



/** -----------------------------------------------------------------------------
    Destroyer
    -----------------------------------------------------------------------------
*/
HGF3DCameraTransfoModel::~HGF3DCameraTransfoModel()
    {
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void HGF3DCameraTransfoModel::ConvertDirect(double*   pio_pXInOut,
                                            double*   pio_pYInOut,
                                            double*   pio_pZInOut) const
    {
    // Obtain center of perspective
    HGF3DPoint PerspectiveCenter = m_CameraData.GetPerspectiveCenter();


    // ground to photo transformation
    HGF3DPoint PhotoCoord(*pio_pXInOut - PerspectiveCenter.GetX(),
                          *pio_pYInOut - PerspectiveCenter.GetY(),
                          *pio_pZInOut - PerspectiveCenter.GetZ());

    // Apply rotation matrix
    PhotoCoord.Transform(m_RotationMatrix);

    HASSERT(PhotoCoord.GetZ() != 0.0);


    double F = m_CameraData.GetFocalDistance() / PhotoCoord.GetZ();

    HGF2DPosition PrincipalPoint = m_CameraData.GetPrincipalPoint();

    // Photo coordinate
    double XPhoto = PrincipalPoint.GetX() - (F * PhotoCoord.GetX());
    double YPhoto = PrincipalPoint.GetY() - (F * PhotoCoord.GetY());


    *pio_pXInOut = XPhoto;
    *pio_pYInOut = YPhoto;
    *pio_pZInOut = F;

    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void HGF3DCameraTransfoModel::ConvertInverse(double*   pio_pXInOut,
                                             double*   pio_pYInOut,
                                             double*   pio_pZInOut) const
    {
    // The camera side elevation cannot be 0 (at focal center)
    HPRECONDITION(*pio_pZInOut != 0.0);

    // Obtain center of perspective
    HGF3DPoint PerspectiveCenter = m_CameraData.GetPerspectiveCenter();
    HGF2DPosition PrincipalPoint = m_CameraData.GetPrincipalPoint();


    double XPhoto = *pio_pXInOut;
    double YPhoto = *pio_pYInOut;
    double F = *pio_pZInOut;

    double XPhotoCoord;
    double YPhotoCoord;
    double ZPhotoCoord;

    XPhotoCoord = (PrincipalPoint.GetX() - XPhoto) / F;
    YPhotoCoord = (PrincipalPoint.GetY() - YPhoto) / F;
    ZPhotoCoord = m_CameraData.GetFocalDistance() / F;


    // ground to photo transformation
    HGF3DPoint PhotoCoord(XPhotoCoord,
                          YPhotoCoord,
                          ZPhotoCoord);

    // Apply rotation matrix
    PhotoCoord.Transform(m_InvRotationMatrix);


    *pio_pXInOut = PhotoCoord.GetX() + PerspectiveCenter.GetX();
    *pio_pYInOut = PhotoCoord.GetY() + PerspectiveCenter.GetY();
    *pio_pZInOut = PhotoCoord.GetZ() + PerspectiveCenter.GetZ();

    }




//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void HGF3DCameraTransfoModel::ConvertDirect(double    pi_XIn,
                                            double    pi_YIn,
                                            double    pi_ZIn,
                                            double*   po_pXOut,
                                            double*   po_pYOut,
                                            double*   po_pZOut) const
    {
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;
    *po_pZOut = pi_ZIn;

    ConvertDirect(po_pXOut, po_pYOut, po_pZOut);
    }


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void HGF3DCameraTransfoModel::ConvertInverse(double    pi_XIn,
                                             double    pi_YIn,
                                             double    pi_ZIn,
                                             double*   po_pXOut,
                                             double*   po_pYOut,
                                             double*   po_pZOut) const
    {
    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;
    *po_pZOut = pi_ZIn;

    ConvertInverse(po_pXOut, po_pYOut, po_pZOut);
    }


