//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/utl/hcd/src/HCDCodecLZW.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

// Methods for class HCDCodecLZW
//-----------------------------------------------------------------------------
  
#include <ImageppInternal.h>

#include <Imagepp/all/h/HCDCodecLZW.h>
#include <Imagepp/all/h/HCDLZWDecoder.h>
#include <Imagepp/all/h/HCDLZWEncoder.h>

#define XREPEAT4(n, op)        \
    switch (n) {        \
    default: { HSINTX i; for (i = n-4; i > 0; i--) { op; } } \
    case 2:  op;        \
    case 1:  op;        \
    case 0:  ;            \
    }

#define HCD_CODEC_NAME     L"LZW"

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


// Byte* lzw_encode(Byte *in, int max_bits)
// {
// 	int len = (int)_len(in), bits = 9, next_shift = 512;
// 	ushort code, c, nc, next_code = M_NEW;
// 	lzw_enc_t *d = (lzw_enc_t*)_new(lzw_enc_t, 512);
// 
// 	if (max_bits > 16) max_bits = 16;
// 	if (max_bits < 9 ) max_bits = 12;
// 
// 	Byte *out = (Byte *)_new(ushort, 4);
// 	int out_len = 0, o_bits = 0;
// 	uint32_t tmp = 0;
// 
// 	inline void write_bits(ushort x) 
//         {
// 		tmp = (tmp << bits) | x;
// 		o_bits += bits;
// 		if ((int)_len(out) <= out_len) _extend(out);
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
	int out_len = 0;

    lzw_dec_t *d = (lzw_dec_t *)_new(lzw_dec_t, 512);
	int len, j, next_shift = 512, bits = 9, n_bits = 0;
	ushort code, c, t, next_code = M_NEW;

	uint32_t tmp = 0;
	
    clear_table(); /* in case encoded bits didn't start with M_CLR */
	for (len = (int)_len(in); len;) {
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

// int main()
// {
// 	int i, fd = open("unixdict.txt", O_RDONLY);
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
// if pi_Predictor == 2, the data must be V24R8B8G8 or V32 
// for V16... see HCDCodecLZWPredicateExt
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW(size_t     pi_Width,
                         size_t     pi_Height,
                         size_t     pi_BitsPerPixel,
                         unsigned short pi_Predictor)
    : HCDCodecImage(HCD_CODEC_NAME,
                    pi_Width,
                    pi_Height,
                    pi_BitsPerPixel)
    {
    m_Predictor = pi_Predictor;
    }

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecLZW::HCDCodecLZW(const HCDCodecLZW& pi_rObj)
    : HCDCodecImage(pi_rObj)
    {
    m_Predictor = pi_rObj.m_Predictor;
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

//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecLZW::DecompressSubset(const void*  pi_pInData,
                                     size_t pi_InDataSize,
                                     void*  po_pOutBuffer,
                                     size_t pi_OutBufferSize)
    {
    HDEBUGCODE(uint64_t OUTBOUND_OFFSET = (uint64_t)(po_pOutBuffer) + pi_OutBufferSize;);

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

        if( m_Predictor == 2 )
            {
            HUINTX stride = GetBitsPerPixel() / 8;

            HUINTX rowSize = GetWidth() * stride;
            HUINTX cc      = rowSize;
            size_t row     = pi_OutBufferSize;

            char* op = (char*) po_pOutBuffer;

            while( row >= rowSize )
                {
                char* cp = (char*) op;

                HASSERT((uint64_t)cp < OUTBOUND_OFFSET);

                cc = rowSize;
                stride = GetBitsPerPixel() / 8;
                if (cc > stride)
                    {
                    cc -= stride;
                    // Pipeline the most common cases.
                    if (stride == 3)
                        {
                        HASSERT(((uint64_t)cp + 3) <= OUTBOUND_OFFSET);

                        uint32_t cr = cp[0];
                        uint32_t cg = cp[1];
                        uint32_t cb = cp[2];
                        do
                            {
                            cc -= 3, cp += 3;

                            HASSERT(((uint64_t)cp + 3) <= OUTBOUND_OFFSET);

                            cp[0] = ((cr += cp[0]) & 0xFF);
                            cp[1] = ((cg += cp[1]) & 0xFF);
                            cp[2] = ((cb += cp[2]) & 0xFF);
                            }
                        while ( cc > 0);
                        }
                    else if (stride == 4)
                        {
                        HASSERT(((uint64_t)cp + 4) <= OUTBOUND_OFFSET);

                        uint32_t cr = cp[0];
                        uint32_t cg = cp[1];
                        uint32_t cb = cp[2];
                        uint32_t ca = cp[3];
                        do
                            {
                            cc -= 4, cp += 4;

                            HASSERT(((uint64_t)cp + 4) <= OUTBOUND_OFFSET);

                            cp[0] = ((cr += cp[0]) & 0xFF);
                            cp[1] = ((cg += cp[1]) & 0xFF);
                            cp[2] = ((cb += cp[2]) & 0xFF);
                            cp[3] = ((ca += cp[3]) & 0xFF);
                            }
                        while ( cc > 0);
                        }
                    else
                        {
                        do
                            {
                            HASSERT(((uint64_t)cp + stride) <= OUTBOUND_OFFSET);

                            XREPEAT4(stride, cp[stride] += *cp; cp++)
                            cc -= stride;
                            }
                        while ( cc > 0);
                        }
                    }
                row -= rowSize;

                op += rowSize;
                HASSERT(((uint64_t)op) <= OUTBOUND_OFFSET);
                }
            }

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




//HCDCodecLZWPredicateExt---------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecLZWPredicateExt::HCDCodecLZWPredicateExt()
: HCDCodecLZW()
{
}

//-----------------------------------------------------------------------------
// public
// Default constructor
//-----------------------------------------------------------------------------
HCDCodecLZWPredicateExt::HCDCodecLZWPredicateExt(size_t     pi_Width,
                                                 size_t     pi_Height,
                                                 size_t     pi_BitsPerPixel,
                                                 uint16_t    pi_Predictor, 
                                                 uint32_t    pi_SamplesPerPixel)
: HCDCodecLZW(pi_Width,
              pi_Height,
              pi_BitsPerPixel,
              pi_Predictor)

{
    //Only to solve TFS 88368 on V8i.
    assert(pi_Predictor == 2 && pi_BitsPerPixel == 32 && pi_SamplesPerPixel == 1);

    m_Predictor = pi_Predictor;
    m_SamplesPerPixel = pi_SamplesPerPixel;
}

//-----------------------------------------------------------------------------
// public
// Copy constructor
//-----------------------------------------------------------------------------
HCDCodecLZWPredicateExt::HCDCodecLZWPredicateExt(const HCDCodecLZWPredicateExt& pi_rObj)
: HCDCodecLZW(pi_rObj)
{
    m_Predictor = pi_rObj.m_Predictor;
    m_SamplesPerPixel = pi_rObj.m_SamplesPerPixel;    
}

//-----------------------------------------------------------------------------
// public
// Destructor
//-----------------------------------------------------------------------------
HCDCodecLZWPredicateExt::~HCDCodecLZWPredicateExt()
{
}

//-----------------------------------------------------------------------------
// public
// Clone
//-----------------------------------------------------------------------------
HCDCodec* HCDCodecLZWPredicateExt::Clone() const
{
    return new HCDCodecLZWPredicateExt(*this);
}



//-----------------------------------------------------------------------------
// public
// CompressSubset
//-----------------------------------------------------------------------------
size_t HCDCodecLZWPredicateExt::CompressSubset(const void* pi_pInData,
                                   size_t pi_InDataSize,
                                   void* po_pOutBuffer,
                                   size_t po_OutBufferSize)
{
    assert("Not implemented");
    return 0;
}
  
//-----------------------------------------------------------------------------
// public
// DecompressSubset
//-----------------------------------------------------------------------------
#define REPEAT4(n, op)		\
    switch (n) {		\
    default: { size_t i; for (i = n-4; i > 0; i--) { op; } } \
    case 4:  op;		\
    case 3:  op;		\
    case 2:  op;		\
    case 1:  op;		\
    case 0:  ;			\
    }


size_t HCDCodecLZWPredicateExt::DecompressSubset(const void*  pi_pInData,
                                           size_t pi_InDataSize,
                                           void*  po_pOutBuffer,
                                           size_t pi_OutBufferSize)
{
    if (m_Predictor != 2 || GetBitsPerPixel() != 32 || m_SamplesPerPixel != 1)
        return 0;
    
    HDEBUGCODE(uint64_t OUTBOUND_OFFSET = (uint64_t)(po_pOutBuffer) + pi_OutBufferSize;);

    size_t OutDataSize = 0;

    // is it the first packet?
    if(GetSubsetPosY() == 0)
        {
        SetCurrentState(STATE_DECOMPRESS);
        }
  
    //TR 232508
    HCDLZWDecoder lzwDecoder;
    OutDataSize = lzwDecoder.Decode((Byte const*)pi_pInData, pi_InDataSize, (Byte*)po_pOutBuffer, pi_OutBufferSize);

    if (OutDataSize > 0)
        {    
        // libTiff 5.0 LZW bug.
        HASSERT(!(((Byte*)pi_pInData)[0] == 0 && (((Byte*)pi_pInData)[1] & 0x1)));
        
        if( m_Predictor == 2 )
            {
            HUINTX stride = GetBitsPerPixel() / 8;

            HUINTX rowSize = GetWidth() * stride;
            HUINTX cc      = rowSize;
            size_t row     = pi_OutBufferSize;
        
            char* op = (char*) po_pOutBuffer;

            while( row >= rowSize )
                {
                char* cp = (char*) op;

                HASSERT((uint64_t)cp < OUTBOUND_OFFSET);

                cc = rowSize;
                stride = GetBitsPerPixel() / 8;

                assert(cc > stride);
                
                //Taken from v8i_src\imagepp\ext\gdal\frmts\gtiff\libtiff\tif_predict.c.
                size_t internStride = 1;                 
                uint32_t* wp = (uint32_t*) cp;
                size_t wc = cc / 4;

                assert((cc%(4*internStride))==0);

                if (wc > internStride) 
                    {
                    wc -= internStride;
                    do 
                        {
                        REPEAT4(internStride, wp[internStride] += wp[0]; wp++)
                        wc -= internStride;
                        } while (wc > 0);
                    }
                
                row -= rowSize;

                op += rowSize;
                HASSERT(((uint64_t)op) <= OUTBOUND_OFFSET);
                }
            }

        SetCompressedImageIndex(GetCompressedImageIndex() + pi_InDataSize);
        SetSubsetPosY(GetSubsetPosY() + GetSubsetHeight());

        OutDataSize = ((GetSubsetWidth() + GetLinePaddingBits()) / 8) * GetSubsetHeight();

        if(GetSubsetPosY() == GetHeight())
            Reset();    
        }
   
    return OutDataSize;
}