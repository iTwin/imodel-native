/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/PointCloudVortex.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <RealityPlatform/RealityDataHandler.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=======================================================================================    
//! Class that encapsulates Vortex services
//! @bsiclass
//=======================================================================================    
struct PointCloudVortex
{

private:
    static void TransformAddRotation(TransformR transform, double xRotation, double yRotation, double zRotation, DPoint3dCR rotationOrigin);
    static void TransformAddTranslation(TransformR transform, DPoint3dCR translation);
    static void GetRotationMatrix(double xRotation, double yRotation, double zRotation, RotMatrixR rotMatrix);
    static void GetPointCloudBounds(DPoint3dR lowerBound, DPoint3dR upperBound, TransformCR transform, DPoint3dCR inLowerBound, DPoint3dCR inUpperBound);

public:
    //! Close POD file
    //! @param[in]  cloudFileHandle Handle to the POD file.
    static void     ClosePOD(PtHandle &cloudFileHandle);

    //! Read the points from the POD file.
    //! @param[out] pThumbnailBmp Bitmap created from the POD points.
    //! @param[in]  width Width (in pixels) of the bitmap to create.
    //! @param[in]  height Height (in pixels) of the bitmap to create.
    //! @param[in]  cloudHandle Handle to the point cloud (not the handle to the point cloud file, but a handle to a point cloud in a scene).
    //! @param[in]  densityValue Number of points to read from the POD file.
    //! @param[in]  transform Transformation to apply to the point cloud (it may be zoomed, rotated, panned, ...)
    //! @param[in]  needsWhiteBackground By default, the bitmap's background is black, but with some point clouds with mostly darker points, it is 
    //!             necessary to use a white background. If this parameter is true, a white background is used, otherwise it is black.
    //! @return NOERROR if no error occurred, or an error code otherwise.
    static HRESULT  ExtractPointCloud(  HBITMAP*        pThumbnailBmp, 
                                        uint32_t          width, 
                                        uint32_t          height, 
                                        PtHandle        cloudHandle, 
                                        float           densityValue, 
                                        TransformCR     transform, 
                                        bool            needsWhiteBackground);

    //! Get the RGB colors for point cloud classification.
    //! @param[out] classificationColors
    static void     GetClassificationColors(RgbFactor classificationColors[256]);

    //! Get the Transform for a point cloud to be mapped on a bitmap of a specified size.
    //! @param[out] outTransform Transformation matrix.
    //! @param[in]  cloudHandle Handle to the point cloud (not the handle to the point cloud file, but a handle to a point cloud in a scene).
    //! @param[in]  bitmapWidth Width (in pixels) of the bitmap to create.
    //! @param[in]  bitmapHeight Height (in pixels) of the bitmap to create.
    //! @param[in]  pointCloudView Indicate which view is desired for the thumbnail.
    static void     GetTransformForThumbnail(TransformR outTransform, PtHandle cloudHandle, UINT bitmapWidth, UINT bitmapHeight, PointCloudView  pointCloudView);

    //! Load Vortex DLLs and initialize Vortex
    static void     Initialize();

    //! Open POD file
    //! @param[out] cloudFileHandle Handle to the POD file.
    //! @param[out] cloudHandle Handle to the first POD in the scene.
    //! @param[in]  fileName Name of the POD file to open.
    //! @return S_OK if the POD file was correctly opened, E_FAIL otherwise.
    static HRESULT  OpenPOD(PtHandle &cloudFileHandle, PtHandle &cloudHandle, WString fileName);

    //! Use a small number of points of the point cloud to determine if the point cloud should be displayed on a white background.
    //! @param[in]  cloudHandle Handle to the point cloud.
    //! @return true if the point cloud should be displayed on a white background, false otherwise
    static bool     PointCloudNeedsWhiteBackground(PtHandle cloudHandle);

    static void GetPointCloudBounds(PtHandle cloudHandle, double* lower, double* upper);
    static PtHandle GetMetadataHandle(PtHandle cloudHandle);
    static bool GetMetaTag(PtHandle metaHandle, WCharCP tagName, WCharP value);
    
};


END_BENTLEY_REALITYPLATFORM_NAMESPACE