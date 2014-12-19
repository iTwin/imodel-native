//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPaletteOctreeR8G8B8.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once


/** -----------------------------------------------------------------------------
    Node class that is used in the internal tree of the
    HRPPaletteOctreeR8G8B8 class.

    @see HRPPaletteOctreeR8G8B8
    -----------------------------------------------------------------------------
*/
struct HRPPaletteOctreeR8G8B8Node
    {
    bool  IsLeaf;

    uint32_t PixelCount;

    Byte Red;
    Byte Green;
    Byte Blue;
    Byte Index;

    struct HRPPaletteOctreeR8G8B8Node* pChild[8];
    };


/** -----------------------------------------------------------------------------
    This class is used to map 24 bits RGB colors to a set of colors in a palette.
    Colors must first be added using the AddCompositeValue method. Then,
    queries can be made using the GetIndex method. This last method will return
    the closest color in the set it has been given.

    The algorithm is based on the "Octree color quantization" method by
    Ian Ashdown.

    @see HRPQuantizedPaletteR8G8B8
    -----------------------------------------------------------------------------
*/
class HRPPaletteOctreeR8G8B8
    {
public:
    // Primary methods
    _HDLLg                 HRPPaletteOctreeR8G8B8();

    _HDLLg virtual         ~HRPPaletteOctreeR8G8B8();

    _HDLLg void                AddCompositeValue(const void* pi_pValue, Byte pi_Index);

    _HDLLg void                AddRGBValue(Byte pi_Red, Byte pi_Green, Byte pi_Blue, Byte pi_Index);

    _HDLLg Byte           GetIndex(Byte pi_Red, Byte pi_Green, Byte pi_Blue) const;

    void            FlushEntries();

protected:

private:
    // Attributes
    struct HRPPaletteOctreeR8G8B8Node*   m_pTree;

    // Methods
    struct HRPPaletteOctreeR8G8B8Node*  AddColor(
        struct HRPPaletteOctreeR8G8B8Node* pi_pNode,
        Byte pi_Red,
        Byte pi_Green,
        Byte pi_Blue,
        Byte Levels,
        Byte pi_Index);

    void            DeleteTree(struct HRPPaletteOctreeR8G8B8Node* pi_pNode);
    };
