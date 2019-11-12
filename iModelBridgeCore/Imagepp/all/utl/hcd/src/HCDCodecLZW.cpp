//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>  See COPYRIGHT.md in the repository root for full copyright notice.
//:>
//:>+--------------------------------------------------------------------------------------

// Methods for class HCDCodecLZW
//-----------------------------------------------------------------------------
  
#include <ImageppInternal.h>

#include <ImagePP/all/h/HCDCodecLZW.h>
#include <ImagePP/all/h/HCDLZWDecoder.h>
#include <ImagePP/all/h/HCDLZWEncoder.h>

#define XREPEAT4(n, op)        \
    switch (n) {        \
    default: { HSINTX i; for (i = n-4; i > 0; i--) { op; } } \
    case 2:  op;        \
    case 1:  op;        \
    case 0:  ;            \
    }

#define HCD_CODEC_NAME     "LZW"

// Extracted from http://rosettacode.org/wiki/LZW_compression
#ifdef LZW_ANOTHER_DECODER  // for testing.
/* -------- aux stuff ---------- */
void* mem_alloc(size_t item_size, size_t n_item)
{
	size_t *x = (size_t*)calloc(1, sizeof(size_t)*2 + n_item * item_size);
	x[0] = item_size;
	x[1] = n_item;
	return x + 2;
}

void* mem_extend(void *m, size_t new_n)
{
	size_t *x = (size_t*)m - 2;
	x = (size_t *)realloc(x, sizeof(size_t) * 2 + *x * new_n);
	if (new_n > x[1])
		memset((char*)(x + 2) + x[0] * x[1], 0, x[0] * (new_n - x[1]));
	x[1] = new_n;
	return x + 2;
}

inline void _clear(void *m)
{
	size_t *x = (size_t*)m - 2;
	memset(m, 0, x[0] * x[1]);
}

#define _new(type, n)	mem_alloc(sizeof(type), n)
#define _del(m)		{ free((size_t*)(m) - 2); m = 0; }
#define _len(m)		*((size_t*)m - 1)
#define _setsize(m, n, type)	m = (type*)mem_extend(m, n)
#define _extend(m)	m = (Byte*)mem_extend(m, _len(m) * 2)


/* ----------- LZW stuff -------------- */
typedef uint8_t Byte;
typedef uint16_t ushort;

#define M_CLR	256	/* clear table marker */
#define M_EOD	257	/* end-of-data marker */
#define M_NEW	258	/* new code index */

/* encode and decode dictionary structures.
   for encoding, entry at code index is a list of indices that follow current one,
   i.e. if code 97 is 'a', code 387 is 'ab', and code 1022 is 'abc',
   then dict[97].next['b'] = 387, dict[387].next['c'] = 1022, etc. */
typedef struct {
	ushort next[256];
} lzw_enc_t;

/* for decoding, dictionary contains index of whatever prefix index plus trailing
   byte.  i.e. like previous example,
   	dict[1022] = { c: 'c', prev: 387 },
   	dict[387]  = { c: 'b', prev: 97 },
   	dict[97]   = { c: 'a', prev: 0 }
   the "back" element is used for temporarily chaining indices when resolving
   a code to bytes
 */
typedef struct {
	ushort prev, back;
	Byte c;
} lzw_dec_t;


