//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrp/src/HRPQuantizedPaletteR8G8B8.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRPQuantizedPaletteR8G8B8.h>
#include <Imagepp/all/h/HRPHistogram.h>


/**----------------------------------------------------------------------------
   Constructor for HRPQuantizedPaletteR8G8B8
   ----------------------------------------------------------------------------
*/
HRPQuantizedPaletteR8G8B8::HRPQuantizedPaletteR8G8B8()
    : HRPQuantizedPalette()
    {
    m_Precision = 0;
    m_pTree = 0;
    for(int32_t i=0; i < 9; i++)
        m_ReducibleNodes[i].clear();
    m_LeafCount = 0;
    }


/**----------------------------------------------------------------------------
   Constructor for HRPQuantizedPaletteR8G8B8
   ----------------------------------------------------------------------------
*/
HRPQuantizedPaletteR8G8B8::HRPQuantizedPaletteR8G8B8(unsigned short pi_MaxEntries,
                                                     Byte pi_Precision)
    : HRPQuantizedPalette(pi_MaxEntries)
    {
    m_Precision = pi_Precision;

    m_pTree = NULL;

    // reset to NULL reducible nodes
    for(int i=0; i < 9; i++)
        m_ReducibleNodes[i].clear();

    m_LeafCount = 0;
    }


/**----------------------------------------------------------------------------
   Destructor for HRPQuantizedPaletteR8G8B8
   ----------------------------------------------------------------------------
*/
HRPQuantizedPaletteR8G8B8::~HRPQuantizedPaletteR8G8B8()
    {
    // delete the octree
    if(m_pTree)
        DeleteTree(m_pTree);
    }

/**----------------------------------------------------------------------------
   Add a composite value to the quantized palette with an weight associated

   @param pi_pValue A composite value.
   @param pi_Count  Weight of the color (normally a value from an historgram).
   ----------------------------------------------------------------------------
*/
bool HRPQuantizedPaletteR8G8B8::AddCompositeValue(const void* pi_pValue,
                                                   uint32_t    pi_Count)
    {
    HPRECONDITION(pi_pValue != 0);
    bool   Ret = true;

    // Check if the color is in the ignore colors list
    //
    size_t NbIgnoreValues = m_IgnoreValues.size();
    int  DontSkipTheValue(true);
    if (NbIgnoreValues > 0)
        {
        if (NbIgnoreValues == 1)
            {
            // 0 --> values are equal and false
            DontSkipTheValue = memcmp(&(m_IgnoreValues[0]), pi_pValue, 3);
            }
        else
            {
            for (size_t i=0; i<NbIgnoreValues; i++)
                {
                if (memcmp(&(m_IgnoreValues[0]), pi_pValue, 3) == 0)
                    {
                    NbIgnoreValues     = 0;     // Stop the iteration and skip the color
                    DontSkipTheValue   = false;
                    }
                }
            }
        }

    // Skip the color ?
    if(DontSkipTheValue)
        {

        // extract the R, G and B values
        Byte Red     = ((Byte*)pi_pValue)[0];
        Byte Green   = ((Byte*)pi_pValue)[1];
        Byte Blue    = ((Byte*)pi_pValue)[2];

        // add the color in the quantized palette
        if(m_pTree = AddColor(m_pTree,Red,Green,Blue,0, pi_Count))
            {
            // if there are too much entries, we reduce the octree
            while(m_LeafCount > 262144)
                {
                // find the deepest level containing at least one reducible node
                uint32_t i;
                for(i = m_Precision;
                    (i > 0) && (m_ReducibleNodes[i].size() == 0);
                    i--);

                NodeList::iterator SelectedItr(m_ReducibleNodes[i].begin());
                HASSERT(SelectedItr != m_ReducibleNodes[i].end());

                HRPQuantizedPaletteR8G8B8Node* pNode = *SelectedItr;
                HASSERT(pNode != 0);

                // REMOVE the node from the reducible list HERE
                m_ReducibleNodes[i].erase(SelectedItr);

                m_LeafCount -= pNode->MergeChildren();

                pNode->IsLeaf = true;

                m_LeafCount++;
                }
            }
        else
            {
            Ret = false;
            }
        }

    return Ret;
    }

/**----------------------------------------------------------------------------
   Use this method to ignore some colors in the QuantizePalette, the colors
   set will be ignore by the AddCompositeValue.
   Useful, when the application lock an entry with a specify color.

   @param pi_pValue A composite value to ignore.
   ----------------------------------------------------------------------------
*/
void HRPQuantizedPaletteR8G8B8::AddIgnoreValue (const void* pi_pValue)
    {
    uint32_t Value;
    memcpy(&Value, pi_pValue, 3);
    m_IgnoreValues.push_back(Value);
    }


