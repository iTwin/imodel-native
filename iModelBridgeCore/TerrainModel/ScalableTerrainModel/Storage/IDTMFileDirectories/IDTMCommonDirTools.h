//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMCommonDirTools.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : MetadataDir
//-----------------------------------------------------------------------------

#pragma once


#include <STMInternal/Storage/IDTMTypes.h>

#include <STMInternal/Storage/HTGFFAttributeManager.h>

namespace IDTMFile {

template <typename T>
struct AttributeHelper
    {
    typedef T               UnderlyingType;

    template <typename ArrayType, typename DirectoryType>
    bool                    Load               (const DirectoryType&    pi_Dir,
                                                AttributeID             pi_ID,
                                                ArrayType&              po_rArray) const
        {
        return pi_Dir.AttributeMgr().GetPacket<UnderlyingType>(pi_ID, po_rArray.EditPacket());
        }

    template <typename ArrayType, typename DirectoryType>
    bool                    LoadIfPresent      (const DirectoryType&    pi_Dir,
                                                AttributeID             pi_ID,
                                                ArrayType&              po_rArray) const
        {
        Load(pi_Dir, pi_ID, po_rArray);
        return true; // Alway return true as the array may not be present
        }



    template <typename ArrayType, typename DirectoryType>
    bool                    Save               (DirectoryType&          pio_Dir,
                                                AttributeID             pi_ID,
                                                const ArrayType&        pi_rArray) const
        {
        return pio_Dir.AttributeMgr().SetPacket<UnderlyingType>(pi_ID, pi_rArray.GetPacket());
        }

    template <typename ArrayType, typename DirectoryType>
    bool                    SaveIfChanged      (DirectoryType&          pio_Dir,
                                                AttributeID             pi_ID,
                                                const ArrayType&        pi_rArray) const
        {
        if (!pi_rArray.GetPacket().IsBufferOwner() || pi_rArray.GetSize() == 0) // Save only if packet was modified
            return true;

        return Save(pio_Dir, pi_ID, pi_rArray);
        }
    };




struct ArrayHelper
    {
    static const size_t MIN_CAPACITY = 1024;

    /*---------------------------------------------------------------------------------**//**
    * @description  Return an array index checking for array out of bounds. Will returns
    *               pi_DefaultValue if array is out of bounds.
    * @bsimethod                                                  Raymond.Gauthier   4/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename ArrayType>
    typename const ArrayType::value_type&
    GetIndex               (const ArrayType&                        pi_rArray,
                            size_t                                  pi_Index,
                            typename const ArrayType::value_type&   pi_DefaultValue = ArrayType::value_type()) const
        {
        if (pi_Index >= pi_rArray.GetSize())
            {
            HPRECONDITION(!"Array index out of bounds!");
            return pi_DefaultValue;
            }

        return pi_rArray.Get()[pi_Index];
        }

    /*---------------------------------------------------------------------------------**//**
    * @description  Return an array index to edit expanding the array so that its new size
    *               contains requested index.  When array is expanded, all new elements
    *               are initialized to pi_DefaultValue. If pi_NewMinCapacity is set,
    *               internal buffer will be expanded to at least this specified capacity
    *               (when resized). Otherwise, array's capacity will be expended by
    *               EXTRA_CAPACITY (when resized).
    * @bsimethod                                                  Raymond.Gauthier   4/2010
    +---------------+---------------+---------------+---------------+---------------+------*/
    template <typename ArrayType>
    typename ArrayType::value_type&
    EditIndex              (ArrayType&                              pi_rArray,
                            size_t                                  pi_Index,
                            typename const ArrayType::value_type&   pi_DefaultValue = ArrayType::value_type(),
                            size_t                                  pi_NewMinCapacity = 0) const
        {
        if (pi_Index >= pi_rArray.GetSize())
            {
            const size_t NewSize = pi_Index + 1;

            if (pi_Index >= pi_rArray.GetCapacity())
                {
                size_t NewCapacity = (std::max)(pi_NewMinCapacity, (std::max)(MIN_CAPACITY, NewSize));
                NewCapacity += NewCapacity >> 1;

                pi_rArray.Reserve(NewCapacity);
                }

            ArrayType::iterator NewDataIter = pi_rArray.Resize(NewSize);
            fill(NewDataIter, pi_rArray.EndEdit(), pi_DefaultValue);
            }

        return pi_rArray.Edit()[pi_Index];
        }

    };

} //End namespace IDTMFile