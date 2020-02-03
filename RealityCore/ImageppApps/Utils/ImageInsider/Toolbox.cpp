/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/ImageInsider/Toolbox.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//-----------------------------------------------------------------------------
// toolbox.cpp : implementation file
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "toolbox.h"
#include "afxpriv.h"  // required for A2W macro.
#include "shlobj.h"

//-----------------------------------------------------------------------------
// HGS Implementation Includes 
//-----------------------------------------------------------------------------
#include <Imagepp/all/h/HRPPixelConverter.h>


//-----------------------------------------------------------------------------
// Class construtor
//-----------------------------------------------------------------------------
Chronometer::Chronometer()
{
    m_ToReset           = true;
    m_IsStarted         = false;
    m_duration          = 0;
    m_Count             = 0;
    m_durationBest      = 0;
    m_durationWorst     = 0;
    m_bestIteration     = 0;
    m_worstIteration    = 0;
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

void Chronometer::Start()
{
    m_start = clock();

    if (m_ToReset)
    {
        m_startTime   = time(NULL);
        m_duration    = 0;
        m_ToReset     = false;
        m_Count       = 0;
    }

    ++m_Count;
    m_IsStarted   = true;
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::Stop()
{
   if (m_IsStarted)
   {
       m_finish = clock();
       m_finishTime = time(NULL);

       double  duration = (double)(m_finish - m_start) / CLOCKS_PER_SEC;

       // if this is the first iteration
       if (m_Count == 1)
       {
               m_durationWorst  = duration;
               m_worstIteration = m_Count;
               m_durationBest   = duration;
               m_bestIteration  = m_Count;
       }
       else
       {
           if (m_durationWorst < duration) 
           {
               m_durationWorst = duration;
               m_worstIteration = m_Count;
           }

           if (m_durationBest > duration)
           {
               m_durationBest = duration;
               m_bestIteration = m_Count;
           }
       }

       m_duration += duration;
       m_IsStarted = false;

       WChar buffer[200];
       _stprintf(buffer, _TEXT("%8.3f"), duration);

       return Utf8String(buffer);
   }
   return GetDuration();
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

void Chronometer::Reset()
{
    m_ToReset   = true;
    m_IsStarted = false;
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetCount()
{
   WChar buffer[200];
   _stprintf(buffer, _TEXT("%8ld"), m_Count);

   return Utf8String(buffer);
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetBestIteration()
{
   WChar buffer[200];
   _stprintf(buffer, _TEXT("%8ld"), m_bestIteration);

   return Utf8String(buffer);
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetWorstIteration()
{
   WChar buffer[200];
   _stprintf(buffer, _TEXT("%8ld"), m_worstIteration);

   return Utf8String(buffer);
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetDuration()
{
   WChar buffer[200];
   _stprintf(buffer, _TEXT("%8.3f"), m_duration);

   return Utf8String(buffer);
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetBestDuration()
{
   WChar buffer[200];

   if (m_durationBest != 0.0)
        _stprintf(buffer, _TEXT("%8.3f"), m_durationBest);
   else
       _stprintf(buffer, _TEXT("%8.3f"), 0.0);

   return Utf8String(buffer);
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetAverageDuration()
{
   WChar buffer[200];

   if ((m_Count != 0) && (m_duration != 0.0))
       _stprintf(buffer, _TEXT("%8.3f"), m_duration / m_Count);
   else
       _stprintf(buffer, _TEXT("%8.3f"), 0.0);

   return Utf8String(buffer);
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetWorstDuration()
{
   WChar buffer[200];

   if (m_durationWorst != 0.0)
        _stprintf(buffer, _TEXT("%8.3f"), m_durationWorst);
   else
       _stprintf(buffer, _TEXT("%8.3f"), 0.0);

   return Utf8String(buffer);
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetStartTime()
{
   return Utf8String(_tctime(&m_startTime));
}

//-----------------------------------------------------------------------------
// Public 
//-----------------------------------------------------------------------------

Utf8String Chronometer::GetFinishTime()
{
   return Utf8String(_tctime(&m_finishTime));
}

//-----------------------------------------------------------------------------        
// Convert 2 different PixelsSpace
//-----------------------------------------------------------------------------        

void ConvertThePixels(uint32_t                        pi_Width,
					  uint32_t                        pi_Height,
					  const HFCPtr<HRPPixelType>&   pi_rpSrcPixelType,
					  const HFCPtr<HCDPacket>&      pi_rpSrcPacket, 
					  const HFCPtr<HRPPixelType>&   pi_rpDstPixelType,
					  HFCPtr<HCDPacket>&            po_rpDstPacket)
{

	HPRECONDITION(po_rpDstPacket != 0);

	// calc bytes per row for the source and destination 
	uint32_t SrcBytesPerRow = (pi_rpSrcPixelType->CountPixelRawDataBits() * pi_Width + 7) / 8;
	uint32_t DstBytesPerRow = (pi_rpDstPixelType->CountPixelRawDataBits() * pi_Width + 7) / 8;

	// calc the uncompressed data source and destination size
	uint32_t SrcUncompressedDataSize = SrcBytesPerRow * pi_Height;
	uint32_t DstUncompressedDataSize = DstBytesPerRow * pi_Height;
		
	// create a packet to store the source pixels uncompressed
	HArrayAutoPtr<Byte> pSrcPixels(new Byte[SrcUncompressedDataSize]);
	HFCPtr<HCDPacket> pDecompressedSrcPacket(new HCDPacket(pSrcPixels, SrcUncompressedDataSize, SrcUncompressedDataSize));

	// uncompress the source pixels
	if (pi_rpSrcPacket->GetCodec() != 0)
		pi_rpSrcPacket->Decompress(pDecompressedSrcPacket);
	else
		memcpy(pDecompressedSrcPacket->GetBufferAddress(), pi_rpSrcPacket->GetBufferAddress(), SrcUncompressedDataSize);
 	
	// create a pixels converter 
	HFCPtr<HRPPixelConverter> pConverter(pi_rpSrcPixelType->GetConverterTo(pi_rpDstPixelType));
 
	// Convert the pixels line by line
	Byte* pSrcBuffer     = pSrcPixels;	
	HArrayAutoPtr<Byte>  pDstPixels(new Byte[DstUncompressedDataSize]);
	Byte* pDstBuffer     = pDstPixels;

    if (pi_rpSrcPixelType->GetChannelOrg().GetChannelIndex(HRPChannelType::ALPHA, 0) != HRPChannelType::FREE)
    {
        // set the buffer to white
	    memset(pDstPixels, 255, DstUncompressedDataSize);	 	

	    for(uint32_t LineIndex = 0; LineIndex < pi_Height; LineIndex++)
	    {
		    pConverter->Compose(pSrcBuffer, pDstBuffer, pi_Width);
		    pDstBuffer += DstBytesPerRow;
		    pSrcBuffer += SrcBytesPerRow;
	    }
    }
    else
    {
	    for(uint32_t LineIndex = 0; LineIndex < pi_Height; LineIndex++)
	    {
		    pConverter->Convert(pSrcBuffer, pDstBuffer, pi_Width);
		    pDstBuffer += DstBytesPerRow;
		    pSrcBuffer += SrcBytesPerRow;
	    }
    }

	// Compress the pixels 
	HFCPtr<HCDPacket> pDecompressedDstPacket(new HCDPacket(pDstPixels, DstUncompressedDataSize, DstUncompressedDataSize));	
	
	if (po_rpDstPacket->GetCodec() != 0)
	{
		// test if there is a buffer already defined
		if(po_rpDstPacket->GetBufferSize() == 0)
			po_rpDstPacket->SetBufferOwnership(true);	

		pDecompressedDstPacket->Compress(po_rpDstPacket);
	}
	else
	{
		// test if there is a buffer already defined
		if(po_rpDstPacket->GetBufferSize() == 0)
		{
			po_rpDstPacket->SetBuffer(pDstPixels.release(), DstUncompressedDataSize);
            po_rpDstPacket->SetBufferOwnership(true);
		}
		else
			memcpy(po_rpDstPacket->GetBufferAddress(), pDecompressedDstPacket->GetBufferAddress(), DstUncompressedDataSize);
	}
}

//-------------------------------------------------------------------------------------------------
// Constructor and destructor
//-------------------------------------------------------------------------------------------------

GCIPalette::GCIPalette() : CPalette()                       
{
}

//-------------------------------------------------------------------------------------------------

GCIPalette::~GCIPalette()
{
}

//-------------------------------------------------------------------------------------------------

bool GCIPalette::CreateIdenticalPalette(CPalette *p_palette)
{   
   LOGPALETTE *plp;
   DWORD       dwSize;

   // Create a copy of logical palette 
   dwSize = sizeof (LOGPALETTE) + ((p_palette->GetEntryCount() - 1) * sizeof (PALETTEENTRY));

   if ((plp = (LOGPALETTE*) HeapAlloc(GetProcessHeap (), 0, dwSize)) != NULL) 
   {
      plp->palVersion    = 0x300;
      plp->palNumEntries = (WORD)p_palette->GetEntryCount();
      p_palette->GetPaletteEntries(0, (UINT)p_palette->GetEntryCount(), plp->palPalEntry);
      this->CreatePalette(plp);
      HeapFree(GetProcessHeap (), 0, plp);
      return true;
   }
   return false;
}

//-------------------------------------------------------------------------------------------------

bool GCIPalette::CreateOptimizedPaletteFromHandle(HANDLE hImage, int p_numberColorBits)
{
    HCURSOR hCursor;
    int     bpp;
    bool    l_status;

    bpp = BitsPerPixel (hImage);
    if (bpp <= 8)
        l_status = CreateExactPalette (hImage);
    else 
        if ((bpp == 16) || (bpp == 24) || (bpp == 32)) 
        {
           hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
           l_status = CreateOctreePalette (hImage, 236, p_numberColorBits);  
           SetCursor (hCursor);
        }
        else 
           return false;

    return l_status;
}


//-------------------------------------------------------------------------------------------------
// Private members
//-------------------------------------------------------------------------------------------------

bool GCIPalette::CreateExactPalette(HANDLE hImage)
{
    DIBSECTION ds;
    UINT nColors, i;
    RGBQUAD* prgb;
    LOGPALETTE* plp;
    HDC hdc, hdcMem;
    HANDLE hOldBitmap;
    DWORD dwSize;

    // Get the number of colors in the image
    ::GetObject (hImage, sizeof (ds), &ds);
    if (ds.dsBmih.biClrUsed != 0)
        nColors = ds.dsBmih.biClrUsed;
    else
        nColors = 1 << ds.dsBmih.biBitCount;

    if (nColors > 256) // Sanity check
        return false;

    // Retrieve the image's color table
    if ((prgb = (RGBQUAD*) HeapAlloc (GetProcessHeap (), 0,
        nColors * sizeof (RGBQUAD))) == NULL)
        return false;

    hdc = GetDC (NULL);
    hdcMem = CreateCompatibleDC (hdc);
    hOldBitmap = SelectObject (hdcMem, hImage);
    nColors = min (nColors, GetDIBColorTable (hdcMem, 0, nColors, prgb));
    SelectObject (hdcMem, hOldBitmap);
    DeleteDC (hdcMem);
    ReleaseDC (NULL, hdc);

    if (nColors == 0) { // Another sanity check
        HeapFree (GetProcessHeap (), 0, prgb);
        return false;
    }
        
    // Create a logical palette from the colors in the color table
    dwSize = sizeof (LOGPALETTE) + ((nColors - 1) * sizeof (PALETTEENTRY));
    if ((plp = (LOGPALETTE*) HeapAlloc (GetProcessHeap (), 0,
        dwSize)) == NULL) {
        HeapFree (GetProcessHeap (), 0, prgb);
        return false;
    }

    plp->palVersion = 0x300;
    plp->palNumEntries = (WORD) nColors;

    for (i=0; i<nColors; i++) {
        plp->palPalEntry[i].peRed   = prgb[i].rgbRed;
        plp->palPalEntry[i].peGreen = prgb[i].rgbGreen;
        plp->palPalEntry[i].peBlue  = prgb[i].rgbBlue;
        plp->palPalEntry[i].peFlags = 0;
    }

    // Create CPalette
    this->CreatePalette(plp);

    HeapFree (GetProcessHeap (), 0, plp);
    HeapFree (GetProcessHeap (), 0, prgb);

    return true;
}

//-------------------------------------------------------------------------------------------------

bool GCIPalette::CreateOctreePalette(HANDLE hImage, UINT nMaxColors, UINT nColorBits)
{
    DIBSECTION ds;
    int i, j, nPad;
    BYTE* pbBits;
    WORD* pwBits;
    DWORD* pdwBits;
    DWORD rmask, gmask, bmask;
    int rright, gright, bright;
    int rleft, gleft, bleft;
    BYTE r, g, b;
    WORD wColor;
    DWORD dwColor, dwSize;
    LOGPALETTE* plp;
    NODE* pTree;
    UINT nLeafCount, nIndex;
    NODE* pReducibleNodes[9];

    // Initialize octree variables
    pTree = NULL;
    nLeafCount = 0;
    if (nColorBits > 8) // Just in case
        return false;
    for (i=0; i<=(int) nColorBits; i++)
        pReducibleNodes[i] = NULL;

    // Scan the DIB and build the octree
    ::GetObject (hImage, sizeof (ds), &ds);
    nPad = ds.dsBm.bmWidthBytes - (((ds.dsBmih.biWidth *
        ds.dsBmih.biBitCount) + 7) / 8);

    switch (ds.dsBmih.biBitCount) {

    case 16: // One case for 16-bit DIBs
        if (ds.dsBmih.biCompression == BI_BITFIELDS) {
            rmask = ds.dsBitfields[0];
            gmask = ds.dsBitfields[1];
            bmask = ds.dsBitfields[2];
        }
        else {
            rmask = 0x7C00;
            gmask = 0x03E0;
            bmask = 0x001F;
        }

        rright = GetRightShiftCount (rmask);
        gright = GetRightShiftCount (gmask);
        bright = GetRightShiftCount (bmask);

        rleft = GetLeftShiftCount (rmask);
        gleft = GetLeftShiftCount (gmask);
        bleft = GetLeftShiftCount (bmask);

        pwBits = (WORD*) ds.dsBm.bmBits;
        for (i=0; i<ds.dsBmih.biHeight; i++) {
            for (j=0; j<ds.dsBmih.biWidth; j++) {
                wColor = *pwBits++;
                b = (BYTE) (((wColor & (WORD) bmask) >> bright) << bleft);
                g = (BYTE) (((wColor & (WORD) gmask) >> gright) << gleft);
                r = (BYTE) (((wColor & (WORD) rmask) >> rright) << rleft);
                AddColor (&pTree, r, g, b, nColorBits, 0, &nLeafCount,
                    pReducibleNodes);
                while (nLeafCount > nMaxColors)
                    ReduceTree (nColorBits, &nLeafCount, pReducibleNodes);
            }
            pwBits = (WORD*) (((BYTE*) pwBits) + nPad);
        }
        break;

    case 24: // Another for 24-bit DIBs
        pbBits = (BYTE*) ds.dsBm.bmBits;
        for (i=0; i<ds.dsBmih.biHeight; i++) {
            for (j=0; j<ds.dsBmih.biWidth; j++) {
                b = *pbBits++;
                g = *pbBits++;
                r = *pbBits++;
                AddColor (&pTree, r, g, b, nColorBits, 0, &nLeafCount,
                    pReducibleNodes);
                while (nLeafCount > nMaxColors)
                    ReduceTree (nColorBits, &nLeafCount, pReducibleNodes);
            }
            pbBits += nPad;
        }
        break;

    case 32: // And another for 32-bit DIBs
        if (ds.dsBmih.biCompression == BI_BITFIELDS) {
            rmask = ds.dsBitfields[0];
            gmask = ds.dsBitfields[1];
            bmask = ds.dsBitfields[2];
        }
        else {
            rmask = 0x00FF0000;
            gmask = 0x0000FF00;
            bmask = 0x000000FF;
        }

        rright = GetRightShiftCount (rmask);
        gright = GetRightShiftCount (gmask);
        bright = GetRightShiftCount (bmask);

        pdwBits = (DWORD*) ds.dsBm.bmBits;
        for (i=0; i<ds.dsBmih.biHeight; i++) {
            for (j=0; j<ds.dsBmih.biWidth; j++) {
                dwColor = *pdwBits++;
                b = (BYTE) ((dwColor & bmask) >> bright);
                g = (BYTE) ((dwColor & gmask) >> gright);
                r = (BYTE) ((dwColor & rmask) >> rright);
                AddColor (&pTree, r, g, b, nColorBits, 0, &nLeafCount,
                    pReducibleNodes);
                while (nLeafCount > nMaxColors)
                    ReduceTree (nColorBits, &nLeafCount, pReducibleNodes);
            }
            pdwBits = (DWORD*) (((BYTE*) pdwBits) + nPad);
        }
        break;

    default: // DIB must be 16, 24, or 32-bit!
        return false;
    }

    if (nLeafCount > nMaxColors) { // Sanity check
        DeleteTree (&pTree);
        return false;
    }

    // Create a logical palette from the colors in the octree
    dwSize = sizeof (LOGPALETTE) + ((nLeafCount - 1) * sizeof (PALETTEENTRY));
    if ((plp = (LOGPALETTE*) HeapAlloc (GetProcessHeap (), 0, dwSize)) == NULL) 
    {
        DeleteTree (&pTree);
        return false;
    }

    plp->palVersion = 0x300;
    plp->palNumEntries = (WORD) nLeafCount;
    nIndex = 0;
    GetPaletteColors (pTree, plp->palPalEntry, &nIndex);

    // Create a CPalette
    this->CreatePalette(plp);

    HeapFree (GetProcessHeap (), 0, plp);
    DeleteTree (&pTree);

    return true;
}

//-------------------------------------------------------------------------------------------------

void GCIPalette::AddColor(NODE** ppNode, BYTE r, BYTE g, BYTE b, UINT nColorBits,
    UINT nLevel, UINT* pLeafCount, NODE** pReducibleNodes)
{
    int nIndex, shift;
    static BYTE mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

    // If the node doesn't exist, create it
    if (*ppNode == NULL)
        *ppNode = CreateNode (nLevel, nColorBits, pLeafCount,
            pReducibleNodes);

    // Update color information if it's a leaf node
    if ((*ppNode)->bIsLeaf) {
        (*ppNode)->nPixelCount++;
        (*ppNode)->nRedSum += r;
        (*ppNode)->nGreenSum += g;
        (*ppNode)->nBlueSum += b;
    }

    // Recurse a level deeper if the node is not a leaf
    else {
        shift = 7 - nLevel;
        nIndex = (((r & mask[nLevel]) >> shift) << 2) |
            (((g & mask[nLevel]) >> shift) << 1) |
            ((b & mask[nLevel]) >> shift);
        AddColor (&((*ppNode)->pChild[nIndex]), r, g, b, nColorBits,
            nLevel + 1, pLeafCount, pReducibleNodes);
    }
}

//-------------------------------------------------------------------------------------------------

NODE* GCIPalette::CreateNode(UINT nLevel, UINT nColorBits, UINT* pLeafCount,
    NODE** pReducibleNodes)
{
    NODE* pNode;

    if ((pNode = (NODE*) HeapAlloc (GetProcessHeap (), HEAP_ZERO_MEMORY,
        sizeof (NODE))) == NULL)
        return NULL;

    pNode->bIsLeaf = (nLevel == nColorBits) ? true : false;
    if (pNode->bIsLeaf)
        (*pLeafCount)++;
    else { // Add the node to the reducible list for this level
        pNode->pNext = pReducibleNodes[nLevel];
        pReducibleNodes[nLevel] = pNode;
    }
    return pNode;
}

//-------------------------------------------------------------------------------------------------

void GCIPalette::ReduceTree(UINT nColorBits, UINT* pLeafCount, NODE** pReducibleNodes)
{
    int i;
    NODE* pNode;
    UINT nRedSum, nGreenSum, nBlueSum, nChildren;

    // Find the deepest level containing at least one reducible node
    for (i=nColorBits - 1; (i>0) && (pReducibleNodes[i] == NULL); i--);

    // Reduce the node most recently added to the list at level i
    pNode = pReducibleNodes[i];
    pReducibleNodes[i] = pNode->pNext;

    nRedSum = nGreenSum = nBlueSum = nChildren = 0;
    for (i=0; i<8; i++) {
        if (pNode->pChild[i] != NULL) {
            nRedSum += pNode->pChild[i]->nRedSum;
            nGreenSum += pNode->pChild[i]->nGreenSum;
            nBlueSum += pNode->pChild[i]->nBlueSum;
            pNode->nPixelCount += pNode->pChild[i]->nPixelCount;
            HeapFree (GetProcessHeap (), 0, pNode->pChild[i]);
            pNode->pChild[i] = NULL;
            nChildren++;
        }
    }

    pNode->bIsLeaf = true;
    pNode->nRedSum = nRedSum;
    pNode->nGreenSum = nGreenSum;
    pNode->nBlueSum = nBlueSum;
    *pLeafCount -= (nChildren - 1);
}

//-------------------------------------------------------------------------------------------------

void GCIPalette::DeleteTree(NODE** ppNode)
{
    int i;

    for (i=0; i<8; i++) {
        if ((*ppNode)->pChild[i] != NULL)
            DeleteTree (&((*ppNode)->pChild[i]));
    }
    HeapFree (GetProcessHeap (), 0, *ppNode);
    *ppNode = NULL;
}

//-------------------------------------------------------------------------------------------------

void GCIPalette::GetPaletteColors(NODE* pTree, PALETTEENTRY* pPalEntries, UINT* pIndex)
{
    int i;

    if (pTree->bIsLeaf) {
        pPalEntries[*pIndex].peRed =
            (BYTE) ((pTree->nRedSum) / (pTree->nPixelCount));
        pPalEntries[*pIndex].peGreen =
            (BYTE) ((pTree->nGreenSum) / (pTree->nPixelCount));
        pPalEntries[*pIndex].peBlue =
            (BYTE) ((pTree->nBlueSum) / (pTree->nPixelCount));
        (*pIndex)++;
    }
    else {
        for (i=0; i<8; i++) {
            if (pTree->pChild[i] != NULL)
                GetPaletteColors (pTree->pChild[i], pPalEntries, pIndex);
        }
    }
}

//-------------------------------------------------------------------------------------------------

int GCIPalette::GetRightShiftCount(DWORD dwVal)
{
    int i;

    for (i=0; i<sizeof (DWORD) * 8; i++) {
        if (dwVal & 1)
            return i;
        dwVal >>= 1;
    }
    return -1;
}

//-------------------------------------------------------------------------------------------------

int GCIPalette::GetLeftShiftCount(DWORD dwVal)
{
    int nCount, i;

    nCount = 0;
    for (i=0; i<sizeof (DWORD) * 8; i++) {
        if (dwVal & 1)
            nCount++;
        dwVal >>= 1;
    }
    return (8 - nCount);
}

//-------------------------------------------------------------------------------------------------

UINT GCIPalette::BitsPerPixel(HANDLE hImage)
{
    DIBSECTION ds;

    ::GetObject (hImage, sizeof (ds), &ds);
    return (UINT) ds.dsBmih.biBitCount;
}

//-------------------------------------------------------------------------------------------------

Utf8String GetLongPathName(Utf8String pi_Path) 
{
	LPSHELLFOLDER psfDesktop = NULL;
	ULONG chEaten = 0;
	LPITEMIDLIST pidlShellItem = NULL;
	WCHAR szLongPath[_MAX_PATH] = { 0 };

	// Get the Desktop's shell folder interface
	HRESULT hr = SHGetDesktopFolder(&psfDesktop);

    WString pi_PathW(pi_Path.c_str(), BentleyCharEncoding::Utf8);

	// Request an ID list (relative to the desktop) for the short pathname
    USES_CONVERSION;
	hr = psfDesktop->ParseDisplayName(NULL, 
                                      NULL, 
                                      T2OLE((LPTSTR)pi_PathW.c_str()),
                                      &chEaten, 
						              &pidlShellItem, 
                                      NULL);
	psfDesktop->Release();  // Release the desktop's IShellFolder

	if (FAILED(hr)) 
	{
		// If we couldn't get an ID list for short pathname, it must not exist.
		return false;
		lstrcpyW(szLongPath, L"Error: Path not found!");
	}
	else
	{
		// We did get an ID list, convert it to a long pathname
		SHGetPathFromIDListW(pidlShellItem, szLongPath);

		// Free the ID list allocated by ParseDisplayName
		LPMALLOC pMalloc = NULL;
		SHGetMalloc(&pMalloc);
		pMalloc->Free(pidlShellItem);
		pMalloc->Release();
	}

	return Utf8String((LPCTSTR)CString(W2T(szLongPath)));	
}

//-------------------------------------------------------------------------------------------------
