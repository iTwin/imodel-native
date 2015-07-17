/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementHandler/PublicAPI/IMrDTMDataRef.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
/*__PUBLISH_SECTION_START__*/
#pragma once
/*__PUBLISH_SECTION_END__*/

#include <DgnPlatform/DgnDocumentManager.h>

#include <TerrainModel\ElementHandler\IRasterTextureSource.h>
#include <TerrainModel\ElementHandler\IMultiResolutionGridMaterialManager.h>

#include <DgnPlatform/XAttributeHandler.h>



/*__PUBLISH_SECTION_START__*/

//#include <TerrainModel\ElementHandler\DTMDataRef.h>

/*__PUBLISH_SECTION_END__*/

#include <DgnGeoCoord/DgnGeoCoord.h>

//#include "DTMDataRefCachingManager.h"
#include <ScalableTerrainModel/IMrDTM.h>
#include <ScalableTerrainModel/IMrDTMCreator.h>
#include <ScalableTerrainModel/MrDTMDefs.h>

#include <ScalableTerrainModel/IMrDTMClipContainer.h>
//#include "MrDTMXAttributeHandler.h"

//#include "DTMDisplayUtils.h"
#include <TerrainModel\ElementHandler\IMrDTMProgressiveDisplay.h>


USING_NAMESPACE_RASTER

/*__PUBLISH_SECTION_START__*/
BEGIN_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE

USING_NAMESPACE_BENTLEY_MRDTM

enum MrDTMStatus
    {   
    MRDTMSTATUS_FILE_LOADED = 0,

    MRDTMSTATUS_ERROR_BEGIN = 1000000,

    MRDTMSTATUS_INVALID_FILE,
    MRDTMSTATUS_MISSING_FILE,
    MRDTMSTATUS_UNSUPPORTED_VERSION,

    MRDTMSTATUS_ERROR_END = 1999999,
    MRDTMSTATUS_WARNING_BEGIN,
    MRDTMSTATUS_MISSING_RASTER_DRAPING_INFO,                
    MRDTMSTATUS_MISSING_RASTER_DRAPING_FILE,                
    MRDTMSTATUS_WARNING_END = 2999999
    };

//Those equations map the range of possible values in the GUI (i.e. : [1 - 100]) to 
//a density range which will allow the user to browse six resolution levels for a given view.
#define MRDTM_VIEW_POINT_DENSITY_TO_GUI(viewPointDensity) (-14.32 * log(viewPointDensity) + 99.81)
#define MRDTM_GUI_TO_VIEW_POINT_DENSITY(guiValue) exp((guiValue - 99.81) / -14.32)
 
typedef void (*FPNotifyChange)(ElementHandleCR);                        

class IPlotterInfo
    {
    protected :

        virtual DPoint2d _GetPrinterResolutionInInches () const = 0;        

    public : 
        
        DTMELEMENT_EXPORT DPoint2d GetPrinterResolutionInInches () const;        
    };

