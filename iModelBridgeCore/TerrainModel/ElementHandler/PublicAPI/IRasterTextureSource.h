/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/IRasterTextureSource.h $
|    $RCSfile: IRasterTextureSource.h,v $
|   $Revision: 1.13 $
|       $Date: 2012/01/20 23:30:17 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

//BEIJING_WIP_DCARTES; I think this should be move in Descartes sources if possible!

#include <RasterCore\DgnRaster.h>
#include <DgnGeoCoord/DgnGeoCoord.h>


BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

//
// Class Declarations

typedef UInt64 SourceID;

/*--------------------------------------------------------------------------------------+
|class IRasterTextureSourceCacheFile
|      This interface is supported by any kind of complex raster texture source 
|      (e.g. : source involving multiple raster files) who supports caching data 
|      (e.g. : lowest resolution data) in a cache file. 
+--------------------------------------------------------------------------------------*/  
struct IRasterTextureSourceCacheFile : RefCounted <IRefCounted>
    {
    public : 

        //Decimation method could be added but the enumeration items must not be 
        //changed/removed or their values modified since those values are stored 
        //the Scalable Terrain Model element.
        enum DecimationMethod
            {
            NOT_SPECIFIED_DECIMATION = 0,
            NEAREST_NEIGHBOR_DECIMATION,
            AVERAGE_DECIMATION
            };

    protected : 

        virtual int  _Create(Bentley::WString& fileName, 
                             DecimationMethod  decimationMethod) = 0;

        virtual int  _GetDecimationMethod(DecimationMethod& decimationMethod) const = 0;        

        virtual bool _IsPresent() const = 0;

        virtual bool _IsUpToDate() const = 0;

        virtual int  _Set(Bentley::WString& fileName) = 0;
        
    public : 
               
        DTMELEMENT_EXPORT int  Create(Bentley::WString& fileName, 
                                      DecimationMethod  decimationMethod = NEAREST_NEIGHBOR_DECIMATION);

        DTMELEMENT_EXPORT int  GetDecimationMethod(DecimationMethod& decimationMethod) const;        

        DTMELEMENT_EXPORT bool IsPresent() const;

        DTMELEMENT_EXPORT bool IsUpToDate() const;        

        DTMELEMENT_EXPORT int  Set(Bentley::WString& fileName);
    };

typedef RefCountedPtr<IRasterTextureSourceCacheFile> IRasterTextureSourceCacheFilePtr;


struct IRasterTextureSourceLookAhead : RefCounted <IRefCounted>
    {
    protected : 
                
        virtual int _RequestLookAhead(const DPoint2d* lookAheadShapePtsInMeters, int nbPoints, double resXInMetersPerPixel, double resYInMetersPerPixel) = 0;                    

                
    public : 
       
        DTMELEMENT_EXPORT int RequestLookAhead(const DPoint2d* lookAheadShapePtsInMeters, int nbPoints, double resXInMetersPerPixel, double resYInMetersPerPixel);        
    };

typedef RefCountedPtr<IRasterTextureSourceLookAhead> IRasterTextureSourceLookAheadPtr;


struct IRasterForTexture : RefCounted <IRefCounted>
    {
    protected : 
                
        virtual int _GetURL(Bentley::WString& url) const = 0;           

    public : 
       
        DTMELEMENT_EXPORT int GetURL(Bentley::WString& url) const;           
    };

typedef RefCountedPtr<IRasterForTexture> IRasterForTexturePtr;


struct IMosaicTexture : RefCounted <IRefCounted>
    {
    protected : 

        virtual int _GetNbRasters() const = 0;          
        
        virtual int _GetFirstRasterForTexture(IRasterForTexturePtr& rasterForTexturePtr) = 0;

        virtual int _GetNextRasterForTexture(IRasterForTexturePtr& rasterForTexturePtr) = 0;

        virtual int _Add(Bentley::DgnPlatform::Raster::DgnRaster* dgnRasterP, ViewInfoCP viP, Bentley::GeoCoordinates::DgnGCSPtr& pDestinationGCS) = 0;

        virtual int _Add(IRasterForTexturePtr& rasterForTexturePtr) = 0;

        virtual int _Clear() = 0;

        virtual bool _IsDirty() const = 0;

        virtual int _Update() = 0;       

        virtual int _Update(const DRange3d* pEffectiveRangeInDestGCS) = 0;       

    public :                 

        DTMELEMENT_EXPORT int GetNbRasters() const;  

        DTMELEMENT_EXPORT int GetFirstRasterForTexture(IRasterForTexturePtr& rasterForTexturePtr);

        DTMELEMENT_EXPORT int GetNextRasterForTexture(IRasterForTexturePtr& rasterForTexturePtr);

        DTMELEMENT_EXPORT int Add(Bentley::DgnPlatform::Raster::DgnRaster* dgnRasterP, ViewInfoCP viP, Bentley::GeoCoordinates::DgnGCSPtr& pDestinationGCS);

        DTMELEMENT_EXPORT int Add(IRasterForTexturePtr& rasterForTexturePtr);

        DTMELEMENT_EXPORT int Clear();

        DTMELEMENT_EXPORT bool IsDirty() const;

        DTMELEMENT_EXPORT int Update();       

        DTMELEMENT_EXPORT int Update(const DRange3d* pEffectiveRangeInDestGCS);       
 
    };

