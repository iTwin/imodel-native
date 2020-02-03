

#include "CacheMultiAccessPch.h"
#include "HRFCacheFile.h"
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>

#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
#include <ImagePP/h/HmrMacro.h>

//include for the capabilities
#include <Imagepp/all/h/HRFUtility.h>

#include <Imagepp/all/h/HCDCodecIJG.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HCDCodecHMRCCITT.h>
#include <Imagepp/all/h/HCDCodecHMRPackBits.h>
#include <Imagepp/all/h/HCDCodecFlashpix.h>
#include <Imagepp/all/h/HCDCodecHMRRLE1.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HCDCodecLZW.h>

#include <Imagepp/all/h/HRPComplexFilter.h>
#include <Imagepp/all/h/HRPChannelOrgGray.h>
#include <Imagepp/all/h/HRPChannelOrgRGB.h>
#include <Imagepp/all/h/HRPConvFiltersV24R8G8B8.h>
#include <Imagepp/all/h/HRPFunctionFilters.h>
#include <Imagepp/all/h/HRPMapFilters8.h>
#include <Imagepp/all/h/HRPPixelPalette.h>
#include <Imagepp/all/h/HRPPixelTypeV24R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV24B8G8R8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32R8G8B8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV32B8G8R8X8.h>
#include <Imagepp/all/h/HRPPixelTypeV24PhotoYCC.h>
#include <Imagepp/all/h/HRPPixelTypeV32PR8PG8PB8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV32PRPhotoYCCA8.h>
#include <Imagepp/all/h/HRPPixelTypeV32CMYK.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI1R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8A8.h>
#include <Imagepp/all/h/HRPPixelTypeI8VA8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeI8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV8Gray8.h>
#include <Imagepp/all/h/HRPPixelTypeV16PRGray8A8.h>
#include <Imagepp/all/h/HRPPixelTypeV16Gray16.h>
#include <Imagepp/all/h/HRPPixelTypeV16Int16.h>
#include <Imagepp/all/h/HRPPixelTypeI4R8G8B8.h>
#include <Imagepp/all/h/HRPPixelTypeV1GrayWhite1.h>
#include <Imagepp/all/h/HRPPixelTypeV8GrayWhite8.h>
#include <Imagepp/all/h/HRPPixelTypeV1Gray1.h>
#include <Imagepp/all/h/HRPTypeAdaptFilters.h>
#include <Imagepp/all/h/HRPPixelTypeV16B5G5R5.h>
#include <Imagepp/all/h/HRPPixelTypeV16R5G6B5.h>
#include <Imagepp/all/h/HRPPixelTypeV48R16G16B16.h>
#include <Imagepp/all/h/HRPPixelTypeV32Float32.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16A16.h>
#include <Imagepp/all/h/HRPPixelTypeV64R16G16B16X16.h>
#include <Imagepp/all/h/HRPPixelTypeV96R32G32B32.h>

#include <Imagepp/all/h/HGF2DAffine.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DProjective.h>
#include <Imagepp/all/h/HGF2DStretch.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DTranslation.h>


USING_NAMESPACE_IMAGEPP


// const to fixe certain size of the file sytem.
const std::size_t HEADER_PAGE_OFFSET = 200;
const std::size_t HEADER_PAGE_SIZE = 200;
const std::size_t HEADER_RES_OFFSET = 3000;
const std::size_t HEADER_RES_SIZE = 3000;
const std::size_t HEADER_FREE_BLOCK_OFFSET = 100000000;
const std::size_t HEADER_SIZE = 150000000;
const std::size_t BLOCK_SIZE = 100000; // 100 KB
WString pathExt = L"mapFile.cache.ctiff";
const char* cPathExt = "mapFile.cache.ctiff";