// Byte* lzw_encode(Byte *in, int32_t max_bits)
// {
// 	int32_t len = (int32_t)_len(in), bits = 9, next_shift = 512;
// 	ushort code, c, nc, next_code = M_NEW;
// 	lzw_enc_t *d = (lzw_enc_t*)_new(lzw_enc_t, 512);
// 
// 	if (max_bits > 16) max_bits = 16;
// 	if (max_bits < 9 ) max_bits = 12;
// 
// 	Byte *out = (Byte *)_new(ushort, 4);
// 	int32_t out_len = 0, o_bits = 0;
// 	uint32_t tmp = 0;
// 
// 	inline void write_bits(ushort x) 
//         {
// 		tmp = (tmp << bits) | x;
// 		o_bits += bits;
// 		if ((int32_t)_len(out) <= out_len) _extend(out);
// 		while (o_bits >= 8) 
//             {
// 			o_bits -= 8;
// 			out[out_len++] = tmp >> o_bits;
// 			tmp &= (1 << o_bits) - 1;
// 		}
// 	}
// 
// 	//write_bits(M_CLR);
// 	for (code = *(in++); --len; ) {
// 		c = *(in++);
// 		if ((nc = d[code].next[c]))
// 			code = nc;
// 		else {
// 			write_bits(code);
// 			nc = d[code].next[c] = next_code++;
// 			code = c;
// 		}
// 
// 		/* next new code would be too long for current table */
// 		if (next_code == next_shift) {
// 			/* either reset table back to 9 bits */
// 			if (++bits > max_bits) {
// 				/* table clear marker must occur before bit reset */
// 				write_bits(M_CLR);
// 
// 				bits = 9;
// 				next_shift = 512;
// 				next_code = M_NEW;
// 				_clear(d);
// 			} else	/* or extend table */
// 				_setsize(d, next_shift *= 2);
// 		}
// 	}
// 
// 	write_bits(code);
// 	write_bits(M_EOD);
// 	if (tmp) write_bits(tmp);
// 
// 	_del(d);
// 
// 	_setsize(out, out_len);
// 	return out;
// }

#define write_out(c) \
	while (out_len >= _len(out)) _extend(out);\
		out[out_len++] = (Byte)c;

#define get_code() \
		while(n_bits < bits) {\
			if (len > 0) {\
				len --;\
				tmp = (tmp << 8) | *(in++);\
				n_bits += 8;\
			} else {\
				tmp = tmp << (bits - n_bits);\
				n_bits = bits;\
			}\
		}\
		n_bits -= bits;\
		code = tmp >> n_bits;\
		tmp &= (1 << n_bits) - 1;

#define clear_table() \
		_clear(d); \
		for (j = 0; j < 256; j++) d[j].c = (Byte)j; \
		next_code = M_NEW;\
		next_shift = 512;\
		bits = 9;

Byte* lzw_decode(Byte *in)
{
	Byte *out = (Byte *)_new(Byte, 4);
	int32_t out_len = 0;

    lzw_dec_t *d = (lzw_dec_t *)_new(lzw_dec_t, 512);
	int32_t len, j, next_shift = 512, bits = 9, n_bits = 0;
	ushort code, c, t, next_code = M_NEW;

	uint32_t tmp = 0;
	
    clear_table(); /* in case encoded bits didn't start with M_CLR */
	for (len = (int32_t)_len(in); len;) {
		get_code();
		if (code == M_EOD) break;
		if (code == M_CLR) {
			clear_table();
			continue;
		}

		if (code >= next_code) {
			HASSERT(!"fprintf(stderr, Bad sequence\n);");
			_del(out);
			goto bail;
		}

		d[next_code].prev = c = code;
		while (c > 255) {
			t = d[c].prev; d[t].back = c; c = t;
		}

		d[next_code - 1].c = c;

		while (d[c].back) {
			write_out(d[c].c);
			t = d[c].back; d[c].back = 0; c = t;
		}
		write_out(d[c].c);

		if (++next_code >= next_shift) {
			if (++bits > 16) {
				/* if input was correct, we'd have hit M_CLR before this */
				HASSERT(!"fprintf(stderr, Too many bits\n);");
				_del(out);
				goto bail;
			}
			_setsize(d, next_shift *= 2, lzw_dec_t);
		}
	}

	/* might be ok, so just whine, don't be drastic */
	if (code != M_EOD) fputs("Bits did not end in EOD\n", stderr);

	_setsize(out, out_len, Byte);
bail:	_del(d);
	return out;
}