class IMrDTMDataRef : public DTMDataRef
    {       

    protected : 

        virtual int _GetOverviewDTM(Bentley::TerrainModel::DTMPtr& overviewDTMPtr) = 0;


    
        virtual int         _GetProgressiveDataBlockManager(IMrDTMProgressiveDataBlockManagerPtr& progressiveDataBlockManagerPtr,
                                                                          BcDTMP                               dtmToLoadProgressively, 
                                                                          ViewContextP                         context, 
                                                                          bool                                  isDrapingRequired) = 0;   

         //Creation and data source modification functions
        virtual int            _GetIMrDTMCreator(Bentley::MrDTM::IMrDTMCreatorPtr& iMrDTMCreatorPtr) = 0;          

        virtual int            _SaveIMrDTMCreatorToFile() = 0;

        //Information functions  
        virtual const WString& _GetDescription() const = 0;          

        virtual int            _SetDescription(const WString& description) = 0;         
    
        virtual const DgnDocumentPtr       _GetMrDTMDocument() const = 0;          

        virtual int                        _SetMrDTMDocument(DgnDocumentPtr& mrdtmDocumentPtr) = 0;         
                   
        virtual const DgnDocumentMonikerPtr _GetFileMoniker() const = 0;          
    /*           
        virtual int                        _SetFileMoniker(DgnDocumentMonikerPtr& fileMonikerPtr) = 0;         
      */                                      
        virtual bool           _CanLocate() const = 0;          

        virtual int            _SetLocate(bool canLocate) = 0;     

        virtual bool           _IsAnchored() const = 0;

        virtual int            _SetAnchored(bool isAnchored) = 0;

        virtual MrDTMStatus    _GetIDTMFileLastLoadStatus() = 0;         
             
        virtual bool           _IsOpenInReadOnly() = 0;

        virtual int            _SetReadOnly(bool isReadOnly) = 0;

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetGeoCS() = 0;         

        virtual int                                 _SetGeoCS(const Bentley::GeoCoordinates::BaseGCSPtr& baseGCSPtr) = 0;         

        virtual int                                 _GetExtentInGeoCS(DRange3d& extentInGeoCS, bool inUors) = 0;
            
        //Display control function    

        virtual int            _GetEdgeMethod(int& edgeMethod) = 0;

        virtual double         _GetEdgeMethodLengthInStorage() = 0;

        virtual int            _GetEdgeMethodLength(double& edgeMethodLength) = 0;

        virtual bool           _GetClipActivation() const = 0;

        virtual int            _SetClipActivation(bool isActivated) = 0;              

        virtual int            _GetNbClips() const = 0;

        virtual int            _GetClip(DPoint3d*& clipPointsP,
                                        int&       numberOfPoints, 
                                        bool&      isClipMask,
                                        int        clipInd) const = 0;

        virtual int            _AddClip(DPoint3d* clipPointsP,
                                        int       numberOfPoints, 
                                        bool      isClipMask) = 0;    

        virtual int            _RemoveClip(int toRemoveClipInd) = 0;

        virtual int            _RemoveAllClip() = 0;        
    
        virtual double         _GetPointDensityForShadedView() = 0;                        

        virtual int            _SetPointDensityForShadedView(double pi_minScreenPixelsPerPoint) = 0;
    
        virtual double         _GetPointDensityForWireframeView() = 0;                        

        virtual int            _SetPointDensityForWireframeView(double pi_minScreenPixelsPerPoint) = 0;
         
        //High resolution raster texturing     
        virtual int            _ClearCachedMaterials() = 0;

        virtual int            _GetRasterTextureCacheFilePath(WString& cacheFilePath) = 0;

        virtual int            _GetRasterTexturePSSFilePath(WString& pssFilePath) = 0;
                 
        virtual IRasterTextureSourcePtr _GetRasterTextureSource() = 0;

        virtual int                                            _SetRasterTextureSource(IRasterTextureSourcePtr& rasterTextureSourcePtr) = 0;

        virtual IMultiResolutionGridMaterialManagerPtr _GetMultiResGridMaterialManager() = 0;
    
        virtual bool           _IsVisibleForView(short viewNumber) = 0;

        virtual int            _SetVisibilityForView(short viewNumber, bool isVisible) = 0;
   
        //Synchronization functions

        //-	State (empty, dirty, up-to-date + time stamp)                        
            
        virtual int            _GetMrDTMState() = 0;                        

        virtual bool           _GetLastUpToDateCheckTime(time_t& last) = 0;
    
        virtual int            _Generate() = 0;   
            
        virtual int            _GenerateRasterTextureCache() = 0;   
    
        virtual int            _GetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod& decimationMethod) const = 0;   

        virtual int            _SetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod decimationMethod) = 0;   

        virtual int            _IsRasterTextureCacheFileUpToDate(bool& isUpToDate, Element::EditElementHandle* elmHandleP = 0) = 0;  
    
        virtual bool           _IsUpToDate() = 0;       

        virtual Int64          _GetPointCountForResolution(int resolutionIndex) = 0;     

        virtual DgnGCSP        _GetDestinationGCS() = 0;

    public : 
    
    DTMELEMENT_EXPORT static bool IsValidMrDTMElement(ElementHandleCR el);
        
    DTMELEMENT_EXPORT static IMrDTMProgressiveDisplayPtr GetMrDTMProgressiveDisplayInterface();   

    DTMELEMENT_EXPORT static void SetMrDTMProgressiveDisplayInterface(IMrDTMProgressiveDisplayPtr mrDtmProgressiveDisplayPtr);   

                      static IPlotterInfo* GetPlotterInterface();    

    DTMELEMENT_EXPORT static void SetPlotterInterface(IPlotterInfo* plotterInterface);   

    DTMELEMENT_EXPORT int         GetOverviewDTM(Bentley::TerrainModel::DTMPtr& overviewDTMPtr);   


    DTMELEMENT_EXPORT int         GetProgressiveDataBlockManager(IMrDTMProgressiveDataBlockManagerPtr& progressiveDataBlockManagerPtr,
                                                                      BcDTMP                               dtmToLoadProgressively, 
                                                                      ViewContextP                         context, 
                                                                      bool                                  isDrapingRequired);   

    //Notification of change
    DTMELEMENT_EXPORT static int AddChangeListener(FPNotifyChange fpChangeLister);
    DTMELEMENT_EXPORT static int RemoveChangeListener(FPNotifyChange fpChangeLister);
    
    //Creation and data source modification functions
    DTMELEMENT_EXPORT int            GetIMrDTMCreator(Bentley::MrDTM::IMrDTMCreatorPtr& iMrDTMCreatorPtr);          

    DTMELEMENT_EXPORT int            SaveIMrDTMCreatorToFile();

    //Information functions  
    DTMELEMENT_EXPORT const WString& GetDescription() const;          

    DTMELEMENT_EXPORT int            SetDescription(const WString& description);         
    
    DTMELEMENT_EXPORT const DgnDocumentPtr        GetMrDTMDocument() const;          

    DTMELEMENT_EXPORT int                        SetMrDTMDocument(DgnDocumentPtr& mrdtmDocumentPtr);         
                   
    DTMELEMENT_EXPORT const DgnDocumentMonikerPtr GetFileMoniker() const;          
