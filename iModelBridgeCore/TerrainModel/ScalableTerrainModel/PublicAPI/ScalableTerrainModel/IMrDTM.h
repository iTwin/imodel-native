/*--------------------------------------------------------------------------------------+
|
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <GeoCoord/BaseGeoCoord.h>
#include <ScalableTerrainModel/IMrDTMQuery.h>
#include <ScalableTerrainModel/MrDTMDefs.h>
#include <Bentley/RefCounted.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

struct IMrDTM;
typedef RefCountedPtr<IMrDTM> IMrDTMPtr;

struct IMrDTMTileTriangulatorManager;
typedef RefCountedPtr<IMrDTMTileTriangulatorManager> IMrDTMTileTriangulatorManagerPtr;

namespace GeoCoords {
struct GCS;
} 

enum CountType { COUNTTYPE_POINTS, COUNTTYPE_LINEARS, COUNTTYPE_BOTH };

struct Count;

/*=================================================================================**//**
* Interface implemented by MRDTM engines.
* @bsiclass                                                     Bentley Systems
+===============+===============+===============+===============+===============+======*/
struct IMrDTM abstract:  Bentley::TerrainModel::IDTM
    {
    private:
        

    /*__PUBLISH_SECTION_END__*/
    /*__PUBLISH_CLASS_VIRTUAL__*/
    protected:                         

        //Methods for the public interface.                
        virtual int                    _GenerateSubResolutions() = 0;      

        virtual __int64                _GetBreaklineCount() const = 0;

        virtual MrDTMCompressionType   _GetCompressionType() const = 0;        
        
        virtual Count                  _GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const = 0;

        virtual int                    _GetNbResolutions(DTMQueryDataType queryDataType) const = 0;

        virtual IMrDTMQueryPtr         _GetQueryInterface(DTMQueryType     queryType, 
                                                          DTMQueryDataType queryDataType) const = 0;

        virtual IMrDTMQueryPtr         _GetQueryInterface(DTMQueryType                         queryType, 
                                                          DTMQueryDataType                     queryDataType, 
                                                          Bentley::GeoCoordinates::BaseGCSPtr& targetGCSPtr,
                                                          const DRange3d&                      extentInTargetGCS) const = 0;


        virtual const GeoCoords::GCS&               _GetGCS() const = 0;

        virtual StatusInt                           _SetGCS(const GeoCoords::GCS& sourceGCS) = 0;

        virtual MrDTMState                          _GetState() const = 0;     
                       
        virtual bool                                _IsProgressive() const = 0;

        virtual bool                                _IsReadOnly() const = 0;

        virtual bool                                _IsShareable() const = 0;
        
        //Synchonization with data sources functions
        virtual bool                                _InSynchWithSources() const = 0; 

        virtual bool                                _LastSynchronizationCheck(time_t& last) const = 0;        

        virtual int                                 _SynchWithSources() = 0;  

        virtual int                                 _GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const = 0;

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Gets the number of points of the DTM.
        //! @return The number of points of the DTM..

        //! Gets the draping interface.
        //! @return The draping interface.

        BENTLEYSTM_EXPORT int                    GenerateSubResolutions();

        BENTLEYSTM_EXPORT __int64                GetBreaklineCount() const;
            
        BENTLEYSTM_EXPORT MrDTMCompressionType   GetCompressionType() const;

        BENTLEYSTM_EXPORT int                    GetNbResolutions(DTMQueryDataType queryDataType) const;          

        BENTLEYSTM_EXPORT IMrDTMQueryPtr         GetQueryInterface(DTMQueryType     queryType, 
                                                             DTMQueryDataType queryDataType) const;

        BENTLEYSTM_EXPORT IMrDTMQueryPtr         GetQueryInterface(DTMQueryType                         queryType, 
                                                             DTMQueryDataType                     queryDataType, 
                                                             Bentley::GeoCoordinates::BaseGCSPtr& targetGCS,
                                                             const DRange3d&                      extentInTargetGCS) const;


        BENTLEYSTM_EXPORT const Bentley::GeoCoordinates::BaseGCSPtr&
                                           GetBaseGCS() const;
        BENTLEYSTM_EXPORT StatusInt              SetBaseGCS(const Bentley::GeoCoordinates::BaseGCSPtr& sourceGCS);

        BENTLEYSTM_EXPORT const GeoCoords::GCS&  GetGCS() const;
        BENTLEYSTM_EXPORT StatusInt              SetGCS(const GeoCoords::GCS& gcs);

        BENTLEYSTM_EXPORT MrDTMState             GetState() const;        

        BENTLEYSTM_EXPORT bool                   IsProgressive() const;

        BENTLEYSTM_EXPORT bool                   IsReadOnly() const;

        BENTLEYSTM_EXPORT bool                   IsShareable() const;                        
         
        //Synchonization with data sources functions
        BENTLEYSTM_EXPORT bool                   InSynchWithSources() const; 

        // Deprecated. Remove.
        bool                                InSynchWithDataSources() const { return InSynchWithSources(); }

        BENTLEYSTM_EXPORT bool                   LastSynchronizationCheck(time_t& last) const;        

        BENTLEYSTM_EXPORT int                    SynchWithSources(); 

        BENTLEYSTM_EXPORT int                    GetRangeInSpecificGCS(DPoint3d& lowPt, DPoint3d& highPt, Bentley::GeoCoordinates::BaseGCSPtr& targetGCS) const;

        BENTLEYSTM_EXPORT Count                  GetCountInRange (const DRange2d& range, const CountType& type, const unsigned __int64& maxNumberCountedPoints) const;

    
        BENTLEYSTM_EXPORT static IMrDTMPtr        GetFor                 (const WChar*          filePath,
                                                                    bool                    openReadOnly,
                                                                    bool                    openShareable,
                                                                    StatusInt&              status);

        BENTLEYSTM_EXPORT static IMrDTMPtr        GetFor                 (const WChar*          filePath,
                                                                    bool                    openReadOnly,
                                                                    bool                    openShareable);

    };


struct Count
    {
    unsigned __int64 m_nbPoints;
    unsigned __int64 m_nbLinears;
    Count(unsigned __int64 nbPoints, unsigned __int64 nbLinears) { m_nbPoints = nbPoints; m_nbLinears = nbLinears; }
    };

END_BENTLEY_MRDTM_NAMESPACE