typedef RefCountedPtr<IMosaicTexture> IMosaicTexturePtr;


struct IRasterTextureSource : RefCounted <IRefCounted>
    {
    protected :                         

        virtual const StringList*                _GetSourceFileNames() const = 0;           

        virtual bool                             _IsSourceForFileNames(StringList* fileNamesP) const = 0;    
                                                                 
        virtual int                              _GetMinPixelSizeInUors(double& xSize,
                                                                        double& ySize, 
                                                                        double  uorsPerMeter) const = 0;           
        
        virtual void                             _GetSourceExtentInUors(DRange2d& sourceExtent) const = 0;           
        
        virtual SourceID                         _GetSourceID() const = 0;
        
        virtual StatusInt                        _GetTexture(byte*            pixelBufferP,                 
                                                             unsigned __int64 pixelBufferSizeInBytes,   
                                                             unsigned int     textureWidthInPixels,
                                                             unsigned int     textureHeightInPixels,                                            
                                                             DRange2d&        textureExtentInUors) const = 0;                       
               
        virtual bool                             _GetReprojectionGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                                                     Bentley::GeoCoordinates::BaseGCSPtr& destinationGCSPtr) = 0;

        virtual StatusInt                        _SetReprojectionGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                                                     Bentley::GeoCoordinates::BaseGCSPtr& destinationGCSPtr) = 0;        

        //Optional interfaces
        virtual IRasterTextureSourceCacheFilePtr _GetCacheFileInterface() = 0;

        virtual IRasterTextureSourceLookAheadPtr _GetLookAheadInterface() = 0;
        
        virtual IMosaicTexturePtr                _GetRasterTextureMosaicInterface() = 0;        


    public : 

        DTMELEMENT_EXPORT const StringList*   GetSourceFileNames();           

        DTMELEMENT_EXPORT bool                IsSourceForFileNames(StringList* fileNamesP);    
        
        DTMELEMENT_EXPORT int                 GetMinPixelSizeInUors(double& xSize, 
                                                                    double& ySize, 
                                                                    double  uorsPerMeter);                   
        
        DTMELEMENT_EXPORT void                GetSourceExtentInUors(DRange2d& sourceExtent);           
        
        DTMELEMENT_EXPORT SourceID            GetSourceID();
        
        DTMELEMENT_EXPORT StatusInt           GetTexture(byte*            pixelBufferP,                 
                                                         unsigned __int64 pixelBufferSizeInBytes,   
                                                         unsigned int     textureWidthInPixels,
                                                         unsigned int     textureHeightInPixels,                                            
                                                         DRange2d&        textureExtentInUors) const;                       
        
        DTMELEMENT_EXPORT bool                GetReprojectionGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                                                 Bentley::GeoCoordinates::BaseGCSPtr& destinationGCSPtr);

        DTMELEMENT_EXPORT StatusInt           SetReprojectionGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                                                 Bentley::GeoCoordinates::BaseGCSPtr& destinationGCSPtr);

        //Optional interfaces (i.e. : can be null).
        DTMELEMENT_EXPORT IRasterTextureSourceCacheFilePtr GetCacheFileInterface();

        DTMELEMENT_EXPORT IRasterTextureSourceLookAheadPtr GetLookAheadInterface();

        DTMELEMENT_EXPORT IMosaicTexturePtr                GetRasterTextureMosaicInterface();

    }; 

typedef RefCountedPtr<IRasterTextureSource> IRasterTextureSourcePtr;

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE


BEGIN_EXTERN_C


END_EXTERN_C
