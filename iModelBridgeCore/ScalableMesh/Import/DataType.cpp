/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/DataType.cpp $
|    $RCSfile: DataType.cpp,v $
|   $Revision: 1.8 $
|       $Date: 2011/10/20 18:47:39 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/DataType.h>
#include <ScalableMesh/Import/Plugin/DataTypeV0.h>
#include <ScalableMesh/Import/Plugin/DataTypeRegistry.h>
#include "PluginRegistryHelper.h"

using namespace std;

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct StaticDataTypeCreator
    {
private:
    typedef Plugin::V0::StaticDataTypeCreatorBase   
                                            Base;
    friend                                  Base;

    const Base*                             m_baseP;



public:
    typedef const Base*                     ID;

    explicit                                StaticDataTypeCreator              (const Base&                     baseInstance);

                                            StaticDataTypeCreator              (const StaticDataTypeCreator&    rhs);
     StaticDataTypeCreator&                 operator=                          (const StaticDataTypeCreator&    rhs);

                                            ~StaticDataTypeCreator             ();

    ID                                      GetID                              () const { return m_baseP; }
     const DataType&                        Create                             () const;
    };

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeRegistryImpl : public PluginRegistry<StaticDataTypeCreator>
    {
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeRegistry& DataTypeRegistry::GetInstance ()
    {
    static DataTypeRegistry SINGLETON;
    return SINGLETON;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeRegistry::DataTypeRegistry ()
    :   m_implP(new DataTypeRegistryImpl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeRegistry::~DataTypeRegistry ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeRegistry::V0ID DataTypeRegistry::Register (const V0Creator& creator)
    {
    return m_implP->Register(StaticDataTypeCreator(creator));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DataTypeRegistry::Unregister (V0ID creatorID)
    {
    m_implP->Unregister(creatorID);
    }

END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE(0)

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeBase::DataTypeBase     (ClassID                     pi_id, 
                                const DataTypeFamily&       pi_family,
                                size_t                      pi_orgCapacity)     
    :   m_classID(pi_id),
        m_family(pi_family),
        m_orgGroup(pi_orgCapacity),
        m_isComplete(false),
        m_isPOD(false),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeBase::DataTypeBase     (ClassID                     pi_id, 
                                const DataTypeFamily&       pi_family,
                                const DimensionOrgGroup&    pi_orgGroup)     
    :   m_classID(pi_id),
        m_family(pi_family),
        m_orgGroup(pi_orgGroup),
        m_isComplete(false),
        m_isPOD(false),
        m_implP(0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeBase::DataTypeBase (const DataTypeFamily&   pi_family,
                            size_t                  pi_orgCapacity)
    :   m_classID(0),
        m_family(pi_family),
        m_orgGroup(pi_orgCapacity),
        m_isComplete(false),
        m_isPOD(false),
        m_implP(0)
    {
    m_classID = this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeBase::DataTypeBase (const DataTypeFamily&       pi_family,
                            const DimensionOrgGroup&    pi_orgGroup)
    :   m_classID(0),
        m_family(pi_family),
        m_orgGroup(pi_orgGroup),
        m_isComplete(false),
        m_isPOD(false),
        m_implP(0)
    {
    m_classID = this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeBase::~DataTypeBase () 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DataTypeBase::AddOrg (const DimensionOrg& pi_rOrg) 
    { 
    return m_orgGroup.push_back(pi_rOrg); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeCreatorBase::DataTypeCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeCreatorBase::~DataTypeCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataType DataTypeCreatorBase::CreateFrom (DataTypeBase* implP) const
    { 
    return DataType(implP); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeCreatorBase::StaticDataTypeCreatorBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeCreatorBase::~StaticDataTypeCreatorBase ()
    {
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& StaticDataTypeCreatorBase::Create () const
    {
    return _Create();
    }


END_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN_VXX_NAMESPACE


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

USING_NAMESPACE_BENTLEY_SCALABLEMESH_IMPORT_PLUGIN

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeCreator::StaticDataTypeCreator (const Base& baseInstance)
        :   m_baseP(&baseInstance)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeCreator::StaticDataTypeCreator (const StaticDataTypeCreator& rhs)
    :   m_baseP(rhs.m_baseP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeCreator& StaticDataTypeCreator::operator= (const StaticDataTypeCreator& rhs)
    {
    m_baseP = rhs.m_baseP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StaticDataTypeCreator::~StaticDataTypeCreator ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataType& StaticDataTypeCreator::Create () const 
    { 
    return m_baseP->_Create(); 
    } 



namespace {

bool RoleRangeIsValid (const DimensionOrgGroup& orgGroup, UInt roleQty)
    {
    struct IsRoleValid : std::unary_function<DimensionDef, bool>
        {
        UInt        m_roleQty;
        explicit IsRoleValid (UInt roleQty) : m_roleQty(roleQty) {}

        bool operator () (const DimensionDef& rhs) const
            {
            return rhs.GetRole() < m_roleQty;
            }
        };

    struct IsOrgRoleValid : std::unary_function<DimensionOrg, bool>
        {
        UInt        m_roleQty;
        explicit IsOrgRoleValid (UInt roleQty) : m_roleQty(roleQty) {}

        bool operator () (const DimensionOrg& rhs) const
            {
            return rhs.end() == find_if(rhs.begin(), rhs.end(), not1(IsRoleValid(m_roleQty)));
            }
        };


    return orgGroup.end() == find_if(orgGroup.begin(), orgGroup.end(), not1(IsOrgRoleValid(roleQty)));
    }

}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataType::DataType (DataTypeBase* pi_pImpl) 
    :   m_pImpl(pi_pImpl),  
        m_classID(m_pImpl->m_classID)
    {
    assert(RoleRangeIsValid(GetOrgGroup(), GetFamily().GetRoleQty()));

    // Pre-compute some important states.
    pi_pImpl->m_isComplete = m_pImpl->m_orgGroup.IsComplete();
    pi_pImpl->m_isPOD = m_pImpl->m_orgGroup.IsPOD();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataType::DataType (const DataType& rhs)
    :   m_pImpl(rhs.m_pImpl),  
        m_classID(m_pImpl->m_classID)
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataType& DataType::operator= (const DataType& rhs)
    {
    m_pImpl = rhs.m_pImpl;
    m_classID = m_pImpl->m_classID;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataType::~DataType ()
{
}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DataType::GetDimensionOrgCount () const
    {
    return GetOrgGroup().GetSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DataType::GetDimensionCount () const
    {
    size_t count = 0;
    std::transform(GetOrgGroup().begin(), GetOrgGroup().end(), ImagePP::AccumulateIter(count), 
                   mem_fun_ref(&DimensionOrg::GetSize));

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DataType::GetSize () const
    {
    return GetOrgGroup().GetTypeSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataType::IsComplete () const
    {
    return m_pImpl->m_isComplete;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataType::IsPOD () const
    {
    return m_pImpl->m_isPOD;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DataType::IsStatic () const
    {
    return 0 != m_classID;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DimensionOrgGroup& DataType::GetOrgGroup () const 
    {
    return m_pImpl->m_orgGroup;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DataTypeFamily& DataType::GetFamily() const 
    {
    return m_pImpl->m_family;
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeSet::Impl
    {
    typedef std::vector<DataType>   DataTypeList;
    DataTypeList                    m_types;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::DataTypeSet () 
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::DataTypeSet                        (const DataType&         pi_1) 
    :   m_implP(new Impl)
    { 
    Add(pi_1); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::DataTypeSet   (const DataType& pi_1,
                            const DataType& pi_2) 
    :   m_implP(new Impl)
    { 
    Add(pi_1); Add(pi_2); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::DataTypeSet   (const DataType&         pi_1,
                            const DataType&         pi_2,
                            const DataType&         pi_3) 
    :   m_implP(new Impl)
    { 
    Add(pi_1); Add(pi_2); Add(pi_3);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::DataTypeSet   (const DataType&     t1,
                            const DataType&     t2,
                            const DataType&     t3,
                            const DataType&     t4)
    :   m_implP(new Impl)
    { 
    Add(t1); Add(t2); Add(t3); Add(t4);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::~DataTypeSet ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::DataTypeSet (const DataTypeSet& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet& DataTypeSet::operator= (const DataTypeSet& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::const_iterator DataTypeSet::begin () const 
    { 
    return &*m_implP->m_types.begin(); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeSet::const_iterator DataTypeSet::end () const 
    { 
    return &*m_implP->m_types.end(); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DataTypeSet::GetCount () const 
    { 
    return m_implP->m_types.size(); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DataTypeSet::Add (const DataType&         pi_type)
    { 
    m_implP->m_types.push_back(pi_type); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DataTypeSet::push_back (const DataType& pi_type)
    {
    m_implP->m_types.push_back(pi_type); 
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DataTypeFactory::Impl : public ShareableObjectTypeTrait<Impl>::type
    {
    const DataTypeRegistry&     m_registry;
    explicit                    Impl                       (const DataTypeRegistry&     registry)
        :   m_registry(registry)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFactory::DataTypeFactory ()
    :   m_implP(new Impl(DataTypeRegistry::GetInstance()))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFactory::DataTypeFactory (const DataTypeRegistry& registry)
    :   m_implP(new Impl(registry))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFactory::DataTypeFactory (const DataTypeFactory& rhs)
    :   m_implP(rhs.m_implP)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataTypeFactory::~DataTypeFactory ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataType DataTypeFactory::Create   (const DataTypeFamily&       typeFamily,
                                    const DimensionOrg&         dimensionsSpec) const
    {
    struct Type : public Plugin::V0::DataTypeBase
        {
        explicit Type (const DataTypeFamily&    typeFamily,
                       const DimensionOrg&      dimensionsSpec)
            :   DataTypeBase(typeFamily)
            {
            AddOrg (dimensionsSpec);
            }
        };

    return DataType(new Type(typeFamily, dimensionsSpec));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DataType DataTypeFactory::Create   (const DataTypeFamily&       typeFamily,
                                    const DimensionOrgGroup&    dimensionsSpec) const
    {
    struct Type : public Plugin::V0::DataTypeBase
        {
        explicit Type (const DataTypeFamily&        typeFamily,
                       const DimensionOrgGroup&     dimensionsSpec)
            :   DataTypeBase(typeFamily, dimensionsSpec)
            {
            }
        };

    return DataType(new Type(typeFamily, dimensionsSpec));
    }


END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
