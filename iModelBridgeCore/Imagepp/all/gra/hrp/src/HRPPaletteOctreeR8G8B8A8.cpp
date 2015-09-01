//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPPaletteOctreeR8G8B8A8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPPaletteOctreeR8G8B8A8.h>

HRPPaletteOctreeR8G8B8A8 g_PaletteOctreeR8G8B8A8;


/**----------------------------------------------------------------------------
   Constructor for HRPPaletteOctreeR8G8B8A8
   ----------------------------------------------------------------------------
*/
HRPPaletteOctreeR8G8B8A8::HRPPaletteOctreeR8G8B8A8()
    {
    m_pTree = 0;
    }


/**----------------------------------------------------------------------------
   Destructor for HRPPaletteOctreeR8G8B8A8
   ----------------------------------------------------------------------------
*/
HRPPaletteOctreeR8G8B8A8::~HRPPaletteOctreeR8G8B8A8()
    {
    // delete the octree
    if(m_pTree)
        DeleteTree(m_pTree);
    }


/**----------------------------------------------------------------------------
   Add a composite value to the quantized palette with an index associated.
   ----------------------------------------------------------------------------
*/
void HRPPaletteOctreeR8G8B8A8::AddCompositeValue(const void* pi_pValue,
                                                 Byte      pi_Index)
    {
    HPRECONDITION(pi_pValue != 0);

    // extract the R, G and B values
    Byte Red     = ((Byte*)pi_pValue)[0];
    Byte Green   = ((Byte*)pi_pValue)[1];
    Byte Blue    = ((Byte*)pi_pValue)[2];
    Byte Alpha   = ((Byte*)pi_pValue)[3];

    // add the color in the quantized palette
    m_pTree = AddColor(m_pTree,Red,Green,Blue,Alpha,0,pi_Index);
    HASSERT(m_pTree != 0);
    }


/**----------------------------------------------------------------------------
   Add a color in the tree
   ----------------------------------------------------------------------------
*/
struct HRPPaletteOctreeR8G8B8A8Node* HRPPaletteOctreeR8G8B8A8::AddColor(
    struct HRPPaletteOctreeR8G8B8A8Node* pi_pNode,
    Byte pi_Red,
    Byte pi_Green,
    Byte pi_Blue,
    Byte pi_Alpha,
    Byte pi_Levels,
    Byte pi_Index)
    {

    static const Byte Mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    // if the node is null
    if(!pi_pNode)
        {
        // we allocate a new node
        if(!(pi_pNode = new HRPPaletteOctreeR8G8B8A8Node))
            return 0;

        // we reset the elements
        pi_pNode->Red = pi_Red;
        pi_pNode->Green = pi_Green;
        pi_pNode->Blue = pi_Blue;
        pi_pNode->Alpha = pi_Alpha;
        pi_pNode->IsLeaf = true;
        pi_pNode->Index = pi_Index;

        for(int i=0; i<16; i++)
            pi_pNode->pChild[i] = 0;
        }
    else
        {
        // is it a leaf
        if(pi_pNode->IsLeaf)
            {
            // is it the same color?
            if( pi_Red == pi_pNode->Red &&
                pi_Green == pi_pNode->Green &&
                pi_Blue == pi_pNode->Blue &&
                pi_Alpha == pi_pNode->Alpha)
                return pi_pNode; // do nothing

            pi_pNode->IsLeaf = false;

            // redistribute the actual color
            Byte Shift = 7 - pi_Levels;
            Byte Index =  (((pi_pNode->Red   & Mask[pi_Levels])  >> Shift) << 1) |
                            (((pi_pNode->Green & Mask[pi_Levels])  >> Shift) << 2) |
                            (((pi_pNode->Blue  & Mask[pi_Levels])  >> Shift) << 0) |
                            (((pi_pNode->Alpha & Mask[pi_Levels])  >> Shift) << 3) ;
            pi_pNode->pChild[Index] = AddColor( pi_pNode->pChild[Index],
                                                pi_pNode->Red,
                                                pi_pNode->Green,
                                                pi_pNode->Blue,
                                                pi_pNode->Alpha,
                                                pi_Levels + 1,
                                                pi_pNode->Index);
            }


        Byte Shift = 7 - pi_Levels;
        Byte Index =  (((pi_Red   & Mask[pi_Levels])  >> Shift) << 1) |
                        (((pi_Green & Mask[pi_Levels])  >> Shift) << 2) |
                        (((pi_Blue  & Mask[pi_Levels])  >> Shift) << 0) |
                        (((pi_Alpha & Mask[pi_Levels])  >> Shift) << 3) ;
        pi_pNode->pChild[Index] = AddColor( pi_pNode->pChild[Index],
                                            pi_Red,
                                            pi_Green,
                                            pi_Blue,
                                            pi_Alpha,
                                            pi_Levels + 1,
                                            pi_Index);
        }

    return pi_pNode;
    }


