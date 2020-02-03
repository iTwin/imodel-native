/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/PixelConverter.cs $
|    $RCSfile: PixelConverter.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Diagnostics;

namespace Bentley.ImageViewer
{
    //Contains the data for a single pixel
    public struct RawPixelData
    {
        public int PaletteIndex;
        public int Channel1;
        public int Channel2;
        public int Channel3;
        public int Channel4;
    }
    //contains all managed pixel type
    public enum PixelType
    {
        HRPPixelTypeI1R8G8B8 ,
        HRPPixelTypeI1R8G8B8A8 ,
        HRPPixelTypeI1R8G8B8A8RLE, 
        HRPPixelTypeI1R8G8B8RLE, 
        HRPPixelTypeI2R8G8B8, 
        HRPPixelTypeI4R8G8B8, 
        HRPPixelTypeI4R8G8B8A8,
        HRPPixelTypeI8Gray8, 
        HRPPixelTypeI8R8G8B8, 
        HRPPixelTypeI8R8G8B8A8,
        HRPPixelTypeI8R8G8B8Mask, 
        HRPPixelTypeI8VA8R8G8B8, 
        HRPPixelTypeV1Gray1,
        HRPPixelTypeV1GrayWhite1,
        HRPPixelTypeV8Gray8 , 
        HRPPixelTypeV8GrayWhite8, 
        HRPPixelTypeV16B5G6R5, 
        HRPPixelTypeV16Gray16,
        HRPPixelTypeV16Int16, 
        HRPPixelTypeV16PRGray8A8, 
        HRPPixelTypeV16R5G6B5, 
        HRPPixelTypeV24B8G8R8 , 
        HRPPixelTypeV24PhotoYCC , 
        HRPPixelTypeV24R8G8B8 , 
        HRPPixelTypeV32A8R8G8B8 , 
        HRPPixelTypeV32B8G8R8X8 ,
        HRPPixelTypeV32CMYK ,
        HRPPixelTypeV32Float32 , 
        HRPPixelTypeV32PR8PG8PB8A8 , 
        HRPPixelTypeV32PRPhotoYCCA8 ,
        HRPPixelTypeV32R8G8B8A8 , 
        HRPPixelTypeV32B8G8R8A8, 
        HRPPixelTypeV32R8G8B8X8 ,
        HRPPixelTypeV48R16G16B16 ,
        HRPPixelTypeV64R16G16B16A16 ,
        HRPPixelTypeV64R16G16B16X16 ,
        HRPPixelTypeV96R32G32B32 ,
    }

    public class PixelConverter
    {
    private readonly Byte[] s_LookUpTable = {
    0,1,1,2,2,3,4,5,6,7,
    8,9,10,11,12,13,14,15,16,17,
    18,19,20,22,23,24,25,26,28,29,
    30,31,33,34,35,36,38,39,40,41,
    43,44,45,47,48,49,51,52,53,55,
    56,57,59,60,61,63,64,65,67,68,
    70,71,72,74,75,76,78,79,81,82,
    83,85,86,88,89,91,92,93,95,96,
    98,99,101,102,103,105,106,108,109,111,
    112,113,115,116,118,119,121,122,123,125,
    126,128,129,130,132,133,134,136,137,138,
    140,141,142,144,145,146,148,149,150,152,
    153,154,155,157,158,159,160,162,163,164,
    165,166,168,169,170,171,172,174,175,176,
    177,178,179,180,182,183,184,185,186,187,
    188,189,190,191,192,194,195,196,197,198,
    199,200,201,202,203,204,204,205,206,207,
    208,209,210,211,212,213,213,214,215,216,
    217,217,218,219,220,221,221,222,223,223,
    224,225,225,226,227,227,228,229,229,230,
    230,231,231,232,233,233,234,234,235,235,
    236,236,236,237,237,238,238,238,239,239,
    240,240,240,241,241,241,242,242,242,242,
    243,243,243,244,244,244,244,245,245,245,
    245,245,246,246,246,246,246,247,247,247,
    247,247,247,248,248,248,248,248,248,249,
    249,249,249,249,249,249,249,249,250,250,
    250,250,250,250,250,250,250,250,251,251,
    251,251,251,251,251,251,251,251,251,251,
    251,251,252,252,252,252,252,252,252,252,
    252,252,252,252,252,252,252,252,252,253,
    253,253,253,253,253,253,253,253,253,253,
    253,253,253,253,253,253,253,254,254,254,
    254,254,254,254,254,254,254,254,254,254,
    254,254,255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,255,255,
    255
    };

    private readonly double[,] AdjustementMatrixFromCMYK = { { 0.85 , 0.1 , 0.05 } , { 0.2 , 0.7 , 0.1 } , { 0.05 , 0.15 , 0.8 } }; 