// int32_t main()
// {
// 	int32_t i, fd = open("unixdict.txt", O_RDONLY);
// 
// 	if (fd == -1) {
// 		fprintf(stderr, "Can't read file\n");
// 		return 1;
// 	};
// 
// 	struct stat st;
// 	fstat(fd, &st);
// 
// 	Byte *in = _new(char, st.st_size);
// 	read(fd, in, st.st_size);
// 	_setsize(in, st.st_size);
// 	close(fd);
// 
// 	printf("input size:   %d\n", _len(in));
// 
// 	Byte *enc = lzw_encode(in, 9);
// 	printf("encoded size: %d\n", _len(enc));
// 
// 	Byte *dec = lzw_decode(enc);
// 	printf("decoded size: %d\n", _len(dec));
// 
// 	for (i = 0; i < _len(dec); i++)
// 		if (dec[i] != in[i]) {
// 			printf("bad decode at %d\n", i);
// 			break;
// 		}
// 
// 	if (i == _len(dec)) printf("Decoded ok\n");
// 
// 
// 	_del(in);
// 	_del(enc);
// 	_del(dec);
// 
// 	return 0;
// }
#endif


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW()
    : HCDCodecImage(HCD_CODEC_NAME)
{
}

//-----------------------------------------------------------------------------
// public
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW(size_t     pi_Width,
    size_t     pi_Height,
    size_t     pi_BitsPerPixel,
    uint16_t   pi_Predictor,
    uint32_t   pi_samplesPerPixel)
    : HCDCodecImage(HCD_CODEC_NAME,
        pi_Width,
        pi_Height,
        pi_BitsPerPixel)
{
    m_Predictor = pi_Predictor;
    m_samplePerPixels = pi_samplesPerPixel;
    BeAssert(2 == m_Predictor ? pi_samplesPerPixel != 0 : true);
}

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW(const HCDCodecLZW& pi_rObj)
    : HCDCodecImage(pi_rObj)
{
    m_Predictor = pi_rObj.m_Predictor;
    m_samplePerPixels = pi_rObj.m_samplePerPixels;
}

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecLZW::~HCDCodecLZW()
{
}

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecLZW::Clone() const
{
    return new HCDCodecLZW(*this);
}

