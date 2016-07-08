#pragma once

#include "SMSQLiteStore.h"



template<class EXTENT> SMIndexMasterHeader<EXTENT>& SMIndexMasterHeader<EXTENT>::operator=(const SQLiteIndexHeader& indexHeader)
    {
    if (indexHeader.m_rootNodeBlockID != SQLiteNodeHeader::NO_NODEID) m_rootNodeBlockID = HPMBlockID(indexHeader.m_rootNodeBlockID);
    m_balanced = indexHeader.m_balanced;
    m_depth = indexHeader.m_depth;
    m_terrainDepth = indexHeader.m_terrainDepth;
    m_HasMaxExtent = indexHeader.m_HasMaxExtent;
    m_MaxExtent = ExtentOp<EXTENT>::Create(indexHeader.m_MaxExtent.low.x, indexHeader.m_MaxExtent.low.y, indexHeader.m_MaxExtent.low.z,
                                           indexHeader.m_MaxExtent.high.x, indexHeader.m_MaxExtent.high.y, indexHeader.m_MaxExtent.high.z);
    m_numberOfSubNodesOnSplit = indexHeader.m_numberOfSubNodesOnSplit;
    m_singleFile = indexHeader.m_singleFile;
    m_SplitTreshold = indexHeader.m_SplitTreshold;
    m_textured = indexHeader.m_textured;
    m_isTerrain = indexHeader.m_isTerrain;
    return *this;
    }

template<class EXTENT> operator SMIndexMasterHeader<EXTENT>::SQLiteIndexHeader()
    {
    SQLiteIndexHeader header;
    header.m_rootNodeBlockID = m_rootNodeBlockID.IsValid() ? m_rootNodeBlockID.m_integerID : -1;
    header.m_balanced = m_balanced;
    header.m_depth = m_depth;
    header.m_terrainDepth = m_terrainDepth;
    header.m_HasMaxExtent = m_HasMaxExtent;
    header.m_MaxExtent = DRange3d::From(ExtentOp<EXTENT>::GetXMin(m_MaxExtent), ExtentOp<EXTENT>::GetYMin(m_MaxExtent), ExtentOp<EXTENT>::GetZMin(m_MaxExtent),
                                        ExtentOp<EXTENT>::GetXMax(m_MaxExtent), ExtentOp<EXTENT>::GetYMax(m_MaxExtent), ExtentOp<EXTENT>::GetZMax(m_MaxExtent));
    header.m_numberOfSubNodesOnSplit = m_numberOfSubNodesOnSplit;
    header.m_singleFile = m_singleFile;
    header.m_SplitTreshold = m_SplitTreshold;
    header.m_textured = m_textured;
    header.m_isTerrain = m_isTerrain;
    return header;
    }