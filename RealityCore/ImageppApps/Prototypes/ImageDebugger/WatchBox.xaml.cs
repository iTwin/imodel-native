/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/WatchBox.xaml.cs $
|    $RCSfile: WatchBox.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System.Drawing;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media.Imaging;

namespace Bentley.ImageViewer
{
    /// <summary>
    /// Interaction logic for WatchBox.xaml
    /// </summary>
    public partial class WatchBox : UserControl
    {
        private Watch m_Watch;
        private WatchWindow m_Parent;

        public WatchBox()
        {
            InitializeComponent();
        }
        public WatchBox(WatchWindow parent) : this()
        {
            m_Parent = parent;
        }

        private void ButtonDelete_Click(object sender , RoutedEventArgs e)
        {
            m_Parent.DeleteWatch(m_Watch);
            ( (StackPanel) this.Parent ).Children.Remove(this);
        }

        public void SetWatch(Watch watch)
        {
            m_Watch = watch;
            TextBlock textbloc = new TextBlock();
            textbloc.Text = watch.Name;
            this.WatchName.Header = textbloc;
            this.PixelTypeValue.Content = watch.PixelType.ToString();
            this.WidthValue.Content = watch.Width;
            this.HeightValue.Content = watch.Height;

            if( watch.BitmapThumbnail != null )
                this.Thumbnail.Source = ToBitmapSource(watch.BitmapThumbnail);
        }
        private BitmapImage ToBitmapSource(Bitmap bitmap)
        {
            MemoryStream stream = new MemoryStream();
            bitmap.Save(stream , System.Drawing.Imaging.ImageFormat.Bmp);
            BitmapImage output = new BitmapImage();
            output.BeginInit();
            stream.Seek(0 , SeekOrigin.Begin);
            output.StreamSource = stream;
            output.EndInit();
            return output;
        }
        private void ButtonVisualize_Click(object sender , RoutedEventArgs e)
        {
            if(m_Watch.BitmapThumbnail != null)
            m_Parent.SendToVisualiser(m_Watch);
        }
    }
}
