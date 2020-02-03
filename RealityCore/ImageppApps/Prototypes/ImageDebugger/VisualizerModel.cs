/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/VisualizerModel.cs $
|    $RCSfile: VisualizerModel.cs, $
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
using System.Drawing;
using System.Drawing.Drawing2D;

namespace Bentley.ImageViewer
{
    public class VisualizerModel /*: IDisposable*/
    {
        private Watch m_Watch; //current watch of the visualizer
        private PixelType m_PixelType; // current pixeltype of the watch (not necesseraly the one read in the variable)
        private int m_Width; //current width of the watch
        private int m_Height; //current height of the watch
        private double m_ZoomFactor = 1; 
        private Bitmap m_Bitmap; //bitmap to print
        private byte[] m_BitmapData; //converter buffer of pixel
        private int m_BufferLength; //current length of the buffer
        private System.Windows.Point m_MouseStartingPoint; // start point of the mouse when used to pan the bitmap
        private VisualizerView m_View; //current view window 
        private PixelConverter m_Converter;
        private BitmapManipulator m_Manipulator;
        private Color m_BackgroundColor; 
        private ImageViewerPackage m_Parent;
        private WatchWindowModel m_WatchWindowModel;
        private Matrix m_ViewToPhysical; //transformation matrix between the view and the bitmap
        private DebuggerConnector m_debuggerConnector;

        public VisualizerModel(ImageViewerPackage parent)
        {
            m_Parent = parent;
            m_Converter = new PixelConverter();
            m_Manipulator = new BitmapManipulator();
            m_View = parent.GetControl();
            m_View.SetModel(this);
        }

        public DebuggerConnector Connector
        {
            set { m_debuggerConnector = value; }
        }

        public bool IsHexaDisplay
        {
            get { return m_Parent.GetHexaDisplay(); }
        }
        public void SetBitmap(Watch watch)
        {
            m_Bitmap = null;
            m_View.UpdateBackgroundSize();
            m_View.SetBaseBitmap();

            m_Watch = watch;
            if( m_Watch.BitmapBuffer.Length > 0 )
            {
                if( m_Watch.PixelType != PixelType.HRPPixelTypeV32B8G8R8A8 )
                {
                    m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(m_Watch.BitmapBuffer , m_Watch.PixelType , m_Watch.Palette , m_Watch.Width , m_Watch.Height);
                }
                else
                    m_BitmapData = watch.BitmapBuffer;
                m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Watch.Width , m_Watch.Height , m_BitmapData);
            }
            m_ZoomFactor = 1;
            m_Width = m_Watch.Width;
            m_Height = m_Watch.Height;
            m_PixelType = m_Watch.PixelType;
            m_ViewToPhysical = new Matrix();
            m_BufferLength = m_Watch.BitmapBuffer.Length;

            if( PixelConverter.HasPalette(m_PixelType) )
                m_View.ShowPalette();
            else
                m_View.HidePalette();

            Rectangle rect = m_View.GetDisplayRect();

            Bitmap bitmapToPaint = new Bitmap((int) m_View.ActualWidth ,(int) m_View.ActualHeight - 130);

            Graphics graphics = Graphics.FromImage(bitmapToPaint);
            graphics.Clear(m_BackgroundColor);
            if(m_Bitmap != null)
            graphics.DrawImage(m_Bitmap , new Point(0 , 0));

            m_View.SetBitmap(bitmapToPaint , m_Watch.PixelType , m_Watch.Width , m_Watch.Height , m_Watch.Name, m_Watch.BitmapBuffer.Length);

            if( m_Bitmap == null )
                m_View.HasBitmap = false;

        }

        public void UpdatePointToPaint(System.Windows.Point position)
        {
            m_ViewToPhysical.Translate((float)(m_MouseStartingPoint.X - position.X) , (float)( m_MouseStartingPoint.Y - position.Y));

            m_MouseStartingPoint = position;
            
            PrintBitmap();
        }

