#pragma once


#include "HRFCacheFile.h"
#include <Imagepp/all/h/HRFResolutionDescriptor.h>
#include <Imagepp/all/h/HRFResolutionEditor.h>

class HRFCacheFileEditor
	{
	public:
		HRFCacheFileEditor(HFCPtr<HRFCacheFile> pi_CacheFile,
			uint32_t              pi_Page,
			unsigned short       pi_Resolution,
			HRFResolutionEditor*  pi_pSrcResolutionEditor);

		virtual ~HRFCacheFileEditor();

		HSTATUS		Read(size_t offsetX, size_t offsetY, Byte* buffer);
		HSTATUS		Write(size_t offsetX, size_t offsetY, Byte* buffer);
		HSTATUS		Read(size_t offsetX, size_t offsetY, HFCPtr<HCDPacket>& po_rpPacket);
		HSTATUS		Write(size_t offsetX, size_t offsetY, HFCPtr<HCDPacket>& po_rpPacket);


		
	private:

		HFCPtr<HRFCacheFile>					m_CacheFile;
		HRFResolutionEditor*					m_pSrcResolutionEditor;
		uint32_t							m_ResolutionOffset;
	};