/**----------------------------------------------------------------------------
   Pair leaves under a node to reduce the total number of leaves to the
   maximum allowed. This method is only used when reducing the node completely
   would remove too many colors.
   ----------------------------------------------------------------------------
*/
void HRPQuantizedPaletteR8G8B8::PairLeaves(HRPQuantizedPaletteR8G8B8Node* pi_pNode)
    {
    size_t i = 0;

    // First step : pair least significant bits (blue)
    while (i < 7 && m_LeafCount > GetMaxEntries())
        {
        if (pi_pNode->pChild[i] != 0)
            {
            if (!pi_pNode->pChild[i]->IsLeaf)
                {
                m_LeafCount -= pi_pNode->pChild[i]->MergeChildren();
                m_LeafCount++;
                pi_pNode->pChild[i]->IsLeaf = true;
                }

            if (pi_pNode->pChild[i+1] != 0)
                {
                // Make sure we merge two leaves.
                if (!pi_pNode->pChild[i+1]->IsLeaf)
                    {
                    m_LeafCount -= pi_pNode->pChild[i+1]->MergeChildren();
                    m_LeafCount++;
                    pi_pNode->pChild[i+1]->IsLeaf = true;
                    }

                pi_pNode->pChild[i]->RedSum       += pi_pNode->pChild[i+1]->RedSum;
                pi_pNode->pChild[i]->GreenSum     += pi_pNode->pChild[i+1]->GreenSum;
                pi_pNode->pChild[i]->BlueSum      += pi_pNode->pChild[i+1]->BlueSum;
                pi_pNode->pChild[i]->PixelCount   += pi_pNode->pChild[i+1]->PixelCount;

                delete pi_pNode->pChild[i+1];
                pi_pNode->pChild[i+1] = NULL;
                m_LeafCount--;
                }
            }
        else
            {
            if (pi_pNode->pChild[i+1] != 0)
                {
                // Child 'i' is null, but the pairing node isn't. Invert the two nodes
                // so that subsequent steps find the useful node at the good
                // position i.e.: even positions.
                pi_pNode->pChild[i] = pi_pNode->pChild[i+1];
                pi_pNode->pChild[i+1] = 0;

                if (!pi_pNode->pChild[i]->IsLeaf)
                    {
                    m_LeafCount -= pi_pNode->pChild[i]->MergeChildren();
                    m_LeafCount++;
                    pi_pNode->pChild[i]->IsLeaf = true;
                    }
                }
            }

        i += 2;
        }

    // Second step : pair reds.
    i = 0;
    while (i < 5 && m_LeafCount > GetMaxEntries())
        {
        // Here, we're sure that we have leaves because of the
        // first pairing loop.

        if (pi_pNode->pChild[i] != 0)
            {
            if (pi_pNode->pChild[i+2] != 0)
                {
                HASSERT(pi_pNode->pChild[i]->IsLeaf);
                HASSERT(pi_pNode->pChild[i+2]->IsLeaf);

                pi_pNode->pChild[i]->RedSum       += pi_pNode->pChild[i+2]->RedSum;
                pi_pNode->pChild[i]->GreenSum     += pi_pNode->pChild[i+2]->GreenSum;
                pi_pNode->pChild[i]->BlueSum      += pi_pNode->pChild[i+2]->BlueSum;
                pi_pNode->pChild[i]->PixelCount   += pi_pNode->pChild[i+2]->PixelCount;

                delete pi_pNode->pChild[i+2];
                pi_pNode->pChild[i+2] = NULL;
                m_LeafCount--;
                }
            }

        i += 4;
        }

    HASSERT((m_LeafCount - pi_pNode->GetTotalLeafCount() + 1) < GetMaxEntries());
    }


/**----------------------------------------------------------------------------
   Clear the contents of the quantized palette.
   ----------------------------------------------------------------------------
*/
void HRPQuantizedPaletteR8G8B8::FlushEntries()
    {
    // delete the current octree if there is one
    if(m_pTree)
        DeleteTree(m_pTree);

    m_pTree = NULL;

    // reset to NULL reducible nodes
    for(int i=0; i < 9; i++)
        m_ReducibleNodes[i].clear();

    m_LeafCount = 0;
    }


