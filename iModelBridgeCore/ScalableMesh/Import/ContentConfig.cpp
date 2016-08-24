/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/ContentConfig.cpp $
|    $RCSfile: ContentConfig.cpp,v $
|   $Revision: 1.9 $
|       $Date: 2011/11/18 15:50:52 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>
#include <ScalableMesh/Import/ContentConfig.h>
#include <ScalableMesh/Import/Config/Content/All.h>
#include <ScalableMesh/Type/IScalableMeshPoint.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


static const DataType POINT_TYPE_FAMILY(PointType3d64fCreator().Create());
struct TypeConfigImpl : public RefCountedBase
    {
    friend struct TypeConfig;
    private:
        DataType m_type;
        bool m_isSet;
    public:
        TypeConfigImpl()
            : m_type(POINT_TYPE_FAMILY)
            {
            m_isSet = false;
            }
        TypeConfigImpl(const DataType& type) : m_type(type) { m_isSet = true; }
        TypeConfigImpl(const TypeConfigImpl& type) : m_type(type.m_type) { m_isSet = type.m_isSet; }
    };

TypeConfig::TypeConfig()
    : m_pImpl(new TypeConfigImpl())
    {}

TypeConfig::~TypeConfig()
    {}

TypeConfig::TypeConfig(const DataType& type)
    : m_pImpl(new TypeConfigImpl(type))
    {}

TypeConfig::TypeConfig(const TypeConfig& rhs)
    : m_pImpl(new TypeConfigImpl(*rhs.m_pImpl))
    {}

const DataType&                     TypeConfig::GetType() const
    {
    return m_pImpl->m_type;
    }
bool                     TypeConfig::IsSet() const
    {
    return m_pImpl->m_isSet;
    }



struct ScalableMeshConfigImpl : public RefCountedBase
    {
    friend struct ScalableMeshConfig;
    private:
        ScalableMeshData m_smData;
        bool m_isSet;
    public:
        ScalableMeshConfigImpl() : m_smData(ScalableMeshData::GetNull()) { m_isSet = false; }
        ScalableMeshConfigImpl(const ScalableMeshData& data) : m_smData(data) { m_isSet = true; }
        ScalableMeshConfigImpl(const ScalableMeshConfigImpl& smConfig) : m_smData(smConfig.m_smData) { m_isSet = smConfig.m_isSet; }
    };

ScalableMeshConfig::ScalableMeshConfig()
    : m_pImpl(new ScalableMeshConfigImpl())
    {}

ScalableMeshConfig::~ScalableMeshConfig()
    {}

ScalableMeshConfig::ScalableMeshConfig(const ScalableMeshData& data)
    : m_pImpl(new ScalableMeshConfigImpl(data))
    {}

ScalableMeshConfig::ScalableMeshConfig(const ScalableMeshConfig& rhs)
    : m_pImpl(new ScalableMeshConfigImpl(*rhs.m_pImpl))
    {}

const ScalableMeshData&                     ScalableMeshConfig::GetScalableMeshData() const
    {
    return m_pImpl->m_smData;
    }

bool                     ScalableMeshConfig::IsSet() const
    {
    return m_pImpl->m_isSet;
    }


struct GCSConfigImpl : public RefCountedBase
    {
    friend struct GCSConfig;
    enum
        {
        FLAG_PREPEND_TO_EXISTING_LOCAL_TRANSFORM = 0x1,
        FLAG_PRESERVE_EXISTING_IF_GEOREFERENCED = 0x2,
        FLAG_PRESERVE_EXISTING_IF_LOCAL_CS = 0x4,
        };
    private:
        GCS m_gcs;
        uint32_t m_flags;
        bool m_isSet;
    public:
        GCSConfigImpl() : m_gcs(GCS::GetNull()) { m_isSet = false; }
        GCSConfigImpl(const GCS& gcs, uint32_t flags) : m_gcs(gcs), m_flags(flags) { m_isSet = true; }
        GCSConfigImpl(const GCSConfigImpl& gcsConfig) : m_gcs(gcsConfig.m_gcs), m_flags(gcsConfig.m_flags) { m_isSet = gcsConfig.m_isSet; }
    };

GCSConfig::GCSConfig()
    : m_pImpl(new GCSConfigImpl())
    {}

GCSConfig::~GCSConfig()
    {}

