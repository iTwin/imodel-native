/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/handler/MultiResolutionGridMaterialManager.cpp $
|    $RCSfile: HIEMultiResolutionGridMaterialManager.cpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/11/15 17:54:58 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include "StdAfx.h"  // Always include first

#include <TerrainModel\ElementHandler\IMultiResolutionGridMaterialManager.h>

/*----------------------------------------------------------------------------+
|ITexturedElement Method Definition Section - Begin
+----------------------------------------------------------------------------*/
const DRange3d& ITexturedElement::GetRange() const
    {
    return _GetRange();
    }

bool ITexturedElement::DrapePointOnElement(DPoint3d& pointInUors) const
    {
    return _DrapePointOnElement(pointInUors);
    }   
/*----------------------------------------------------------------------------+
|ITexturedElement Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|ICachedMaterial Method Definition Section - Begin
+----------------------------------------------------------------------------*/
StatusInt ICachedMaterial::Initialize(const WString& cachedMaterialInitDataP)
    {
    return _Initialize(cachedMaterialInitDataP);
    }

WString ICachedMaterial::Serialize()
    {
    return _Serialize();
    }

const DRange3d& ICachedMaterial::GetMaterialEffectiveRange()
    {
    return _GetMaterialEffectiveRange();
    }

const MRImageTileId& ICachedMaterial::GetMaterialTileID()
    {
    return _GetMaterialTileID();
    }
        
const Material* ICachedMaterial::GetMaterialProperties()
    {
    return _GetMaterialProperties();
    }

const DRange3d& ICachedMaterial::GetTextureRange()
    {
    return _GetTextureRange();
    }
    
void ICachedMaterial::GetTextureSizeInPixels(uint32_t& textureWidthInPixels, 
                                             uint32_t& textureHeightInPixels)
    {
    _GetTextureSizeInPixels(textureWidthInPixels, textureHeightInPixels);   
    }

SourceID ICachedMaterial::GetTextureSourceID()
    {
    return _GetTextureSourceID();
    }
/*----------------------------------------------------------------------------+
|ICachedMaterial Method Definition Section - End
+----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------+
|IMultiResolutionGridManager Method Definition Section - Begin
+----------------------------------------------------------------------------*/
bool IMultiResolutionGridManager::GetTileExtentInUors(DRange2d&            tileExtent, 
                                                      const MRImageTileId& tileId) const 
    {
    return _GetTileExtentInUors(tileExtent, tileId); 
    }

void IMultiResolutionGridManager::GetVisibleTilesInView(MRImageTileIdVectorR      visibleTiles, 
                                                        ViewContextP              pViewContext, 
                                                        double                    minScreenPixelsPerDrapePixel,
                                                        const ITexturedElementPtr pTextureElement) const
    {
    _GetVisibleTilesInView(visibleTiles, pViewContext, minScreenPixelsPerDrapePixel, pTextureElement);
    }
        
/*----------------------------------------------------------------------------+
|IMultiResolutionGridManager Method Definition Section - End
+----------------------------------------------------------------------------*/ 

/*----------------------------------------------------------------------------+
|IMultiResolutionGridMaterialManager Method Definition Section - Begin
+----------------------------------------------------------------------------*/
void IMultiResolutionGridMaterialManager::ClearCachedMaterials()
    {
    _ClearCachedMaterials();
    }
        
ICachedMaterialPtr IMultiResolutionGridMaterialManager::GetMaterial(const MRImageTileId& gridId, DgnModelRefR modelRef)
    {
    return _GetMaterial(gridId, modelRef);                        
    }

int IMultiResolutionGridMaterialManager::PrepareLookAhead(MRImageTileIdVectorR visibleTiles, DgnModelRefR modelRef) 
    {
    return _PrepareLookAhead(visibleTiles, modelRef);
    }

int IMultiResolutionGridMaterialManager::SetupLookAheadForTile(const MRImageTileId& tileId, DgnModelRefR modelRef) 
    {
    return _SetupLookAheadForTile(tileId, modelRef);
    }

/*----------------------------------------------------------------------------+
|IMultiResolutionGridMaterialManager Method Definition Section - End
+----------------------------------------------------------------------------*/       

/*----------------------------------------------------------------------------+
|IMultiResolutionGridManagerCreator Method Definition Section - Begin
+----------------------------------------------------------------------------*/   
IMultiResolutionGridManagerPtr IMultiResolutionGridManagerCreator::CreateSimpleManager(DRange2d& dataExtentInUors, 
                                                                                       DPoint2d& dataResolutionInUors)
    {
    return _CreateSimpleManager(dataExtentInUors, dataResolutionInUors);
    }

IMultiResolutionGridMaterialManagerPtr IMultiResolutionGridManagerCreator::CreateMaterialManager(SourceID        rasterTextureSourceID,     
                                                                                                 DRange2d&       dataExtentInUors,                                                                                                          
                                                                                                 const DPoint2d& defaultMinPixelSizeInUors,
                                                                                                 double          uorsPerMeter)
    {
    return _CreateMaterialManager(rasterTextureSourceID, dataExtentInUors, defaultMinPixelSizeInUors, uorsPerMeter);
    }
/*----------------------------------------------------------------------------+
|IMultiResolutionGridManagerCreator Method Definition Section - End
+----------------------------------------------------------------------------*/       