        private static Dictionary<string , PixelType> m_PixelTypeDictionary;
        private static bool m_DictionaryInitialized;
        public PixelConverter()
        {
            if( !m_DictionaryInitialized )
            {
                m_PixelTypeDictionary = new Dictionary<string , PixelType>();
                foreach( PixelType values in Enum.GetValues(typeof(PixelType)) )
                {
                    m_PixelTypeDictionary.Add(Enum.GetName(typeof(PixelType) , values) , values); 
                }
            }
            m_DictionaryInitialized = true;
        }
        public static PixelType GetPixelType(string PixelTypeStr)
        {
            return m_PixelTypeDictionary[PixelTypeStr];
        }
        public byte[] ConvertToV32B8G8R8A8(byte[] buffer , PixelType pixelType , byte[] palette , int width , int height)
        {
            switch( pixelType )
            {
                case PixelType.HRPPixelTypeI1R8G8B8 :
                    return ConvertFromI1R8G8B8(buffer , palette);
                case PixelType.HRPPixelTypeI1R8G8B8A8 :
                    return ConvertFromI1R8G8B8A8(buffer , palette);
                case PixelType.HRPPixelTypeI1R8G8B8A8RLE :
                    return ConvertFromI1R8G8B8A8RLE(buffer , palette , width , height);
                case PixelType.HRPPixelTypeI1R8G8B8RLE :
                    return ConvertFromI1R8G8B8RLE(buffer , palette , width , height);
                case PixelType.HRPPixelTypeI2R8G8B8 : 
                    return ConvertFromI2R8G8B8(buffer , palette);
                case PixelType.HRPPixelTypeI4R8G8B8 : 
                    return ConvertFromI4R8G8B8(buffer , palette);
                case PixelType.HRPPixelTypeI4R8G8B8A8 : 
                    return ConvertFromI4R8G8B8A8(buffer , palette);
                case PixelType.HRPPixelTypeI8Gray8 : 
                    return ConvertFromI8Gray8(buffer , palette);
                case PixelType.HRPPixelTypeI8R8G8B8 :
                    return ConvertFromI18R8G8B8(buffer , palette);
                case PixelType.HRPPixelTypeI8R8G8B8A8 :
                    return ConvertFromI8R8G8B8A8(buffer , palette);
                case PixelType.HRPPixelTypeI8R8G8B8Mask :
                    return ConvertFromI8R8G8B8Mask(buffer , palette);
                case PixelType.HRPPixelTypeI8VA8R8G8B8 :
                    return ConvertFromI8VAR8G8B8(buffer , palette);
                case PixelType.HRPPixelTypeV1Gray1 :
                    return ConvertFromV1Gray1(buffer);
                case PixelType.HRPPixelTypeV1GrayWhite1 :
                    return ConvertFromV1GrayWhite1(buffer);
                case PixelType.HRPPixelTypeV8Gray8 :
                    return ConvertFromV8Gray8(buffer);
                case PixelType.HRPPixelTypeV8GrayWhite8 :
                    return ConvertFromV8GrayWhite8(buffer);
                case PixelType.HRPPixelTypeV16B5G6R5 :
                    return ConvertFromV16B5G6R5(buffer);
                case PixelType.HRPPixelTypeV16Gray16 :
                    return ConvertFromV16Gray16(buffer);
                case PixelType.HRPPixelTypeV16Int16 :
                    return ConvertFromV16Int16(buffer);
                case PixelType.HRPPixelTypeV16PRGray8A8 :
                    return ConvertFromV16PRGray8A8(buffer);
                case PixelType.HRPPixelTypeV16R5G6B5 :
                    return ConvertFromV16R5G6B5(buffer);
                case PixelType.HRPPixelTypeV24B8G8R8 :
                    return ConvertFromV24B8G8R8(buffer);
                case PixelType.HRPPixelTypeV24PhotoYCC :
                    return ConvertFromV24PhotoYCC(buffer);
                case PixelType.HRPPixelTypeV24R8G8B8 :
                    return ConvertFromV24R8G8B8(buffer);
                case PixelType.HRPPixelTypeV32A8R8G8B8 :
                    return ConvertFromV32A8R8G8B8(buffer);
                case PixelType.HRPPixelTypeV32B8G8R8X8 :
                    return ConvertFromV32B8G8R8X8(buffer);
                case PixelType.HRPPixelTypeV32CMYK :
                    return ConvertFromV32CMYK(buffer);
                case PixelType.HRPPixelTypeV32Float32 :
                    return ConvertFromV32Float32(buffer);
                case PixelType.HRPPixelTypeV32PR8PG8PB8A8 :
                    return ConvertFromV32PR8PG8PB8A8(buffer);
                case PixelType.HRPPixelTypeV32PRPhotoYCCA8 :
                    return ConvertFromV32PRPhotoYCCA8(buffer);
                case PixelType.HRPPixelTypeV32R8G8B8A8 :
                    return ConvertFromV32R8G8B8A8(buffer);
                case PixelType.HRPPixelTypeV32B8G8R8A8 :
                    return buffer;
                case PixelType.HRPPixelTypeV32R8G8B8X8 :
                    return ConvertFromV32R8G8B8X8(buffer);
                case PixelType.HRPPixelTypeV48R16G16B16 :
                    return ConvertFromV48R16G16B16(buffer);
                case PixelType.HRPPixelTypeV64R16G16B16A16 :
                    return ConvertFromV64R16G16B16A16(buffer);
                case PixelType.HRPPixelTypeV64R16G16B16X16 :
                    return ConvertFromV64R16G16B16X16(buffer);
                case PixelType.HRPPixelTypeV96R32G32B32 :
                    return ConvertFromV96R32G32B32(buffer);
                default :
                    Debug.Assert(false , "Unrecognized pixel type"); 
                    return buffer; 

            }
        }

        private byte[] ConvertFromV32Float32(byte[] buffer)
        {
            return buffer; //no conversion in imagePP
        }

        private byte[] ConvertFromV24PhotoYCC(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length*4/3];
            int numberOfPixel = buffer.Length / 3;
            Int32 RDisplay;
            Int32 GDisplay;
            Int32 BDisplay;

            Int32 L;
            Int32 C1;
            Int32 C2;

            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                L = (int) (1.3587 * buffer[(i*3)]);
                C1 = (int) (2.2179 * (buffer[(i*3) + 1] - 156));
                C2 = (int) (1.8215 * (buffer[(i*3) + 2] - 137));

                RDisplay = L + C2;
                GDisplay = (int)(L - (0.194*C1) - (0.509*C2));
                BDisplay = L + C1;

                BDisplay = BDisplay < 0 ? 0 : BDisplay < 361 ? BDisplay : 360;
                GDisplay = GDisplay < 0 ? 0 : GDisplay < 361 ? GDisplay : 360;
                RDisplay = RDisplay < 0 ? 0 : RDisplay < 361 ? RDisplay : 360; 

