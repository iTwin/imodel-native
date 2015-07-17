/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMQuery.h $
|    $RCSfile: IMrDTMQuery.h,v $
|   $Revision: 1.17 $
|       $Date: 2012/11/29 17:30:53 $
|     $Author: Mathieu.St-Pierre $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

//#include <GeoCoord/BaseGeoCoord.h>
#include <TerrainModel/TerrainModel.h>
#include <TerrainModel/Core/IDTM.h>
#include <ScalableTerrainModel/MrDTMDefs.h>
#include <ScalableTerrainModel/IMrDTMFeature.h>
//#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

struct IMrDTMQuery;
struct IMrDTMQueryParameters;
struct IMrDTMFixResolutionIndexQueryParams;
struct IMrDTMFixResolutionMaxPointsQueryParams;
struct IMrDTMFullResolutionQueryParams;
struct IMrDTMFullResolutionLinearQueryParams;
struct IMrDTMViewDependentQueryParams;
struct IMrDTMQueryAllLinearsQueryParams;

typedef RefCountedPtr<IMrDTMQuery>                            IMrDTMQueryPtr;
typedef RefCountedPtr<IMrDTMQueryParameters>                  IMrDTMQueryParametersPtr;
typedef RefCountedPtr<IMrDTMFixResolutionIndexQueryParams>     IMrDTMFixResolutionIndexQueryParamsPtr;
typedef RefCountedPtr<IMrDTMFixResolutionMaxPointsQueryParams> IMrDTMFixResolutionMaxPointsQueryParamsPtr;
typedef RefCountedPtr<IMrDTMFullResolutionQueryParams>         IMrDTMFullResolutionQueryParamsPtr;
typedef RefCountedPtr<IMrDTMFullResolutionLinearQueryParams>   IMrDTMFullResolutionLinearQueryParamsPtr;
typedef RefCountedPtr<IMrDTMViewDependentQueryParams>          IMrDTMViewDependentQueryParamsPtr;
typedef RefCountedPtr<IMrDTMQueryAllLinearsQueryParams>        IMrDTMQueryAllLinearsQueryParamsPtr;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IMrDTMQueryParameters abstract: virtual public RefCountedBase
    {            
    protected: 
        
                 IMrDTMQueryParameters();                    
        virtual ~IMrDTMQueryParameters();

        virtual bool _GetTriangulationState() = 0;        
        virtual void _SetTriangulationState(bool pi_isReturnedDataTriangulated) = 0;                

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetSourceGCS() = 0;

        virtual Bentley::GeoCoordinates::BaseGCSPtr _GetTargetGCS() = 0;

        virtual void _SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                             Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr) = 0;

        virtual long _GetEdgeOptionTriangulationParam() = 0;            

        virtual double _GetMaxSideLengthTriangulationParam() = 0;
                            
        virtual void _SetEdgeOptionTriangulationParam(long edgeOption) = 0;            

        virtual void _SetMaxSideLengthTriangulationParam(double maxSideLength) = 0;
                               
    public:    
            
        BENTLEYSTM_EXPORT bool GetTriangulationState();        
        BENTLEYSTM_EXPORT void SetTriangulationState(bool pi_isReturnedDataTriangulated);                


        BENTLEYSTM_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetSourceGCS();
        BENTLEYSTM_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetTargetGCS();

        BENTLEYSTM_EXPORT void SetGCS(Bentley::GeoCoordinates::BaseGCSPtr& sourceGCSPtr, 
                                      Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr);
                
        BENTLEYSTM_EXPORT long GetEdgeOptionTriangulationParam();
            
        BENTLEYSTM_EXPORT double GetMaxSideLengthTriangulationParam();
            
        BENTLEYSTM_EXPORT void SetEdgeOptionTriangulationParam(long edgeOption);
            
        BENTLEYSTM_EXPORT void SetMaxSideLengthTriangulationParam(double maxSideLength);
            
        BENTLEYSTM_EXPORT static double GetToleranceTriangulationParam();
    };

struct IMrDTMFullResolutionQueryParams : public virtual IMrDTMQueryParameters
    {    

    protected :

        IMrDTMFullResolutionQueryParams();
        virtual ~IMrDTMFullResolutionQueryParams();
                
        virtual size_t _GetMaximumNumberOfPoints() = 0;            

        virtual void _SetMaximumNumberOfPoints(size_t maximumNumberOfPoints) = 0;            
        
        virtual bool _GetReturnAllPtsForLowestLevel() = 0;

        virtual void _SetReturnAllPtsForLowestLevel(bool returnAllPts) = 0;                               

    
    public : 
               
        BENTLEYSTM_EXPORT static IMrDTMFullResolutionQueryParamsPtr CreateParams();  
        
        BENTLEYSTM_EXPORT size_t GetMaximumNumberOfPoints();            

        BENTLEYSTM_EXPORT void SetMaximumNumberOfPoints(size_t maximumNumberOfPoints);            
        
        BENTLEYSTM_EXPORT bool GetReturnAllPtsForLowestLevel();

        BENTLEYSTM_EXPORT void SetReturnAllPtsForLowestLevel(bool returnAllPts);                               
    };

struct IMrDTMFullResolutionLinearQueryParams : public virtual IMrDTMFullResolutionQueryParams
    {                 
    protected :

        IMrDTMFullResolutionLinearQueryParams();
        virtual ~IMrDTMFullResolutionLinearQueryParams();


        virtual size_t _GetMaximumNumberOfPointsForLinear() = 0;            

        virtual int _SetMaximumNumberOfPointsForLinear(size_t maximumNumberOfPointsForLinear) = 0;                   
                
        virtual void _SetUseDecimation(bool useDecimation) = 0;
           
        virtual bool _GetUseDecimation() = 0;
        
        virtual void _SetCutLinears(bool cutLinears) = 0;

        virtual bool _GetCutLinears() = 0;        

        virtual void _SetAddLinears(const bool addLinears) = 0;        

        virtual bool _GetAddLinears() = 0;        

        virtual const std::vector<int>& _GetFilteringFeatureTypes(bool& doIncludeFilteringFeatureTypes) = 0;      

        //When no feature type is specified all feature types are returned.
        virtual int                     _SetFilteringFeatureTypes(const std::vector<int>& filteringFeatureTypes, bool doIncludeFilteringFeatures) = 0;      

        virtual void                    _SetIncludeFilteringFeatureTypes(const bool& doIncludeFilteringFeatures) = 0;         
       
    public : 

        BENTLEYSTM_EXPORT static IMrDTMFullResolutionLinearQueryParamsPtr CreateParams();    

        BENTLEYSTM_EXPORT size_t GetMaximumNumberOfPointsForLinear();            

        BENTLEYSTM_EXPORT int SetMaximumNumberOfPointsForLinear(size_t maximumNumberOfPointsForLinear);                   

        BENTLEYSTM_EXPORT void SetUseDecimation(bool useDecimation);
           
        BENTLEYSTM_EXPORT bool GetUseDecimation();
        
        BENTLEYSTM_EXPORT void SetCutLinears(bool cutLinears);

        BENTLEYSTM_EXPORT bool GetCutLinears();        

        BENTLEYSTM_EXPORT void SetAddLinears(const bool addLinears);        

        BENTLEYSTM_EXPORT bool GetAddLinears();        

        BENTLEYSTM_EXPORT const std::vector<int>& GetFilteringFeatureTypes(bool& doIncludeFilteringFeatureTypes);      

        //When no feature type is specified all feature types are returned.
        BENTLEYSTM_EXPORT int                     SetFilteringFeatureTypes(const std::vector<int>& filteringFeatureTypes, bool doIncludeFilteringFeatures);      

        BENTLEYSTM_EXPORT void                    SetIncludeFilteringFeatureTypes(const bool& doIncludeFilteringFeatures);      
    };

//MS Should probably be in MrDTMQuery.h
struct ISrDTMViewDependentQueryParams : public virtual IMrDTMQueryParameters
    {                
    protected :        

        ISrDTMViewDependentQueryParams();
        virtual ~ISrDTMViewDependentQueryParams();

        virtual const DPoint3d* _GetViewBox() const = 0;                    
                            
        virtual void _SetViewBox(const DPoint3d viewBox[]) = 0;         

    public : 

        const DPoint3d* GetViewBox() const;
                            
        BENTLEYSTM_EXPORT void SetViewBox(const DPoint3d viewBox[]);        
    };

struct IMrDTMViewDependentQueryParams : public virtual ISrDTMViewDependentQueryParams 
    {
    protected :
     
        IMrDTMViewDependentQueryParams();
        virtual ~IMrDTMViewDependentQueryParams();

        virtual double        _GetMinScreenPixelsPerPoint() const = 0;

        virtual bool          _GetUseSameResolutionWhenCameraIsOff() const = 0;

        virtual bool          _GetUseSplitThresholdForLevelSelection() const = 0;

        virtual bool          _GetUseSplitThresholdForTileSelection() const = 0;

        virtual const double* _GetRootToViewMatrix() const = 0;

        virtual void          _SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint) = 0;

        virtual void          _SetRootToViewMatrix(const double rootToViewMatrix[][4]) = 0;    

        virtual void          _SetUseSameResolutionWhenCameraIsOff(bool useSameResolution) = 0;        

        virtual void          _SetUseSplitThresholdForLevelSelection(bool useSplitThreshold) = 0;        

        virtual void          _SetUseSplitThresholdForTileSelection(bool useSplitThreshold) = 0;                        
                    
               
    public : 

        double        GetMinScreenPixelsPerPoint() const;

        bool          GetUseSameResolutionWhenCameraIsOff() const;

        bool          GetUseSplitThresholdForLevelSelection() const;

        bool          GetUseSplitThresholdForTileSelection() const;

        const double* GetRootToViewMatrix() const;
    
                                
        BENTLEYSTM_EXPORT void   SetMinScreenPixelsPerPoint(double minScreenPixelsPerPoint);

        BENTLEYSTM_EXPORT void   SetRootToViewMatrix(const double rootToViewMatrix[][4]);    

        BENTLEYSTM_EXPORT void   SetUseSameResolutionWhenCameraIsOff(bool useSameResolution);        

        BENTLEYSTM_EXPORT void   SetUseSplitThresholdForLevelSelection(bool useSplitThreshold);        

        BENTLEYSTM_EXPORT void   SetUseSplitThresholdForTileSelection(bool useSplitThreshold);        
        
        BENTLEYSTM_EXPORT static IMrDTMViewDependentQueryParamsPtr CreateParams();       
    };

template<class POINT> class MrDTMFixResolutionViewPointQuery;

struct IMrDTMFixResolutionIndexQueryParams : public virtual IMrDTMQueryParameters
    {    
    protected :

        IMrDTMFixResolutionIndexQueryParams();
        virtual ~IMrDTMFixResolutionIndexQueryParams();

        virtual int  _GetResolutionIndex() const = 0;

        virtual void _SetResolutionIndex(int resolutionIndex) = 0;                                    

    public : 

        int GetResolutionIndex() const;
                                    
        BENTLEYSTM_EXPORT void SetResolutionIndex(int resolutionIndex);                            

        BENTLEYSTM_EXPORT static IMrDTMFixResolutionIndexQueryParamsPtr CreateParams();     
    };


struct IMrDTMFixResolutionMaxPointsQueryParams : public virtual IMrDTMQueryParameters
    {
    template<class POINT>
    friend class MrDTMFixResolutionViewPointQuery;
    
    protected :

        IMrDTMFixResolutionMaxPointsQueryParams();
        virtual ~IMrDTMFixResolutionMaxPointsQueryParams();

        virtual __int64 _GetMaxNumberPoints() = 0;

        virtual void    _SetMaximumNumberPoints(__int64 maxNumberPoints) = 0;            
                
    public :                 

        __int64 GetMaxNumberPoints();

        BENTLEYSTM_EXPORT void SetMaximumNumberPoints(__int64 maxNumberPoints);    

        BENTLEYSTM_EXPORT static IMrDTMFixResolutionMaxPointsQueryParamsPtr CreateParams();  
    };

struct IMrDTMQueryAllLinearsQueryParams : public virtual IMrDTMFullResolutionLinearQueryParams
    {    
    protected :

        IMrDTMQueryAllLinearsQueryParams();
        virtual ~IMrDTMQueryAllLinearsQueryParams();

        virtual list<IMrDTMFeaturePtr> _GetFeatures() = 0;        

#ifdef SCALABLE_TERRAIN_MODEL_PRIVATE_SECTION
        virtual void                   _SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features) = 0;
#endif

    public :                        

#ifdef SCALABLE_TERRAIN_MODEL_PRIVATE_SECTION
        void SetFeatures (const list<HFCPtr<HVEDTMLinearFeature>>& features);
#endif
                
        BENTLEYSTM_EXPORT list<IMrDTMFeaturePtr> GetFeatures();            

        BENTLEYSTM_EXPORT static IMrDTMQueryAllLinearsQueryParamsPtr CreateParams();  
    };

/*============================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IMrDTMQuery abstract: RefCountedBase
    {
    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/

    private:  

    protected: 
                
        virtual int _Query(Bentley::TerrainModel::DTMPtr&   dtmPtr, 
                           const DPoint3d*                  pClipShapePts, 
                           int                              nbClipShapePts, 
                           const IMrDTMQueryParametersPtr&  mrDTMQueryParamsPtr) const = 0;

        virtual int _GetNbClip() const = 0;
        virtual int _AddClip(DPoint3d* clipPointsP,
                             int       numberOfPoints, 
                             bool      isClipMask) = 0;    

        virtual int _RemoveAllClip() = 0;    

    /*__PUBLISH_SECTION_START__*/
    public:

    /* 
    * Warning: Descartes depends on these status indexes. Do not try to play with those when backward compatibility
    *          is required.
    */          
    enum Status
        {
        S_SUCCESS,
        S_ERROR,
        S_NBPTSEXCEEDMAX, 
        S_QTY,
        };
        
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..
        BENTLEYSTM_EXPORT int Query(Bentley::TerrainModel::DTMPtr&    dtmPtr,                            
                                    const DPoint3d*                   pClipShapePts, 
                                    int                               nbClipShapePts, 
                                    const IMrDTMQueryParametersPtr&   mrDTMQueryParamsPtr) const;         
        
		BENTLEYSTM_EXPORT int GetNbClip() const;
            
        BENTLEYSTM_EXPORT int AddClip(DPoint3d* clipPointsP,
                                      int       numberOfPoints, 
                                      bool      isClipMask);    

        BENTLEYSTM_EXPORT int RemoveAllClip();    
    };

END_BENTLEY_MRDTM_NAMESPACE