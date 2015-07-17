/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/Import/DimensionDescription.cpp $
|    $RCSfile: DimensionDescription.cpp,v $
|   $Revision: 1.4 $
|       $Date: 2011/08/26 18:47:03 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableTerrainModelPCH.h>
#include <ScalableTerrainModel/Import/DataTypeDescription.h>
#include <ScalableTerrainModel/Import/Exceptions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionType::Info
    {
    const WString                m_name;
    size_t                          m_size;

    explicit                        Info                               (const WChar*      name,
                                                                        size_t              size)
        :   m_name(name),
            m_size(size)
        {
        }

    bool                            operator==                         (const Info&         rhs) const
        {
        return m_name == rhs.m_name;
        }
    bool                            operator<                          (const Info&         rhs) const
        {
        return m_name < rhs.m_name;
        }
    };


namespace {


DimensionType::Info                 s_typeInfoVoid                 (L"Void",            0);
DimensionType::Info                 s_typeInfoUInt8                (L"UInt8",           1);
DimensionType::Info                 s_typeInfoUInt16               (L"UInt16",          2);
DimensionType::Info                 s_typeInfoUInt32               (L"UInt32",          4);
DimensionType::Info                 s_typeInfoFloat32              (L"Float32",         4);
DimensionType::Info                 s_typeInfoFloat64              (L"Float64",         8);



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct TypeInfoRegistry
    {
private:
    typedef std::set<DimensionType::Info>
                                    TypeInfoMapping;
    TypeInfoMapping                 m_typeInfos;

public:
    typedef const WChar*         ID;

    static TypeInfoRegistry&        GetInstance                        ()
        {
        static TypeInfoRegistry SINGLETON;
        return SINGLETON;
        }

    explicit                        TypeInfoRegistry                   ()
        {
        Register(s_typeInfoVoid);
        Register(s_typeInfoUInt8);
        Register(s_typeInfoUInt16);
        Register(s_typeInfoUInt32);
        Register(s_typeInfoFloat32);
        Register(s_typeInfoFloat64);
        }

    ID                              Register                           (const DimensionType::Info&      typeInfo)
        {
        typedef std::pair<TypeInfoMapping::iterator, bool> ReturnType;

        ReturnType ret = m_typeInfos.insert(typeInfo);
        assert(ret.second);

        return ret.first->m_name.c_str();
        }

    void                            Unregister                         (ID                              id)
        {
        m_typeInfos.erase(DimensionType::Info(id, 0)); // TDORAY: Accumulate in a vector instead.
        }

    
    const DimensionType::Info&      GetFor                             (const WChar*                 name) const
        {
        const TypeInfoMapping::const_iterator foundIt = m_typeInfos.find(DimensionType::Info(name, 0));

        if (m_typeInfos.end() == foundIt)
            throw CustomException(L"Type not found!");

        return *foundIt;
        }
    

    };


}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionType::DimensionType   (ID              id,
                                const Info&     info)
    :   m_id(id),
        m_infoP(&info)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& DimensionType::GetName () const
    {
    return m_infoP->m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* DimensionType::GetNameCStr () const
    {
    return m_infoP->m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionType::GetSize () const
    {
    return m_infoP->m_size;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionType::EqualTo (const DimensionType& rhs) const
    {
    return m_infoP->m_name == rhs.m_infoP->m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionType::LessThan (const DimensionType& rhs) const
    {
    return m_infoP->m_name < rhs.m_infoP->m_name;
    }


const DimensionType& DimensionType::GetVoid     () { static const DimensionType SINGLETON(ID_VOID,     s_typeInfoVoid); return SINGLETON; }
const DimensionType& DimensionType::GetUInt8    () { static const DimensionType SINGLETON(ID_UINT8,    s_typeInfoUInt8); return SINGLETON; }
const DimensionType& DimensionType::GetUInt16   () { static const DimensionType SINGLETON(ID_UINT16,   s_typeInfoUInt16); return SINGLETON; }
const DimensionType& DimensionType::GetUInt32   () { static const DimensionType SINGLETON(ID_UINT32,   s_typeInfoUInt32); return SINGLETON; }
const DimensionType& DimensionType::GetFloat32  () { static const DimensionType SINGLETON(ID_FLOAT32,  s_typeInfoFloat32); return SINGLETON; }
const DimensionType& DimensionType::GetFloat64  () { static const DimensionType SINGLETON(ID_FLOAT64,  s_typeInfoFloat64); return SINGLETON; }

const DimensionType& DimensionType::GetUnknown  () { return GetVoid(); }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DimensionType& DimensionType::GetFor (ID id)
    {
    assert(ID_QTY > id); // TDORAY: Consider throwing instead?

    const DimensionType* TYPES[] = 
        {
        &GetVoid(), 
        &GetUInt8(), 
        &GetUInt16(), 
        &GetUInt32(), 
        &GetFloat32(), 
        &GetFloat64(),
        };
    HSTATICASSERT((ID_QTY == (sizeof TYPES / sizeof TYPES[0])));  

    return *TYPES[id];
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionType DimensionType::GetFor (const WChar* name)
    {
    return DimensionType(ID_CUSTOM, TypeInfoRegistry::GetInstance().GetFor(name));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionType::Register::Register  (const WChar*  name,
                                    size_t          size)
    :   m_id(TypeInfoRegistry::GetInstance().Register(Info(name, size)))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionType::Register::~Register ()
    {
    TypeInfoRegistry::GetInstance().Unregister(m_id);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionDef::GetTypeSize () const 
    {
    return m_type.GetSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionDef::DimensionDef () 
    :   m_type(DimensionType::GetUnknown()), m_role(0) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionDef::DimensionDef (const DimensionType& pi_Type)
    :   m_type(pi_Type), m_role(0) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionDef::DimensionDef     (const DimensionType&    pi_Type, 
                                DimensionRole           pi_Role) 
    :   m_type(pi_Type), m_role(pi_Role)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionDef::~DimensionDef ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const DimensionType& DimensionDef::GetType () const 
    {
    return m_type;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionRole DimensionDef::GetRole () const 
    {
    return m_role;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionDef::SetType (const DimensionType& pi_Type)    
    {
    m_type = pi_Type;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionDef::SetRole (DimensionRole pi_Role)
    {
    m_role = pi_Role;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionDef::IsNative () const
    {
    return m_type.IsNative();    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionDef::IsComplete () const
    {
    return (DimensionType::ID_UNKNOWN != m_type.GetID()) && (0 != m_role);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionOrg::Impl
    {
    static const size_t                 UNDEFINED_TYPE_SIZE;

    bvector<DimensionDef>               m_dimensions;
    mutable size_t                      m_typeSize;


    explicit                            Impl                           ()
        :   m_typeSize(0) 
        {
        }

    explicit                            Impl                           (size_t              capacity)
        :   m_typeSize(0) 
        {
        m_dimensions.reserve(capacity);
        }

    explicit                            Impl                           (const_iterator      pi_Begin, 
                                                                        const_iterator      pi_End)
        :   m_dimensions(pi_Begin, pi_End),
            m_typeSize(ComputeTypeSize()) 
        {
        }

    void                                InvalidateTypeSize             ()
        {
        m_typeSize = UNDEFINED_TYPE_SIZE;
        }

    size_t                              GetTypeSize                    () const              
        {
        return (UNDEFINED_TYPE_SIZE == m_typeSize) ? m_typeSize = ComputeTypeSize() : m_typeSize;
        }

    size_t                              ComputeTypeSize                () const;

    };



const size_t DimensionOrg::Impl::UNDEFINED_TYPE_SIZE = (std::numeric_limits<size_t>::max)();


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg  (size_t pi_Capacity)    
    :   m_implP(new Impl(pi_Capacity))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg (const DimensionDef& d0)
    :   m_implP(new Impl)
    { push_back(d0); m_implP->ComputeTypeSize(); }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg (const DimensionDef&      d0,
                            const DimensionDef&      d1)
    :   m_implP(new Impl)
    { push_back(d0); push_back(d1);}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg (const DimensionDef&      d0,
                            const DimensionDef&      d1,
                            const DimensionDef&      d2)
    :   m_implP(new Impl)
    { push_back(d0); push_back(d1); push_back(d2);}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg (const DimensionDef&      d0,
                            const DimensionDef&      d1,
                            const DimensionDef&      d2,
                            const DimensionDef&      d3)
    :   m_implP(new Impl)
    { push_back(d0); push_back(d1); push_back(d2); push_back(d3);}

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg (const DimensionDef&      d0,
                            const DimensionDef&      d1,
                            const DimensionDef&      d2,
                            const DimensionDef&      d3,
                            const DimensionDef&      d4)
    :   m_implP(new Impl)
    { push_back(d0); push_back(d1); push_back(d2); push_back(d3); push_back(d4);}


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg (const_iterator  pi_Begin, 
                            const_iterator  pi_End)         
    :   m_implP(new Impl(pi_Begin, pi_End))
    {

    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::~DimensionOrg ()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::DimensionOrg (const DimensionOrg& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg& DimensionOrg::operator= (const DimensionOrg& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionDef& DimensionOrg::operator[]  (size_t      pi_Idx)        
    {
    m_implP->InvalidateTypeSize();
    return m_implP->m_dimensions[pi_Idx];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DimensionDef& DimensionOrg::operator[] (size_t pi_Idx) const   
    {
    return m_implP->m_dimensions[pi_Idx];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::const_iterator DimensionOrg::cbegin () const                     
    {
    return &(*m_implP->m_dimensions.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::const_iterator DimensionOrg::cend () const
    {
    return &(*m_implP->m_dimensions.end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::const_iterator DimensionOrg::begin () const                     
    {
    return cbegin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::const_iterator DimensionOrg::end () const
    {
    return cend();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::iterator DimensionOrg::begin  ()                           
    {
    m_implP->InvalidateTypeSize();
    return &(*m_implP->m_dimensions.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg::iterator DimensionOrg::end ()                           
    {
    m_implP->InvalidateTypeSize();
    return &(*m_implP->m_dimensions.end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionOrg::push_back (const DimensionDef& pi_rDimension)
    {
    m_implP->InvalidateTypeSize();
    m_implP->m_dimensions.push_back(pi_rDimension);
    }  

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionOrg::Impl::ComputeTypeSize () const
    {
    size_t TotalSize = 0;
    std::transform(m_dimensions.begin(), m_dimensions.end(), ImagePP::AccumulateIter(TotalSize), 
                   std::mem_fun_ref(&DimensionDef::GetTypeSize));
    return TotalSize;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionOrg::GetSize () const                     
    {
    return m_implP->m_dimensions.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionOrg::GetTypeSize () const                     
    {
    return m_implP->GetTypeSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionOrg::IsPOD () const
    {
    return end() == std::find_if(begin(), end(), not1(std::mem_fun_ref(&DimensionDef::IsNative)));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionOrg::IsComplete () const
    {
    return end() == std::find_if(begin(), end(), not1(std::mem_fun_ref(&DimensionDef::IsComplete)));
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                   Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct DimensionOrgGroup::Impl
    {
    static const size_t                 UNDEFINED_TYPE_SIZE;

    bvector<DimensionOrg>               m_separateOrgs;
    mutable size_t                      m_typeSize;


    explicit                            Impl                           (size_t              pi_Capacity)
        :   m_typeSize(0)  
        {
        m_separateOrgs.reserve(pi_Capacity);
        }

    explicit                            Impl                           (const_iterator  pi_Begin, 
                                                                        const_iterator  pi_End)
        :   m_separateOrgs(pi_Begin, pi_End),
            m_typeSize(ComputeTypeSize())
        {
        }

    void                                InvalidateTypeSize             ()
        {
        m_typeSize = UNDEFINED_TYPE_SIZE;
        }

    size_t                              GetTypeSize                    () const              
        {
        return (UNDEFINED_TYPE_SIZE == m_typeSize) ? m_typeSize = ComputeTypeSize() : m_typeSize;
        }

    size_t                              ComputeTypeSize                () const;

    

    };


const size_t DimensionOrgGroup::Impl::UNDEFINED_TYPE_SIZE = (std::numeric_limits<size_t>::max)();



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionOrgGroup::Impl::ComputeTypeSize () const                     
    {
    size_t TotalSize = 0;
    std::transform(m_separateOrgs.begin(), m_separateOrgs.end(), ImagePP::AccumulateIter(TotalSize), 
                   std::mem_fun_ref(&DimensionOrg::GetTypeSize));
    return TotalSize;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::DimensionOrgGroup (size_t pi_Capacity) 
    :   m_implP(new Impl(pi_Capacity))          
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::DimensionOrgGroup   (const_iterator  pi_Begin, 
                                        const_iterator  pi_End)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::~DimensionOrgGroup ()
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::DimensionOrgGroup (const DimensionOrgGroup& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {

    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup& DimensionOrgGroup::operator= (const DimensionOrgGroup& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrg& DimensionOrgGroup::operator[] (size_t      pi_Idx) 
    {
    m_implP->InvalidateTypeSize();
    return m_implP->m_separateOrgs[pi_Idx];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const DimensionOrg& DimensionOrgGroup::operator[] (size_t pi_Idx) const   
    {
    return m_implP->m_separateOrgs[pi_Idx];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::iterator DimensionOrgGroup::begin ()    
    {
    m_implP->InvalidateTypeSize(); 
    return &(*m_implP->m_separateOrgs.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::iterator DimensionOrgGroup::end ()                        
    {
    m_implP->InvalidateTypeSize();
    return &(*m_implP->m_separateOrgs.end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::const_iterator DimensionOrgGroup::cbegin () const                     
    {
    return &(*m_implP->m_separateOrgs.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::const_iterator DimensionOrgGroup::cend () const                     
    {
    return &(*m_implP->m_separateOrgs.end());
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::const_iterator DimensionOrgGroup::begin () const                     
    {
    return cbegin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DimensionOrgGroup::const_iterator DimensionOrgGroup::end () const                     
    {
    return cend();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void DimensionOrgGroup::push_back (const DimensionOrg& pi_rOrg)
    {
    m_implP->InvalidateTypeSize();
    m_implP->m_separateOrgs.push_back(pi_rOrg);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionOrgGroup::GetSize () const                     
    {
    return m_implP->m_separateOrgs.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DimensionOrgGroup::GetTypeSize () const              
    {
    return m_implP->GetTypeSize();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionOrgGroup::IsPOD () const
    {
    return end() == std::find_if(begin(), end(), not1(std::mem_fun_ref(&DimensionOrg::IsPOD)));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DimensionOrgGroup::IsComplete () const
    {
    return end() == std::find_if(begin(), end(), not1(std::mem_fun_ref(&DimensionOrg::IsComplete)));
    }

END_BENTLEY_MRDTM_IMPORT_NAMESPACE