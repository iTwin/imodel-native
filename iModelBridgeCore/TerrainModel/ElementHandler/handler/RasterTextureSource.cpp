/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/RasterTextureSource.cpp $
|    $RCSfile: HIEMultiResolutionGridMaterialManager.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/11/15 17:54:58 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include "StdAfx.h"  // Always include first

#include <TerrainModel\ElementHandler\IRasterTextureSource.h>

/*----------------------------------------------------------------------------+
|IRasterTextureSourceLookAhead Method Definition Section - Begin
+----------------------------------------------------------------------------*/  
int IRasterTextureSourceLookAhead::RequestLookAhead(const DPoint2d* lookAheadShapePtsInMeters, int nbPoints, double resXInMetersPerPixel, double resYInMetersPerPixel) 

    {
    return _RequestLookAhead(lookAheadShapePtsInMeters, nbPoints, resXInMetersPerPixel, resYInMetersPerPixel);
    }
/*----------------------------------------------------------------------------+
|IRasterTextureSourceLookAhead Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|IRasterForTexture Method Definition Section - Begin
+----------------------------------------------------------------------------*/
int IRasterForTexture::GetURL(WString& url) const
    {
    return _GetURL(url);
    }
/*----------------------------------------------------------------------------+
|IRasterForTexture Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|IMosaicTexture Method Definition Section - Begin
+----------------------------------------------------------------------------*/
int IMosaicTexture::GetNbRasters() const
    {
    return _GetNbRasters();
    }

int IMosaicTexture::GetFirstRasterForTexture(IRasterForTexturePtr& rasterForTexturePtr)
    {
    return _GetFirstRasterForTexture(rasterForTexturePtr);
    }

int IMosaicTexture::GetNextRasterForTexture(IRasterForTexturePtr& rasterForTexturePtr)
    {
    return _GetNextRasterForTexture(rasterForTexturePtr);
    }

int IMosaicTexture::Add(DgnRasterP dgnRasterP, ViewInfoCP viP, Bentley::GeoCoordinates::DgnGCSPtr& pDestinationGCS) 
    {
    return _Add(dgnRasterP, viP, pDestinationGCS);
    }

int IMosaicTexture::Add(IRasterForTexturePtr& rasterForTexturePtr) 
    {
    return _Add(rasterForTexturePtr);
    }

int IMosaicTexture::Clear()
    {
    return _Clear();
    }

bool IMosaicTexture::IsDirty() const
    {
    return _IsDirty();
    }

int IMosaicTexture::Update()
    {
    return _Update();
    }

int IMosaicTexture::Update(const DRange3d* pEffectiveRangeInDestGCS)
    {
    return _Update(pEffectiveRangeInDestGCS);
    }

/*----------------------------------------------------------------------------+
|IMosaicTexture Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|IRasterTextureSourceCacheFile Method Definition Section - Begin
+----------------------------------------------------------------------------*/
int IRasterTextureSourceCacheFile::Create(WString&                                        fileName, 
                                          IRasterTextureSourceCacheFile::DecimationMethod decimationMethod)
    {
    return _Create(fileName, decimationMethod);
    } 

int IRasterTextureSourceCacheFile::GetDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod& decimationMethod) const
    {
    return _GetDecimationMethod(decimationMethod);
    }
 
bool IRasterTextureSourceCacheFile::IsPresent() const
    {
    return _IsPresent();
    }

bool IRasterTextureSourceCacheFile::IsUpToDate() const
    {
    return _IsUpToDate();
    }

int IRasterTextureSourceCacheFile::Set(Bentley::WString& fileName)
    {
    return _Set(fileName);
    }
/*----------------------------------------------------------------------------+
|IRasterTextureSourceCacheFile Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|IRasterTextureSource Method Definition Section - Begin
+----------------------------------------------------------------------------*/
const StringList* IRasterTextureSource::GetSourceFileNames()
    {
    return _GetSourceFileNames();           
    }

bool IRasterTextureSource::IsSourceForFileNames(StringList* fileNamesP)
    {
    return _IsSourceForFileNames(fileNamesP);    
    }
                       
int IRasterTextureSource::GetMinPixelSizeInUors(double& xSize, double& ySize, double uorsPerMeter)
    {
    return _GetMinPixelSizeInUors(xSize, ySize, uorsPerMeter);
    }
        
void IRasterTextureSource::GetSourceExtentInUors(DRange2d& sourceExtent)
    {
    _GetSourceExtentInUors(sourceExtent);
    }
        
SourceID IRasterTextureSource::GetSourceID()
    {
    return _GetSourceID();
    }
        
StatusInt IRasterTextureSource::GetTexture(byte*            pixelBufferP,                 
                                           unsigned __int64 pixelBufferSizeInBytes,   
                                           unsigned int     textureWidthInPixels,
                                           unsigned int     textureHeightInPixels,                                            
                                           DRange2d&        textureExtentInUors) const
    {
    return _GetTexture(pixelBufferP, pixelBufferSizeInBytes, textureWidthInPixels, textureHeightInPixels, textureExtentInUors);
    }

IRasterTextureSourceCacheFilePtr IRasterTextureSource::GetCacheFileInterface()
    {
    return _GetCacheFileInterface();
    }

IRasterTextureSourceLookAheadPtr IRasterTextureSource::GetLookAheadInterface()
    {
    return _GetLookAheadInterface();
    }
    
IMosaicTexturePtr IRasterTextureSource::GetRasterTextureMosaicInterface()

    {
    return _GetRasterTextureMosaicInterface();
    }

bool IRasterTextureSource::GetReprojectionGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                              Bentley::GeoCoordinates::BaseGCSPtr& destinationGCSPtr)
    {
    return _GetReprojectionGCS(sourceGCSPtr, destinationGCSPtr);
    }

StatusInt IRasterTextureSource::SetReprojectionGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                                   Bentley::GeoCoordinates::BaseGCSPtr& destinationGCSPtr)
    {
    return _SetReprojectionGCS(sourceGCSPtr, destinationGCSPtr);    
    }
    
/*----------------------------------------------------------------------------+
|IRasterTextureSource Method Definition Section - End
+----------------------------------------------------------------------------*/  