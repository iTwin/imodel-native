/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/PointCloudVortex.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include "PointCloudVortex.h"

#include <PointoolsVortexAPI_DLL/vortexLicense.c>
#include <PointoolsVortexAPI_DLL/PTAPI/PointoolsVortexAPI_import.cpp>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::Initialize()
    {
#ifdef DEBUG_PH
    clock_t processStartTime = clock();  
    PointCloudPreviewHandlerLogger::GetLogger()->messagev (LOG_INFO, L"In Initialize");
#endif

    LoadPointoolsDLL ("PointoolsVortexAPI.dll");

    ptInitialize(vortexLicCode);

#ifdef DEBUG_PH
    clock_t processEndTime = clock();  
    double processTime = (double)(processEndTime - processStartTime) / CLOCKS_PER_SEC;                  
    PointCloudPreviewHandlerLogger::GetLogger()->messagev (LOG_INFO, L"Out Initialize - elapsed time: %.4f", processTime);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT PointCloudVortex::OpenPOD(PtHandle &cloudFileHandle, PtHandle &cloudHandle, WString fileName)
    {
    //cloudFileHandle = PtVortex::OpenPOD (fileName.c_str());
    cloudFileHandle = ptOpenPOD(fileName.c_str());
    if (cloudFileHandle == 0)
        {            
        return E_FAIL;
        }
    //cloudHandle = PtVortex::GetCloudHandleByIndex(cloudFileHandle, 0); // first cloud in the scene
    cloudHandle = ptGetCloudHandleByIndex(cloudFileHandle, 0); // first cloud in the scene
    return S_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::ClosePOD(PtHandle &cloudFileHandle)
    {
    //PtVortex::RemoveScene(cloudFileHandle);
    ptRemoveScene(cloudFileHandle);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
HRESULT PointCloudVortex::ExtractPointCloud(HBITMAP*        pThumbnailBmp,
                                            uint32_t          bitmapWidth, 
                                            uint32_t          bitmapHeight, 
                                            PtHandle        cloudHandle, 
                                            float           densityValue, 
                                            TransformCR     transform,
                                            bool            needsWhiteBackground)
    {
    try 
        {
        // Get bounds for first point cloud in the scene
        float       lower [3], upper [3];
        //PtVortex::CloudBounds (cloudHandle, lower, upper);
        /*PTres result = */ptCloudBounds(cloudHandle, lower, upper);

        // Check if point cloud has RGB, Intensity or Classification channel
        // Priority to choose a channel: 1) RGB, 2) Classification, 3) Intensity
        wchar_t     name [256];
        uint32_t      numPoints;
        uint32_t      specification;
        bool      visible;
        //PtVortex::CloudInfo (cloudHandle, name, numPoints, specification, visible);
        /*PTres result =*/ ptCloudInfo(cloudHandle, name, numPoints, specification, visible);
        bool useRgbChannel = false;
        bool useIntensityChannel = false;
        bool useClassificationChannel = false;
        if (specification & PT_HAS_RGB)
            {
            useRgbChannel = true;
            }
        else if (specification & PT_HAS_CLASSIFICATION)
            {
            useClassificationChannel = true;
            }
        else if (specification & PT_HAS_INTENSITY)
            {
            useIntensityChannel = true;
            }

#ifdef DEBUG_PH
        if (specification & PT_HAS_RGB)
            PointCloudPreviewHandlerLogger::GetLogger()->messagev (LOG_INFO, L"   Has RGB channel");
        if (specification & PT_HAS_INTENSITY)
            PointCloudPreviewHandlerLogger::GetLogger()->messagev (LOG_INFO, L"   Has INTENSITY channel");
        if (specification & PT_HAS_CLASSIFICATION)
            PointCloudPreviewHandlerLogger::GetLogger()->messagev (LOG_INFO, L"   Has Classification channel");
#endif

        // Create the HBITMAP 
        BITMAPINFOHEADER BitInfo;
        BitInfo.biSize          = sizeof(BITMAPINFOHEADER);
        BitInfo.biWidth         = bitmapWidth;
        BitInfo.biHeight        = bitmapHeight;
        BitInfo.biPlanes        = 1;
        BitInfo.biBitCount      = 32;
        BitInfo.biCompression   = BI_RGB;
        BitInfo.biSizeImage     = 0;
        BitInfo.biXPelsPerMeter = 0;
        BitInfo.biYPelsPerMeter = 0;
        BitInfo.biClrUsed       = 0;
        BitInfo.biClrImportant  = 0;

        // BITMAP creation   
        void* pDataToCopy;
        *pThumbnailBmp = CreateDIBSection(NULL, (BITMAPINFO*)&BitInfo, DIB_RGB_COLORS, &pDataToCopy, NULL, 0);

        // Buffer allocation for bitmap
        // Each line in the DIB must be padded on a long [(+3)&-4]
        long DIBWidth = (((bitmapWidth * 4L) + 3L) & -4L);
        BYTE* pBitmapData = reinterpret_cast<BYTE *>(pDataToCopy);

        // Allocate buffers. The buffer size does not seem to have much influence, since we don't request a large number of points.
        const uint32_t bufferSize = 10000;
        double *pPointsBuffer = (double*) malloc (bufferSize * 3 * sizeof (double));
        BYTE   *pRgbBuffer = NULL;
        Byte  *pClassificationBuffer = NULL;
        int16_t  *pIntensityBuffer = NULL;

        RgbFactor classificationColors[256];
        if (useRgbChannel)
            {
            pRgbBuffer = (BYTE*) malloc (bufferSize * 3 * sizeof (BYTE));
            }
        else if (useClassificationChannel)
            {
            pClassificationBuffer = (Byte*) malloc (bufferSize * sizeof (Byte));
            GetClassificationColors(classificationColors);
            }
        else if (useIntensityChannel)
            {
            pIntensityBuffer = (int16_t*) malloc (bufferSize * sizeof (int16_t));
            }

        // If the point cloud is displayed as RGB or intensity, it's possible that most points are dark. 
        // In this case, we use a white background to better see the point cloud
        if (useRgbChannel || useIntensityChannel)
            {
            if (needsWhiteBackground)
                {
                // Most points are dark; set background to white so that point cloud is visible.
                memset(pBitmapData, 255, DIBWidth * bitmapHeight);
                }
            }

        // Bytes to use to define color. Default color is white (255, 255, 255).
	    BYTE rgba[4];
	    rgba[0] = 255; // blue
	    rgba[1] = 255; // green
	    rgba[2] = 255; // red
	    rgba[3] = 255; // alpha

        // Create a bounding box query
        //PtHandle queryHandle = PtVortex::CreateBoundingBoxQuery (lower[0], lower[1], lower[2], upper[0], upper[1], upper[2]);
        PtHandle queryHandle = ptCreateBoundingBoxQuery(lower[0], lower[1], lower[2], upper[0], upper[1], upper[2]);
	    //PtVortex::ResetQuery(queryHandle);
		ptResetQuery(queryHandle);
	    //PtVortex::SetQueryScope(queryHandle, cloudHandle);
		ptSetQueryScope(queryHandle, cloudHandle);
        //PtVortex::SetQueryRGBMode (queryHandle, QUERY_RGB_MODE_ACTUAL);
		ptSetQueryRGBMode(queryHandle, PT_QUERY_RGB_MODE_ACTUAL);

        // Set density (number of points retrieved) according to number of pixels in bitmap. Using the total number of pixels in the bitmap
        // seem to produce generally good results.
        //PtVortex::SetQueryDensity (queryHandle, QUERY_DENSITY_LIMIT, densityValue);
		ptSetQueryDensity(queryHandle, PT_QUERY_DENSITY_LIMIT, densityValue);
        //PtVortex::SetEnabledState (PT_RGB_SHADER, 1);
        ptEnable(PT_RGB_SHADER);

        DPoint3d pt3dOut;
        Byte idxClassif;

        // Query points as long as there are available (or until we have a number equal to densityValue, which is more likely to happen)
	    int numPointsBuffer;
        //numPointsBuffer = PtVortex::GetQueryPointsd (queryHandle, bufferSize, pPointsBuffer, pRgbBuffer, pIntensityBuffer, NULL, pClassificationBuffer);
		numPointsBuffer = ptGetQueryPointsd(queryHandle, bufferSize, pPointsBuffer, pRgbBuffer, pIntensityBuffer, NULL, pClassificationBuffer);

	    while (numPointsBuffer > 0)
		    {
		    LONG32 row, col;
            for (uint32_t i=0 ; i < ((uint32_t)numPointsBuffer * 3); i += 3)
	            {
                // Transform point using transfo. matrix
                DPoint3d pt3d;
                pt3d.x = pPointsBuffer[i];
                pt3d.y = pPointsBuffer[i+1];
                pt3d.z = pPointsBuffer[i+2];
                bsiTransform_multiplyDPoint3d (&transform, &pt3dOut, &pt3d);
                col = (LONG32) pt3dOut.x;
                row = (LONG32) pt3dOut.y;

                if (row < 0.0 || row >= (LONG32) bitmapHeight)
                    continue;
                if (col < 0.0 || col >= (LONG32) bitmapWidth)
                    continue;

                HASSERT (row >= 0 && row < (LONG32) bitmapHeight);
                HASSERT (col >= 0 && col < (LONG32) bitmapWidth);

                if (pRgbBuffer)
                    {
	                rgba[0] = pRgbBuffer[i+2]; // blue
	                rgba[1] = pRgbBuffer[i+1]; // green
	                rgba[2] = pRgbBuffer[i];   // red
                    }
                else if (pClassificationBuffer)
                    {
                    idxClassif = pClassificationBuffer[i/3];
	                rgba[0] = (BYTE)classificationColors[idxClassif].blue;
	                rgba[1] = (BYTE)classificationColors[idxClassif].green;
	                rgba[2] = (BYTE)classificationColors[idxClassif].red;
                    }
                else if (pIntensityBuffer)
                    {
                    int32_t intens = (int32_t) pIntensityBuffer[i/3];
                    int32_t normalizedIntens = (intens + 32768) / 256;    // Intensity is between -32768 and 32767
                    HASSERT (normalizedIntens >= 0 && normalizedIntens < 256);

	                rgba[0] = (BYTE)normalizedIntens; // blue
	                rgba[1] = (BYTE)normalizedIntens; // green
	                rgba[2] = (BYTE)normalizedIntens;   // red
                    }

                int posBitmap = (DIBWidth * row) + col * 4;
	            memcpy(pBitmapData + posBitmap, rgba, 4);
	            }

            if ((uint32_t) numPointsBuffer < bufferSize)
                {
                // Bug workaround. Sometimes, GetQueryPointsd infinitely returns the same number of points when numPointsBuffer < bufferSize
                break;
                }

		    //numPointsBuffer = PtVortex::GetQueryPointsd (queryHandle, bufferSize, pPointsBuffer, pRgbBuffer, pIntensityBuffer, NULL, pClassificationBuffer);
		    numPointsBuffer = ptGetQueryPointsd(queryHandle, bufferSize, pPointsBuffer, pRgbBuffer, pIntensityBuffer, NULL, pClassificationBuffer);
            }

        if (pPointsBuffer) free (pPointsBuffer);
        if (pRgbBuffer) free (pRgbBuffer);
        if (pIntensityBuffer) free (pIntensityBuffer);

        return NOERROR;
        }
    catch (long)
        {
        return ResultFromScode(E_OUTOFMEMORY);
        }

    catch ( exception &e )
        {
        //C++ exception
        ostringstream errorStr;

        errorStr << "Caught " << e.what( ) << endl;
        errorStr << "Type " << typeid( e ).name( ) << endl;
        return ResultFromScode(E_OUTOFMEMORY);
        }

    catch(...)
        {
        return ResultFromScode(E_OUTOFMEMORY);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudVortex::PointCloudNeedsWhiteBackground(PtHandle cloudHandle)
    {
    bool needsWhiteBackground = false;

    float       lower [3], upper [3];
	//PtVortex::CloudBounds (cloudHandle, lower, upper);
	ptCloudBounds(cloudHandle, lower, upper);

    // Check if point cloud has a RGB channel
    wchar_t     name [256];
    uint32_t      numPoints;
    uint32_t      specification;
    bool      visible;
    //PtVortex::CloudInfo (cloudHandle, name, numPoints, specification, visible);
	ptCloudInfo(cloudHandle, name, numPoints, specification, visible);

    double *pPointsBuffer = NULL;
    BYTE   *pRgbBuffer = NULL;
    int16_t  *pIntensityBuffer = NULL;

    bool useRgbChannel = false;
    bool useIntensityChannel = false;
    if (specification & PT_HAS_RGB)
        {
        useRgbChannel = true;
        }
    else if (specification & PT_HAS_INTENSITY)
        {
        useIntensityChannel = true;
        }
    else
        {
        // This point cloud has no RGB or intensity channel, so we don't need to set the background to white (points are drawn white)
        needsWhiteBackground = false;
        goto cleanup;
        }

    // Allocate buffers. We request 1000 points to compute an average color
    const uint32_t bufferSize = 1000;
    pPointsBuffer = (double*) malloc (bufferSize * 3 * sizeof (double));
    if (useRgbChannel)
        {
        pRgbBuffer = (BYTE*) malloc (bufferSize * 3 * sizeof (BYTE));
        }
    else if (useIntensityChannel)
        {
        pIntensityBuffer = (int16_t*) malloc (bufferSize * sizeof (int16_t));
        }

    // Create a bounding box query
    //PtHandle queryHandle = PtVortex::CreateBoundingBoxQuery (lower[0], lower[1], lower[2], upper[0], upper[1], upper[2]);
    PtHandle queryHandle = ptCreateBoundingBoxQuery(lower[0], lower[1], lower[2], upper[0], upper[1], upper[2]);
    //PtVortex::ResetQuery(queryHandle);
	ptResetQuery(queryHandle);
    //PtVortex::SetQueryScope(queryHandle, cloudHandle);
	ptSetQueryScope(queryHandle, cloudHandle);
    //PtVortex::SetQueryRGBMode (queryHandle, QUERY_RGB_MODE_ACTUAL);
	ptSetQueryRGBMode(queryHandle, PT_QUERY_RGB_MODE_ACTUAL);

    // Set density (number of points retrieved)
    //PtVortex::SetQueryDensity (queryHandle, QUERY_DENSITY_LIMIT, (float)bufferSize);
	ptSetQueryDensity(queryHandle, PT_QUERY_DENSITY_LIMIT, (float)bufferSize);
    //PtVortex::SetEnabledState (PT_RGB_SHADER, 1);
    ptEnable(PT_RGB_SHADER);

    // Query points
	int numPointsBuffer;
    //numPointsBuffer = PtVortex::GetQueryPointsd (queryHandle, bufferSize, pPointsBuffer, pRgbBuffer, pIntensityBuffer, NULL, NULL);
    numPointsBuffer = ptGetQueryPointsd(queryHandle, bufferSize, pPointsBuffer, pRgbBuffer, pIntensityBuffer, NULL, NULL);
    if (numPointsBuffer == 0)
        {
        needsWhiteBackground = false;
        goto cleanup;
        }

    BYTE colorThreshold = 25;
    if (useRgbChannel)
        {
        int64_t rgbTotal[3]; 
        rgbTotal[0] = 0;
        rgbTotal[1] = 0;
        rgbTotal[2] = 0;
        for (uint32_t i=0 ; i < ((uint32_t)numPointsBuffer * 3); i += 3)
	        {
	        rgbTotal[0] += pRgbBuffer[i+2]; // blue
	        rgbTotal[1] += pRgbBuffer[i+1]; // green
	        rgbTotal[2] += pRgbBuffer[i];   // red
	        }

        int64_t bAverage = rgbTotal[0] / numPointsBuffer;
        int64_t gAverage = rgbTotal[1] / numPointsBuffer;
        int64_t rAverage = rgbTotal[2] / numPointsBuffer;

        if (rAverage < colorThreshold && gAverage < colorThreshold && bAverage < colorThreshold)
            {
            // Most points are dark; set background to white so that point cloud is visible.
            needsWhiteBackground = true;
            }
        else
            {
            needsWhiteBackground = false;
            }
        }
    else
        {
        int64_t intensTotal = 0; 
        for (uint32_t i=0 ; i < ((uint32_t)numPointsBuffer); i ++)
	        {
            int32_t intens = (int32_t) pIntensityBuffer[i];
            int32_t normalizedIntens = (intens + 32768) / 256;    // Intensity is between -32768 and 32767
            HASSERT (normalizedIntens >= 0 && normalizedIntens < 256);

	        intensTotal += normalizedIntens;
            }

        int64_t intensAverage = intensTotal / numPointsBuffer;

        if (intensAverage < colorThreshold)
            {
            // Most points are dark; set background to white so that point cloud is visible.
            needsWhiteBackground = true;
            }
        else
            {
            needsWhiteBackground = false;
            }
        }

cleanup:
    if (pPointsBuffer) free (pPointsBuffer);
    if (pRgbBuffer) free (pRgbBuffer);
    if (pIntensityBuffer) free (pIntensityBuffer);

    return needsWhiteBackground;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::GetClassificationColors(RgbFactor classificationColors[256])
    {
    for (int i = 0; i < 256; i++)
        {
        switch (i)
            {
            // For LAS classes, we reuse the same default colors used by MicroStation (see DgnDisplay\library\PointCloud\PointCloudDisplayStyleListEH.cpp)
            case 0:
                // PCDISPLAYSTYLES_PointCloudCreated
                classificationColors[i].red = 255;
                classificationColors[i].blue = 0;
                classificationColors[i].green = 255;
                break;

            case 1:
                // PCDISPLAYSTYLES_PointCloudUnclassified
                classificationColors[i].red = 255;
                classificationColors[i].blue = 255;
                classificationColors[i].green = 255;
                break;

            case 2:
                // PCDISPLAYSTYLES_PointCloudGround
                classificationColors[i].red = 255;
                classificationColors[i].blue = 0;
                classificationColors[i].green = 0;
                break;

            case 3:
                // PCDISPLAYSTYLES_PointCloudLowVegetation
                classificationColors[i].red = 0;
                classificationColors[i].blue = 50;
                classificationColors[i].green = 200;
                break;

            case 4:
                // PCDISPLAYSTYLES_PointCloudMediumVegetation
                classificationColors[i].red = 0;
                classificationColors[i].blue = 100;
                classificationColors[i].green = 200;
                break;

            case 5:
                // PCDISPLAYSTYLES_PointCloudHighVegetation
                classificationColors[i].red = 0;
                classificationColors[i].blue = 200;
                classificationColors[i].green = 200;
                break;

            case 6:
                // PCDISPLAYSTYLES_PointCloudBuilding
                classificationColors[i].red = 150;
                classificationColors[i].blue = 150;
                classificationColors[i].green = 150;
                break;

            case 7:
                // PCDISPLAYSTYLES_PointCloudLowPoint
                classificationColors[i].red = 0;
                classificationColors[i].blue = 100;
                classificationColors[i].green = 50;
                break;

            case 8:
                // PCDISPLAYSTYLES_PointCloudModelKeyPoint
                classificationColors[i].red = 255;
                classificationColors[i].blue = 255;
                classificationColors[i].green = 255;
                break;

            case 9:
                // PCDISPLAYSTYLES_PointCloudWater
                classificationColors[i].red = 0;
                classificationColors[i].blue = 255;
                classificationColors[i].green = 0;
                break;

            case 10:
                // Reserved
                classificationColors[i].red = 255;
                classificationColors[i].blue = 255;
                classificationColors[i].green = 255;
                break;

            case 11:
                // Other
                classificationColors[i].red = 255;
                classificationColors[i].blue = 255;
                classificationColors[i].green = 0;
                break;

            case 12:
                // Other
                classificationColors[i].red = 0;
                classificationColors[i].blue = 0;
                classificationColors[i].green = 0;
                break;

            default:
                // Default is white
                classificationColors[i].red = 255;
                classificationColors[i].blue = 255;
                classificationColors[i].green = 255;
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::GetTransformForThumbnail(TransformR outTransform, PtHandle cloudHandle, UINT bitmapWidth, UINT bitmapHeight, PointCloudView  pointCloudView)
    {
    Transform transform;
    float       lower [3], upper [3];
	//PtVortex::CloudBounds (cloudHandle, lower, upper);
	ptCloudBounds(cloudHandle, lower, upper);
    bsiTransform_initIdentity(&transform);

    // Rotate according to the desired view (for Top view, nothing to do, the point cloud is already in top view)
    double xRotation = 0.0;
    double yRotation = 0.0;
    double zRotation = 0.0;
    if (pointCloudView == PointCloudView::Iso)
        {
        xRotation = PI / -4.0;
        zRotation = PI / 4.0;
        }
    else if (pointCloudView == PointCloudView::Front)
        {
        xRotation = PI / -2.0;
        }
    else if (pointCloudView == PointCloudView::Right)
        {
        zRotation = PI / -2.0;
        yRotation = PI / -2.0;
        }

    DPoint3d rotationOrigin;
    rotationOrigin.x = lower[0] + ((upper[0] - lower[0]) / 2.0);
    rotationOrigin.y = lower[1] + ((upper[1] - lower[1]) / 2.0);
    rotationOrigin.z = lower[2] + ((upper[2] - lower[2]) / 2.0);
    TransformAddRotation(transform, xRotation, yRotation, zRotation, rotationOrigin);

    // Compute new bounds based on rotation that we just made
    DPoint3d currentLower;
    currentLower.x = lower[0];
    currentLower.y = lower[1];
    currentLower.z = lower[2];
    DPoint3d currentUpper;
    currentUpper.x = upper[0];
    currentUpper.y = upper[1];
    currentUpper.z = upper[2];
    DPoint3d newLowerBound;
    DPoint3d newUpperBound;
    GetPointCloudBounds(newLowerBound, newUpperBound, transform, currentLower, currentUpper);

    // Translate point cloud to the origin
    DPoint3d translation;
    translation.x = 0.0 - newLowerBound.x;
    translation.y = 0.0 - newLowerBound.y;
    translation.z = 0.0;
    TransformAddTranslation(transform, translation);

    // Scale point cloud to fit on the bitmap
    Transform trfScale;

    // Compute stretch factor to fit points into bitmap (substract 2 to leave a 1 pixel border)
    double scalingX = (bitmapWidth - 2) / (newUpperBound.x - newLowerBound.x);
    double scalingY = (bitmapHeight - 2) / (newUpperBound.y - newLowerBound.y);
    // Make sure scaling is the same in X and Y
    double scaling = (scalingY > scalingX ? scalingX : scalingY);

    bsiTransform_initIdentity(&trfScale);
    bsiTransform_scaleMatrixColumns (&trfScale, &trfScale, scaling, scaling, scaling);
    bsiTransform_multiplyTransformTransform(&transform, &trfScale, &transform);

    // Translate one pixel to center the point cloud (reason: look at the (bitmapWidth - 2) above that creates a border).
    // This will leave a 1 pixel border around all bitmap.
    translation.x = 1.0;
    translation.y = 1.0;
    translation.z = 0.0;
    Transform trfTranslate;
    bsiTransform_initIdentity(&trfTranslate);
    bsiTransform_setTranslation (&trfTranslate, &translation);

    bsiTransform_multiplyTransformTransform(&outTransform, &trfTranslate, &transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::TransformAddRotation(TransformR transform, double xRotation, double yRotation, double zRotation, DPoint3dCR rotationOrigin)
    {
    // Translate to rotation origin
    DPoint3d translation;
    translation.x = 0.0 - rotationOrigin.x;
    translation.y = 0.0 - rotationOrigin.y;
    translation.z = 0.0 - rotationOrigin.z;
    Transform trfTranslate;
    bsiTransform_initIdentity(&trfTranslate);
    bsiTransform_setTranslation (&trfTranslate, &translation);

    Transform trfResult;
    bsiTransform_multiplyTransformTransform(&trfResult, &trfTranslate, &transform);

    // Rotate around origin
    RotMatrix rotMatrix;
    GetRotationMatrix(xRotation, yRotation, zRotation, rotMatrix);
    bsiTransform_multiplyRotMatrixTransform(&trfResult, &rotMatrix, &trfResult);

    // Translate to original position
    translation.x = rotationOrigin.x;
    translation.y = rotationOrigin.y;
    translation.z = rotationOrigin.z;
    bsiTransform_initIdentity(&trfTranslate);
    bsiTransform_setTranslation (&trfTranslate, &translation);

    bsiTransform_multiplyTransformTransform(&transform, &trfTranslate, &trfResult);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::TransformAddTranslation(TransformR transform, DPoint3dCR translation)
    {
    Transform trfTranslate;
    bsiTransform_initIdentity(&trfTranslate);
    bsiTransform_setTranslation (&trfTranslate, &translation);
    bsiTransform_multiplyTransformTransform(&transform, &trfTranslate, &transform);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::GetRotationMatrix(double xRotation, double yRotation, double zRotation, RotMatrixR rotMatrix)
    {
    // Create X rotation matrix
    RotMatrix rotMatrixX;
    DVec3d vec;
    vec.Init (1, 0, 0);
    bsiRotMatrix_initFromVectorAndRotationAngle (&rotMatrixX, &vec, xRotation);

    // Create Z rotation matrix
    RotMatrix rotMatrixZ;
    vec.Init (0, 0, 1);
    bsiRotMatrix_initFromVectorAndRotationAngle (&rotMatrixZ, &vec, zRotation);

    bsiRotMatrix_multiplyRotMatrixRotMatrix (&rotMatrix, &rotMatrixX, &rotMatrixZ);

    // Create Y rotation matrix
    RotMatrix rotMatrixY;
    vec.Init (0, 1, 0);
    bsiRotMatrix_initFromVectorAndRotationAngle (&rotMatrixY, &vec, yRotation);

    bsiRotMatrix_multiplyRotMatrixRotMatrix (&rotMatrix, &rotMatrix, &rotMatrixY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Eric.Paquet                     02/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::GetPointCloudBounds(DPoint3dR lowerBound, DPoint3dR upperBound, TransformCR transform, DPoint3dCR inLowerBound, DPoint3dCR inUpperBound)
    {
    // Create 8 points from the bounding box and rotate these points to get the required bounding rectangle for the view
    DPoint3d boxPoints[8];
    boxPoints[0].x = inLowerBound.x;
    boxPoints[0].y = inLowerBound.y;
    boxPoints[0].z = inLowerBound.z;

    boxPoints[1].x = inLowerBound.x;
    boxPoints[1].y = inLowerBound.y;
    boxPoints[1].z = inUpperBound.z;

    boxPoints[2].x = inLowerBound.x;
    boxPoints[2].y = inUpperBound.y;
    boxPoints[2].z = inLowerBound.z;

    boxPoints[3].x = inLowerBound.x;
    boxPoints[3].y = inUpperBound.y;
    boxPoints[3].z = inUpperBound.z;

    boxPoints[4].x = inUpperBound.x;
    boxPoints[4].y = inLowerBound.y;
    boxPoints[4].z = inLowerBound.z;

    boxPoints[5].x = inUpperBound.x;
    boxPoints[5].y = inLowerBound.y;
    boxPoints[5].z = inUpperBound.z;

    boxPoints[6].x = inUpperBound.x;
    boxPoints[6].y = inUpperBound.y;
    boxPoints[6].z = inLowerBound.z;

    boxPoints[7].x = inUpperBound.x;
    boxPoints[7].y = inUpperBound.y;
    boxPoints[7].z = inUpperBound.z;

    // Transform points
    bsiTransform_multiplyDPoint3dArrayInPlace(&transform, boxPoints, 8);

    // Find new upper and lower points
    lowerBound.x = boxPoints[0].x;
    lowerBound.y = boxPoints[0].y;
    lowerBound.z = boxPoints[0].z;
    upperBound.x = boxPoints[0].x;
    upperBound.y = boxPoints[0].y;
    upperBound.z = boxPoints[0].z;
    for (int i = 1; i < 8; i++)
        {
        if (boxPoints[i].x < lowerBound.x)
            lowerBound.x = boxPoints[i].x;
        if (boxPoints[i].y < lowerBound.y)
            lowerBound.y = boxPoints[i].y;
        if (boxPoints[i].z < lowerBound.z)
            lowerBound.z = boxPoints[i].z;

        if (boxPoints[i].x > upperBound.x)
            upperBound.x = boxPoints[i].x;
        if (boxPoints[i].y > upperBound.y)
            upperBound.y = boxPoints[i].y;
        if (boxPoints[i].z > upperBound.z)
            upperBound.z = boxPoints[i].z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void PointCloudVortex::GetPointCloudBounds(PtHandle cloudHandle, double* lower, double* upper)
    {
    ptCloudBoundsd(cloudHandle, lower, upper);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PtHandle PointCloudVortex::GetMetaDataHandle(PtHandle cloudHandle)
    {
    return ptGetMetaDataHandle(cloudHandle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool PointCloudVortex::GetMetaTag(PtHandle metadataHandle, WCharCP tagName, WCharP value)
    {
    return ptGetMetaTag(metadataHandle, tagName, value);
    }
