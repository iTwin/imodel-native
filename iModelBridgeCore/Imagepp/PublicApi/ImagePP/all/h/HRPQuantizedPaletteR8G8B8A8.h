//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRPQuantizedPaletteR8G8B8A8.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include "HRPQuantizedPalette.h"
#include "HRPPixelPalette.h"

BEGIN_IMAGEPP_NAMESPACE
struct HRPQuantizedPaletteR8G8B8A8Node
    {
    // is the node a leaf
    bool  IsLeaf;

    mutable bool PixelCountCached;

    // number of pixels represented by this node
    mutable uint32_t PixelCount;

    // sums of the colour components of the represented pixels
    uint32_t RedSum;
    uint32_t GreenSum;
    uint32_t BlueSum;
    uint32_t AlphaSum;

    // There are 16 possible childs to each node (4 bits combined)
    struct HRPQuantizedPaletteR8G8B8A8Node* pChild[16];

    uint32_t GetTotalPixelCount() const
        {
        // Once this method is called, don't add new pixels
        // to the tree. The pixelcounts are being cached here.

        if (PixelCountCached || IsLeaf)
            {
            return PixelCount;
            }
        else
            {
            PixelCount = 0;

            for (int i = 0 ; i < 16 ; i++)
                if (pChild[i] != 0)
                    {
                    // Save some recursion
                    if (pChild[i]->IsLeaf)
                        PixelCount += pChild[i]->PixelCount;
                    else
                        PixelCount += pChild[i]->GetTotalPixelCount();
                    }

            PixelCountCached = true;

            return PixelCount;
            }
        }

    uint32_t GetTotalLeafCount() const
        {
        if (IsLeaf)
            {
            return 1;
            }
        else
            {
            uint32_t Result = 0;

            for (int i = 0 ; i < 16 ; i++)
                if (pChild[i] != 0)
                    {
                    // Save some recursion
                    if (pChild[i]->IsLeaf)
                        Result++;
                    else
                        Result += pChild[i]->GetTotalLeafCount();
                    }

            return Result;
            }
        }

    uint32_t MergeChildren()
        {
        uint32_t LeafNodesDestroyed = 0;

        if (!IsLeaf)
            {
            PixelCount = 0;
            RedSum = 0;
            GreenSum = 0;
            BlueSum = 0;
            AlphaSum = 0;

            for (int i = 0 ; i < 16 ; i++)
                {
                if (pChild[i] != 0)
                    {
                    if (pChild[i]->IsLeaf)
                        LeafNodesDestroyed++;
                    else
                        LeafNodesDestroyed += pChild[i]->MergeChildren();

                    RedSum     += pChild[i]->RedSum;
                    GreenSum   += pChild[i]->GreenSum;
                    BlueSum    += pChild[i]->BlueSum;
                    AlphaSum   += pChild[i]->AlphaSum;
                    PixelCount += pChild[i]->PixelCount;

                    delete pChild[i];
                    pChild[i] = NULL;
                    }
                }
            }

        return LeafNodesDestroyed;
        }
    };


class HRPQuantizedPaletteR8G8B8A8 : public HRPQuantizedPalette
    {
public:

    // Primary methods
    HRPQuantizedPaletteR8G8B8A8();
    HRPQuantizedPaletteR8G8B8A8(unsigned short pi_MaxEntries,
                                Byte pi_Precision);

    virtual         ~HRPQuantizedPaletteR8G8B8A8();

    virtual bool   AddCompositeValue(const void* pi_pValue,
                                      uint32_t    pi_Count=1);

    virtual unsigned short GetPalette(HRPPixelPalette* po_pPixelPalette,
                               HRPHistogram*    po_pHistogram) const;

    virtual void    FlushEntries();

    virtual void    AddIgnoreValue(const void* pi_pValue);


protected:

    HRPQuantizedPaletteR8G8B8A8(const HRPQuantizedPaletteR8G8B8A8& pi_rObj);
    HRPQuantizedPaletteR8G8B8A8& operator=(const HRPQuantizedPaletteR8G8B8A8& pi_rObj);

private:

    // Attributes

    // pointer to the tree
    struct HRPQuantizedPaletteR8G8B8A8Node*
            m_pTree;

    typedef list<HRPQuantizedPaletteR8G8B8A8Node*> NodeList;

    // number of reducible nodes (8 possible levels + the parent node)
    NodeList        m_ReducibleNodes[9];

    // number of actual leafs (maximum beeing the GetMaximum())
    uint32_t        m_LeafCount;

    // deep in the tree (8 is the maximum)
    uint32_t        m_Precision;

    vector<uint32_t, allocator<uint32_t> >
    m_IgnoreValues;

    // Methods
    struct HRPQuantizedPaletteR8G8B8A8Node*
    AddColor(
        struct HRPQuantizedPaletteR8G8B8A8Node* pi_pNode,
        Byte pi_Red,
        Byte pi_Green,
        Byte pi_Blue,
        Byte pi_Alpha,
        Byte Levels,
        uint32_t Count);

    uint32_t        GetColors( const struct HRPQuantizedPaletteR8G8B8A8Node* pTree,
                               HRPPixelPalette*                              po_pPixelPalette,
                               uint32_t                                      EntryIndex,
                               HRPHistogram*                                 po_pHistogram) const;

    void            PairLeaves(HRPQuantizedPaletteR8G8B8A8Node* pi_pNode);

    void            DeleteTree(struct HRPQuantizedPaletteR8G8B8A8Node* pi_pNode);

    void            ReduceNextNode();
    };
END_IMAGEPP_NAMESPACE