        public void PrintBitmap()
        {
            m_View.UpdateBackgroundSize();
           Rectangle rect = m_View.GetDisplayRect();

           Bitmap bitmapToPaint = new Bitmap(rect.Width , rect.Height);

           Matrix physicalToView = m_ViewToPhysical.Clone();
           physicalToView.Invert();
           
           Graphics graphics = Graphics.FromImage(bitmapToPaint);
           graphics.Clear(m_BackgroundColor);
           graphics.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
           graphics.MultiplyTransform(physicalToView , MatrixOrder.Append);
           if( m_Bitmap != null )
               graphics.DrawImage(m_Bitmap , new Point(0 , 0));

           m_View.SetBitmap(bitmapToPaint);

           if( m_Bitmap == null )
               m_View.HasBitmap = false;
           else
               m_View.HasBitmap = true;
        }

        public string GetPosition(System.Windows.Point position)
        {
            Point[] pointArray = new Point[1];
            pointArray[0] = new Point((int) position.X , (int) position.Y); 
            m_ViewToPhysical.TransformPoints(pointArray);
            return "(" +  pointArray[0].X + "," + pointArray[0].Y + ")";
        }

        public void SetMouseStartingPosition(System.Windows.Point point)
        {
            m_MouseStartingPoint = point;
        }

        public RawPixelData GetPixelData(double X , double Y)
        {
            Point[] pointArray = new Point[1];
            pointArray[0] = new Point((int) X , (int) Y);
            m_ViewToPhysical.TransformPoints(pointArray);

            if( pointArray[0].X >= 0 && pointArray[0].X < m_Width && pointArray[0].Y >= 0 && pointArray[0].Y < m_Height )
            {
                try
                {
                    return m_Converter.GetPixelData(pointArray[0].X , pointArray[0].Y , m_Watch.BitmapBuffer , m_PixelType , m_Watch.Width , m_Watch.Height , m_Watch.Palette);
                }
                catch
                {
                return new RawPixelData(); 
                }
            }
            else
                return new RawPixelData();
        }

        public double ZoomIn(double X, double Y)
        {
            m_ZoomFactor*=2;
            ZoomChanged(m_ZoomFactor/2.0, X, Y);
            return m_ZoomFactor;
        }
        public double ZoomOut(double X, double Y)
        {
            m_ZoomFactor/=2;
            ZoomChanged(m_ZoomFactor*2.0, X, Y);
            return m_ZoomFactor;
        }
        private void ZoomChanged(double ancientZoomFactor, double X, double Y)
        {
            if( m_ZoomFactor <  ( 1/256.0 ) ) //prevent the zoomfactor from getting too small
            {
                m_ZoomFactor = ( 1/256.0 );
            }
            else if( m_ZoomFactor > 2048 )
            {
                m_ZoomFactor = 2048;
            }
            Rectangle rect = m_View.GetCurrentDisplayRect();

            Point[] newCenter = new Point[1];
            newCenter[0] = new Point((int) X, (int) Y);

            m_ViewToPhysical.TransformPoints(newCenter);

            m_ViewToPhysical.Translate((float) -newCenter[0].X , (float) -newCenter[0].Y, MatrixOrder.Append);
            m_ViewToPhysical.Scale((float) (ancientZoomFactor/m_ZoomFactor) , (float) (ancientZoomFactor/m_ZoomFactor), MatrixOrder.Append);
            m_ViewToPhysical.Translate((float) newCenter[0].X , (float) newCenter[0].Y, MatrixOrder.Append);

            PrintBitmap();

        }