//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecLZW::CompressSubset(const void* pi_pInData,
    size_t pi_InDataSize,
    void* po_pOutBuffer,
    size_t po_OutBufferSize)
{
    size_t OutDataSize(0);

    HCDLZWEncoder   LZWEncoder;
    OutDataSize = LZWEncoder.Encode((Byte*)pi_pInData, pi_InDataSize, (Byte*)po_pOutBuffer, po_OutBufferSize);

    return OutDataSize;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                2/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename DataType_T, uint32_t ChannelCount_T>
void DecodePredicate(size_t width, size_t height, Byte* pPixels, size_t dataSize)
{
    // Based on: https://www.fileformat.info/format/tiff/corion-lzw.htm and \Imagepp\ext\gdal\frmts\gtiff\libtiff\tif_predict.c
    size_t rowSize = width * sizeof(DataType_T) * ChannelCount_T;

    for (size_t row = 0; row < height; ++row)
        {
        if ((row + 1) * rowSize > dataSize)
            {
            // Some app(gdal_translate) won't include the padding data for the last block so in these cases
            // the last block is smaller than expected but the raster is still valid. 
            // Unfortunately HTIFFFile::ReadDataWithPacket doesn't set the effective pixel width/height so 
            // we have no way to know that it is the last block so assert that we are on a row boundary and stop decoding.
            BeAssert("unexpected end of data buffer" && dataSize % rowSize == 0);
            break;
            }

        DataType_T* pPixelsRow = (DataType_T*)(pPixels + row * rowSize);

        for (size_t col = 1; col < width; ++col)
            {
            for (uint32_t channel = 0; channel < ChannelCount_T; ++channel)
                {
                pPixelsRow[col * ChannelCount_T + channel] += pPixelsRow[(col - 1) * ChannelCount_T + channel];
                }
            }
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                2/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<class Data_t>
void DecodePredicate(size_t width, size_t height, uint32_t channelCount, Byte* pPixels, size_t dataSize)
{    
    switch (channelCount)
        {
        case 1:
            DecodePredicate<Data_t, 1>(width, height, pPixels, dataSize);
            break;
        case 2:
            DecodePredicate<Data_t, 2>(width, height, pPixels, dataSize);
            break;
        case 3:
            DecodePredicate<Data_t, 3>(width, height, pPixels, dataSize);
            break;
        case 4:
            DecodePredicate<Data_t, 4>(width, height, pPixels, dataSize);
            break;
        default:
            BeAssert(!"Channel count not supported");
        }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.Marchand                2/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void HCDCodecLZW::DecodeHorizontalPredicate(Byte* po_pOutBuffer, size_t dataSize)
{
    BeAssert(m_samplePerPixels > 0);
    size_t bitsPerChannel = m_samplePerPixels > 0 ? GetBitsPerPixel() / m_samplePerPixels : 0;

    switch (bitsPerChannel)
        {
        case 8:
            DecodePredicate<uint8_t>(GetWidth(), GetHeight(), m_samplePerPixels, po_pOutBuffer, dataSize);
            break;
        case 16:
            DecodePredicate<uint16_t>(GetWidth(), GetHeight(), m_samplePerPixels, po_pOutBuffer, dataSize);
            break;
        case 32:
            DecodePredicate<uint32_t>(GetWidth(), GetHeight(), m_samplePerPixels, po_pOutBuffer, dataSize);
            break;
        default:
            BeAssert(!"Predictor not supported");
            break;
        }
}

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecLZW::DecompressSubset(const void*  pi_pInData,
                                     size_t pi_InDataSize,
                                     void*  po_pOutBuffer,
                                     size_t pi_OutBufferSize)
    {
    size_t OutDataSize = 0;

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }

    //TR 232508
    HCDLZWDecoder lzwDecoder;
    OutDataSize = lzwDecoder.Decode(static_cast<Byte const*>(pi_pInData), pi_InDataSize, 
                                    static_cast<Byte*>(po_pOutBuffer), pi_OutBufferSize);
    
#ifdef LZW_ANOTHER_DECODER // For testing.
    Byte *in = (Byte *)_new(Byte, pi_InDataSize);
    memcpy(in, pi_pInData, pi_InDataSize);
    _setsize(in, pi_InDataSize, Byte);

    Byte *dec = lzw_decode(in);

    memcpy(po_pOutBuffer, dec, _len(dec));

    OutDataSize = _len(dec);

    _del(in);
    _del(dec);
#endif

    if (OutDataSize > 0)
        {
        // libTiff 5.0 LZW bug.
        HASSERT(!( ((Byte*)pi_pInData)[0] == 0 && (((Byte*)pi_pInData)[1] & 0x1)));

#ifdef LZW_A_SLOW_DECODER
        lzwDecoder.Decode2((Byte*)pi_pInData, pi_InDataSize, (Byte*)po_pOutBuffer, pi_OutBufferSize);
#endif
        
        if(m_Predictor == 2 )
            DecodeHorizontalPredicate(static_cast<Byte*>(po_pOutBuffer), OutDataSize);
            
        SetCompressedImageIndex(GetCompressedImageIndex() + pi_InDataSize);
        SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

        if(GetSubsetPosY() == GetHeight())
            Reset();
        }

    return OutDataSize;
    }


//-----------------------------------------------------------------------------
// public
// SetDimensions
//-----------------------------------------------------------------------------
void HCDCodecLZW::SetDimensions(size_t pi_Width, size_t pi_Height)
    {
    HCDCodecImage::SetDimensions(pi_Width, pi_Height);

    /*

    CODE EN CONSEQUENCE

    */
    }

//-----------------------------------------------------------------------------
// public
// IsBitsPerPixelSupported
//-----------------------------------------------------------------------------
bool HCDCodecLZW::IsBitsPerPixelSupported(size_t pi_Bits) const
    {
    if((pi_Bits % 8) == 0)
        return true;
    else
        return false;
    }
