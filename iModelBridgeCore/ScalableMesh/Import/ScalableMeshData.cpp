#include <ScalableMeshPCH.h>
#include "../STM/ImagePPHeaders.h"
#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct ScalableMeshData::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    vector<DRange3d>            m_extent;
    UpToDateState               m_upToDateState;
    time_t                      m_time;
    SMis3D                      m_isRepresenting3dData;
    bool                        m_isGroundDetection;
    bvector<uint32_t>           m_classesToImport;
    bool                        m_isGISData;
    WString                     m_elevationProperty;
    DTMFeatureType              m_linearFeatureType;
    DTMFeatureType              m_polygonFeatureType;
    bool                        m_isGridData;
    // not persisted information
    //NEEDS_WORK_SM Temp variable for YII
    __int64                     m_maximumNbPoints;
    std::vector<DRange3d>       m_vectorRangeAdd;

    explicit                    Impl            (const std::vector<DRange3d>& extent, const time_t time, SMis3D isRepresenting3dData = SMis3D::isUnknown, bool isGroundDetection = false, const bvector<uint32_t>& classesToImport = bvector<uint32_t>(), UpToDateState state=UpToDateState::UP_TO_DATE, __int64 maximumNbPoints = numeric_limits<__int64>::max(), std::vector<DRange3d> vecRangeAdd = {})
        : m_extent(extent),
          m_upToDateState(state),
          m_time(time), 
          m_vectorRangeAdd(vecRangeAdd),
          m_isRepresenting3dData(isRepresenting3dData),
          m_isGroundDetection(isGroundDetection),
          m_classesToImport(classesToImport),
          m_isGISData(false),
          m_maximumNbPoints(maximumNbPoints),
          m_elevationProperty(L""),
          m_linearFeatureType(DTMFeatureType::Breakline),
          m_polygonFeatureType(DTMFeatureType::Breakline),
          m_isGridData(false)
        {
        }      

    static void                   UpdateForEdit   (ImplPtr& instancePtr)
        {
        if(instancePtr->IsShared())
            instancePtr = new Impl(*instancePtr);
        }
    };

ScalableMeshData::~ScalableMeshData()
    {

    }

const ScalableMeshData& ScalableMeshData::GetNull()
    {
    static const ScalableMeshData NULL_ScalableMeshData(new Impl(std::vector<DRange3d>(), time_t()));
    return NULL_ScalableMeshData;
    }

ScalableMeshData::ScalableMeshData(Impl* implP)
    : m_implP(implP)
    {
    }

ScalableMeshData::ScalableMeshData(const ScalableMeshData& rhs)
    {        
    m_implP = new Impl(rhs.m_implP->m_extent, rhs.m_implP->m_time, rhs.m_implP->m_isRepresenting3dData, rhs.m_implP->m_isGroundDetection, rhs.m_implP->m_classesToImport, rhs.m_implP->m_upToDateState, rhs.m_implP->m_maximumNbPoints, rhs.m_implP->m_vectorRangeAdd);
    m_implP->m_isGISData = rhs.m_implP->m_isGISData;
    m_implP->m_elevationProperty = rhs.m_implP->m_elevationProperty;
    m_implP->m_linearFeatureType = rhs.m_implP->m_linearFeatureType;
    m_implP->m_polygonFeatureType = rhs.m_implP->m_polygonFeatureType;
    m_implP->m_isGridData = rhs.m_implP->m_isGridData;
    }

ScalableMeshData& ScalableMeshData::operator=(const ScalableMeshData&    rhs)
    {
    m_implP = rhs.m_implP;
    return *this;
    }

vector<DRange3d>& ScalableMeshData::GetExtent()
    {
    return m_implP->m_extent;
    }

DRange3d& ScalableMeshData::GetExtentByLayer(int id)
    {
    //if(id < m_implP->m_extent.size())
    return m_implP->m_extent[id];
    }

void ScalableMeshData::SetExtents(vector<DRange3d>& extent)
    {
    m_implP->m_extent = extent;
    }

void ScalableMeshData::SetExtent(int id, DRange3d& extent)
    {
    if(id+1 > (int)m_implP->m_extent.size())
        m_implP->m_extent.resize(id + 1);
    m_implP->m_extent[id] = extent;
    }