                bufferConverted[( i*4 )] = s_LookUpTable[BDisplay];
                bufferConverted[( i*4 )+1] = s_LookUpTable[GDisplay];
                bufferConverted[( i*4 )+2] = s_LookUpTable[RDisplay];
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV16R5G6B5(byte[] buffer) // which side is msb??
        {
            byte[] bufferConverted = new byte[buffer.Length*2];
            int numberOfPixel = buffer.Length / 2;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                int value = ( ( buffer[( i*2 )] << 8 ) | buffer[( i*2 )+1] );
                bufferConverted[( i*4 )] = (byte) ( value & 0x1F );
                bufferConverted[( i*4 )+1] = (byte) ( value >> 5 & 0x3F );
                bufferConverted[( i*4 )+2] = (byte) ( value >> 11 );
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV16PRGray8A8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length*2];
            int numberOfPixel = buffer.Length / 2;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                if(buffer[(i*2)+1] != 0)
                {
                    byte value =(byte) (buffer[(i*2)]*255 / buffer[(i*2)+1]);
                    bufferConverted[( i*4 )] = value;
                    bufferConverted[( i*4 )+1] = value;
                    bufferConverted[( i*4 )+2] = value;
                }
                else
                {
                    bufferConverted[( i*4 )] = 0;
                    bufferConverted[( i*4 )+1] = 0;
                    bufferConverted[( i*4 )+2] = 0;
                }
                bufferConverted[( i*4 )+3] = buffer[( i*2 )+1];
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV16Int16(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length*2];
            int numberOfPixel = buffer.Length/2;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[( i*4 )] = (byte) ( buffer[i] - 0x8000 );
                bufferConverted[( i*4 )+1] = (byte) ( buffer[i] - 0x8000 );
                bufferConverted[( i*4 )+2] = (byte) ( buffer[i] - 0x8000 );
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }


        private byte[] ConvertFromV16Gray16(byte[] buffer) // which side is msb??
        {
            byte[] bufferConverted = new byte[buffer.Length*2];
            int numberOfPixel = buffer.Length/2;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[( i*4 )] = (byte)(buffer[i]);
                bufferConverted[( i*4 )+1] = (byte)(buffer[i]);
                bufferConverted[( i*4 )+2] = (byte) (buffer[i]);
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV16B5G6R5(byte[] buffer)  // which side is msb??
        {
            byte[] bufferConverted = new byte[buffer.Length*2];
            int numberOfPixel = buffer.Length / 2;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                int value = ((buffer[(i*2)] << 8) | buffer[(i*2)+1]);
                bufferConverted[( i*4 )] = (byte) ( value >> 11 );
                bufferConverted[( i*4 )+1] = (byte) ( value >> 5 & 0x3F );
                bufferConverted[( i*4 )+2] = (byte) ( value & 0x1F );
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV8GrayWhite8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length*4];
            int numberOfPixel = buffer.Length;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[( i*4 )] = (byte)(255-buffer[i]);
                bufferConverted[( i*4 )+1] = (byte) (255-buffer[i]);
                bufferConverted[( i*4 )+2] = (byte) (255-buffer[i]);
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV1GrayWhite1(byte[] buffer)
        {
            byte[] bufferConverted = new byte[( buffer.Length )*8*4];
            int numberOfPixel = buffer.Length*8;
            for( int i = 0 ; i < numberOfPixel ; i+=8 )
            {
                for( int j = 0 ; j < 8 ; j++ )
                {
                    if( ( buffer[i/8] >> j & 0x1 ) == 0 )
                    {
                        bufferConverted[( i*4 )+( j*4 )] = 255;
                        bufferConverted[( i*4 )+( j*4 )+1] = 255;
                        bufferConverted[( i*4 )+( j*4 )+2] = 255;
                        bufferConverted[( i*4 )+( j*4 )+3] = 255;
                    }
                    else
                    {
                        bufferConverted[( i*4 )+( j*4 )] = 0;
                        bufferConverted[( i*4 )+( j*4 )+1] = 0;
                        bufferConverted[( i*4 )+( j*4 )+2] = 0;
                        bufferConverted[( i*4 )+( j*4 )+3] = 255;
                    }

                }
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV1Gray1(byte[] buffer)
        {
            byte[] bufferConverted = new byte[( buffer.Length )*8*4];
            int numberOfPixel = buffer.Length*8;
            for( int i = 0 ; i < numberOfPixel ; i+=8 )
            {
                for( int j = 0 ; j < 8 ; j++ )
                {
                    if( ( buffer[i/8] >> j & 0x1 ) == 0 )
                    {
                        bufferConverted[(i*4)+(j*4)] = 0;
                        bufferConverted[( i*4 )+( j*4 )+1] = 0;
                        bufferConverted[( i*4 )+( j*4 )+2] = 0;
                        bufferConverted[( i*4 )+( j*4 )+3] = 255;
                    }
                    else
                    {
                        bufferConverted[( i*4 )+( j*4 )] = 255;
                        bufferConverted[( i*4 )+( j*4 )+1] = 255;
                        bufferConverted[( i*4 )+( j*4 )+2] = 255;
                        bufferConverted[( i*4 )+( j*4 )+3] = 255;
                    }

                }
            }
            return bufferConverted;
        }

        private byte[] ConvertFromI8VAR8G8B8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*2];
                int numberOfPixel = buffer.Length / 2;
                for( int i = 0 ; i < numberOfPixel ; i++ )
                {
                    RawPixelData pixelData = GetPixelDataFromPalette(buffer[i*2] , palette , 3);
                    bufferConverted[i*4] = (byte) pixelData.Channel3;
                    bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                    bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                    bufferConverted[( i*4 )+3] = buffer[( i*2 )+1];
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[1];
        }

        private byte[] ConvertFromI8R8G8B8Mask(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*4];
                int numberOfPixel = buffer.Length;
                for( int i = 0 ; i < numberOfPixel ; i+=8 )
                {
                    if( buffer[i] == 0 )
                    {
                        bufferConverted[i*4] = 0;
                        bufferConverted[( i*4 )+1] = 0;
                        bufferConverted[( i*4 )+2] = 0;
                        bufferConverted[( i*4 )+3] = 255;
                    }
                    else
                    {
                        bufferConverted[i*4] = 255;
                        bufferConverted[( i*4 )+1] = 255;
                        bufferConverted[( i*4 )+2] = 255;
                        bufferConverted[( i*4 )+3] = 255;
                    }
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromI8R8G8B8A8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*4];
                int numberOfPixel = buffer.Length;
                for( int i = 0 ; i < numberOfPixel ; i++ )
                {
                    RawPixelData pixelData = GetPixelDataFromPalette(buffer[i] , palette , 4);
                    bufferConverted[i*4] = (byte) pixelData.Channel3;
                    bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                    bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                    bufferConverted[( i*4 )+3] = (byte) pixelData.Channel4;
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromI8Gray8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferHalfConverted = new byte[( buffer.Length )];
                int numberOfPixel = buffer.Length;
                for( int i = 0 ; i < numberOfPixel ; i++ )
                {
                    bufferHalfConverted[i] = (byte) GetPixelDataFromPalette(buffer[i] , palette , 1).Channel1;
                }
                return ConvertFromV8Gray8(bufferHalfConverted);
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromI4R8G8B8A8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*4*2];
                int numberOfPixel = buffer.Length*2;
                for( int i = 0 ; i < numberOfPixel ; i+=2 )
                {
                    for( int j = 0 ; j < 8 ; j+=4 )
                    {
                        RawPixelData pixelData = GetPixelDataFromPalette(( buffer[i] >> j ) & 0x3 , palette , 4);
                        bufferConverted[i*4] = (byte) pixelData.Channel3;
                        bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                        bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                        bufferConverted[( i*4 )+3] = (byte) pixelData.Channel4;
                    }
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromI4R8G8B8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*4*2];
                int numberOfPixel = buffer.Length*2;
                for( int i = 0 ; i < numberOfPixel ; i+=2 )
                {
                    for( int j = 0 ; j < 8 ; j+=4 )
                    {
                        RawPixelData pixelData = GetPixelDataFromPalette(( buffer[i] >> j ) & 0x3 , palette , 3);
                        bufferConverted[i*4] = (byte) pixelData.Channel3;
                        bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                        bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                        bufferConverted[( i*4 )+3] = 255;
                    }
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromI2R8G8B8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*4*4];
                int numberOfPixel = buffer.Length*4;
                for( int i = 0 ; i < numberOfPixel ; i+=4 )
                {
                    for( int j = 0 ; j < 8 ; j+=2 )
                    {
                        RawPixelData pixelData = GetPixelDataFromPalette(( buffer[i] >> j ) & 0x3 , palette , 3);
                        bufferConverted[i*4] = (byte) pixelData.Channel3;
                        bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                        bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                        bufferConverted[( i*4 )+3] = 255;
                    }
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromI1R8G8B8RLE(byte[] buffer , byte[] palette , int width , int height)
        {
            byte[] bufferConverted = new byte[width*height*4];
            int numberOfPixel = width*height;
            RawPixelData[] colorPixel = new RawPixelData[2];
            if( palette != null )
            {
                colorPixel[0] = GetPixelDataFromPalette(0 , palette , 3);
                colorPixel[1] = GetPixelDataFromPalette(1 , palette , 3);
            }
            else
            {
                colorPixel[0] = new RawPixelData();
                colorPixel[1] = new RawPixelData();
                colorPixel[0].Channel1 = colorPixel[0].Channel2 = colorPixel[0].Channel3 = 0;
                colorPixel[1].Channel1 = colorPixel[1].Channel2 = colorPixel[1].Channel3 = 255; 
            }
            int color = 1;
            int startingIndex = 0;
            for( int i = 0 ; i < buffer.Length-1 ; i+=2 )
            {
                int count = buffer[i];
                count  = ( count | buffer[i+1] << 8 );
                for( int j = 0 ; j < count && startingIndex + j*4 + 3 < bufferConverted.Length ; j++ )
                {
                    if((startingIndex + (j * 4)) % (width*4) == 0)
                        color = ( color == 0 ? 1 : 0 );
                    bufferConverted[startingIndex + j*4] = (byte) colorPixel[color].Channel3;
                    bufferConverted[startingIndex + j*4 + 1] = (byte) colorPixel[color].Channel2;
                    bufferConverted[startingIndex + j*4 + 2] = (byte) colorPixel[color].Channel1;
                    bufferConverted[startingIndex + j*4 + 3] = 255;
                }
                startingIndex += ( count * 4 );
                color = (color == 0 ? 1 : 0);
            }
            return bufferConverted;
        }

        private byte[] ConvertFromI1R8G8B8A8RLE(byte[] buffer , byte[] palette , int width , int height)
        {
            byte[] bufferConverted = new byte[width*height*4];
            int numberOfPixel = width*height;
            RawPixelData[] colorPixel = new RawPixelData[2];
            if( palette != null )
            {
                colorPixel[0] = GetPixelDataFromPalette(0 , palette , 4);
                colorPixel[1] = GetPixelDataFromPalette(1 , palette , 4);
            }
            else
            {
                colorPixel[0] = new RawPixelData();
                colorPixel[1] = new RawPixelData();
                colorPixel[0].Channel1 = colorPixel[0].Channel2 = colorPixel[0].Channel3 = colorPixel[0].Channel4 = 0;
                colorPixel[1].Channel1 = colorPixel[1].Channel2 = colorPixel[1].Channel3 = colorPixel[1].Channel4 = 255;
            }
            int color = 1;
            int startingIndex = 0;
            for( int i = 0 ; i < buffer.Length ; i+=2 )
            {
                int count = buffer[i];
                count  = ( count | buffer[i+1] << 8 );
                for( int j = 0 ; j < count ; j++ )
                {
                    if( ( startingIndex + ( j * 4 ) ) % ( width*4 ) == 0 )
                        color = ( color == 0 ? 1 : 0 );
                    bufferConverted[startingIndex + j*4] = (byte) colorPixel[color].Channel3;
                    bufferConverted[startingIndex + j*4 + 1] = (byte) colorPixel[color].Channel2;
                    bufferConverted[startingIndex + j*4 + 2] = (byte) colorPixel[color].Channel1;
                    bufferConverted[startingIndex + j*4 + 3] = (byte) colorPixel[color].Channel4;
                }
                startingIndex += (count * 4);
                color = (color == 0 ? 1 : 0);
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV32CMYK(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length];
            int numberOfPixel = bufferConverted.Length/4;
            int black; 
            int red;
            int green;
            int blue;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                black = 255 - buffer[i*4+3];
                red = ( black * ( 255 - buffer[i*4] ) ) / 255;
                green = ( black * ( 255 - buffer[i*4+1] ) ) / 255;
                blue = ( black * ( 255 - buffer[i*4+2] ) )/255;

                bufferConverted[i*4] = (byte) ( AdjustementMatrixFromCMYK[0 , 0]*red + AdjustementMatrixFromCMYK[0 , 1]*green + AdjustementMatrixFromCMYK[0 , 2]*blue );
                bufferConverted[i*4 + 1] = (byte) (AdjustementMatrixFromCMYK[1 , 0]*red + AdjustementMatrixFromCMYK[1 , 1]*green + AdjustementMatrixFromCMYK[1 , 2]*blue);
                bufferConverted[i*4 + 2] = (byte) (AdjustementMatrixFromCMYK[2 , 0]*red + AdjustementMatrixFromCMYK[2 , 1]*green + AdjustementMatrixFromCMYK[2 , 2]*blue);
                bufferConverted[i*4 + 3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV32PR8PG8PB8A8(byte[] buffer)//do we need to change according to endianess of computer??
        {
            byte[] bufferConverted = new byte[buffer.Length];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                if( buffer[(i*4)+3] != 0 )
                {
                    if( ((buffer[(i*4) + 2] * 255) / buffer[(i*4)+3]) < 255 )
                        bufferConverted[i*4] =  (byte) ((buffer[i*4 + 2] * 255) / buffer[(i*4)+3]); 
                    else
                        bufferConverted[i*4] = 255;
                    if( ((buffer[(i*4) + 1] * 255) / buffer[(i*4)+3]) < 255 )
                        bufferConverted[i*4 + 1] = (byte)((buffer[i*4 + 1] * 255) / buffer[(i*4)+3]);
                    else
                        bufferConverted[(i*4) + 1] = 255;
                    if( ((buffer[(i*4)] * 255) / buffer[(i*4)+3]) < 255 )
                        bufferConverted[(i*4) + 2] = (byte) ((buffer[i*4] * 255) / buffer[(i*4)+3]);
                    else
                        bufferConverted[i*4 + 2] = 255;
                }
                else
                {
                    bufferConverted[i*4] = 0;
                    bufferConverted[i*4 + 1] = 0;
                    bufferConverted[i*4 + 2] = 0;
                }

                bufferConverted[( i*4 )+ 3] = buffer[( i*4 )+3];
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV32PRPhotoYCCA8(byte[] buffer) 
        {
            byte[] bufferConverted = new byte[buffer.Length];
            int numberOfPixel = bufferConverted.Length/4;
            double AlphaPercent;
            double PreMultY;
            int R;
            int B;
            int G;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                PreMultY = 1.35689*buffer[i*4];
                AlphaPercent = buffer[i*4+3] / (double)255;
                
                B = (int) Math.Round( PreMultY + (2.218*buffer[i*4 + 1]) + (-395.94*AlphaPercent));
                G = (int) Math.Round( PreMultY + (-0.43*buffer[i*4 + 1]) + (-0.927*buffer[i*4 + 2]) +(1994.44*AlphaPercent));
                R = (int) Math.Round( PreMultY + (1.82187*buffer[i*4 + 2]) + (-249.571*AlphaPercent));

                B = B < 0 ? 0 : B < 361 ? B : 360;
                G = G < 0 ? 0 : G < 361 ? G : 360;
                R = R < 0 ? 0 : R < 361 ? R : 360; 

                bufferConverted[i*4] = s_LookUpTable[B];
                bufferConverted[i*4 + 1] = s_LookUpTable[G];
                bufferConverted[i*4 + 2] = s_LookUpTable[R];
                bufferConverted[i*4 + 3] = buffer[i*4+3];
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV32A8R8G8B8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*4+3];
                bufferConverted[i*4 + 1] = buffer[i*4 + 2];
                bufferConverted[i*4 + 2] = buffer[i*4 + 1];
                bufferConverted[i*4 + 3] = buffer[i*4];
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV32B8G8R8X8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*4];
                bufferConverted[i*4 + 1] = buffer[i*4 + 1];
                bufferConverted[i*4 + 2] = buffer[i*4 + 2];
                bufferConverted[i*4 + 3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV32R8G8B8A8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*4 +2];
                bufferConverted[i*4 + 1] = buffer[i*4 + 1];
                bufferConverted[i*4 + 2] = buffer[i*4];
                bufferConverted[i*4 + 3] = buffer[i*4 + 3];
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV32R8G8B8X8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*4 +2];
                bufferConverted[i*4 + 1] = buffer[i*4 + 1];
                bufferConverted[i*4 + 2] = buffer[i*4];
                bufferConverted[i*4 + 3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV48R16G16B16(byte[] buffer)
        {
            byte[] bufferConverted = new byte[( (buffer.Length/6)*4 )];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*6 +4];
                bufferConverted[i*4 + 1] = buffer[i*6 + 2];
                bufferConverted[i*4 + 2] = buffer[i*6];
                bufferConverted[i*4 + 3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV64R16G16B16A16(byte[] buffer)
        {
            byte[] bufferConverted = new byte[( buffer.Length/2 )];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*8 + 4];
                bufferConverted[i*4 + 1] = buffer[i*8 + 2];
                bufferConverted[i*4 + 2] = buffer[i*8];
                bufferConverted[i*4 + 3] = buffer[i*8 + 6];
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV64R16G16B16X16(byte[] buffer)
        {
            byte[] bufferConverted = new byte[( buffer.Length/2 )];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*8+4];
                bufferConverted[i*4 + 1] = buffer[i*8 + 2];
                bufferConverted[i*4 + 2] = buffer[i*8];
                bufferConverted[i*4 + 3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV96R32G32B32(byte[] buffer)
        {
            byte[] bufferConverted = new byte[(buffer.Length/3)];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[i*12+9];
                bufferConverted[i*4 + 1] = buffer[i*12 +4];
                bufferConverted[i*4 + 2] = buffer[i*12];
                bufferConverted[i*4 + 3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV24B8G8R8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[(buffer.Length/3)*4];
            int numberOfPixel = buffer.Length /3;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[i*4] = buffer[( i*3 )];
                bufferConverted[( i*4 )+1] = buffer[( i*3 )+1];
                bufferConverted[( i*4 )+2] = buffer[( i*3 )+2];
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromI1R8G8B8A8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*4*8];
                int numberOfPixel = buffer.Length*8;
                for( int i = 0 ; i < numberOfPixel ; i+=8 )
                {
                    for( int j = 0 ; j < 8 ; j++ )
                    {
                        RawPixelData pixelData = GetPixelDataFromPalette(( buffer[i] >> j ) & 0x1 , palette , 4);
                        bufferConverted[i*4] = (byte) pixelData.Channel3;
                        bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                        bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                        bufferConverted[( i*4 )+3] = (byte) pixelData.Channel4;
                    }
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromI1R8G8B8(byte[] buffer , byte[] palette)
        {
            if( palette != null )
            {
                byte[] bufferConverted = new byte[( buffer.Length )*4*8];
                int numberOfPixel = buffer.Length*8;
                for( int i = 0 ; i < numberOfPixel ; i+=8 )
                {
                    for( int j = 0 ; j < 8 ; j++ )
                    {
                        RawPixelData pixelData = GetPixelDataFromPalette(buffer[i] >> j , palette , 3);
                        bufferConverted[i*4] = (byte) pixelData.Channel3;
                        bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                        bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                        bufferConverted[( i*4 )+3] = 255;
                    }
                }
                return bufferConverted;
            }
            Debug.Assert(false , "Palette not specified"); 
            return new byte[0];
        }

        private byte[] ConvertFromV8Gray8(byte[] buffer)
        {
            byte[] bufferConverted = new byte[buffer.Length*4];
            int numberOfPixel = buffer.Length;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                bufferConverted[(i*4)] = buffer[i];
                bufferConverted[(i*4)+1] = buffer[i];
                bufferConverted[(i*4)+2] = buffer[i];
                bufferConverted[(i*4)+3] = 255;
            }
            return bufferConverted;
        }

        private byte[] ConvertFromV24R8G8B8(byte[] buffer)
        {
                byte[] bufferConverted = new byte[( buffer.Length /3 )*4];
                int numberOfPixel = bufferConverted.Length/4;
                for( int i = 0 ; i < numberOfPixel ; i++ )
                {
                    bufferConverted[i*4] = buffer[( i*3 )+2];
                    bufferConverted[( i*4 )+1] = buffer[( i*3 )+1];
                    bufferConverted[( i*4 )+2] = buffer[( i*3 )];
                    bufferConverted[( i*4 )+3] = 255;
                }
                return bufferConverted;
        }
        private byte[] ConvertFromI18R8G8B8(byte[] buffer , byte[] palette)
        {
            byte[] bufferConverted = new byte[( buffer.Length)*4];
            int numberOfPixel = bufferConverted.Length/4;
            for( int i = 0 ; i < numberOfPixel ; i++ )
            {
                RawPixelData pixelData = GetPixelDataFromPalette(buffer[i] , palette, 3);
                bufferConverted[i*4] = (byte)pixelData.Channel3;
                bufferConverted[( i*4 )+1] = (byte) pixelData.Channel2;
                bufferConverted[( i*4 )+2] = (byte) pixelData.Channel1;
                bufferConverted[( i*4 )+3] = 255;
            }
            return bufferConverted;
        }

        public RawPixelData GetPixelDataFromPalette(int Index , byte[] palette, int paletteEntrySize)
        {
            RawPixelData pixelData = new RawPixelData();
            pixelData.PaletteIndex = Index;
            if( paletteEntrySize > 1 )
            {
                pixelData.Channel3 = palette[Index*paletteEntrySize+2];
                pixelData.Channel2 = palette[Index*paletteEntrySize+1];
            }
            pixelData.Channel1 = palette[Index*paletteEntrySize];    
            if(paletteEntrySize == 4 )
                pixelData.Channel4 = palette[Index*paletteEntrySize+3];
            else
                pixelData.Channel4 = -1;
            return pixelData;
        }

        public RawPixelData GetPixelData(int x , int y , byte[] buffer , PixelType pixelType , int width, int height, byte[] palette) // to correct
        {
            int nbOfChannel = GetNumberOfChannel(pixelType);
            int numberOfBits = GetNumberOfBitsPerPixel(pixelType);
            if( !HasPalette(pixelType) )
            {
                if( pixelType == PixelType.HRPPixelTypeV1Gray1 || pixelType == PixelType.HRPPixelTypeV1GrayWhite1 )
                {
                    int pixelNumber = (int)(( y*width + x )/8.0);
                    int value = buffer[pixelNumber];
                    value = ( value >> ( ( y*width + x )%8 ) & 0x1 );
                    RawPixelData pixelData = new RawPixelData();
                    pixelData.Channel1 = value;
                    pixelData.Channel2 = pixelData.Channel3 = pixelData.Channel4 = -1;
                    return pixelData;
                }
                else
                {
                    RawPixelData pixelData = new RawPixelData();
                    pixelData.Channel1 = buffer[( ( numberOfBits/8 )*y*width )+( x*( numberOfBits/8 ) )];
                    if( nbOfChannel > 1 )
                    {
                        pixelData.Channel2 = buffer[( ( numberOfBits/8 )*y*width )+( x*( numberOfBits/8 ) ) + (numberOfBits/(nbOfChannel*8))];   
                        if( nbOfChannel > 2 )
                        {
                            pixelData.Channel3 = buffer[( ( numberOfBits/8 )*y*width )+( x*( numberOfBits/8 ) )+ (2* ( numberOfBits/( nbOfChannel*8 ) ))];
                            if( nbOfChannel > 3 )
                                pixelData.Channel4 = buffer[( ( numberOfBits/8 )*y*width )+( x*( numberOfBits/8 ) )+ (3* ( numberOfBits/( nbOfChannel*8 ) ))];
                            else
                                pixelData.Channel4 = -1;
                        }
                        else
                        {
                            pixelData.Channel3 = -1;
                            pixelData.Channel4 = -1;
                        }
                    }
                    else
                    {
                        pixelData.Channel2 = -1;
                        pixelData.Channel3 = -1;
                        pixelData.Channel4 = -1;
                    }
                    return pixelData;
                }
            }
            else
            {
                if( PixelType.HRPPixelTypeI1R8G8B8A8RLE == pixelType || PixelType.HRPPixelTypeI1R8G8B8RLE == pixelType )
                {
                    buffer = GetUncompressedRLEBuffer(buffer , width , height);
                    numberOfBits = 8;
                }
                int pixelNumber = (int) ( ( ( y*width + x )/8.0 )*numberOfBits );

                    int index = buffer[pixelNumber];
                    if(numberOfBits == 1)
                        index = (index >> ((y*width + x)%8) & 0x1);
                    if(numberOfBits == 2)
                        index = (index >> (((y*width + x)*2)%8) & 0x3);
                    if(numberOfBits == 4)
                        index = (index >> (((y*width + x)*4)%8) & 0xF);
                    if( pixelType != PixelType.HRPPixelTypeI8VA8R8G8B8 )
                        return GetPixelDataFromPalette(index , palette , nbOfChannel);
                    else
                    {
                        RawPixelData pixel = GetPixelDataFromPalette(index , palette , nbOfChannel-1);
                        pixel.Channel4 = buffer[pixelNumber + 1];
                        return pixel;
                    }
            }
        }

        private byte[] GetUncompressedRLEBuffer(byte[] buffer, int width, int height)
        {
            byte[] bufferConverted = new byte[width*height];
            int numberOfPixel = width*height;
            int color = 1;
            int startingIndex = 0;
            for( int i = 0 ; i < buffer.Length ; i+=2 )
            {
                int count = buffer[i];
                count  = ( count | buffer[i+1] << 8 );
                for( int j = 0 ; j < count ; j++ )
                {
                    if( ( startingIndex + j) % (width) == 0 )
                        color = ( color == 0 ? 1 : 0 );
                    bufferConverted[startingIndex + j] = (byte) color;
                }
                startingIndex += count;
                color = ( color == 0 ? 1 : 0 );
            }
            return bufferConverted;
        }

        public static int GetNumberOfBitsPerPixel(PixelType pixelType)
        {
            switch( pixelType )
            {
                case PixelType.HRPPixelTypeV1Gray1:
                case PixelType.HRPPixelTypeV1GrayWhite1:
                case PixelType.HRPPixelTypeI1R8G8B8:
                case PixelType.HRPPixelTypeI1R8G8B8RLE:
                case PixelType.HRPPixelTypeI1R8G8B8A8:
                case PixelType.HRPPixelTypeI1R8G8B8A8RLE:
                    return 1;
                case PixelType.HRPPixelTypeI2R8G8B8:
                    return 2;
                case PixelType.HRPPixelTypeI4R8G8B8:
                case PixelType.HRPPixelTypeI4R8G8B8A8:
                    return 4;
                case PixelType.HRPPixelTypeI8Gray8:
                case PixelType.HRPPixelTypeV8Gray8:
                case PixelType.HRPPixelTypeV8GrayWhite8:
                case PixelType.HRPPixelTypeI8R8G8B8:
                case PixelType.HRPPixelTypeI8R8G8B8Mask:
                case PixelType.HRPPixelTypeI8R8G8B8A8:
                    return 8;
                case PixelType.HRPPixelTypeI8VA8R8G8B8:
                case PixelType.HRPPixelTypeV16Gray16:
                case PixelType.HRPPixelTypeV16Int16:
                case PixelType.HRPPixelTypeV16PRGray8A8:
                case PixelType.HRPPixelTypeV16B5G6R5:
                case PixelType.HRPPixelTypeV16R5G6B5:
                    return 16;
                case PixelType.HRPPixelTypeV24B8G8R8:
                case PixelType.HRPPixelTypeV24PhotoYCC:
                case PixelType.HRPPixelTypeV24R8G8B8:
                    return 24;
                case PixelType.HRPPixelTypeV32Float32:
                case PixelType.HRPPixelTypeV32A8R8G8B8:
                case PixelType.HRPPixelTypeV32B8G8R8X8:
                case PixelType.HRPPixelTypeV32CMYK:
                case PixelType.HRPPixelTypeV32PR8PG8PB8A8:
                case PixelType.HRPPixelTypeV32PRPhotoYCCA8:
                case PixelType.HRPPixelTypeV32R8G8B8A8:
                case PixelType.HRPPixelTypeV32B8G8R8A8:
                case PixelType.HRPPixelTypeV32R8G8B8X8:
                    return 32;
                case PixelType.HRPPixelTypeV48R16G16B16:
                    return 48;
                case PixelType.HRPPixelTypeV64R16G16B16A16:
                case PixelType.HRPPixelTypeV64R16G16B16X16:
                    return 64;
                case PixelType.HRPPixelTypeV96R32G32B32:
                    return 96;
                default:
                    Debug.Assert(false , "Pixel type not recognized"); 
                    return 32;//add assert
            }
        }

        public static int GetNumberOfChannel(PixelType pixelType)
        {
            switch( pixelType )
            {
                case PixelType.HRPPixelTypeI8Gray8:
                case PixelType.HRPPixelTypeV1Gray1:
                case PixelType.HRPPixelTypeV1GrayWhite1:
                case PixelType.HRPPixelTypeV8Gray8:
                case PixelType.HRPPixelTypeV8GrayWhite8:
                case PixelType.HRPPixelTypeV16Gray16:
                case PixelType.HRPPixelTypeV16Int16:
                case PixelType.HRPPixelTypeV32Float32:
                    return 1;
                case PixelType.HRPPixelTypeV16PRGray8A8:
                    return 2;
                case PixelType.HRPPixelTypeI1R8G8B8:
                case PixelType.HRPPixelTypeI1R8G8B8RLE:
                case PixelType.HRPPixelTypeI2R8G8B8:
                case PixelType.HRPPixelTypeI4R8G8B8:
                case PixelType.HRPPixelTypeI8R8G8B8:
                case PixelType.HRPPixelTypeI8R8G8B8Mask:
                case PixelType.HRPPixelTypeV16B5G6R5:
                case PixelType.HRPPixelTypeV16R5G6B5:
                case PixelType.HRPPixelTypeV24B8G8R8:
                case PixelType.HRPPixelTypeV24PhotoYCC:
                case PixelType.HRPPixelTypeV24R8G8B8:
                case PixelType.HRPPixelTypeV48R16G16B16:
                case PixelType.HRPPixelTypeV96R32G32B32:
                    return 3;
                case PixelType.HRPPixelTypeI1R8G8B8A8:
                case PixelType.HRPPixelTypeI1R8G8B8A8RLE:
                case PixelType.HRPPixelTypeI4R8G8B8A8:
                case PixelType.HRPPixelTypeI8R8G8B8A8:
                case PixelType.HRPPixelTypeI8VA8R8G8B8:
                case PixelType.HRPPixelTypeV32A8R8G8B8:
                case PixelType.HRPPixelTypeV32B8G8R8X8:
                case PixelType.HRPPixelTypeV32CMYK:
                case PixelType.HRPPixelTypeV32PR8PG8PB8A8:
                case PixelType.HRPPixelTypeV32PRPhotoYCCA8:
                case PixelType.HRPPixelTypeV32R8G8B8A8:
                case PixelType.HRPPixelTypeV32B8G8R8A8:
                case PixelType.HRPPixelTypeV32R8G8B8X8:
                case PixelType.HRPPixelTypeV64R16G16B16A16:
                case PixelType.HRPPixelTypeV64R16G16B16X16:
                default:
                    Debug.Assert(false , "Pixel type not recognized"); 
                    return 4;//add assert
            }
        }

        public static bool HasPalette(PixelType pixelType)
        {
            switch( pixelType )
            {
                case PixelType.HRPPixelTypeI1R8G8B8:
                case PixelType.HRPPixelTypeI1R8G8B8A8:
                case PixelType.HRPPixelTypeI1R8G8B8A8RLE:
                case PixelType.HRPPixelTypeI1R8G8B8RLE:
                case PixelType.HRPPixelTypeI2R8G8B8:
                case PixelType.HRPPixelTypeI4R8G8B8:
                case PixelType.HRPPixelTypeI4R8G8B8A8:
                case PixelType.HRPPixelTypeI8Gray8:
                case PixelType.HRPPixelTypeI8R8G8B8:
                case PixelType.HRPPixelTypeI8R8G8B8A8:
                case PixelType.HRPPixelTypeI8R8G8B8Mask:
                case PixelType.HRPPixelTypeI8VA8R8G8B8:
                    return true;
                default:
                    return false;
            }
        }
        public static bool HasPalette(string pixelType)
        {
            return HasPalette(GetPixelType(pixelType));
        }


        public static int GetNumberOfEntryForPalette(PixelType pixelType)
        {
            switch( pixelType )
            {
                case PixelType.HRPPixelTypeI1R8G8B8:
                case PixelType.HRPPixelTypeI1R8G8B8A8:
                case PixelType.HRPPixelTypeI1R8G8B8A8RLE:
                case PixelType.HRPPixelTypeI1R8G8B8RLE:
                    return 2;
                case PixelType.HRPPixelTypeI2R8G8B8:
                    return 4;
                case PixelType.HRPPixelTypeI4R8G8B8:
                case PixelType.HRPPixelTypeI4R8G8B8A8:
                    return 16;
                case PixelType.HRPPixelTypeI8Gray8:
                case PixelType.HRPPixelTypeI8R8G8B8:
                case PixelType.HRPPixelTypeI8R8G8B8A8:
                case PixelType.HRPPixelTypeI8R8G8B8Mask:
                case PixelType.HRPPixelTypeI8VA8R8G8B8:
                    return 256;
                default:
                    return 0;
            }
        }
        public static int GetPaletteEntrySize(PixelType pixelType)
        {
            switch( pixelType )
            {
                case PixelType.HRPPixelTypeI8VA8R8G8B8:
                case PixelType.HRPPixelTypeI8R8G8B8:
                case PixelType.HRPPixelTypeI4R8G8B8:
                case PixelType.HRPPixelTypeI2R8G8B8:
                case PixelType.HRPPixelTypeI1R8G8B8RLE:
                case PixelType.HRPPixelTypeI1R8G8B8:
                    return 3;
                case PixelType.HRPPixelTypeI8R8G8B8A8:
                case PixelType.HRPPixelTypeI8R8G8B8Mask:
                case PixelType.HRPPixelTypeI4R8G8B8A8:
                case PixelType.HRPPixelTypeI1R8G8B8A8RLE:
                case PixelType.HRPPixelTypeI1R8G8B8A8:
                    return 4;
                case PixelType.HRPPixelTypeI8Gray8:
                    return 1;
                default:
                    return 3;
            }
        }
    }
}