// Capabilities to addapt with ResolutionDescriptor
//-----------------------------------------------------------------------------
// HRFcTiffBlockCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffBlockCapabilities : public HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffBlockCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Block Capability

			// Tile Capability
			Add(new HRFTileCapability(HFC_READ_WRITE_CREATE, // AccessMode
				LONG_MAX,              // MaxSizeInBytes
				1,                     // MinWidth
				LONG_MAX,              // MaxWidth
				1,                     // WidthIncrement
				1,                     // MinHeight
				LONG_MAX,              // MaxHeight
				1,                     // HeightIncrement
				false));               // Not Square

			// Strip Capability
			Add(new HRFStripCapability(HFC_READ_WRITE_CREATE,  // AccessMode
				LONG_MAX,               // MaxSizeInBytes
				1,                      // MinHeight
				LONG_MAX,               // MaxHeight
				1));                    // HeightIncrement

			}
	};

//-----------------------------------------------------------------------------
// HRFcTiffCodecV24R8G8B8Capabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV24R8G8B8Capabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecV24R8G8B8Capabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec IJG (Jpeg)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIJG::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			}
	};

//-----------------------------------------------------------------------------
// HRFcTiffCodecV24PhotoYCCCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV24PhotoYCCCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecV24PhotoYCCCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec IJG (Jpeg)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIJG::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			// LZW Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			}
	};

//-----------------------------------------------------------------------------
// HRFcTiffCodecV16R5G6B5Capabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV16R5G6B5Capabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecV16R5G6B5Capabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			}
	};


//-----------------------------------------------------------------------------
// HRFcTiffCodecV32RGBAlphaCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV32RGBAlphaCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecV32RGBAlphaCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec Flashpix
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecFlashpix::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			}
	};



//-----------------------------------------------------------------------------
// HRFcTiffCodecV32CMYK
//-----------------------------------------------------------------------------
class HRFcTiffCodecV32CMYKCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecV32CMYKCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec Packbits
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecHMRPackBits::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			}
	};



//-----------------------------------------------------------------------------
// HRFcTiffCodec1BitCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodec1BitCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodec1BitCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec HMR RLE1
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecHMRRLE1::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec HMR CCITT
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecHMRCCITT::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec HMR PackBits
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecHMRPackBits::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			}
	};

//-----------------------------------------------------------------------------
// HRFcTiffCodecV8GrayCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecV8GrayCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecV8GrayCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec IJG (Jpeg)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIJG::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			}
	};

//-----------------------------------------------------------------------------
// HRFcTiffCodecPaletteCapabilities
// Used by : PixelTypeV16PRGray8A8
//           PixelTypeI2R8G8B8
//           PixelTypeI4R8G8B8, PixelTypeI4R8G8B8A8
//           PixelTypeI8R8G8B8, PixelTypeI8R8G8B8A8
//-----------------------------------------------------------------------------
class HRFcTiffCodecPaletteCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecPaletteCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			}
	};

//-----------------------------------------------------------------------------
// HRFcTiffCodec16BitsPerChannelCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodec16BitsPerChannelCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodec16BitsPerChannelCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			}
	};

//-----------------------------------------------------------------------------
// HRFcTiffCodecFloatCapabilities
//-----------------------------------------------------------------------------
class HRFcTiffCodecFloatCapabilities : public  HRFRasterFileCapabilities
	{
	public:
		// Constructor
		HRFcTiffCodecFloatCapabilities()
			: HRFRasterFileCapabilities()
			{
			// Codec Zlib (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecZlib::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			// Codec Identity
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecIdentity::CLASS_ID,
				new HRFcTiffBlockCapabilities()));

			// Codec LZW (Deflate)
			Add(new HRFCodecCapability(HFC_READ_WRITE_CREATE,
				HCDCodecLZW::CLASS_ID,
				new HRFcTiffBlockCapabilities()));
			}
	};