/*           
    DTMELEMENT_EXPORT int                        SetFileMoniker(DgnDocumentMonikerPtr& fileMonikerPtr);         
  */                                      
    DTMELEMENT_EXPORT bool           CanLocate() const;          

    DTMELEMENT_EXPORT int            SetLocate(bool canLocate);     

    DTMELEMENT_EXPORT bool           IsAnchored() const;

    DTMELEMENT_EXPORT int            SetAnchored(bool isAnchored);

    DTMELEMENT_EXPORT MrDTMStatus    GetIDTMFileLastLoadStatus();         
             
    DTMELEMENT_EXPORT bool           IsOpenInReadOnly();

    DTMELEMENT_EXPORT int            SetReadOnly(bool isReadOnly);

    DTMELEMENT_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetGeoCS();         

    DTMELEMENT_EXPORT int                                 SetGeoCS(const Bentley::GeoCoordinates::BaseGCSPtr& baseGCSPtr);         

    DTMELEMENT_EXPORT int                                 GetExtentInGeoCS(DRange3d& extentInGeoCS, bool inUors);
            
    //Display control function    

    DTMELEMENT_EXPORT int            GetEdgeMethod(int& edgeMethod);

    DTMELEMENT_EXPORT double         GetEdgeMethodLengthInStorage();

    DTMELEMENT_EXPORT int            GetEdgeMethodLength(double& edgeMethodLength);

    DTMELEMENT_EXPORT bool           GetClipActivation() const;

    DTMELEMENT_EXPORT int            SetClipActivation(bool isActivated);              

    DTMELEMENT_EXPORT int            GetNbClips() const;

    DTMELEMENT_EXPORT int            GetClip(DPoint3d*& clipPointsP,
                                                  int&       numberOfPoints, 
                                                  bool&      isClipMask,
                                                  int        clipInd) const;

    DTMELEMENT_EXPORT int            AddClip(DPoint3d* clipPointsP,
                                                  int       numberOfPoints, 
                                                  bool      isClipMask);    

    DTMELEMENT_EXPORT int            RemoveClip(int toRemoveClipInd);

    DTMELEMENT_EXPORT int            RemoveAllClip();        
    
    DTMELEMENT_EXPORT double         GetPointDensityForShadedView();                        

    DTMELEMENT_EXPORT int            SetPointDensityForShadedView(double pi_minScreenPixelsPerPoint);
    
    DTMELEMENT_EXPORT double         GetPointDensityForWireframeView();                        

    DTMELEMENT_EXPORT int            SetPointDensityForWireframeView(double pi_minScreenPixelsPerPoint);
         
    //High resolution raster texturing     
    DTMELEMENT_EXPORT int            ClearCachedMaterials();

    DTMELEMENT_EXPORT int            GetRasterTextureCacheFilePath(WString& cacheFilePath);

    DTMELEMENT_EXPORT int            GetRasterTexturePSSFilePath(WString& pssFilePath);
             
    DTMELEMENT_EXPORT IRasterTextureSourcePtr GetRasterTextureSource();

    DTMELEMENT_EXPORT int                                            SetRasterTextureSource(IRasterTextureSourcePtr& rasterTextureSourcePtr);

    DTMELEMENT_EXPORT IMultiResolutionGridMaterialManagerPtr GetMultiResGridMaterialManager();

    DTMELEMENT_EXPORT bool           IsVisibleForView(short viewNumber);

    DTMELEMENT_EXPORT int            SetVisibilityForView(short viewNumber, bool isVisible);
   
    //Synchronization functions

    //-	State (empty, dirty, up-to-date + time stamp)                        
            
    DTMELEMENT_EXPORT int            GetMrDTMState();                        

    DTMELEMENT_EXPORT bool           GetLastUpToDateCheckTime(time_t& last);
    
    DTMELEMENT_EXPORT int            Generate();   
            
    DTMELEMENT_EXPORT int            GenerateRasterTextureCache();   

    DTMELEMENT_EXPORT int            GetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod& decimationMethod) const;   

    DTMELEMENT_EXPORT int            SetRasterTextureCacheDecimationMethod(IRasterTextureSourceCacheFile::DecimationMethod decimationMethod);   

    DTMELEMENT_EXPORT int            IsRasterTextureCacheFileUpToDate(bool& isUpToDate, Element::EditElementHandle* elmHandleP = 0);  

    DTMELEMENT_EXPORT bool           IsUpToDate();       

    DTMELEMENT_EXPORT Int64          GetPointCountForResolution(int resolutionIndex);                                

    DTMELEMENT_EXPORT DgnGCSP        GetDestinationGCS();

    };

END_BENTLEY_TERRAINMODEL_ELEMENT_NAMESPACE