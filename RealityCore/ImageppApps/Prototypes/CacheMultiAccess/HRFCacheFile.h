#pragma once

#include <Imagepp/all/h/HRFRasterFileCapabilities.h>
#include <Imagepp/all/h/HRFResolutionDescriptor.h>
#include <stdio.h>
#include <fstream>

class cTiffCapabilities : public HRFRasterFileCapabilities
	{
	public:
		cTiffCapabilities();
	};

class HRFCacheFile : public HFCShareableObject<HRFCacheFile>
	{
	public:
		HRFCacheFile(bool loadInMemory);
		virtual ~HRFCacheFile();

		const char* GetPath() const;

		const HFCPtr<HRFRasterFileCapabilities>&			GetCapabilities();
		
		int			GetTileOffset(int tileIndex, int pi_ResolutionOffset, bool reading);
		int			GetResolutionOffset(short pageID, HRFResolutionDescriptor* pi_rpResDescriptor, bool reading);

		bool		ReadShareMemory(long offset, int length, Byte* buffer);
		bool		WriteShareMemory(long offset, int length, Byte* buffer);

		int			GetFreeBlock();
		//bool		TryTakeBlock(int resolutionOffset, int tileIndex);
		//bool		ReleaseBlock(int resolutionOffset, int tileIndex);

	private:

		int			GetPageOffset(short pageID, bool reading);
		

		const char*										m_Path;
		HFCPtr<HRFRasterFileCapabilities>				m_pCapabilities;
		HANDLE											m_pFile;
		int32_t*											m_pTileOffsetTable;
		bool											m_LoadInMemory;

	};

