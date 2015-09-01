//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPPaletteOctreeR8G8B8A8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once


BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Node class that is used in the internal tree of the
    HRPPaletteOctreeR8G8B8A8 class.

    @see HRPPaletteOctreeR8G8B8A8
    -----------------------------------------------------------------------------
*/
struct HRPPaletteOctreeR8G8B8A8Node
    {
    bool  IsLeaf;

    uint32_t PixelCount;

    Byte Red;
    Byte Green;
    Byte Blue;
    Byte Alpha;
    Byte Index;

    struct HRPPaletteOctreeR8G8B8A8Node* pChild[16];
    };


/** -----------------------------------------------------------------------------
    This class is used to map 32 bits RGBA colors to a set of colors in a palette.
    Colors must first be added using the AddCompositeValue method. Then,
    queries can be made using the GetIndex method. This last method will return
    the closest color in the set it has been given.

    The algorithm is based on the "Octree color quantization" method by
    Ian Ashdown. Adapted to handle Alpha channel.

    @see HRPQuantizedPaletteR8G8B8A8
    -----------------------------------------------------------------------------
*/
class HRPPaletteOctreeR8G8B8A8
    {
public:
    // Primary methods
    HRPPaletteOctreeR8G8B8A8();

    virtual         ~HRPPaletteOctreeR8G8B8A8();

    void            AddCompositeValue(const void* pi_pValue, Byte pi_Index);

    Byte            GetIndex(Byte pi_Red, Byte pi_Green, Byte pi_Blue, Byte pi_Alpha) const;

    void            FlushEntries();

protected:

private:
    // Attributes
    struct HRPPaletteOctreeR8G8B8A8Node*   m_pTree;

    // Methods
    struct HRPPaletteOctreeR8G8B8A8Node*  AddColor(
        struct HRPPaletteOctreeR8G8B8A8Node* pi_pNode,
        Byte pi_Red,
        Byte pi_Green,
        Byte pi_Blue,
        Byte pi_Alpha,
        Byte Levels,
        Byte pi_Index);

    void            DeleteTree(struct HRPPaletteOctreeR8G8B8A8Node* pi_pNode);
    };
END_IMAGEPP_NAMESPACE
