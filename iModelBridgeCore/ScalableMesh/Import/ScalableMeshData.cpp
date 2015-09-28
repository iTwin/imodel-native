#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/ScalableMeshData.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct ScalableMeshData::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    vector<DRange3d>            m_extent;
    UpToDateState               m_upToDateState;
    time_t                      m_time;
    SMis3D                      m_isRepresenting3dData;
    bool                        m_isGroundDetection;
    bool                        m_isGISData;
    WString                     m_elevationProperty;
    
    // not persisted information
    //NEEDS_WORK_SM Temp variable for YII
    __int64                     m_maximumNbPoints;
    std::vector<DRange3d>       m_vectorRangeAdd;
              
    explicit                    Impl            (const std::vector<DRange3d>& extent, const time_t time, SMis3D isRepresenting3dData = SMis3D::isUnknown, bool isGroundDetection = false, UpToDateState state=UpToDateState::UP_TO_DATE, __int64 maximumNbPoints = numeric_limits<__int64>::max(), std::vector<DRange3d> vecRangeAdd = {})
        : m_extent(extent),
          m_upToDateState(state),
          m_time(time), 
          m_vectorRangeAdd(vecRangeAdd),
          m_isRepresenting3dData(isRepresenting3dData),
          m_isGroundDetection(isGroundDetection),
          m_isGISData(false),
          m_maximumNbPoints(maximumNbPoints)
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
    m_implP = new Impl(rhs.m_implP->m_extent, rhs.m_implP->m_time, rhs.m_implP->m_isRepresenting3dData, rhs.m_implP->m_isGroundDetection, rhs.m_implP->m_upToDateState, rhs.m_implP->m_maximumNbPoints, rhs.m_implP->m_vectorRangeAdd);
    }

ScalableMeshData::ScalableMeshData(BinaryIStream& stream)
    {
    m_implP = new Impl(std::vector<DRange3d>(), time_t());
    size_t sizeRange;
    stream.read(reinterpret_cast<byte*>(&sizeRange), sizeof(size_t));
    m_implP->m_extent.resize(sizeRange);
    if(!m_implP->m_extent.empty())
        stream.read(reinterpret_cast<byte*>(&m_implP->m_extent[0]), sizeof(DRange3d)*(int)sizeRange);
    stream.read(reinterpret_cast<byte*>(&m_implP->m_upToDateState), sizeof(UpToDateState));
    stream.read(reinterpret_cast<byte*>(&m_implP->m_time), sizeof(time_t));
    stream.read(reinterpret_cast<byte*>(&m_implP->m_isRepresenting3dData), sizeof(m_implP->m_isRepresenting3dData));
    stream.read(reinterpret_cast<byte*>(&m_implP->m_isGroundDetection), sizeof(m_implP->m_isGroundDetection));
    stream.read(reinterpret_cast<byte*>(&m_implP->m_isGISData), sizeof(m_implP->m_isGISData));
    size_t nOfChars;
    stream.read(reinterpret_cast<byte*>(&nOfChars), sizeof(nOfChars));
    char* stringBuffer = new char[nOfChars];
    stream.read(reinterpret_cast<byte*>(stringBuffer), nOfChars);
    m_implP->m_elevationProperty = WString(stringBuffer);
    delete[] stringBuffer;
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

void ScalableMeshData::Serialize(BinaryOStream& stream) const
    {
    if(m_implP->m_extent.empty())
        {
        size_t size = 0;
        stream.write(reinterpret_cast<const byte*>(&size), sizeof(size_t));
        }
    else
        {
        size_t size = m_implP->m_extent.size();
        stream.write(reinterpret_cast<const byte*>(&size), sizeof(size_t));
        stream.write(reinterpret_cast<const byte*>(&m_implP->m_extent[0]), sizeof(DRange3d)*(int)m_implP->m_extent.size());
        }
    stream.write(reinterpret_cast<const byte*>(&m_implP->m_upToDateState), sizeof(UpToDateState));
    stream.write(reinterpret_cast<const byte*>(&m_implP->m_time), sizeof(time_t));
    stream.write(reinterpret_cast<byte*>(&m_implP->m_isRepresenting3dData), sizeof(m_implP->m_isRepresenting3dData));
    stream.write(reinterpret_cast<byte*>(&m_implP->m_isGroundDetection), sizeof(m_implP->m_isGroundDetection));
    stream.write(reinterpret_cast<byte*>(&m_implP->m_isGISData), sizeof(m_implP->m_isGISData));
    size_t charsOfString = m_implP->m_elevationProperty.GetMaxLocaleCharBytes();
    char* stringBuffer = new char[charsOfString];
    stringBuffer = m_implP->m_elevationProperty.ConvertToLocaleChars(stringBuffer);
    stream.write(reinterpret_cast<byte*>(&charsOfString), sizeof(charsOfString));
    stream.write(reinterpret_cast<byte*>(stringBuffer), (BinaryOStream::streamsize)charsOfString);
    delete[] stringBuffer;
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

bool ScalableMeshData::IsGISDataType() const
    {
    return m_implP->m_isGISData;
    }

void ScalableMeshData::SetIsGISDataType(bool isGISData)
    {
    m_implP->m_isGISData= isGISData;
    }

WString ScalableMeshData::ElevationPropertyName() const
    {
    return m_implP->m_elevationProperty;
    }

void ScalableMeshData::SetElevationPropertyName(WString& name)
    {
    m_implP->m_elevationProperty = name;
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