        public void ChangePixelType(string pixelType)
        {
            if( m_Watch.IsWatchOfArray && m_Watch.IsIncompleteArray )
            {
                m_Watch.PixelType = PixelConverter.GetPixelType(pixelType);
                if( m_Watch.CheckIfComplete() )
                {
                    SetBitmap(m_Watch);
                }
            }
            else
            {
                PixelType newPixelType = PixelConverter.GetPixelType(pixelType);
                if( newPixelType != m_PixelType )
                {
                    try
                    {
                        if( newPixelType != PixelType.HRPPixelTypeV32B8G8R8A8 )
                        {
                            m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(m_Watch.BitmapBuffer , newPixelType , m_Watch.Palette , m_Width , m_Height);
                        }
                        m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);
                        m_PixelType = newPixelType;
                        m_ZoomFactor = 1;
                        m_ViewToPhysical.Reset();
                        PrintBitmap();
                        if( m_Watch.IsWatchOfArray )
                            m_Watch.PixelType = m_PixelType;
                    }
                    catch
                    {
                        m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(m_Watch.BitmapBuffer , m_PixelType , m_Watch.Palette , m_Width , m_Height);
                        m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);
                        PrintBitmap();
                    }
                }
            }
        }

        public RawPixelData GetPaletteInfo(int indexInPalette)
        {
            return m_Converter.GetPixelDataFromPalette(indexInPalette , m_Watch.Palette , PixelConverter.GetNumberOfChannel(m_PixelType));
        }

        public void SetColor(Color newColor)
        {
            m_BackgroundColor = newColor;
        }

        public void RevertChanges()
        {
            m_Width = m_Watch.Width;
            m_Height = m_Watch.Height;
            m_View.ChangeRectValue(m_Width , m_Height);
            m_BufferLength = m_Watch.BitmapBuffer.Length;
            m_ZoomFactor = 1;
            m_ViewToPhysical.Reset();

            if( m_Watch.PixelType != PixelType.HRPPixelTypeV32B8G8R8A8 )
            {
                m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(m_Watch.BitmapBuffer , m_Watch.PixelType , m_Watch.Palette , m_Width , m_Height);
            }
            m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);
            m_PixelType = m_Watch.PixelType;


            Rectangle rect = m_View.GetDisplayRect();

            Bitmap bitmapToPaint = new Bitmap((int) m_View.ActualWidth , (int) m_View.ActualHeight - 130);

            Graphics graphics = Graphics.FromImage(bitmapToPaint);
            graphics.Clear(m_BackgroundColor);
            if( m_Bitmap != null )
                graphics.DrawImage(m_Bitmap , new Point(0 , 0));

            m_View.SetBitmap(bitmapToPaint , m_Watch.PixelType , m_Watch.Width , m_Watch.Height , m_Watch.Name , m_Watch.BitmapBuffer.Length);

            if( m_Bitmap == null )
                m_View.HasBitmap = false;


        }

        public void SetWatchModel(WatchWindowModel watchWindowModel)
        {
            m_WatchWindowModel = watchWindowModel;
        }

        public void AddWatch()
        {
            m_Parent.ShowWatch();
            m_WatchWindowModel.AddWatch(m_Watch);
        }

        public string GetCurrentVariable()
        {
            return m_Watch.Name;
        }

        public void Empty()
        {
            m_Watch = new Watch();
            m_View.Empty();
        }

        public bool GetIsPointer()
        {
           return m_Watch.IsWatchOfArray;
        }

        public Watch GetWatch()
        {
            return m_Watch;
        }

        public Color GetBackgroundColor()
        {
            return m_BackgroundColor;
        }

        public void ChangeRect(int width , int height , int bufferSize , bool valueAreUnlocked)
        {
            if( m_Watch.IsWatchOfArray && m_Watch.IsIncompleteArray )
            {
                m_Watch.Width = width;
                m_Watch.Height = height;
                if(m_Watch.Height > 0 && m_Watch.Width > 0 && bufferSize == 0)
                    bufferSize = m_Watch.Width* m_Watch.Height*PixelConverter.GetNumberOfBitsPerPixel(m_Watch.PixelType)/8;
                if( bufferSize != 0 )
                {
                    m_Watch.BitmapBuffer = m_debuggerConnector.GetNewBuffer(m_Watch.Name , bufferSize);
                }
                if( m_Watch.CheckIfComplete() )
                {
                    SetBitmap(m_Watch);
                }
            }
            else
            {
                int nbOfPixel;
                byte[] newBuffer = m_Watch.BitmapBuffer;
                if( m_PixelType != PixelType.HRPPixelTypeI1R8G8B8A8RLE && m_PixelType != PixelType.HRPPixelTypeI1R8G8B8RLE )
                    nbOfPixel = m_BufferLength/PixelConverter.GetNumberOfChannel(m_PixelType);
                else
                    nbOfPixel = m_Width*m_Height;

                if( valueAreUnlocked )
                {
                    if( bufferSize == m_BufferLength )
                    {

                        if( m_PixelType != PixelType.HRPPixelTypeI1R8G8B8A8RLE && m_PixelType != PixelType.HRPPixelTypeI1R8G8B8RLE )
                        {
                            bufferSize = height*width*PixelConverter.GetNumberOfBitsPerPixel(m_PixelType)/8;
                        }
                        try
                        {
                            newBuffer = m_debuggerConnector.GetNewBuffer(m_Watch.Name , bufferSize);
                            m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(newBuffer , m_PixelType , m_Watch.Palette , width , height);
                            m_Height = height;
                            m_Width = width;
                        }
                        catch
                        {
                            m_Height = height;
                            m_Width = width;
                        }
                    }
                    else
                    {
                        newBuffer = m_debuggerConnector.GetNewBuffer(m_Watch.Name , bufferSize);
                        m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(newBuffer , m_PixelType , m_Watch.Palette , m_Width , m_Height);
                    }
                }
                else
                {
                    if( bufferSize == m_BufferLength )
                    {
                        if( height != m_Height )
                        {
                            m_Height = height;
                            m_Width = nbOfPixel / height;
                        }
                        else
                        {
                            m_Width = width;
                            m_Height = nbOfPixel / width;
                        }
                    }
                    else
                    {
                        if( m_PixelType != PixelType.HRPPixelTypeI1R8G8B8A8RLE && m_PixelType != PixelType.HRPPixelTypeI1R8G8B8RLE )
                        {
                            m_Height = ( bufferSize / ( PixelConverter.GetNumberOfBitsPerPixel(m_PixelType)/8 ) )/m_Width;
                        }
                        try
                        {
                            newBuffer = m_debuggerConnector.GetNewBuffer(m_Watch.Name , bufferSize);
                            m_BitmapData = m_Converter.ConvertToV32B8G8R8A8(newBuffer , m_PixelType , m_Watch.Palette , m_Width , m_Height);
                        }
                        catch
                        {
                            m_BitmapData = new byte[1];
                        }
                    }
                }
                m_BufferLength = bufferSize;
                m_ZoomFactor = 1;
                m_ViewToPhysical.Reset();
                if( m_Watch.IsWatchOfArray )
                {
                    m_Watch.Height = m_Height;
                    m_Watch.Width = m_Width;
                    m_Watch.BitmapBuffer = newBuffer;
                }

                m_Bitmap = m_Manipulator.GetBitmapFromByteArray(m_Width , m_Height , m_BitmapData);

                Rectangle rect = m_View.GetDisplayRect();

                Bitmap bitmapToPaint = new Bitmap((int) m_View.ActualWidth , (int) m_View.ActualHeight - 130);

                Graphics graphics = Graphics.FromImage(bitmapToPaint);
                graphics.Clear(m_BackgroundColor);
                if( m_Bitmap != null )
                    graphics.DrawImage(m_Bitmap , new Point(0 , 0));

                m_View.SetBitmap(bitmapToPaint , m_PixelType , m_Width , m_Height , m_Watch.Name , bufferSize);

                if( m_Bitmap == null )
                    m_View.HasBitmap = false;
            }
        }

        public void OutOfScope()
        {
            m_View.Empty();
            m_View.SetOutOfScope(m_Watch.Name);
        }
/*
        protected virtual void Dispose(bool disposing)
        {
            if( disposing )
            {
                m_ViewToPhysical.Dispose();
            }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }*/
    }
}
