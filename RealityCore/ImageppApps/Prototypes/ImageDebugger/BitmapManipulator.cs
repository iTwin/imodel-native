/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/BitmapManipulator.cs $
|    $RCSfile: BitmapManipulator.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/27 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Windows;

namespace Bentley.ImageViewer
{
    /// <summary>Utility class allowing manipulation on bitmaps</summary>
    /// <author>Julien Rossignol</author>
    public class BitmapManipulator
    {

        /*------------------------------------------------------------------------------------**/
        /// <summary>Transform the specified byte array to a System.Drawing.Bitmap</summary>  
        /// <author>Julien Rossignol</author>                            <date>08/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public Bitmap GetBitmapFromByteArray(int width , int height , byte[] buffer)
        {
            try
            {
                if( width*height*4 <= buffer.Length )
                {
                    Bitmap bitmap = new Bitmap(width , height); //initialised bitmap rectangle
                    Rectangle bitmapRect = new Rectangle(0 , 0 , width , height);
                    BitmapData bitmapData = bitmap.LockBits(bitmapRect , ImageLockMode.ReadWrite , bitmap.PixelFormat); //System.Bitmap.Drawing cannot be construct directly from byte array, we lock in memory the buffer
                    Marshal.Copy(buffer , 0 , bitmapData.Scan0 , width*height*4); //we then copy the entire buffer into the scan0 variable of the bitmapdata
                    bitmap.UnlockBits(bitmapData); //we unlock the buffer in memory

                    return bitmap;
                }
                else
                    return null;
            }
            catch
            {
                return null;
            }
        }
        /*------------------------------------------------------------------------------------**/
        /// <summary>Return a 175*100 thumbnail of the specified bitmap</summary>  
        /// <author>Julien Rossignol</author>                            <date>08/2013</date>
        /*--------------+---------------+---------------+---------------+---------------+------*/
        public Bitmap GetThumbnail(Bitmap bitmap, Color background)
        {
            Bitmap thumbnail = new Bitmap(175 , 100); //construct bitmap with good size
            Graphics graphics = Graphics.FromImage(thumbnail);
            graphics.Clear(background); //fill background with the specified color
            graphics.DrawImage(new Bitmap(bitmap,175,100), new PointF()); // draw the stretched bitmap on the thumbnail
            return thumbnail;
        }

    }
}
