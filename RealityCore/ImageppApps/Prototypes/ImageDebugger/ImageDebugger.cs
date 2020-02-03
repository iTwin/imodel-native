using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace Bentley.ImageViewer
{
    public class ImageDebugger
    {
        private byte[] m_UnconvertedBuffer;
        private PixelType m_PixelType;
        private int m_BaseWidth;
        private int m_BaseHeight;
        private int m_Width;
        private int m_Height;
        private PixelType m_BasePixelType;
        private Bitmap m_ZoomInBitmap;
        private int m_ZoomFactor = 1;
        private Bitmap m_Bitmap;
        private Point m_PointToPaint;
        private byte[] m_BitmapData;
        private byte[] m_Palette;
        private System.Windows.Point m_MouseStartingPoint;
        private DebuggerView m_View;
        private PixelConverter m_Converter;
        private BitmapManipulator m_Manipulator;
        private Color m_BackgroundColor;
        private string m_CurrentVariable;
        private WatchWindowModel m_WatchWindowModel;

        public ImageDebugger(ImageViewerPackage parent)
        {
            m_PointToPaint = new Point(0 , 0);
            m_Converter = new PixelConverter();
            m_Manipulator = new BitmapManipulator();
            m_View = parent.GetControl();
            m_View.SetModel(this);
        }

        public void SetBitmap(string pixelType , int width , int height , byte[] bitmapData, byte[] palette, string currentVariable)
        {
            m_BasePixelType = m_PixelType = PixelConverter.GetPixelType(pixelType);
            if( m_PixelType != PixelType.HRPPixelTypeV32B8G8R8A8 )
            {
                m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(bitmapData , m_PixelType , palette , width , height);
            }
            else
                m_BitmapData = m_UnconvertedBuffer;
            m_Bitmap = m_Manipulator.GetBitmapFromByteArray(width , height , m_BitmapData);
            m_ZoomInBitmap = m_Bitmap;
            m_ZoomFactor = 1;
            m_PointToPaint = new Point(0 , 0);
            m_BaseWidth = m_Width = width;
            m_Palette = palette;
            m_CurrentVariable = currentVariable;

            if( PixelConverter.HasPalette(m_PixelType) )
                m_View.ShowPalette();
            else
                m_View.HidePalette();

            m_BaseHeight = m_Height = height;
            m_UnconvertedBuffer = bitmapData;

            Bitmap bitmapToPaint = new Bitmap(m_Bitmap);
            Graphics graphics = Graphics.FromImage(bitmapToPaint);
            graphics.Clear(m_BackgroundColor);
            graphics.DrawImage(m_Bitmap , new Point(0,0));

            m_View.SetBitmap(bitmapToPaint , m_BasePixelType , m_BaseWidth , m_BaseHeight , currentVariable);
        }

        public Point UpdatePointToPaint(System.Windows.Point position)
        {
            m_PointToPaint.X += (int) ( position.X - m_MouseStartingPoint.X );
            m_PointToPaint.Y += (int) ( position.Y - m_MouseStartingPoint.Y );
            m_MouseStartingPoint = position;
            
            PrintNewBitmap();
            return m_PointToPaint;
        }

        private void PrintNewBitmap()
        {
            Bitmap bitmapToPaint;
            Rectangle rect = m_View.GetDisplayRect();

            if(m_ZoomInBitmap.Width < rect.Width && m_ZoomInBitmap.Height < rect.Height)
                bitmapToPaint = new Bitmap(m_ZoomInBitmap.Width, m_ZoomInBitmap.Height);
            else
                bitmapToPaint = new Bitmap(rect.Width , rect.Height);

            Graphics graphics = Graphics.FromImage(bitmapToPaint);
            graphics.Clear(m_BackgroundColor);
            graphics.DrawImage(m_ZoomInBitmap , m_PointToPaint);

            m_View.SetBitmap(bitmapToPaint);
        }

        public string GetPosition(System.Windows.Point position)
        {
                return "(" + ( (int) ( -m_PointToPaint.X + position.X )/m_ZoomFactor )  + "," + ( (int) ( -m_PointToPaint.Y + position.Y )/m_ZoomFactor ) + ")";
        }

        public void SetMouseStartingPosition(System.Windows.Point point)
        {
            m_MouseStartingPoint = point;
        }

        public RawPixelData GetPixelData(int X , int Y)
        {
            int xPosition = (int) ( -m_PointToPaint.X + X )/m_ZoomFactor;
            int yPosition = (int) ( -m_PointToPaint.Y + Y )/m_ZoomFactor;

            if( xPosition > 0 && xPosition < m_Width && yPosition > 0 && yPosition < m_Height )
            {
                try
                {
                return m_Converter.GetPixelData(xPosition , yPosition , m_UnconvertedBuffer , m_PixelType , m_Width, m_Palette);
                }
                catch
                {
                return new RawPixelData(); 
                }
            }
            else
                return new RawPixelData();
        }

        public int ZoomIn()
        {
            m_ZoomFactor++;
            ZoomChanged(m_ZoomFactor-1);
            return m_ZoomFactor;
        }
        public int ZoomOut()
        {
            m_ZoomFactor--;
            ZoomChanged(m_ZoomFactor + 1);
            return m_ZoomFactor;
        }
        private void ZoomChanged(int ancientZoomFactor)
        {
            if( m_ZoomFactor < 1 ) //prevent the zoomfactor from getting too small
                m_ZoomFactor = 1;

            byte[] ZoomInByteArray =  m_Manipulator.GetZoomInBitmap(m_BitmapData , m_ZoomFactor , m_Width);
            if( ZoomInByteArray != null )
            {
                m_ZoomInBitmap = m_Manipulator.GetBitmapFromByteArray(m_Width*m_ZoomFactor , m_Height*m_ZoomFactor , ZoomInByteArray);
                if( m_ZoomInBitmap != null )
                {
                    Rectangle rect = m_View.GetDisplayRect();

                    if( m_ZoomInBitmap.Width * ( ancientZoomFactor/(double) m_ZoomFactor ) > rect.Width && m_ZoomInBitmap.Height > rect.Height && m_ZoomInBitmap.Width > rect.Width && m_ZoomInBitmap.Height * ( ancientZoomFactor/(double) m_ZoomFactor ) > rect.Height )
                    {
                        m_PointToPaint.X = (int) ( ( m_PointToPaint.X - ( rect.Width/2 ) ) * ( m_ZoomFactor / (double) ancientZoomFactor ) + ( rect.Width/2 ) ); // makes sure the zoom is made in the center of the image
                        m_PointToPaint.Y = (int) ( ( m_PointToPaint.Y - ( rect.Height/2 ) ) * ( m_ZoomFactor / (double) ancientZoomFactor ) + ( rect.Height/2 ) );
                    }
                    else if( m_ZoomInBitmap.Height < rect.Height &&  m_ZoomInBitmap.Width < rect.Width && m_ZoomInBitmap.Width * ( ancientZoomFactor/(double) m_ZoomFactor ) < rect.Width && ( ancientZoomFactor/(double) m_ZoomFactor ) < rect.Height )
                    {
                        m_PointToPaint.X = (int) ( m_PointToPaint.X * ( m_ZoomFactor / (double) ancientZoomFactor ) );
                        m_PointToPaint.Y = (int) ( m_PointToPaint.Y * ( m_ZoomFactor / (double) ancientZoomFactor ) );
                    }
                    else if( m_ZoomInBitmap.Height > rect.Height &&  m_ZoomInBitmap.Width > rect.Width && m_ZoomInBitmap.Width * ( ancientZoomFactor/(double) m_ZoomFactor ) < rect.Width && ( ancientZoomFactor/(double) m_ZoomFactor ) < rect.Height )
                    {
                        m_PointToPaint.X = (int) ( ( m_PointToPaint.X - ( ( m_ZoomInBitmap.Width*( ancientZoomFactor/(double) m_ZoomFactor ) )/2 ) ) * ( m_ZoomFactor / (double) ancientZoomFactor ) + ( rect.Width/2 ) ); // makes sure the zoom is made in the center of the image
                        m_PointToPaint.Y = (int) ( ( m_PointToPaint.Y - ( ( m_ZoomInBitmap.Height*( ancientZoomFactor/(double) m_ZoomFactor ) )/2 ) ) * ( m_ZoomFactor / (double) ancientZoomFactor ) + ( rect.Height/2 ) );
                    }
                    else
                    {
                        m_PointToPaint.X = (int) ( ( m_PointToPaint.X - ( ( rect.Width )/2 ) ) * ( m_ZoomFactor / (double) ancientZoomFactor ) + ( m_ZoomInBitmap.Width/2 ) ); // makes sure the zoom is made in the center of the image
                        m_PointToPaint.Y = (int) ( ( m_PointToPaint.Y - ( ( rect.Height )/2 ) ) * ( m_ZoomFactor / (double) ancientZoomFactor ) + ( m_ZoomInBitmap.Height/2 ) );
                    }
                    PrintNewBitmap();
                }
                else
                    m_ZoomFactor = ancientZoomFactor;
            }
            else
                m_ZoomFactor = ancientZoomFactor;
        }

        public void ChangePixelType(string pixelType)
        {
            PixelType newPixelType = PixelConverter.GetPixelType(pixelType);
            if(newPixelType != m_PixelType)
            {
                try
                {
                    if( newPixelType != PixelType.HRPPixelTypeV32B8G8R8A8 )
                    {
                        m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(m_UnconvertedBuffer , newPixelType , m_Palette , m_Width , m_Height);
                    }
                    m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);
                    m_PixelType = newPixelType;
                    m_ZoomFactor = 1;
                    m_PointToPaint = new Point(0 , 0);
                    m_ZoomInBitmap = m_Bitmap;
                    PrintNewBitmap();
                }
                catch
                {
                    m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(m_UnconvertedBuffer , m_PixelType , m_Palette , m_Width , m_Height);
                    m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_BaseWidth , m_Height , m_BitmapData);
                    PrintNewBitmap();
                }
            }
        }

        public void ChangedRect(int width , int height, bool heightAsChanged)
        {
            int nbOfPixel = m_UnconvertedBuffer.Length/PixelConverter.GetNumberOfChannel(m_PixelType);
            if( heightAsChanged )
            {
                if( nbOfPixel % height == 0 )
                {
                    m_Height = height;
                    m_Width = nbOfPixel / height;
                    m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);
                    m_View.SetBitmap(m_Bitmap , m_PixelType , m_Width , m_Height, m_CurrentVariable);
                    m_ZoomFactor = 1;
                    m_PointToPaint = new Point(0 , 0);
                    m_ZoomInBitmap = m_Bitmap;
                }
                else
                {
                    m_View.ChangeRectValue(m_Width , m_Height);
                }
            }
            else
            {
                if( nbOfPixel % width == 0 )
                {
                    m_Width = width;
                    m_Height = nbOfPixel / width;
                    m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);
                    m_View.SetBitmap(m_Bitmap , m_PixelType , m_Width , m_Height, m_CurrentVariable);
                    m_ZoomFactor = 1;
                    m_PointToPaint = new Point(0 , 0);
                    m_ZoomInBitmap = m_Bitmap;
                }
                else
                {
                    m_View.ChangeRectValue(m_Width , m_Height);
                }
            }
        }

        public RawPixelData GetPaletteInfo(int indexInPalette)
        {
            return m_Converter.GetPixelDataFromPalette(indexInPalette , m_Palette , PixelConverter.GetNumberOfChannel(m_PixelType));
        }

        public void SetColor(Color newColor)
        {
            m_BackgroundColor = newColor;
        }

        public void RevertChanges()
        {
            m_Width = m_BaseWidth;
            m_Height = m_BaseHeight;
            m_View.ChangeRectValue(m_Width , m_Height);

            m_ZoomFactor = 1;
            m_PointToPaint = new Point(0 , 0);
            m_ZoomInBitmap = m_Bitmap;

            if( m_PixelType != m_BasePixelType )
            {
                if( m_BasePixelType != PixelType.HRPPixelTypeV32B8G8R8A8 )
                {
                    m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(m_UnconvertedBuffer , m_BasePixelType , m_Palette , m_Width , m_Height);
                }
                m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);
                m_PixelType = m_BasePixelType;
            }

            Bitmap bitmapToPaint = new Bitmap(m_Bitmap);
            Graphics graphics = Graphics.FromImage(bitmapToPaint);
            graphics.Clear(m_BackgroundColor);
            graphics.DrawImage(m_ZoomInBitmap , m_PointToPaint);

            m_View.SetBitmap(bitmapToPaint);
   
        }

        public void SetWatchModel(WatchWindowModel watchWindowModel)
        {
            m_WatchWindowModel = watchWindowModel;
        }

        public void AddWatch()
        {
            m_WatchWindowModel.AddWatch(m_CurrentVariable , m_PixelType , m_Width , m_Height , m_UnconvertedBuffer , m_Palette);
        }

        public string GetCurrentVariable()
        {
            return m_CurrentVariable;
        }
    }
}
