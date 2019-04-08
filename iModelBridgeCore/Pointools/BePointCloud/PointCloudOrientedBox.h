/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudOrientedBox.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BePointCloud/BePointCloudCommon.h>
#include <BePointCloud/ExportMacros.h>

BEGIN_BENTLEY_BEPOINTCLOUD_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Structure used to define clips and create queries on point clouds.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct OrientedBox
    {
    private:
        DVec3d      m_xVec;
        DVec3d      m_yVec;
        DVec3d      m_zVec;
        DPoint3d    m_origin;

    public:
        
        BEPOINTCLOUD_EXPORT OrientedBox ();

        /*---------------------------------------------------------------------------------**//**
        * OrientedBox constructor to use with the clip boundary and mask methods
        * A OrientedBox can only be built using X Y Z directional vectors that are perpendicular to one-another.
        * @param xVec IN    X directional vector from the origin and with a magnitude of the width of the box
        * @param yVec IN    Y directional vector from the origin and with a magnitude of the height of the box  
        * @param zVec IN    Z directional vector from the origin and with a magnitude of the depth of the box
        * @param origin IN  the origin point of the box, can be any of the 8 points    
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        BEPOINTCLOUD_EXPORT OrientedBox (DVec3d& xVec, DVec3d& yVec, DVec3d& zVec, DPoint3d& origin);

        //!OrientedBox destructor
        BEPOINTCLOUD_EXPORT ~OrientedBox ();

        //! Get the X vector of this OrientedBox.
        //! @return X vector
        BEPOINTCLOUD_EXPORT DVec3d const&   GetXVec() const;
        //! Get the Y vector of this OrientedBox.
        //! @return Y vector
        BEPOINTCLOUD_EXPORT DVec3d const&   GetYVec() const;
        //! Get the Z vector of this OrientedBox.
        //! @return Z vector
        BEPOINTCLOUD_EXPORT DVec3d const&   GetZVec() const;
        //! Get the origin of this OrientedBox.
        //! @return origin
        BEPOINTCLOUD_EXPORT DPoint3d const& GetOrigin() const;

        BEPOINTCLOUD_EXPORT void         ApplyTransform(TransformCR trn);
        BEPOINTCLOUD_EXPORT void         Init(DVec3dCR xVec, DVec3dCR yVec, DVec3dCR zVec, DPoint3dCR origin);
        BEPOINTCLOUD_EXPORT static void  ComputeCornersFromOrientedBox (DPoint3d corners[8], OrientedBox const& clipBox, bool ccwOrder = false);

    };

END_BENTLEY_BEPOINTCLOUD_NAMESPACE

