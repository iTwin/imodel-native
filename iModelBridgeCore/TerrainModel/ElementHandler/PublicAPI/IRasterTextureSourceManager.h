/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/IRasterTextureSourceManager.h $
|    $RCSfile: IRasterTextureSourceManager.h,v $
|   $Revision: 1.5 $
|       $Date: 2011/06/01 15:23:56 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

//BEIJING_WIP_DCARTES; I think this should be move in Descartes sources if possible!

#include "IRasterTextureSource.h"

BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//
// Class Declarations
/*--------------------------------------------------------------------------------------+
|class IRasterTextureSourceManager
+--------------------------------------------------------------------------------------*/
struct IRasterTextureSourceManager
    {

    protected : 
    
        virtual IRasterTextureSourcePtr _GetSource(StringList& sourceFileNames) = 0;    
    
        virtual IRasterTextureSourcePtr _GetSource(StringList& sourceFileNames,
                                                   WCharCP cacheFileName) = 0;   
    
        virtual IRasterTextureSourcePtr _GetSource(SourceID sourceID) = 0;  
    
        virtual int _CreatePSSMosaicSource(IRasterTextureSourcePtr& rasterTextureSourcePtr, 
                                           WCharCP                  pssFileName,
                                           bool                     isOnDemand) = 0;

    public : 
        
    /*----------------------------------------------------------------------------------*//**
    * This function returns an opaque pointer to a raster texture source. Note that if no 
    * raster texture source exists for the source raster file(s) a new raster texture source will be 
    * automatically created. Otherwise the existing source is returned.
    *
    * @param    sourceFileNames IN The source file name string, which can be the name of a PSS file 
    *                              or the name of multiple files.    
    * @bsimethod                                                    08/10
    +---------------+---------------+---------------+---------------+---------------+------*/       
    DTMELEMENT_EXPORT IRasterTextureSourcePtr GetSource(StringList& sourceFileNames);    

    /*----------------------------------------------------------------------------------*//**
    * This function returns an opaque pointer to an existing raster texture source. 
    *
    * @param    sourceFileNames IN The name of the source files.    
    * @param    cacheFileName   IN The cache file name.    
    * @bsimethod                                                    03/11
    +---------------+---------------+---------------+---------------+---------------+------*/       
    DTMELEMENT_EXPORT IRasterTextureSourcePtr GetSource(StringList& sourceFileNames,
                                                        WCharCP      cacheFileName);   

    /*----------------------------------------------------------------------------------*//**
    * This function returns an opaque pointer to a raster texture source or 0 if the raster 
    * texture source is not found.     
    *
    * @param    sourceID IN The source ID.
    * @bsimethod                                                    08/10
    +---------------+---------------+---------------+---------------+---------------+------*/  
    DTMELEMENT_EXPORT IRasterTextureSourcePtr GetSource(SourceID sourceID) ;  

    /*----------------------------------------------------------------------------------*//**
    * This function create a PSS defined mosaic raster texture source.  
    *
    * @param    rasterTextureSourcePtr OUT The raster texture source created
    * @param    pssFileName            IN  The PSS file name.
    * @param    isOnDemand             IN  If the mosaic is on demand.
    * @bsimethod                                                    03/11
    +---------------+---------------+---------------+---------------+---------------+------*/  
    DTMELEMENT_EXPORT int CreatePSSMosaicSource(IRasterTextureSourcePtr& rasterTextureSourcePtr, 
                                                WCharCP                  pssFileName,
                                                bool                     isOnDemand);
    }; 

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE



