/*--------------------------------------------------------------------------------------+
|
|     $Source: AutomaticGroundDetection/ScalableMeshPointsProvider.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <TerrainModel\AutomaticGroundDetection\IPointsProvider.h>
#ifdef VANCOUVER_API
#include <DgnGeoCoord\DgnGeoCoord.h>
#else
#include <DgnPlatform\DgnGeoCoord.h>
#endif
#include <Geom/Polyface.h>
#include <ScalableMesh\IScalableMesh.h>


/*
DCPOINTCLOUDCORE_TYPEDEF(ScalableMeshPointsProvider)
DCPOINTCLOUDCORE_REF_COUNTED_PTR(ScalableMeshPointsProvider)
*/

USING_NAMESPACE_GROUND_DETECTION
using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;



BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE


/*=================================================================================**//**
* @bsiclass                                             Marc.Bedard     12/2015
+===============+===============+===============+===============+===============+======*/
struct ScalableMeshPointsProvider : public IPointsProvider
{
public:

    static IPointsProviderPtr CreateFrom(IScalableMeshPtr& smesh, 
                                         DRange3dCR boundingBoxInUors);

    double    GetMeterToSRSFactor() const;
    Transform GetRootToNativeTransform() const;
    
    BentleyStatus GetPoints(bvector<DPoint3d>& points, double* resolution, ClipVectorCP clip) const;


    void SetReprojectionInfo(GeoCoordInterpretation geocoordInterpretation,
                             BaseGCSPtr&            sourceGcs,
                             BaseGCSPtr&            destinationGcs);    

private:
        
    IScalableMeshPtr                                                       m_smesh;
    Transform                                                              m_transform;
    mutable RefCountedPtr<ScalableMeshPointsProvider::IPointsProviderIteratorImpl> m_currentIterator;

    GeoCoordInterpretation m_geocoordInterpretation;
    mutable BaseGCSPtr     m_sourceGcs;
    mutable BaseGCSPtr     m_destinationGcs;
    
    ScalableMeshPointsProvider(IScalableMeshPtr&      smesh, 
                               DRange3dCR             boundingBoxInUors);

    ScalableMeshPointsProvider(ScalableMeshPointsProvider const & object);

    ~ScalableMeshPointsProvider();

    //=======================================================================================
    //! @bsiclass
    //===============+===============+===============+===============+===============+=======
    struct      ScalableMeshPointsProviderPrefetchedIteratorImpl : IPointsProvider::IPointsProviderIteratorImpl
        {
        public:
            friend ScalableMeshPointsProvider;

        private:
            bvector<ReturnType>::iterator          m_endItr;
            bvector<ReturnType>::iterator          m_currentItr;

            ScalableMeshPointsProviderPrefetchedIteratorImpl(bvector<ReturnType>& prefetchedPoints, bool isAtEnd);
            ~ScalableMeshPointsProviderPrefetchedIteratorImpl();

            virtual bool          _IsDifferent(IPointsProvider::IPointsProviderIteratorImpl const& rhs) const override;
            virtual void          _MoveToNext() override;
            virtual ReturnType&   _GetCurrent() const override;
            virtual bool          _IsAtEnd() const override;
        };    

    void    InternalQueryPoints() const;
        
    virtual IPointsProviderPtr _Clone() const;
    virtual size_t         _ComputeMemorySize() const override;
    virtual void           _PrefetchPoints() override;
    virtual void           _ClearPrefetchedPoints() override;
    virtual Transform      _GetMeterToNativeTransform() const override;
    virtual double         _GetExportResolution() const override;
    virtual BeFileName       _GetFileName() const override;
    
    virtual  IPointsProvider::const_iterator _begin() const override;
    virtual  IPointsProvider::const_iterator _end() const override;
    
}; // ScalableMeshPointsProvider

struct ScalableMeshPointsProviderCreator;
typedef RefCountedPtr<ScalableMeshPointsProviderCreator> ScalableMeshPointsProviderCreatorPtr;


struct ScalableMeshPointsProviderCreator : public IPointsProviderCreator
    {
    private :

        IScalableMeshPtr  m_smesh;
        bvector<DPoint3d> m_extractionArea;

        GeoCoordInterpretation m_geocoordInterpretation;
        BaseGCSPtr             m_sourceGcs;
        BaseGCSPtr             m_destinationGcs;

    protected : 

        virtual IPointsProviderPtr _CreatePointProvider(DRange3d const& boundingBoxInUors) override;

        virtual IPointsProviderPtr _CreatePointProvider() override;

        virtual void               _GetAvailableRange(DRange3d& availableRange) override;

    public :


        void SetExtractionArea(const bvector<DPoint3d>& area);

        ScalableMeshPointsProviderCreator(IScalableMeshPtr&      smesh, 
                                          BaseGCSPtr&            sourceGcs,
                                          BaseGCSPtr&            destinationGcs,
                                          GeoCoordInterpretation geocoordInterpretation);

        virtual ~ScalableMeshPointsProviderCreator();
        
        static ScalableMeshPointsProviderCreatorPtr Create(IScalableMeshPtr&      smesh,                                                            
                                                           BaseGCSPtr             sourceGcs = BaseGCSPtr(),
                                                           BaseGCSPtr             destinationGcs = BaseGCSPtr(),
                                                           GeoCoordInterpretation geocoordInterpretation = GeoCoordInterpretation::Cartesian);      
    };


END_BENTLEY_SCALABLEMESH_NAMESPACE