/**----------------------------------------------------------------------------
   Fill a pixel palette from the quantized object

  @param po_pPixelPalette The palette to fill with the most representative colors.
  @param po_pHistogram An optional histogram to fill with the color counts.
         Can be NULL.
   ----------------------------------------------------------------------------
*/
unsigned short HRPQuantizedPaletteR8G8B8::GetPalette(HRPPixelPalette*  po_pPixelPalette,
                                              HRPHistogram*     po_pHistogram) const
    {

    HPRECONDITION(po_pPixelPalette != 0);
    // we verify that the maximum number of entries is equal to the number
    // of entries in the palette
    HPRECONDITION((po_pPixelPalette->GetLockedEntryIndex() == -1 && po_pPixelPalette->CountUsedEntries() >= GetMaxEntries()) ||
                  (po_pPixelPalette->GetLockedEntryIndex() >= 0 && po_pPixelPalette->CountUsedEntries() > GetMaxEntries()));

    uint32_t EntryIndex=0;

    // if there are too much entries, we reduce the octree
    while(m_LeafCount > GetMaxEntries())
        {
        const_cast<HRPQuantizedPaletteR8G8B8*>(this)->ReduceNextNode();
        }

    // we get the colors of the tree
    if(m_pTree)
        EntryIndex = GetColors(m_pTree, po_pPixelPalette, EntryIndex, po_pHistogram);

    return (unsigned short)m_LeafCount;
    }


/**----------------------------------------------------------------------------
   Extract the colors of the octree and put them in the pixel palette
   ----------------------------------------------------------------------------
*/
uint32_t HRPQuantizedPaletteR8G8B8::GetColors(
    const struct HRPQuantizedPaletteR8G8B8Node*
    pi_pTree,
    HRPPixelPalette*   po_pPixelPalette,
    uint32_t           pi_EntryIndex,
    HRPHistogram*      po_pHistogram) const
    {
    // if the tree is a lead, we get the color
    if(pi_pTree->IsLeaf)
        {
        // we get the average color
        Byte Red      = (Byte)(pi_pTree->RedSum / pi_pTree->PixelCount);
        Byte Green    = (Byte)(pi_pTree->GreenSum / pi_pTree->PixelCount);
        Byte Blue     = (Byte)(pi_pTree->BlueSum / pi_pTree->PixelCount);
        Byte Color[4];
        Color[0] = Red;
        Color[1] = Green;
        Color[2] = Blue;
        Color[3] = 0xff; // sometimes, we hava alpha (put total opacity)
        int32_t Entry = pi_EntryIndex;

        // Check if one index is locked
        //
        int32_t LockEntryIndex = po_pPixelPalette->GetLockedEntryIndex();
        if (LockEntryIndex != -1)
            {
            // if the current color is equal to the Lock entry, set the Lock entry
            //                                                  with the same value.
            // else try to put this color in the palette.
            if (memcmp(po_pPixelPalette->GetCompositeValue(LockEntryIndex), Color,
                       po_pPixelPalette->GetPixelEntrySize()) == 0)
                {
                Entry = LockEntryIndex;

                // If the Lock entry, is not the same as the current index, don't increment
                // the index, we skip it later.
                if (((int32_t)pi_EntryIndex) != LockEntryIndex)
                    pi_EntryIndex--;
                }
            else
                {
                // test if we must not write in the current index (if it is a default)
                if(((int32_t)pi_EntryIndex) == LockEntryIndex)
                    Entry = ++pi_EntryIndex;
                }
            }

        // we set the color in the pixel palette
        po_pPixelPalette->SetCompositeValue(Entry,Color);

        // set the appropriate histogram entry if there is one
        if(po_pHistogram != 0)
            po_pHistogram->SetEntryCount(Entry, pi_pTree->PixelCount);

        // increment the index
        pi_EntryIndex++;
        }
    else
        {
        // get the colors for each child tree
        for(uint32_t i = 0; i < 8; i++)
            if(pi_pTree->pChild[i] != NULL)
                pi_EntryIndex = GetColors(  pi_pTree->pChild[i],
                                            po_pPixelPalette,
                                            pi_EntryIndex,
                                            po_pHistogram);

        }

    return pi_EntryIndex;
    }