GCSConfig::GCSConfig(const GCS& gcs, uint32_t flags)
    : m_pImpl(new GCSConfigImpl(gcs, flags))
    {}

GCSConfig::GCSConfig(const GCSConfig& rhs)
    : m_pImpl(new GCSConfigImpl(*rhs.m_pImpl))
    {}

bool                     GCSConfig::IsSet() const
    {
    return m_pImpl->m_isSet;
    }

const GCS&               GCSConfig::GetGCS() const
    {
    return m_pImpl->m_gcs;
    }

GCSConfig&                  GCSConfig::PrependToExistingLocalTransform(bool                        prepend)
    {
    m_pImpl->m_isSet = true;
    SetBitsTo(m_pImpl->m_flags, GCSConfigImpl::FLAG_PREPEND_TO_EXISTING_LOCAL_TRANSFORM, prepend); return *this;
    }

GCSConfig&                  GCSConfig::PreserveExistingIfGeoreferenced(bool                        preserve)
    {
    m_pImpl->m_isSet = true;
    SetBitsTo(m_pImpl->m_flags, GCSConfigImpl::FLAG_PRESERVE_EXISTING_IF_GEOREFERENCED, preserve); return *this;
    }

GCSConfig&                  GCSConfig::PreserveExistingIfLocalCS(bool                        preserve)
    {
    m_pImpl->m_isSet = true;
    SetBitsTo(m_pImpl->m_flags, GCSConfigImpl::FLAG_PRESERVE_EXISTING_IF_LOCAL_CS, preserve); return *this;
    }

bool                                GCSConfig::IsPrependedToExistingLocalTransform() const
    {
    return HasBitsOn(m_pImpl->m_flags, GCSConfigImpl::FLAG_PREPEND_TO_EXISTING_LOCAL_TRANSFORM);
    }
bool                                GCSConfig::IsExistingPreservedIfGeoreferenced() const
    {
    return HasBitsOn(m_pImpl->m_flags, GCSConfigImpl::FLAG_PRESERVE_EXISTING_IF_GEOREFERENCED);
    }

bool                                GCSConfig::IsExistingPreservedIfLocalCS() const
    {
    return HasBitsOn(m_pImpl->m_flags, GCSConfigImpl::FLAG_PRESERVE_EXISTING_IF_LOCAL_CS);
    }
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfig::Impl : public RefCountedBase//ShareableObjectTypeTrait<Impl>::type
    {
    friend struct ContentConfig;
    GCSConfig m_gcsConfig;
    TypeConfig m_typeConfig;
    ScalableMeshConfig m_scalableMeshConfig;

    explicit                Impl                   ()
        {
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig::ContentConfig ()
    :   m_pImpl(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig::~ContentConfig ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig::ContentConfig (const ContentConfig& pi_rRight)
    :   m_pImpl(pi_rRight.m_pImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfig& ContentConfig::operator= (const ContentConfig& pi_rRight)
    {
    m_pImpl = pi_rRight.m_pImpl;
    return *this;
    }


const GCSConfig&       ContentConfig::GetGCSConfig() const
    {
    return m_pImpl->m_gcsConfig;
    }

const TypeConfig&       ContentConfig::GetTypeConfig() const
    {
    return m_pImpl->m_typeConfig;
    }

const ScalableMeshConfig&       ContentConfig::GetScalableMeshConfig() const
    {
    return m_pImpl->m_scalableMeshConfig;
    }

void       ContentConfig::SetGCSConfig(const GCSConfig& gcsConfig)
    {
    m_pImpl->m_gcsConfig = gcsConfig;
    }

void       ContentConfig::SetTypeConfig(const TypeConfig& typeConfig)
    {
    m_pImpl->m_typeConfig = typeConfig;
    }

void       ContentConfig::SetScalableMeshConfig(const ScalableMeshConfig& scalableMeshConfig)
    {
    m_pImpl->m_scalableMeshConfig = scalableMeshConfig;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy::ContentConfigPolicy ()
    :   m_flags(0),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy::~ContentConfigPolicy ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy::ContentConfigPolicy (const ContentConfigPolicy& rhs)
    :   m_flags(rhs.m_flags),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ContentConfigPolicy& ContentConfigPolicy::operator= (const ContentConfigPolicy& rhs)
    {
    m_flags = rhs.m_flags;
    return *this;
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