//-----------------------------------------------------------------------------
// cTiffCapabilities
//-----------------------------------------------------------------------------
cTiffCapabilities::cTiffCapabilities()
	: HRFRasterFileCapabilities()
	{
	// Pixel Type capabilities

	// PixelTypeV24R8G8B8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV24R8G8B8;
	pPixelTypeV24R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV24R8G8B8::CLASS_ID,
		new HRFcTiffCodecV24R8G8B8Capabilities());
	pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV24R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV24R8G8B8);

	// PixelTypeV24B8G8R8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV24B8G8R8;
	pPixelTypeV24B8G8R8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV24B8G8R8::CLASS_ID,
		new HRFcTiffCodecV24R8G8B8Capabilities());
	pPixelTypeV24B8G8R8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV24B8G8R8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV24B8G8R8);

	// PixelTypeV16R5G6B5
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV16R5G6B5;
	pPixelTypeV16R5G6B5 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV16R5G6B5::CLASS_ID,
		new HRFcTiffCodecV16R5G6B5Capabilities());
	// No sampler available
	//pPixelTypeV16R5G6B5->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV16R5G6B5->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV16R5G6B5);


	// PixelTypeV32R8G8B8A8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV32R8G8B8A8;
	pPixelTypeV32R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV32R8G8B8A8::CLASS_ID,
		new HRFcTiffCodecV32RGBAlphaCapabilities());
	pPixelTypeV32R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV32R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV32R8G8B8A8);

	// PixelTypeV32R8G8B8X8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV32R8G8B8X8;
	pPixelTypeV32R8G8B8X8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV32R8G8B8X8::CLASS_ID,
		new HRFcTiffCodecV32RGBAlphaCapabilities());
	pPixelTypeV32R8G8B8X8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV32R8G8B8X8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV32R8G8B8X8);

	// PixelTypeV32B8G8R8X8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV32B8G8R8X8;
	pPixelTypeV32B8G8R8X8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV32B8G8R8X8::CLASS_ID,
		new HRFcTiffCodecV32RGBAlphaCapabilities());
	pPixelTypeV32B8G8R8X8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV32B8G8R8X8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV32B8G8R8X8);

	// PixelTypeV32PR8PG8PB8A8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV32PR8PG8PB8A8;
	pPixelTypeV32PR8PG8PB8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV32PR8PG8PB8A8::CLASS_ID,
		new HRFcTiffCodecV32RGBAlphaCapabilities());
	pPixelTypeV32PR8PG8PB8A8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV32PR8PG8PB8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV32PR8PG8PB8A8);

	// PixelTypeV32CMYK
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV32CMYK;
	pPixelTypeV32CMYK = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV32CMYK::CLASS_ID,
		new HRFcTiffCodecV32CMYKCapabilities());
	pPixelTypeV32CMYK->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV32CMYK->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV32CMYK);

	// PixelTypeV96R32G32B32
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV96R32G32B32;
	pPixelTypeV96R32G32B32 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV96R32G32B32::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeV96R32G32B32->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV96R32G32B32);

	// PixelTypeV1Gray1
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV1Gray1;
	pPixelTypeV1Gray1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV1Gray1::CLASS_ID,
		new HRFcTiffCodec1BitCapabilities());
	pPixelTypeV1Gray1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV1Gray1);

	// PixelTypeV1GrayWhite1
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV1GrayWhite1;
	pPixelTypeV1GrayWhite1 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV1GrayWhite1::CLASS_ID,
		new HRFcTiffCodec1BitCapabilities());
	pPixelTypeV1GrayWhite1->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV1GrayWhite1);

	// PixelTypeV8Gray8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV8Gray8;
	pPixelTypeV8Gray8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV8Gray8::CLASS_ID,
		new HRFcTiffCodecV8GrayCapabilities());
	pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV8Gray8);

	// PixelTypeV8GrayWhite8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV8GrayWhite8;
	pPixelTypeV8GrayWhite8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV8GrayWhite8::CLASS_ID,
		new HRFcTiffCodecV8GrayCapabilities());
	pPixelTypeV8GrayWhite8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV8GrayWhite8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV8GrayWhite8);

	// PixelTypeI1R8G8B8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeI1R8G8B8;
	pPixelTypeI1R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI1R8G8B8::CLASS_ID,
		new HRFcTiffCodec1BitCapabilities());
	pPixelTypeI1R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI1R8G8B8);

	// PixelTypeI1R8G8B8A8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeI1R8G8B8A8;
	pPixelTypeI1R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI1R8G8B8A8::CLASS_ID,
		new HRFcTiffCodec1BitCapabilities());
	pPixelTypeI1R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::ORING4);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI1R8G8B8A8);

	// PixelTypeI2R8G8B8
	// Read/Write/Create capabilities
	// HFCPtr<HRFPixelTypeCapability> pPixelTypeI2R8G8B8;
	// pPixelTypeI2R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
	//                                                 HRPPixelTypeI2R8G8B8::CLASS_ID,
	//                                                 new HRFcTiffCodecPaletteCapabilities());
	// pPixelTypeI2R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	// Add((HFCPtr<HRFCapability>&)pPixelTypeI2R8G8B8);

	HFCPtr<HRFPixelTypeCapability> pPixelTypeI4R8G8B8;
	pPixelTypeI4R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI4R8G8B8::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeI4R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI4R8G8B8);

	// PixelTypeI4R8G8B8A8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeI4R8G8B8A8;
	pPixelTypeI4R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI4R8G8B8A8::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeI4R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI4R8G8B8A8);

	// PixelTypeI8Gray8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeI8Gray8;
	pPixelTypeI8Gray8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI8Gray8::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeI8Gray8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI8Gray8);

	// PixelTypeI8R8G8B8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8;
	pPixelTypeI8R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI8R8G8B8::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeI8R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8);

	// PixelTypeI8R8G8B8A8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeI8R8G8B8A8;
	pPixelTypeI8R8G8B8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI8R8G8B8A8::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeI8R8G8B8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI8R8G8B8A8);

	// PixelTypeI8VAR8G8B8
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeI8VA8R8G8B8;
	pPixelTypeI8VA8R8G8B8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeI8VA8R8G8B8::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeI8VA8R8G8B8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeI8VA8R8G8B8);

	// PixelTypeV16PRGray8A8
	// Read/Write capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV16PRGray8A8;
	pPixelTypeV16PRGray8A8 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV16PRGray8A8::CLASS_ID,
		new HRFcTiffCodecPaletteCapabilities());
	pPixelTypeV16PRGray8A8->AddDownSamplingMethod(HRFDownSamplingMethod::AVERAGE);
	pPixelTypeV16PRGray8A8->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV16PRGray8A8);

	// PixelTypeV48R16B16G16
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV48R16G16B16;
	pPixelTypeV48R16G16B16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV48R16G16B16::CLASS_ID,
		new HRFcTiffCodec16BitsPerChannelCapabilities());
	pPixelTypeV48R16G16B16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV48R16G16B16);

	// HRPPixelTypeV64R16G16B16A16
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV64R16G16B16A16;
	pPixelTypeV64R16G16B16A16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV64R16G16B16A16::CLASS_ID,
		new HRFcTiffCodec16BitsPerChannelCapabilities());
	pPixelTypeV64R16G16B16A16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV64R16G16B16A16);

	// HRPPixelTypeV64R16G16B16A16
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV64R16G16B16X16;
	pPixelTypeV64R16G16B16X16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV64R16G16B16X16::CLASS_ID,
		new HRFcTiffCodec16BitsPerChannelCapabilities());
	pPixelTypeV64R16G16B16X16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV64R16G16B16X16);

	// PixelTypeV16Gray16
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV16Gray16;
	pPixelTypeV16Gray16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV16Gray16::CLASS_ID,
		new HRFcTiffCodec16BitsPerChannelCapabilities());
	pPixelTypeV16Gray16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV16Gray16);

	// PixelTypeV16Int16
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV16Int16;
	pPixelTypeV16Int16 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV16Int16::CLASS_ID,
		new HRFcTiffCodec16BitsPerChannelCapabilities());
	pPixelTypeV16Int16->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);

	Add((HFCPtr<HRFCapability>&)pPixelTypeV16Int16);

	// PixelTypeV32Float32
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV32Float32;
	pPixelTypeV32Float32 = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV32Float32::CLASS_ID,
		new HRFcTiffCodecFloatCapabilities());
	pPixelTypeV32Float32->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);

	Add((HFCPtr<HRFCapability>&)pPixelTypeV32Float32);

	// PixelTypeV24PhotoYCC
	// Read/Write/Create capabilities
	HFCPtr<HRFPixelTypeCapability> pPixelTypeV24PhotoYCC;
	pPixelTypeV24PhotoYCC = new HRFPixelTypeCapability(HFC_READ_WRITE_CREATE,
		HRPPixelTypeV24PhotoYCC::CLASS_ID,
		new HRFcTiffCodecV24PhotoYCCCapabilities());
	pPixelTypeV24PhotoYCC->AddDownSamplingMethod(HRFDownSamplingMethod::NEAREST_NEIGHBOUR);
	Add((HFCPtr<HRFCapability>&)pPixelTypeV24PhotoYCC);


	// Transfo Model
	Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DProjective::CLASS_ID));
	Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DAffine::CLASS_ID));
	Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DSimilitude::CLASS_ID));
	Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DStretch::CLASS_ID));
	Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DTranslation::CLASS_ID));
	Add(new HRFTransfoModelCapability(HFC_READ_WRITE_CREATE, HGF2DIdentity::CLASS_ID));

	// Scanline orientation capability
	Add(new HRFScanlineOrientationCapability(HFC_READ_WRITE_CREATE, HRFScanlineOrientation::UPPER_LEFT_HORIZONTAL));

	// Interleave capability
	Add(new HRFInterleaveCapability(HFC_READ_WRITE_CREATE, HRFInterleaveType::PIXEL));

	// Single resolution capability
	Add(new HRFSingleResolutionCapability(HFC_READ_WRITE_CREATE));

	// MultiResolution Capability
	HFCPtr<HRFCapability> pMultiResolutionCapability = new HRFMultiResolutionCapability(
		HFC_READ_WRITE_CREATE, // AccessMode,
		true,                  // SinglePixelType,
		true,                  // SingleBlockType,
		false,                 // ArbitaryXRatio,
		false);                // ArbitaryYRatio);
	Add(pMultiResolutionCapability);

	// Media type capability
	Add(new HRFStillImageCapability(HFC_READ_WRITE_CREATE));

	// Multi Page capability
	Add(new HRFMultiPageCapability(HFC_READ_WRITE_CREATE));

	// Embeding capability
	Add(new HRFEmbedingCapability(HFC_READ_WRITE_CREATE));

	// Shape capability
	Add(new HRFClipShapeCapability(HFC_READ_WRITE_CREATE, HRFCoordinateType::PHYSICAL));

	// Histogram capability
	Add(new HRFHistogramCapability(HFC_READ_WRITE_CREATE));

	// BlockDataFlag capability
	Add(new HRFBlocksDataFlagCapability(HFC_READ_WRITE_CREATE));

	// Sub sampling capability
	Add(new HRFSubSamplingCapability(HFC_READ_WRITE_CREATE));

	// Tag capability
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDocumentName));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeImageDescription));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributePageName));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeMake));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeModel));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeSoftware));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeDateTime));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeArtist));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeHostComputer));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeInkNames));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeResolutionUnit(0)));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeXResolution(0.0)));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeYResolution(0.0)));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeCopyright));
	Add(new HRFTagCapability(HFC_READ_WRITE_CREATE, new HRFAttributeOnDemandRastersInfo));

	// Add the supported filters.
	Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPBlurAdaptFilter::CLASS_ID));
	Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPContrastFilter::CLASS_ID));
	Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPDetailFilter::CLASS_ID));
	Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPEdgeEnhanceFilter::CLASS_ID));
	Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPFindEdgesFilter::CLASS_ID));
	Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPSharpenAdaptFilter::CLASS_ID));
	Add(new HRFFilterCapability(HFC_READ_WRITE_CREATE, HRPSmoothFilter::CLASS_ID));

	Add(new HRFThumbnailCapability(HFC_READ_WRITE_CREATE,
		128,                   // pi_MinWidth
		128,                   // pi_MaxWidth
		0,                     // pi_WidthIncrement
		128,                   // pi_MinHeight
		128,                   // pi_MaxHeight
		0,                     // pi_HeightIncrement
		65535,                 // pi_MaxSizeInBytes
		false));               // pi_IsComposed

	}