/**----------------------------------------------------------------------------
   Add a color in the tree. New nodes will be created as necessary.
   ----------------------------------------------------------------------------
*/
struct HRPQuantizedPaletteR8G8B8Node* HRPQuantizedPaletteR8G8B8::AddColor(
    struct HRPQuantizedPaletteR8G8B8Node* pi_pNode,
    Byte pi_Red,
    Byte pi_Green,
    Byte pi_Blue,
    Byte pi_Levels,
    uint32_t pi_Count)
    {

    static Byte Mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    // if the node is null
    if(!pi_pNode)
        {

        // we allocate a new node
        pi_pNode = new HRPQuantizedPaletteR8G8B8Node;
        HASSERT(pi_pNode != 0);

        // we reset the elements
#if 0
        pi_pNode->PixelCount = 0;
        pi_pNode->RedSum = 0;
        pi_pNode->GreenSum = 0;
        pi_pNode->BlueSum = 0;
        pi_pNode->IsLeaf = false;
        for(int i=0; i<8; i++)
            pi_pNode->pChild[i]=NULL;
#else
        memset(pi_pNode, 0, sizeof(HRPQuantizedPaletteR8G8B8Node));
#endif

        // if we reach the maximum level
        if(pi_Levels == m_Precision)
            {
            // we declare the node as a leaf
            pi_pNode->IsLeaf = true;

            // we increment the number of leaves
            m_LeafCount++;
            }
        }

    // is the node a leaf
    if(pi_pNode->IsLeaf)
        {
        // if yes, update the content

        if(pi_Count > (ULONG_MAX / 255))
            pi_Count = ULONG_MAX / 255;

        if(pi_pNode->PixelCount < ((ULONG_MAX / 255) - pi_Count))
            pi_pNode->PixelCount += pi_Count;
        else
            pi_Count = ((ULONG_MAX / 255) - pi_pNode->PixelCount);

        pi_pNode->RedSum += (pi_Red * pi_Count);
        pi_pNode->GreenSum += (pi_Green * pi_Count);
        pi_pNode->BlueSum += (pi_Blue * pi_Count);
        }
    else
        {
        // if not, we recurse a level deeper
        Byte Shift = 7 - pi_Levels;
        Byte Index =  (((pi_Red   & Mask[pi_Levels])  >> Shift) << 1) |
                        (((pi_Green & Mask[pi_Levels])  >> Shift) << 2) |
                        (((pi_Blue  & Mask[pi_Levels])  >> Shift) << 0);

        int Children=0;
        for(int i=0; i < 8; i++)
            {
            if(pi_pNode->pChild[i] != NULL)
                Children++;
            }
        bool ChildExists = (pi_pNode->pChild[Index] != 0);

        pi_pNode->pChild[Index] = AddColor( pi_pNode->pChild[Index],
                                            pi_Red,
                                            pi_Green,
                                            pi_Blue,
                                            pi_Levels + 1,
                                            pi_Count);

        // A node is reducible if it has at least
        // two children.
        if (Children == 1 && !ChildExists)
            {
            m_ReducibleNodes[pi_Levels].push_back(pi_pNode);
            }
        }

    return pi_pNode;
    }


/**----------------------------------------------------------------------------
   Delete a tree.
   ----------------------------------------------------------------------------
*/
void HRPQuantizedPaletteR8G8B8::DeleteTree(
    struct HRPQuantizedPaletteR8G8B8Node* pNode)
    {
    // we delete each child of the tree
    for(int i=0; i < 8; i++)
        {
        if(pNode->pChild[i])
            DeleteTree(pNode->pChild[i]);
        }

    // free the node
    delete pNode;
    }


/**----------------------------------------------------------------------------
   Reduce the next logical node, trying to bring the color count down to
   GetMaxEntries(). We try to reduce nodes that are the lowest in the tree,
   i.e. nodes that are currently the most precise. If we're far from our
   color count, we reduce the first one we find. The last nodes are chosen
   to reduce the color regions that are used by the least number of pixels.
   ----------------------------------------------------------------------------
*/
void HRPQuantizedPaletteR8G8B8::ReduceNextNode()
    {
    uint32_t i;
    // find the deepest level containing at least one reducible node
    for(i = m_Precision;
        (i > 0) && (m_ReducibleNodes[i].size() == 0);
        i--);

    NodeList::iterator CurrentItr(m_ReducibleNodes[i].begin());
    NodeList::iterator SelectedItr = m_ReducibleNodes[i].end();

    if (m_LeafCount < (uint32_t)(GetMaxEntries() << 3))
        {
        uint32_t CurrentPixelCount = ULONG_MAX;
        while (CurrentItr != m_ReducibleNodes[i].end())
            {
            uint32_t PixelCount = (*CurrentItr)->GetTotalPixelCount();

            if (PixelCount < CurrentPixelCount)
                {
                SelectedItr = CurrentItr;
                CurrentPixelCount = PixelCount;
                }
            ++CurrentItr;
            }
        }
    else
        {
        // We have so many colors to remove, we might as well merge
        // the first node we find.
        SelectedItr = CurrentItr;
        }

    if (SelectedItr != m_ReducibleNodes[i].end())
        {
        HRPQuantizedPaletteR8G8B8Node* pNode = *SelectedItr;
        HASSERT(pNode != 0);

        // REMOVE the node from the reducible list HERE
        m_ReducibleNodes[i].erase(SelectedItr);

        if ((m_LeafCount - pNode->GetTotalLeafCount() + 1) < GetMaxEntries())
            {
            PairLeaves(pNode);
            }
        else
            {
            m_LeafCount -= pNode->MergeChildren();

            pNode->IsLeaf = true;

            m_LeafCount++;
            }
        }
    }
