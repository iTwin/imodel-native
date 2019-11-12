//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// HRFImportExport inline methods
//-----------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//-----------------------------------------------------------------------------
// Tag interface
// FindTagCP
//-----------------------------------------------------------------------------
template <typename AttributeT> AttributeT const* HRFImportExport::FindTagCP() const 
    {
    for (uint32_t Index=0; (Index < CountTag()); Index++)
        {
        if (AttributeT::ATTRIBUTE_ID == (*m_ListOfValidTag[Index]).GetID())
            {
            return static_cast<AttributeT const*>(m_ListOfValidTag[Index].GetPtr());
            }
        }
    return 0;
    }

//-----------------------------------------------------------------------------
// Tag interface
// FindTagP
//-----------------------------------------------------------------------------
template <typename AttributeT> AttributeT* HRFImportExport::FindTagP() 
    {
    for (uint32_t Index=0; (Index < CountTag()); Index++)
        {
        if (AttributeT::ATTRIBUTE_ID == (*m_ListOfValidTag[Index]).GetID())
            {
            return static_cast<AttributeT*>(m_ListOfValidTag[Index].GetPtr());
            }
        }
    return 0;
    }

//-----------------------------------------------------------------------------
// Public
// HasTag
// Page Tag
//-----------------------------------------------------------------------------
template <typename AttributeT> bool HRFImportExport::HasTag () const
    {
    for (uint32_t Index=0; (Index < CountTag()); Index++)
        {
        if (AttributeT::ATTRIBUTE_ID == (*m_ListOfValidTag[Index]).GetID())
            {
            return true;
            }
        }
    return false;
    }
END_IMAGEPP_NAMESPACE
