#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>
//#include <ScalableMesh/IScalableMeshStream.h>
#include <ScalableMesh/IScalableMeshTime.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

enum UpToDateState
    {
    UP_TO_DATE,
    REMOVE,
    MODIFY,
    ADD,
    PARTIAL_ADD
    };

enum class SMis3D
    {
    is3D,
    is25D,
    isUnknown
    };

struct ScalableMeshData 
{
private:
    struct                                      Impl;

    typedef RefCountedPtr<Impl>                 ImplPtr;
    ImplPtr                                     m_implP;

    explicit                                    ScalableMeshData                (Impl*                      implP);

public:
    IMPORT_DLLE                                 ~ScalableMeshData               ();

    IMPORT_DLLE static const ScalableMeshData&  GetNull                         ();

    IMPORT_DLLE                                 ScalableMeshData                (const ScalableMeshData&    rhs);
    IMPORT_DLLE ScalableMeshData&               operator=                       (const ScalableMeshData&    rhs);

    IMPORT_DLLE std::vector<DRange3d>&               GetExtent                       ();
    IMPORT_DLLE DRange3d&                       GetExtentByLayer                (int id);
    IMPORT_DLLE void                            SetExtents                      (std::vector<DRange3d>& extent);
    IMPORT_DLLE void                            SetExtent                       (int id, DRange3d& extent);
    IMPORT_DLLE void                            AddExtent                       (DRange3d& extent);
    IMPORT_DLLE size_t                          GetLayerCount                   ();
    IMPORT_DLLE void                            ResizeLayer                     (size_t newSize);

    IMPORT_DLLE time_t                          GetTimeFile                     ();
    IMPORT_DLLE void                            SetTimeFile                     (time_t time);

    IMPORT_DLLE UpToDateState&                  GetUpToDateState                ();
    IMPORT_DLLE void                            SetUpToDateState                (UpToDateState state);

    IMPORT_DLLE SMis3D                          IsRepresenting3dData() const;
    IMPORT_DLLE void                            SetRepresenting3dData(SMis3D isRepresenting3dData);
    IMPORT_DLLE void                            SetRepresenting3dData(bool isRepresenting3dData);

    IMPORT_DLLE __int64                         GetMaximumNbPoints() const;
    IMPORT_DLLE void                            SetMaximumNbPoints(__int64 maximumNbPoints);


    IMPORT_DLLE std::vector<DRange3d>           GetVectorRangeAdd();
    IMPORT_DLLE void                            ClearVectorRangeAdd();
    IMPORT_DLLE void                            PushBackVectorRangeAdd(DRange3d range);

    IMPORT_DLLE bool                            IsGroundDetection() const;
    IMPORT_DLLE void                            SetIsGroundDetection(bool isGroundDetection);

    IMPORT_DLLE bool                            IsGISDataType() const;
    IMPORT_DLLE void                            SetIsGISDataType(bool isGISData);

    IMPORT_DLLE WString                         ElevationPropertyName() const;
    IMPORT_DLLE void                            SetElevationPropertyName(WString& name);

    IMPORT_DLLE DTMFeatureType                  GetLinearFeatureType() const;
    IMPORT_DLLE void                            SetLinearFeatureType(DTMFeatureType type);

    IMPORT_DLLE DTMFeatureType                  GetPolygonFeatureType() const;
    IMPORT_DLLE void                            SetPolygonFeatureType(DTMFeatureType type);

    IMPORT_DLLE bool                            IsGridData() const;
    IMPORT_DLLE void                            SetIsGridData(bool isGridData);

};

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
