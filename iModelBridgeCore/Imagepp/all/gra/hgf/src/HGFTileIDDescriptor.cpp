//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFTileIDDescriptor.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HGFTileIDDescriptor
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HGF2DExtent.h>
#include <Imagepp/all/h/HFCGrid.h>

// The class declaration must be the last include file.
#include <Imagepp/all/h/HGFTileIDDescriptor.h>


const uint64_t HGFTileIDDescriptor::INDEX_NOT_FOUND = UINT64_MAX;

//TR 296505: Epsilon error occurs in some cases. We assume that HGFTileIDDescriptor
//           is creating a grid using physical coordinate so we decided to lower the epsilon.
#define TILE_GRID_EPSILON 0.001

//-----------------------------------------------------------------------------
// public
// Default Constructor
//-----------------------------------------------------------------------------
HGFTileIDDescriptor::HGFTileIDDescriptor()
    : m_ImageWidth (0),
      m_ImageHeight(0),
      m_TileSizeX  (1),
      m_TileSizeY  (1)

    {
    m_NumberOfTileX = 1;
    }



//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGFTileIDDescriptor::HGFTileIDDescriptor(uint64_t pi_Width,
                                         uint64_t pi_Height,
                                         uint64_t pi_TileSizeX,
                                         uint64_t pi_TileSizeY)
    : m_ImageWidth (pi_Width),
      m_ImageHeight(pi_Height),
      m_TileSizeX (pi_TileSizeX),
      m_TileSizeY (pi_TileSizeY)
    {
    HPRECONDITION((UINT64_MAX - pi_TileSizeX) >= pi_Width);
    HPRECONDITION((UINT64_MAX - pi_TileSizeY) >= pi_Height);

    // Must be >= 1, but at the construction I must accept the parameters
    // with value at 0.
    //
    if (pi_TileSizeX ==  0)
        m_TileSizeX = 1;
    if (pi_TileSizeY ==  0)
        m_TileSizeY = 1;

    if ((m_NumberOfTileX = (pi_Width + (m_TileSizeX-1L)) / m_TileSizeX) == 0)
        m_NumberOfTileX = 1;
    }

//-----------------------------------------------------------------------------
// public
// Constructor
//-----------------------------------------------------------------------------
HGFTileIDDescriptor::~HGFTileIDDescriptor()
    {
    }


//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HGFTileIDDescriptor::HGFTileIDDescriptor   (const HGFTileIDDescriptor& pi_rObj)
    : m_ImageWidth (pi_rObj.m_ImageWidth),
      m_ImageHeight(pi_rObj.m_ImageHeight),
      m_TileSizeX (pi_rObj.m_TileSizeX),
      m_TileSizeY (pi_rObj.m_TileSizeY)
    {
    m_NumberOfTileX = (m_ImageWidth + (m_TileSizeX-1L)) / m_TileSizeX;
    }

//-----------------------------------------------------------------------------
// public
// operator=
//-----------------------------------------------------------------------------
HGFTileIDDescriptor& HGFTileIDDescriptor::operator=(const HGFTileIDDescriptor& pi_rObj)
    {
    if (this != &pi_rObj)
        {
        // Members
        m_ImageWidth    = pi_rObj.m_ImageWidth;
        m_ImageHeight   = pi_rObj.m_ImageHeight;
        m_TileSizeX     = pi_rObj.m_TileSizeX;
        m_TileSizeY     = pi_rObj.m_TileSizeY;

        m_NumberOfTileX = pi_rObj.m_NumberOfTileX;
        }

    return(*this);
    }



/** -----------------------------------------------------------------------------
    Returns the index of the first tile to process. The GetNextTileIndex() can
   be called to obtain other tile indexes.

    @param pi_rExtent IN The extent over which tile must be iterated upon.
            need processing.

    @return The tile index of the first tile to process.

    @see GetNextTileIndex()
    -----------------------------------------------------------------------------
*/
uint64_t HGFTileIDDescriptor::GetFirstTileIndex (const HGF2DExtent& pi_rExtent)
    {
    if (pi_rExtent.IsDefined())
        {
        HFCGrid Grid(pi_rExtent.GetXMin(),
                     pi_rExtent.GetYMin(),
                     pi_rExtent.GetXMax(),
                     pi_rExtent.GetYMax(), TILE_GRID_EPSILON);

        // Limit to the image
        HPRECONDITION((uint64_t)Grid.GetXMax() <= UINT64_MAX);
        HPRECONDITION((uint64_t)Grid.GetYMax() <= UINT64_MAX);
        HPRECONDITION((uint64_t)Grid.GetXMin() <= UINT64_MAX);
        HPRECONDITION((uint64_t)Grid.GetYMin() <= UINT64_MAX);

        uint64_t xMin = (uint64_t)MAX (0L, Grid.GetXMin());
        uint64_t yMin = (uint64_t)MAX (0L, Grid.GetYMin());
        uint64_t xMax = (uint64_t)MIN (m_ImageWidth - 1L, (uint64_t)MAX(0L, Grid.GetXMax()));
        uint64_t yMax = (uint64_t)MIN (m_ImageHeight - 1L, (uint64_t)MAX(0, Grid.GetYMax()));

        // Not tile in the image
        if ((xMin > xMax) || (yMin > yMax))
            m_CurIndex = INDEX_NOT_FOUND;
        else
            {
            // Compute index;
            m_FirstCurIndex     = ComputeIndex (xMin, yMin);
            m_LastCurIndex      = ComputeIndex (xMax, yMin);

            m_LastFirstColumn   = ComputeIndex (xMin, yMax);

            m_CurIndex = m_FirstCurIndex;
            }
        }
    else
        m_CurIndex = INDEX_NOT_FOUND;

    return (m_CurIndex);
    }