/**----------------------------------------------------------------------------
   Delete a tree
   ----------------------------------------------------------------------------
*/
void HRPPaletteOctreeR8G8B8A8::DeleteTree(
    struct HRPPaletteOctreeR8G8B8A8Node* pNode)
    {
    // we delete each child of the tree
    for(int i=0; i < 16; i++)
        {
        if(pNode->pChild[i])
            DeleteTree(pNode->pChild[i]);
        }

    // free the node
    delete pNode;
    }


/**----------------------------------------------------------------------------
   Get a good representative index for the R, G, B, A values
   ----------------------------------------------------------------------------
*/
Byte HRPPaletteOctreeR8G8B8A8::GetIndex(    Byte pi_Red,
                                              Byte pi_Green,
                                              Byte pi_Blue,
                                              Byte pi_Alpha) const
    {
    // We get a color in the tree "near" the color used in parameter.
    // This is not necessary the nearer color, but an approximation.
    // We take the RGBA color and parse the tree like if we were including
    // a new color in the tree. If a child is null, we search the nearer
    // non-null child using the distance algorithm at this level.

    Byte Level = 0;
    HRPPaletteOctreeR8G8B8A8Node* pNode = m_pTree;
    static const Byte Mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    Byte Shift;
    Byte ChildIndex;
    Byte BlueBit;
    Byte GreenBit;
    Byte RedBit;
    Byte AlphaBit;
    int8_t TmpBlueBit;
    int8_t TmpGreenBit;
    int8_t TmpRedBit;
    int8_t TmpAlphaBit;
    int8_t TmpChildIndex;
    Byte Dist;
    Byte MinDist;
    Byte MinBlueBit=0;
    Byte MinGreenBit=0;
    Byte MinRedBit=0;
    Byte MinAlphaBit=0;

    // recurse in the octree until we are at the lowest level
    while(!pNode->IsLeaf)
        {
        HASSERT(Level <= 7);

        // if not, we recurse a level deeper
        Shift = 7 - Level;

        BlueBit  = (pi_Blue  & Mask[Level])  >> Shift;
        GreenBit = (pi_Green & Mask[Level])  >> Shift;
        RedBit   = (pi_Red & Mask[Level])  >> Shift;
        AlphaBit = (pi_Alpha & Mask[Level])  >> Shift;

        ChildIndex = (RedBit << 1) |
                     (GreenBit << 2) |
                     (BlueBit << 0) |
                     (AlphaBit << 3);

        // verify if the child with that index exists
        if(pNode->pChild[ChildIndex] == NULL)
            {
            // if not, find the closest child using the distance
            // algorithm at this level

            MinDist = 5;

            TmpChildIndex = 15;

            for (TmpAlphaBit = 1 ; TmpAlphaBit >= 0 ; TmpAlphaBit--)
                {
                for(TmpGreenBit = 1; TmpGreenBit >= 0; TmpGreenBit--)
                    {
                    for(TmpRedBit = 1; TmpRedBit >= 0; TmpRedBit--)
                        {
                        for(TmpBlueBit = 1; TmpBlueBit >= 0; TmpBlueBit--)
                            {
                            if(pNode->pChild[TmpChildIndex] != NULL)
                                {
                                Dist = 0;

                                if(TmpAlphaBit != AlphaBit)
                                    Dist++;

                                if(TmpBlueBit != BlueBit)
                                    Dist++;

                                if(TmpGreenBit != GreenBit)
                                    Dist++;

                                if(TmpRedBit != RedBit)
                                    Dist++;

                                if(Dist < MinDist)
                                    {
                                    MinDist = Dist;
                                    ChildIndex = TmpChildIndex;
                                    MinBlueBit = TmpBlueBit;
                                    MinGreenBit = TmpGreenBit;
                                    MinRedBit = TmpRedBit;
                                    MinAlphaBit = TmpAlphaBit;
                                    }
                                }

                            TmpChildIndex--;
                            }
                        }
                    }
                }

            if(MinBlueBit < BlueBit)
                {
                pi_Blue = 0xff;
                }
            else if(MinBlueBit > BlueBit)
                {
                pi_Blue = 0x00;
                }

            if(MinRedBit < RedBit)
                {
                pi_Red = 0xff;
                }
            else if(MinRedBit > RedBit)
                {
                pi_Red = 0x00;
                }

            if(MinGreenBit < GreenBit)
                {
                pi_Green = 0xff;
                }
            else if(MinGreenBit > GreenBit)
                {
                pi_Green = 0x00;
                }

            if(MinAlphaBit < AlphaBit)
                {
                pi_Alpha = 0xff;
                }
            else if(MinAlphaBit > AlphaBit)
                {
                pi_Alpha = 0x00;
                }
            }

        pNode = pNode->pChild[ChildIndex];

        Level++;
        }

    // return the index associated to the composite value
    return(pNode->Index);
    }


/**----------------------------------------------------------------------------
   Clear the entries.
   ----------------------------------------------------------------------------
*/
void HRPPaletteOctreeR8G8B8A8::FlushEntries()
    {
    // delete the current octree if there is one
    if(m_pTree)
        DeleteTree(m_pTree);

    m_pTree = NULL;
    }