void ScalableMeshData::AddExtent(DRange3d& extent)
    {
    m_implP->m_extent.push_back(extent);
    }

size_t ScalableMeshData::GetLayerCount()
    {
    return m_implP->m_extent.size();
    }

void ScalableMeshData::ResizeLayer(size_t newSize)
    {
    m_implP->m_extent.resize(newSize);
    }

UpToDateState& ScalableMeshData::GetUpToDateState()
    {
    return m_implP->m_upToDateState;
    }

void ScalableMeshData::SetUpToDateState(UpToDateState state)
    {
    m_implP->m_upToDateState = state;
    }

time_t ScalableMeshData::GetTimeFile()
    {
    return m_implP->m_time;
    }

void ScalableMeshData::SetTimeFile(time_t time)
    {
    m_implP->m_time = time;
    }

SMis3D ScalableMeshData::IsRepresenting3dData() const 
    {
    return m_implP->m_isRepresenting3dData;
    }

void ScalableMeshData::SetRepresenting3dData(SMis3D isRepresenting3dData)
    {
    m_implP->m_isRepresenting3dData = isRepresenting3dData;
    }

void ScalableMeshData::SetRepresenting3dData(bool isRepresenting3dData)
    {
    m_implP->m_isRepresenting3dData = isRepresenting3dData ? SMis3D::is3D : SMis3D::is25D;
    }

__int64 ScalableMeshData::GetMaximumNbPoints() const
    {
    return m_implP->m_maximumNbPoints;
    }

void ScalableMeshData::SetMaximumNbPoints(__int64 maximumNbPoints)
    {
    m_implP->m_maximumNbPoints = maximumNbPoints;
    }

std::vector<DRange3d> ScalableMeshData::GetVectorRangeAdd() 
    {
    return m_implP->m_vectorRangeAdd;
    }

void ScalableMeshData::ClearVectorRangeAdd() 
    {
    m_implP->m_vectorRangeAdd.clear();
    }

void ScalableMeshData::PushBackVectorRangeAdd(DRange3d range)
    {
    m_implP->m_vectorRangeAdd.push_back(range);
    }

bool ScalableMeshData::IsGroundDetection() const
    {
    return m_implP->m_isGroundDetection;
    }

void ScalableMeshData::SetIsGroundDetection(bool isGroundDetection)
    {
    m_implP->m_isGroundDetection = isGroundDetection;
    }

void ScalableMeshData::GetClassificationToImport(bvector<uint32_t>& classesToImport)
    {
    classesToImport.insert(classesToImport.end(), m_implP->m_classesToImport.begin(), m_implP->m_classesToImport.end());
    }

void ScalableMeshData::SetClassificationToImport(const bvector<uint32_t>& classesToImport)
    {
    m_implP->m_classesToImport = classesToImport;
    }

bool ScalableMeshData::IsGISDataType() const
    {
    return m_implP->m_isGISData;
    }

void ScalableMeshData::SetIsGISDataType(bool isGISData)
    {
    m_implP->m_isGISData= isGISData;
    }

bool ScalableMeshData::IsGridData() const
    {
    return m_implP->m_isGridData;
    }

void ScalableMeshData::SetIsGridData(bool isGridData)
    {
    m_implP->m_isGridData = isGridData;
    }

WString ScalableMeshData::ElevationPropertyName() const
    {
    return m_implP->m_elevationProperty;
    }

void ScalableMeshData::SetElevationPropertyName(WString& name)
    {
    m_implP->m_elevationProperty = name;
    }

DTMFeatureType ScalableMeshData::GetLinearFeatureType() const
    {
    return m_implP->m_linearFeatureType;
    }

void ScalableMeshData::SetLinearFeatureType(DTMFeatureType type)
    {
    m_implP->m_linearFeatureType = type;
    }

DTMFeatureType ScalableMeshData::GetPolygonFeatureType() const
    {
    return m_implP->m_polygonFeatureType;
    }

void ScalableMeshData::SetPolygonFeatureType(DTMFeatureType type)
    {
    m_implP->m_polygonFeatureType = type;
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