/** -----------------------------------------------------------------------------
    Returns the index of the first tile to process. The GetNextTileIndex() can
   be called to obtain other tile indexes.

    @param pi_XMin The lower bound X value in pixel to process tiles upon. This value
                    must be smaller than Image Width - pi_XCount.

    @param pi_YMin The lower bound Y value in pixel to process tiles upon. This value
                  must be smaller than image height - pi_YCount.

    @param pi_XCound IN THe X count in pixels of the area to process.

    @param pi_YCount IN The Y count in pixels of the area to process

    @return The tile index of the first tile to process.

    @see GetNextTileIndex()
    -----------------------------------------------------------------------------
*/
uint64_t HGFTileIDDescriptor::GetFirstTileIndex (uint64_t pi_XMin,   uint64_t pi_YMin,
                                                uint64_t pi_XCount, uint64_t pi_YCount)
    {
    HPRECONDITION((UINT64_MAX - pi_XCount) >= pi_XMin);
    HPRECONDITION((UINT64_MAX - pi_YCount) >= pi_YMin);

    // Check the input Extent dim
    if ((pi_XCount == 0) || (pi_YCount == 0))
        m_CurIndex = INDEX_NOT_FOUND;
    else
        {
        // Compute index;
        pi_XMin = (uint64_t)MAX (0L, pi_XMin);
        pi_YMin = (uint64_t)MAX (0L, pi_YMin);
        uint64_t xMax = (uint64_t)MIN (m_ImageWidth - 1, MAX(0L, pi_XMin + pi_XCount-1));
        uint64_t yMax = (uint64_t)MIN (m_ImageHeight- 1, MAX(0L, pi_YMin + pi_YCount-1));

        m_FirstCurIndex     = ComputeIndex (pi_XMin, pi_YMin);
        m_LastCurIndex      = ComputeIndex (xMax, pi_YMin);
        m_LastFirstColumn   = ComputeIndex (pi_XMin, yMax);

        m_CurIndex = m_FirstCurIndex;
        }

    return (m_CurIndex);
    }

/** -----------------------------------------------------------------------------
Returns the number of tiles intersecting the extent.

@param pi_rExtent IN The extent to be intersected.

@return The number of intersected tiles.

@see GetFirstTileIndex()
-----------------------------------------------------------------------------
*/
uint64_t HGFTileIDDescriptor::GetTileCount(HGF2DExtent& pi_rExtent) const
    {
    HFCGrid Grid(pi_rExtent.GetXMin(),
                 pi_rExtent.GetYMin(),
                 pi_rExtent.GetXMax(),
                 pi_rExtent.GetYMax(), TILE_GRID_EPSILON);

    uint64_t NbTiles = 0;

    // Limit to the image
    HPRECONDITION((uint64_t)Grid.GetXMax() <= UINT64_MAX);
    HPRECONDITION((uint64_t)Grid.GetYMax() <= UINT64_MAX);
    HPRECONDITION((uint64_t)Grid.GetXMin() <= UINT64_MAX);
    HPRECONDITION((uint64_t)Grid.GetYMin() <= UINT64_MAX);

    uint64_t xMin = (uint64_t)MAX (0L, Grid.GetXMin());
    uint64_t yMin = (uint64_t)MAX (0L, Grid.GetYMin());
    uint64_t xMax = (uint64_t)MIN (m_ImageWidth - 1L, (uint64_t)MAX(0L, Grid.GetXMax()));
    uint64_t yMax = (uint64_t)MIN (m_ImageHeight - 1L, (uint64_t)MAX(0, Grid.GetYMax()));

    //Extent is located on the image
    if ((xMin < xMax) && (yMin < yMax))
        {
        // Compute index;
        NbTiles = (uint64_t)((ceil((double)xMax / m_TileSizeX) - floor((double)xMin / m_TileSizeX)) *
                            (ceil((double)yMax / m_TileSizeY) - floor((double)yMin / m_TileSizeY)));
        }

    return NbTiles;
    }

#if 0
// public
// HRABitmap::PrintState - Displays content to specified stream
//-----------------------------------------------------------------------------
void HGFTileIDDescriptor::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE

    po_rOutput

            << "HGFTileIDDescriptor"
            << endl

            << "ImageWidth:    " << m_ImageWidth
            << endl

            << "ImageHeight:    " << m_ImageHeight
            << endl

            << "TileSizeX :    " << m_TileSizeX
            << endl

            << "TileSizeY :    " << m_TileSizeY
            << endl

            << "NumberOfTileX :" << m_NumberOfTileX
            << endl;

#endif
    }
#endif

