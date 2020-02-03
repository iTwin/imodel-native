/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/GWP_UnitTestingApp/Geo Web Publisher Unit Testing App/rasterManipulation.cs $
|    $RCSfile: rasterManipulation.cs, $
|   $Revision: 1 $
|       $Date: 2013/06/03 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.IO;
using System.Drawing;


namespace Geo_Web_Publisher_Unit_Testing_App
{
    /// <summary>Classs used to perform basic operation on a raster</summary>
    /// <author>Julien Rossignol</author>
    class RasterManipulation
    {
        #region methods

        /*------------------------------------------------------------------------------------**/
        /// <summary>Transform a bitmap to a Byte[]</summary>  
        /// <param name="image">Bitmap to transform</param>
        /// <returns>Byte[] created from the bitmap</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public Byte[] ConvertImageToByteArray(Bitmap image)
        {
            Bitmap copyToSave = new Bitmap(image); // copy the bitmap to prevent errors when saving
            using( MemoryStream stream = new MemoryStream() )
            {
                copyToSave.Save(stream , System.Drawing.Imaging.ImageFormat.Png); //save the bitmap n a stream
                return stream.ToArray(); //extract a byte[] from the stream
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Transform a byte array to a bitmap</summary>  
        /// <param name="image">Byte array to convert</param>
        /// <returns>Bitmap created from the byte[]</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        static public Bitmap ConvertByteArrayToBitmap(Byte[] image)
        {
            try
            {
                using( MemoryStream stream = new MemoryStream(image) )
                {
                    return new System.Drawing.Bitmap(stream); //extract bitmap from the stream containing the byte[]
                }
            }
            catch
            {
                return null; //return null if the byte[] does not contains the information for a bitmap (probably a text file)
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Calculate the difference bitmap beetween two byte[]</summary>  
        /// <param name="comparison">byte[] used as a comparison(output from the request)</param>
        /// <param name="baseline">byte[] used as a baseline</param>
        /// <returns>Tricolor bitmap indicating difference between both byte[]</returns>
        /// <author>Julien Rossignol</author>                            <date>06/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public static Bitmap GetDifferenceBitmap(byte[] comparison , byte[] baseline)
        {

            Bitmap comparisonBitmap = RasterManipulation.ConvertByteArrayToBitmap(comparison); //convert the comparison bitmap
            Bitmap baselineBitmap = RasterManipulation.ConvertByteArrayToBitmap(baseline); //convert the baseline bitmap
            Bitmap difference = new Bitmap(comparisonBitmap); // create a new bitmap from the comparison one
            if( baselineBitmap != null )
            {
                if( comparisonBitmap.Width == baselineBitmap.Width && comparisonBitmap.Height == baselineBitmap.Height ) //the difference bitmap is not compute if both bitmap are not the same size
                {
                    Color backgroundColor = Properties.GeoWebPublisherUnitTestingApp.Default.BackgroundColor; //initiliaze pixel color according to settings
                    Color bigDifferenceColor = Properties.GeoWebPublisherUnitTestingApp.Default.BigDifferenceColor;
                    Color smallDifferenceColor = Properties.GeoWebPublisherUnitTestingApp.Default.SmallDifferenceColor;

                    for( int i = 0 ; i < comparisonBitmap.Height ; i++ ) // goes through all bitmap row
                    {
                        for( int j = 0 ; j < comparisonBitmap.Width ; j++ ) // goes through all bitmap columns
                        {
                            Color responseColor = comparisonBitmap.GetPixel(j , i);
                            Color comparisonColor = baselineBitmap.GetPixel(j , i);
                            if( responseColor != comparisonColor ) // compare both color
                            {
                                if( Math.Abs(responseColor.R - comparisonColor.R) > 15 || Math.Abs(responseColor.G - comparisonColor.G) > 15 || Math.Abs(responseColor.B - comparisonColor.B) > 15 ) // if one color has a difference more than 15 
                                    difference.SetPixel(j , i , bigDifferenceColor);
                                else
                                    difference.SetPixel(j , i , smallDifferenceColor);
                            }
                            else
                                difference.SetPixel(j , i , backgroundColor); // if the color are the same
                        }
                    }
                }
            }
            return difference;

        }

        #endregion
    }
}
