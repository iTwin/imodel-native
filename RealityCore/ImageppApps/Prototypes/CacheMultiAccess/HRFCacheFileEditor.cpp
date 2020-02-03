

#include "CacheMultiAccessPch.h"
#include "HRFCacheFileEditor.h"
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecImage.h>
#include <Imagepp/all/h/HCDPacket.h>
#include <Imagepp/all/h/HCDPacketRLE.h>
#include <iostream>


HRFCacheFileEditor::HRFCacheFileEditor(HFCPtr<HRFCacheFile> pi_CacheFile,
	uint32_t              pi_Page,
	unsigned short       pi_Resolution,
	HRFResolutionEditor* pi_pSrcResolutionEditor)
	{

	m_pSrcResolutionEditor = pi_pSrcResolutionEditor;
	m_CacheFile = pi_CacheFile;

	m_ResolutionOffset = m_CacheFile->GetResolutionOffset(pi_Page, m_pSrcResolutionEditor->GetResolutionDescriptor(), false);

	}

HRFCacheFileEditor::~HRFCacheFileEditor()
	{

	}

/*-----------------------------------------------------------------------------------------****
-- Read a complete block to a specific location									Nick.Marquis
---------------------------------------------------------------------------------------------*/
HSTATUS HRFCacheFileEditor::Read(size_t offsetX, size_t offsetY, Byte* buffer)
	{

	HSTATUS status = H_NOT_FOUND;
	//Lock while we play in the meta-data

	int tileIndex = m_pSrcResolutionEditor->GetResolutionDescriptor()->ComputeBlockIndex(offsetX, offsetY);

	int tileOffset = m_CacheFile->GetTileOffset(tileIndex, m_ResolutionOffset, true);

	if (tileOffset == -1)
		{
		if (H_SUCCESS == m_pSrcResolutionEditor->ReadBlock(offsetX, offsetX, buffer))
			{
			if (H_SUCCESS == Write(offsetX, offsetY, buffer))
				status = Read(offsetX, offsetY, buffer);
			}
		}
	else
		{
		//Read memory
		if (m_CacheFile->ReadShareMemory(tileOffset, m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes(), buffer))
			status = H_SUCCESS;
		else
			status = H_NOT_FOUND;
		}

	//std::cout << tileOffset << std::endl;
	return status;
	}


/*-----------------------------------------------------------------------------------------****
-- Read a complete block to a specific location									Nick.Marquis
---------------------------------------------------------------------------------------------*/
HSTATUS	HRFCacheFileEditor::Read(size_t offsetX, size_t offsetY, HFCPtr<HCDPacket>& po_rpPacket)
	{
	//Lock while we play in the meta-data

	if (po_rpPacket->GetBufferSize() == 0)
		{
		// if not, create a buffer
		po_rpPacket->SetBuffer(new Byte[m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()],
			m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes());
		po_rpPacket->SetBufferOwnership(true);
		}

	po_rpPacket->SetDataSize(m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes());
	po_rpPacket->SetCodec(new HCDCodecIdentity(m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()));

	

	return Read(offsetX, offsetY, po_rpPacket->GetBufferAddress());
	}

/*-----------------------------------------------------------------------------------------****
-- Write a complete block to a specific location								Nick.Marquis
---------------------------------------------------------------------------------------------*/
HSTATUS HRFCacheFileEditor::Write(size_t offsetX, size_t offsetY, Byte* buffer)
	{
	HSTATUS status = H_NOT_FOUND;
	//Lock while we play in the meta-data

	int tileIndex = m_pSrcResolutionEditor->GetResolutionDescriptor()->ComputeBlockIndex(offsetX, offsetY);

	int tileOffset = m_CacheFile->GetTileOffset(tileIndex, m_ResolutionOffset, false);

	if (tileOffset == -1)
		status = H_NOT_FOUND;
	else
		{
		//Read memory
		if (m_CacheFile->WriteShareMemory(tileOffset, m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes(), buffer))
			status = H_SUCCESS;
		else
			status = H_NOT_FOUND;
		}

	return status;
	}


/*-----------------------------------------------------------------------------------------****
-- Write a complete block to a specific location								Nick.Marquis
---------------------------------------------------------------------------------------------*/
HSTATUS	HRFCacheFileEditor::Write(size_t offsetX, size_t offsetY, HFCPtr<HCDPacket>& pi_rpPacket)
	{

	//Lock while we play in the meta-data

	// we decompress the data
	HCDPacket Uncompressed(new Byte[m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes()],
		m_pSrcResolutionEditor->GetResolutionDescriptor()->GetBlockSizeInBytes());
	Uncompressed.SetBufferOwnership(true);

	pi_rpPacket->Decompress(&Uncompressed);

	return Write(offsetX, offsetY, Uncompressed.GetBufferAddress());
	}