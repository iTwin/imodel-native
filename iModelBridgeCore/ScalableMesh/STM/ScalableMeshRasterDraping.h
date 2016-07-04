/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ScalableMeshRasterDraping.h $
|    $RCSfile: ScalableMeshRasterDraping.h,v $
|   $Revision: 1.4 $
|       $Date: 2010/08/19 14:38:33 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
|                                                                        |
|    ScalableMeshRasterDraping.h                       (C) Copyright 2001.        |
|                                                BCIVIL Corporation.        |
|                                                All Rights Reserved.    |
|                                                                       |
+----------------------------------------------------------------------*/
#pragma once


/*----------------------------------------------------------------------+
| Exported classes and definition                                       |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| This template class must be exported, so we instanciate and export it |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Include Imagepp header files
+----------------------------------------------------------------------*/


/*----------------------------------------------------------------------+
| Include MicroStation header files
+----------------------------------------------------------------------*/
#include <qvision/qvision.h>

/*----------------------------------------------------------------------+
| CONSTANT definitions                                                  |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| CLASS definitions                                                     |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
| Core DTM Methods                                                      |
+----------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|Class ScalableMeshRasterDraping
+----------------------------------------------------------------------------*/

class ImageppOpaqueData;

struct TextureDescription
    {
    unsigned char* m_pTextureBitmapData;
    int            m_textureBitmapWidth;
    int            m_textureBitmapHeight;
    double         m_xMinInUors;
    double         m_xMaxInUors;
    double         m_yMinInUors;
    double         m_yMaxInUors;
    };

class ScalableMeshRasterDraping
    {
public:    

    BENTLEY_SM_EXPORT static ScalableMeshRasterDraping* Create(const AString& pi_rFileNameDescriptors);    

    BENTLEY_SM_EXPORT virtual ~ScalableMeshRasterDraping();

    
                    
    BENTLEY_SM_EXPORT void CreateTextureBitmapForCurrentView(vector<TextureDescription>&  rTextures,                                                            
                                                       double&                      textureMinXInUor, 
                                                       double&                      textureMinYInUor, 
                                                       double&                      textureMaxXInUor, 
                                                       double&                      textureMaxYInUor, 
                                                       DMatrix4dCP                  rootToViewMatrix,                                                           
                                                       ViewContextP                 icontext,
                                                       DPoint3d*                         pFencePt,
                                                       int                          nbFencePt);

    BENTLEY_SM_EXPORT void ReleaseLastTextureBitmap();
    
    BENTLEY_SM_EXPORT static void LocateWorldClusterInCurrentModel(double meterPerMasterUnits,
                                                             double masterUnitsPerUor,
                                                             double globalOrgX,
                                                             double globalOrgY);

    const ImageppOpaqueData* GetImageppOpaqueData() const;
            
private:

    void ComputePlaneNormal(DVec3d& rNormalVec,
                            DPoint3d*    pFencePt,
                            int     nbFencePt);

    struct CopyFromAreaInfo
    {       
        vector<DPoint3d> m_clipPoints;
        double           m_xMinInUors;
        double           m_xMaxInUors;
        double           m_yMinInUors;
        double           m_yMaxInUors;
        double           m_zMinInUors;
        double           m_zMaxInUors;
        double           m_surfaceWidthInPixels;
        double           m_surfaceHeightInPixels;
    };

    void ComputeDifferentResAreas(vector<CopyFromAreaInfo>& rCopyFromAreas,
                                  DMatrix4dCP               pRootToViewMatrix,
                                  DMatrix4dCP               pViewToRootMatrix,
                                  DPoint3d&                 dtmPlanePtInView,
                                  DVec3d&                   dtmPlaneNormalInView,
                                  double                    pi_xMin,
                                  double                    pi_xMax,
                                  double                    pi_yMin,
                                  double                    pi_yMax);

    double ComputeMinPixelSizeRequired(double&           po_rMinBitmapSizeInUor,
                                       CopyFromAreaInfo& po_copyFromArea,                      
                                       DMatrix4dCP pi_pViewToRootMatrix,   
                                       double      pi_xMin,
                                       double      pi_xMax,
                                       double      pi_yMin,
                                       double      pi_yMax,   
                                       DPoint3d    dtmPlanePtInView,
                                       DVec3d      dtmPlaneNormalInView);

    void ComputeDestinationExtentFromSourceExtent(double&     po_dstXMin,
                                                  double&     po_dstXMax,
                                                  double&     po_dstYMin,
                                                  double&     po_dstYMax,    
                                                  DMatrix4dCP pi_pSourceToDestinationMatrix, 
                                                  double      pi_srcXMin,
                                                  double      pi_srcXMax,
                                                  double      pi_srcYMin,
                                                  double      pi_srcYMax, 
                                                  double      pi_srcZ, 
                                                  DPoint3d*   pi_pDtmPlanePtInSrc = 0,
                                                  DVec3d*     pi_pDtmPlaneNormalInSrc = 0);
  

//Visible DTM extent driven algorithm - doesn't seems to work because the
//extent might be much outside the view.
#if 0
    void ComputeDifferentResAreasUor(DMatrix4dCP  pi_pRootToViewMatrix,   
                                  double       pi_xMin,
                                  double       pi_xMax,
                                  double       pi_yMin,
                                  double       pi_yMax,                                  
                                  double       pi_zValue);

    double ComputeMinPixelSizeRequiredUor(double&      po_rMaxBitmapSize,
                                       DMatrix4dCP  pi_pRootToViewMatrix,   
                                       double       pi_xMin,
                                       double       pi_xMax,
                                       double       pi_yMin,
                                       double       pi_yMax,                                                                         
                                       double       pi_zValue);
#endif
    
    ScalableMeshRasterDraping(const AString& pi_rFileNameDescriptors);
                               
    //This class is used to hide the existence of Imagepp to the users of the ScalableMeshRasterDraping class.
    ImageppOpaqueData* m_pImageppOpaqueData;        
};