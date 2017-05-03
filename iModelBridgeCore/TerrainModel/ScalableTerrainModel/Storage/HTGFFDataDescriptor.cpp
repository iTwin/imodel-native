//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/HTGFFDataDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/HTGFFDataDescriptor.h>

;

namespace HTGFF {

const Dimension::Role Dimension::UNDEFINED_ROLE = (numeric_limits<Dimension::Role>::max) ();
const size_t Dimension::s_TypeSize[TYPE_QTY] = {0, 1, 1, 2, 2, 4, 4, 4, 8, 8, 8};



const Dimension::Role Dimension::GetUndefinedRole ()
    {
    return UNDEFINED_ROLE;
    }


Dimension Dimension::CreateIntFrom   (TypeSize    pi_Size,
                                      bool        pi_Signed,
                                      Role        pi_Role)
    {
    return (pi_Signed) ? CreateSIntFrom(pi_Size, pi_Role) : CreateUIntFrom(pi_Size, pi_Role);
    }

Dimension Dimension::CreateSIntFrom  (TypeSize    pi_Size,
                                      Role        pi_Role)
    {
    HPRECONDITION(pi_Size < TYPE_SIZE_QTY);
    static const Type TYPEID_FROM_TYPE_SIZE [TYPE_SIZE_QTY] = {TYPE_SINT8, TYPE_SINT8, TYPE_SINT16, TYPE_SINT32, TYPE_SINT64};

    return Dimension(TYPEID_FROM_TYPE_SIZE[pi_Size], pi_Role);
    }

Dimension Dimension::CreateUIntFrom  (TypeSize    pi_Size,
                                      Role        pi_Role)
    {
    HPRECONDITION(pi_Size < TYPE_SIZE_QTY);
    static const Type TYPEID_FROM_TYPE_SIZE [TYPE_SIZE_QTY] = {TYPE_UINT8, TYPE_UINT8, TYPE_UINT16, TYPE_UINT32, TYPE_UINT64};

    return Dimension(TYPEID_FROM_TYPE_SIZE[pi_Size], pi_Role);
    }

Dimension Dimension::CreateFloatFrom (TypeSize    pi_Size,
                                      Role        pi_Role)
    {
    HPRECONDITION(pi_Size < TYPE_SIZE_QTY && TYPE_SIZE_32 <= pi_Size);
    static const Type TYPEID_FROM_TYPE_SIZE [TYPE_SIZE_QTY] = {TYPE_FLOAT_32, TYPE_FLOAT_32, TYPE_FLOAT_32, TYPE_FLOAT_32, TYPE_FLOAT_64};

    return Dimension(TYPEID_FROM_TYPE_SIZE[pi_Size], pi_Role);
    }

Dimension Dimension::CreateFrom  (Type pi_Type,
                                  Role pi_Role)
    {
    return Dimension(pi_Type, pi_Role);
    }

const Dimension& Dimension::CreateVoid ()
    {
    static const Dimension VOID_DIMENSION(TYPE_VOID);
    return VOID_DIMENSION;
    }

const Dimension& Dimension::CreateByte ()
    {
    static const Dimension VOID_DIMENSION(TYPE_UINT8);
    return VOID_DIMENSION;
    }

const Dimension& Dimension::CreateChar ()
    {
    static const Dimension VOID_DIMENSION(TYPE_SINT8);
    return VOID_DIMENSION;
    }

const Dimension& Dimension::CreateWideChar ()
    {
    static const Dimension VOID_DIMENSION(TYPE_SINT16);
    return VOID_DIMENSION;
    }


Dimension::Dimension ()
    :   m_Type(TYPE_VOID),
        m_Role(GetUndefinedRole())
    {

    }

Dimension::Dimension (Type    pi_Type,
                      Role    pi_Role )
    :   m_Type(pi_Type),
        m_Role(pi_Role)
    {

    }

bool Dimension::operator< (const Dimension&  pi_rRight) const
    {
    if (m_Type < pi_rRight.m_Type)
        return true;
    if (m_Type > pi_rRight.m_Type)
        return false;

    if (m_Role < pi_rRight.m_Role)
        return true;

    return false;
    }

bool Dimension::operator== (const Dimension& pi_rRight) const
    {
    return m_Type == pi_rRight.m_Type && m_Role == pi_rRight.m_Role;
    }

Dimension::Type Dimension::GetType   () const
    {
    return m_Type;
    }

size_t Dimension::GetSize () const
    {
    return s_TypeSize[m_Type];
    }

Dimension::Role Dimension::GetRole () const
    {
    return m_Role;
    }

void Dimension::SetRole (Role pi_Role)
    {
    m_Role = pi_Role;
    }


const DataType& DataType::CreateVoid ()
    {
    static const DataType TYPE;
    return TYPE;
    }

const DataType& DataType::CreateByte ()
    {
    static const DataType TYPE(Dimension::CreateByte());
    return TYPE;
    }

const DataType& DataType::CreateChar ()
    {
    static const DataType TYPE(Dimension::CreateChar());
    return TYPE;
    }

const DataType& DataType::CreateWideChar ()
    {
    static const DataType TYPE(Dimension::CreateWideChar());
    return TYPE;
    }


DataType::DataType (const Dimension&      pi_Dimension)
    :   m_Dimensions(&pi_Dimension, &pi_Dimension + 1),
        m_Size(pi_Dimension.GetSize())
    {


    }


DataType::DataType   (const_iterator      pi_DimensionBegin,
                      const_iterator      pi_DimensionEnd)
    :   m_Dimensions(pi_DimensionBegin, pi_DimensionEnd),
        m_Size(0)
    {
    HPRECONDITION(0 < m_Dimensions.size());
    transform(m_Dimensions.begin(), m_Dimensions.end(), AccumulateIter(m_Size), mem_fun_ref(&Dimension::GetSize));
    }

// Structured packet data
DataType::DataType   (const Dimension::Type* pi_DimensionTypeBegin,
                      const Dimension::Type* pi_DimensionTypeEnd)
    :   m_Dimensions(pi_DimensionTypeBegin, pi_DimensionTypeEnd),
        m_Size(0)
    {
    HPRECONDITION(0 < m_Dimensions.size());
    transform(m_Dimensions.begin(), m_Dimensions.end(), AccumulateIter(m_Size), mem_fun_ref(&Dimension::GetSize));
    }

bool DataType::operator< (const DataType& pi_rRight) const
    {
    return lexicographical_compare(m_Dimensions.begin(), m_Dimensions.end(),
                                   pi_rRight.m_Dimensions.begin(), pi_rRight.m_Dimensions.end());
    }

bool DataType::operator== (const DataType& pi_rRight) const
    {
    if (GetDimensionQty() != pi_rRight.GetDimensionQty())
        return false;

    return equal(m_Dimensions.begin(), m_Dimensions.end(), pi_rRight.m_Dimensions.begin());
    }

DataType::const_iterator DataType::Begin () const
    {
    return &*m_Dimensions.begin();
    }

DataType::const_iterator DataType::End   () const
    {
    return &*m_Dimensions.end();
    }

DataType::iterator DataType::Begin   ()
    {
    return &*m_Dimensions.begin();
    }

DataType::iterator DataType::End     ()
    {
    return &*m_Dimensions.end();
    }

size_t DataType::GetDimensionQty  () const
    {
    return m_Dimensions.size();
    }

size_t DataType::GetSize  () const
    {
    return m_Size;
    }

} // END namespace HTGFF