/*-----------------------------------------------------------------------------------------****
--Constructor of a cache file													Nick.Marquis
---------------------------------------------------------------------------------------------*/
	HRFCacheFile::HRFCacheFile(bool loadInMemory)
	{

	BeFileName localPath;
	BeFileName::BeGetTempPath(localPath);
	localPath.AppendToPath(pathExt.c_str());

	const wchar_t *str = localPath.c_str();
	char* cPath = new char[wcslen(str) + 1];
	wcstombs_s(NULL, cPath, wcslen(str) + 1, str, wcslen(str) + 1);

	m_Path = cPath;
	m_pCapabilities = NULL;	

	// &NM temporary, need to check if it's a new file and init only new file

	m_pFile = CreateFile(localPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	int32_t* initBuffer = new int32_t[HEADER_SIZE];
	memset(initBuffer, 1, HEADER_SIZE);
	WriteFile(m_pFile, initBuffer, HEADER_SIZE, 0, 0);
	SetFilePointer(m_pFile, 0, 0, FILE_BEGIN);


	// temporary hardcode one instance
	int numberOfInstance = 0;
	WriteFile(m_pFile, &numberOfInstance, sizeof(int), 0, 0);
	SetFilePointer(m_pFile, 0, 0, FILE_BEGIN);

	// Add an instance of the program
	ReadFile(m_pFile, &numberOfInstance, sizeof(int), 0, 0);
	numberOfInstance++;
	SetFilePointer(m_pFile, 0, 0, FILE_BEGIN);
	WriteFile(m_pFile, &numberOfInstance, sizeof(int), 0, 0);

	//Put 0 for the number of page
	int numberOfpage = 0;
	WriteFile(m_pFile, &numberOfpage, sizeof(int), 0, 0);

	// init free block list
	SetFilePointer(m_pFile, HEADER_FREE_BLOCK_OFFSET, 0, FILE_BEGIN);
	int blocksCount = 0;
	WriteFile(m_pFile, &blocksCount, sizeof(int), 0, 0);

	// we need to load the tile offset header in memory
	m_LoadInMemory = loadInMemory;
	if (loadInMemory)
		m_pTileOffsetTable = new int32_t[HEADER_SIZE - HEADER_FREE_BLOCK_OFFSET];

	}


/*-----------------------------------------------------------------------------------------****
-- Destructor																	Nick.Marquis
---------------------------------------------------------------------------------------------*/
HRFCacheFile::~HRFCacheFile()
	{
	}

/*-----------------------------------------------------------------------------------------****
-- Get the path of the cache file   											Nick.Marquis
---------------------------------------------------------------------------------------------*/
const char* HRFCacheFile::GetPath() const
	{
	return m_Path;
	}


/*-----------------------------------------------------------------------------------------****
-- find a page in the file or create a new page									Nick.Marquis
---------------------------------------------------------------------------------------------*/
int	HRFCacheFile::GetPageOffset(short pageID, bool reading)
	{
	SetFilePointer(m_pFile, 4, 0, FILE_BEGIN);

	int pageCount;
	ReadFile(m_pFile, &pageCount, sizeof(int), 0, 0);

	// find the right page with the page id
	int ID;
	for (int i = 0; i < pageCount; ++i)
		{
		SetFilePointer(m_pFile, ((i * 8) + 8), 0, FILE_BEGIN);
		ReadFile(m_pFile, &ID, sizeof(int), 0, 0);
		if (ID == pageID)
			{
			// get the page offset if it does exist
			ReadFile(m_pFile, &ID, sizeof(int), 0, 0);
			return ID;
			}
		}

	if (reading)
		return -1;

	
	// Add the page to the list if it doesn't exist yet.
	int page = (int)pageID;
	SetFilePointer(m_pFile, ((pageCount * 8) + 8), 0, FILE_BEGIN);
	WriteFile(m_pFile, &page, sizeof(int), 0, 0);
	int pageOffset = HEADER_PAGE_OFFSET + (pageCount * HEADER_PAGE_SIZE);
	WriteFile(m_pFile, &pageOffset, sizeof(int), 0, 0);

	//init the res count in the page to 0
	SetFilePointer(m_pFile, pageOffset, 0, FILE_BEGIN);
	int resolutionCount = 0;
	WriteFile(m_pFile, &resolutionCount, sizeof(int), 0, 0);

	// update the number of page
	SetFilePointer(m_pFile, 4, 0, FILE_BEGIN);
	pageCount++;
	WriteFile(m_pFile, &pageCount, sizeof(int), 0, 0);

	return pageOffset;
	}


/*-----------------------------------------------------------------------------------------****
-- Return the offset of the resolution for a specific page in the cache file	Nick.Marquis
---------------------------------------------------------------------------------------------*/
int	HRFCacheFile::GetResolutionOffset(short pageID, HRFResolutionDescriptor* pi_rpResDescriptor, bool reading)
	{
	int pageOffset = GetPageOffset(pageID, reading);

	SetFilePointer(m_pFile, pageOffset, 0, FILE_BEGIN);

	int resCount;
	ReadFile(m_pFile, &resCount, sizeof(int), 0, 0);

	// find the right res with the res delta
	int resOffset;
	float newResDelta = pi_rpResDescriptor->GetResolutionXRatio() * pi_rpResDescriptor->GetResolutionYRatio();
	float resDelta;
	for (int i = 0; i < resCount; ++i)
		{
		SetFilePointer(m_pFile, ((i * 8) + pageOffset + 4), 0, FILE_BEGIN);
		ReadFile(m_pFile, &resDelta, sizeof(float), 0, 0);
		if (resDelta == newResDelta)
			{
			// get the res offset if it does exist
			ReadFile(m_pFile, &resOffset, sizeof(int), 0, 0);
			return resOffset;
			}
		}

	if (reading)
		return -1;

	// add the res to the list if it doesn't exist yet.
	SetFilePointer(m_pFile, ((resCount * 8) + pageOffset + 4), 0, FILE_BEGIN);
	WriteFile(m_pFile, &newResDelta, sizeof(float), 0, 0);
	resOffset = HEADER_RES_OFFSET + (resCount * HEADER_RES_SIZE);
	WriteFile(m_pFile, &resOffset, sizeof(int), 0, 0);

	// Init the res to 0;
	SetFilePointer(m_pFile, resOffset, 0, FILE_BEGIN);
	int tileCount = 0; 
	WriteFile(m_pFile, &tileCount, sizeof(int), 0, 0);

	// Update the number of res
	SetFilePointer(m_pFile, pageOffset, 0, FILE_BEGIN);
	resCount++;
	WriteFile(m_pFile, &resCount, sizeof(int), 0, 0);

	return resOffset;
	}


/*-----------------------------------------------------------------------------------------****
-- Return the offset of a tile in the cache file for a specific resolution		Nick.Marquis
---------------------------------------------------------------------------------------------*/
int HRFCacheFile::GetTileOffset(int tileIndex, int pi_ResolutionOffset, bool reading)
	{
	int tileOffset;

	if (m_LoadInMemory)
		{
		
		if (m_pTileOffsetTable[tileIndex] != 0)
			return m_pTileOffsetTable[tileIndex];
		

		if (reading)
			return -1;

		tileOffset = GetFreeBlock();
		m_pTileOffsetTable[tileIndex] = tileOffset;

		}
	else
		{

		SetFilePointer(m_pFile, pi_ResolutionOffset + (tileIndex*4) + 4, 0, FILE_BEGIN);
		ReadFile(m_pFile, &tileOffset, sizeof(int), 0, 0);

		if (tileOffset != 16843009)
			return tileOffset;

		// if reading, don't create a new space
		if (reading)
			return -1;

		// Add the tile to the list if it doesn't exist yet.
		tileOffset = GetFreeBlock();
		SetFilePointer(m_pFile, pi_ResolutionOffset + (tileIndex * 4) + 4, 0, FILE_BEGIN);
		WriteFile(m_pFile, &tileOffset, sizeof(int), 0, 0);

		}

	return tileOffset;
	}


/*-----------------------------------------------------------------------------------------****
-- Return the offset of the first free block									Nick.Marquis
---------------------------------------------------------------------------------------------*/
int HRFCacheFile::GetFreeBlock()
	{
	// &NM will need a releaseBlock method and to take a block in free space not just at the end

	SetFilePointer(m_pFile, HEADER_FREE_BLOCK_OFFSET, 0, FILE_BEGIN);

	int blocksCount;
	ReadFile(m_pFile, &blocksCount, sizeof(int), 0, 0);

	SetFilePointer(m_pFile, (HEADER_FREE_BLOCK_OFFSET + (blocksCount * 4)), 0, FILE_BEGIN);
	int blockOffset = (blocksCount * BLOCK_SIZE) + HEADER_SIZE;
	WriteFile(m_pFile, &blockOffset, sizeof(int), 0, 0);

	// Increase block count
	SetFilePointer(m_pFile, HEADER_FREE_BLOCK_OFFSET, 0, FILE_BEGIN);
	blocksCount++;
	WriteFile(m_pFile, &blocksCount, sizeof(int), 0, 0);
	
	return blockOffset;
	}


/*-----------------------------------------------------------------------------------------****
-- Lock the file only to a specific block										Nick.Marquis
---------------------------------------------------------------------------------------------*/
//bool HRFCacheFile::TryTakeBlock(int resolutionOffset, int tileIndex)
//	{
//
//	Byte* addr = (Byte*)m_regionHeader->get_address();
//	int* addrInt = (int*)addr + resolutionOffset;
//
//	int tileCount = addrInt[0];
//	addrInt++;
//
//	// Find the right block
//	for (int i = 0; i < tileCount; ++i)
//		{
//		if (addrInt[i * 3] == tileIndex)
//			{
//			//Return false if the block is taken, true otherwise
//			if (addrInt[i * 3 + 2] == 1)
//				return false;
//			else
//				{
//				// Take the block
//				addrInt[i * 3 + 2] = 1;
//				return true;
//				}	
//			}
//		}
//	return false;
//	}


/*-----------------------------------------------------------------------------------------****
-- Unlock the file only to a specific block						     			Nick.Marquis
---------------------------------------------------------------------------------------------*/
//bool HRFCacheFile::ReleaseBlock(int resolutionOffset, int tileIndex)
//	{
//
//	Byte* addr = (Byte*)m_regionHeader->get_address();
//	int* addrInt = (int*)addr + resolutionOffset;
//
//	int tileCount = addrInt[0];
//	addrInt++;
//
//	// Find the right block
//	for (int i = 0; i < tileCount; ++i)
//		{
//		if (addrInt[i * 3] == tileIndex)
//			{
//			//Return false if the block isn't taken, true otherwise
//			if (addrInt[i * 3 + 2] == 1)
//				{
//				// Release the block
//				addrInt[i * 3 + 2] = 0;
//				return true;
//				}
//			else
//				return false;
//			}
//		}
//	return false;
//	}


/*-----------------------------------------------------------------------------------------****
-- Return the capabilities for the cache file									Nick.Marquis
---------------------------------------------------------------------------------------------*/
const HFCPtr<HRFRasterFileCapabilities>&	HRFCacheFile::GetCapabilities()
	{
	if (m_pCapabilities == 0)
		m_pCapabilities = new cTiffCapabilities();

	return m_pCapabilities;
	}


/*-----------------------------------------------------------------------------------------****
-- Lowest level to read the cache file											Nick.Marquis
---------------------------------------------------------------------------------------------*/
bool HRFCacheFile::ReadShareMemory(long offset, int length, Byte* buffer)
	{
	SetFilePointer(m_pFile, offset, 0, FILE_BEGIN);

	// Read file
	ReadFile(m_pFile, buffer, length, 0, 0);

	//&NM when there is an error?
	return true;
	}


/*-----------------------------------------------------------------------------------------****
-- Lowest level to write to the cache file										Nick.Marquis
---------------------------------------------------------------------------------------------*/
bool HRFCacheFile::WriteShareMemory(long offset, int length, Byte* buffer)
	{
	SetFilePointer(m_pFile, offset, 0, FILE_BEGIN);

	// Write to file
	WriteFile(m_pFile, buffer, length, 0, 0);

	////&NM when there is an error?
	return true;
	}
