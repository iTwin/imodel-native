/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/VisualizerView.xaml.cs $
|    $RCSfile: VisualizerView.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System.Drawing;
using System.IO;
using System;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Input;

namespace Bentley.ImageViewer
{
    /// <summary>
    /// Interaction logic for DebuggerView.xaml
    /// </summary>
    public partial class VisualizerView : UserControl
    {
        private bool m_HasBitmap = false;
        private bool m_MouseDown = false;
        private VisualizerModel m_Model;
        private bool m_IsUpdatingBitmap;

        public VisualizerView()
        {
            InitializeComponent();
            this.pixelTypeList.ItemsSource = System.Enum.GetNames(typeof(PixelType));
        }
        public void SetModel(VisualizerModel model)
        {
            m_Model = model;
            LoadSettings();
        }
        public void SetBitmap(Bitmap bitmap , PixelType pixelType , int width , int height, string variableName, int bufferSize)
        {
            m_IsUpdatingBitmap = true;
            this.ImageVisualizer.Source = ToBitmapSource(bitmap);
            this.heightValue.Text = height.ToString();
            this.widthValue.Text =  width.ToString();
            this.pixelTypeList.Text = pixelType.ToString();
            this.m_HasBitmap = true;
            this.zoomFactorValue.Content = 1 + "x";
            this.NameOfVariable.Content = variableName;
            this.bufferSizeValue.Text = bufferSize.ToString();
            m_IsUpdatingBitmap = false;
        }
        public void SetBitmap(Bitmap bitmap)
        {
            this.ImageVisualizer.Source = ToBitmapSource(bitmap);
        }
        public BitmapImage ToBitmapSource(Bitmap bitmap)
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

        private void mouseMove_Event(object sender , System.Windows.Input.MouseEventArgs e)
        {
            if( m_HasBitmap )
            {
                System.Windows.Point position = e.GetPosition(this.ImageVisualizer);
                if( m_MouseDown && e.MiddleButton == System.Windows.Input.MouseButtonState.Pressed)
                {
                  m_Model.UpdatePointToPaint(position);
                }
                if( position.X > 0 && position.Y > 0 && position.X < ImageVisualizer.Source.Width && position.Y < ImageVisualizer.Source.Height )
                {
                    this.positionValue.Content = m_Model.GetPosition(position);
                    GetPixelData(position.X , position.Y);
                }
            }
        }

        private void image_mouseDown(object sender , System.Windows.Input.MouseButtonEventArgs e)
        {
            if( e.ChangedButton == MouseButton.Middle )
            {
                m_MouseDown = true;
                System.Windows.Point position = e.GetPosition(this.ImageVisualizer);
                m_Model.SetMouseStartingPosition(position);
            }
        }

        private void GetPixelData(double x , double y)
        {
            RawPixelData pixelData = m_Model.GetPixelData(x, y);
            string channelValue = "("+FormatChannelValue(pixelData.Channel1);

          if( pixelData.Channel2 != -1 )
              channelValue += "," + FormatChannelValue(pixelData.Channel2);
          if( pixelData.Channel3 != -1 )
              channelValue += "," + FormatChannelValue(pixelData.Channel3);
          if(pixelData.Channel4 != -1)
              channelValue += "," + FormatChannelValue(pixelData.Channel4);

          channelValue += ")";

          ChannelValues.Content = channelValue;
          paletteIdValue.Text = FormatChannelValue(pixelData.PaletteIndex);
        }

        private string FormatChannelValue(int channelValue)
        {
            if(m_Model.IsHexaDisplay)
                    return "0x" + channelValue.ToString("X");
            else
                    return channelValue.ToString();
        }

        private void image_mouseUp(object sender , System.Windows.Input.MouseButtonEventArgs e)
        {
            if( e.ChangedButton == MouseButton.Middle )
            {
                m_MouseDown = false;
            }
        }

        private void ImageVisualiser_MouseWheel(object sender , MouseWheelEventArgs e)
        {
            if( m_HasBitmap )
            {
                if( e.Delta > 1 ) //if mouse wheel was pressed frontward
                    this.zoomFactorValue.Content = m_Model.ZoomIn(e.GetPosition(this.ImageVisualizer).X , e.GetPosition(this.ImageVisualizer).Y) + "x";
                else //if mouse wheel was pressed backward
                    this.zoomFactorValue.Content = m_Model.ZoomOut(e.GetPosition(this.ImageVisualizer).X , e.GetPosition(this.ImageVisualizer).Y) + "x";
            }
        }

        private void ComboBox_SelectionChanged(object sender , SelectionChangedEventArgs e)
        {
        if(!m_IsUpdatingBitmap)
            m_Model.ChangePixelType((string)this.pixelTypeList.SelectedValue);
        }
        public void ChangeRectValue(int width , int height)
        {
            m_IsUpdatingBitmap = true;
            this.heightValue.Text = height.ToString();
            this.widthValue.Text = width.ToString();
            m_IsUpdatingBitmap = false;
        }

        private void heightValue_KeyDown(object sender , KeyEventArgs e)
        {
                if( e.Key == Key.Enter )
                    m_Model.ChangeRect(Convert.ToInt32(widthValue.Text) , Convert.ToInt32(heightValue.Text) , Convert.ToInt32(bufferSizeValue.Text) , (bool) this.LockButton.IsChecked);
        }

        private void widthValue_KeyDown(object sender , KeyEventArgs e)
        {
                if( e.Key == Key.Enter )
                    m_Model.ChangeRect(Convert.ToInt32(widthValue.Text) , Convert.ToInt32(heightValue.Text) , Convert.ToInt32(bufferSizeValue.Text) , (bool) this.LockButton.IsChecked);
        }
        private void bufferValue_KeyDown(object sender , KeyEventArgs e)
        {
                if( e.Key == Key.Enter )
                    m_Model.ChangeRect(Convert.ToInt32(widthValue.Text) , Convert.ToInt32(heightValue.Text) , Convert.ToInt32(bufferSizeValue.Text) , (bool)this.LockButton.IsChecked);
        }

        private void Setting_Click(object sender , System.Windows.RoutedEventArgs e)
        {
            SettingsForm settingsForm = new SettingsForm();
            if( settingsForm.ShowDialog() == true )
            {
                DisplayBackground.Background = new SolidColorBrush(settingsForm.ColorWpf);
                m_Model.SetColor(settingsForm.ColorBitmap);
                if( m_HasBitmap )
                    m_Model.PrintBitmap();
                else
                    SetBaseBitmap();
                SaveSettings(settingsForm.ColorBitmap);               
            }
        }

        private void SaveSettings(System.Drawing.Color color)
        {
            Properties.Settings.Default.BackgroundColor = color;
            Properties.Settings.Default.Save();
        }
        private void LoadSettings()
        {
            System.Drawing.Color colorBitmap = Properties.Settings.Default.BackgroundColor; 
            m_Model.SetColor(colorBitmap);
            System.Windows.Media.Color color = new System.Windows.Media.Color();
            color.A = colorBitmap.A;
            color.B = colorBitmap.B;
            color.G = colorBitmap.G;
            color.R = colorBitmap.R;
            DisplayBackground.Background = new SolidColorBrush(color);
        }
        private void paletteIdValue_KeyDown(object sender , KeyEventArgs e)
        {
            if( e.Key == Key.Enter )
            {
                RawPixelData pixelData;
                if( ! m_Model.IsHexaDisplay )
                    pixelData = m_Model.GetPaletteInfo(Convert.ToInt32(paletteIdValue.Text));
                else
                    pixelData = m_Model.GetPaletteInfo(Convert.ToInt32(paletteIdValue.Text , 16));

                string channelValue = "("+FormatChannelValue(pixelData.Channel1);

                if( pixelData.Channel2 != -1 )
                    channelValue += "," + FormatChannelValue(pixelData.Channel2);
                if( pixelData.Channel3 != -1 )
                    channelValue += "," + FormatChannelValue(pixelData.Channel3);
                if( pixelData.Channel4 != -1 )
                    channelValue += "," + FormatChannelValue(pixelData.Channel4);

                channelValue += ")";

                ChannelValues.Content = channelValue;

                paletteIdValue.Text = FormatChannelValue(pixelData.PaletteIndex);
            }
        }

        public void ShowPalette()
        {
            this.paletteIdValue.Visibility = System.Windows.Visibility.Visible;
            this.paletteLabel.Visibility = System.Windows.Visibility.Visible;
        }

        public void HidePalette()
        {
            this.paletteIdValue.Visibility = System.Windows.Visibility.Collapsed;
            this.paletteLabel.Visibility = System.Windows.Visibility.Collapsed;
        }
        private void Reset_Click(object sender , System.Windows.RoutedEventArgs e)
        {
            if( m_HasBitmap )
            {
                zoomFactorValue.Content = "1x";
                m_Model.RevertChanges();
            }
        }

        public Rectangle GetDisplayRect()
        {
            return new Rectangle(0 , 0 , (int) DisplayBackground.ActualWidth , (int) DisplayBackground.ActualHeight);            
        }

        private void AddWatch_Click(object sender , System.Windows.RoutedEventArgs e)
        {
            if( m_HasBitmap )
                m_Model.AddWatch();
        }

        private void StackPanel_IsVisibleChanged(object sender , System.Windows.DependencyPropertyChangedEventArgs e)
        {
            if( this.IsVisible == false )
                m_Model.Empty();
        }

        public void Empty()
        {
            m_HasBitmap = false;
            ImageVisualizer.Source = null;
            NameOfVariable.Content = "";
            Bitmap emptyBitmap = new Bitmap(100 , 100);
            Graphics graphicsForEmptyBitmap = Graphics.FromImage(emptyBitmap);
            graphicsForEmptyBitmap.Clear(m_Model.GetBackgroundColor());
            ImageVisualizer.Source = ToBitmapSource(emptyBitmap);
            this.bufferSizeValue.Text = 0.ToString();
            this.heightValue.Text = 0.ToString();
            this.widthValue.Text = 0.ToString();
            HidePalette();
            this.pixelTypeList.SelectedValue = "HRPPixelTypeV24R8G8B8";
            this.positionValue.Content = "";
            this.ChannelValues.Content = "";

        }

        public Rectangle GetCurrentDisplayRect()
        {
            return new Rectangle(0 , 0 , (int) ImageVisualizer.Width , (int) ImageVisualizer.Height);
        }

        public void SetBaseBitmap()
        {
            Bitmap emptyBitmap = new Bitmap(100 , 100);
            Graphics graphicsForEmptyBitmap = Graphics.FromImage(emptyBitmap);
            graphicsForEmptyBitmap.Clear(m_Model.GetBackgroundColor());
            ImageVisualizer.Source = ToBitmapSource(emptyBitmap);
        }

        private void DisplayBackground_SizeChanged(object sender , System.Windows.SizeChangedEventArgs e)
        {
            UpdateBackgroundSize();
            if( m_HasBitmap )
            m_Model.PrintBitmap();
        }
        public void UpdateBackgroundSize()
        {
            if( this.ActualHeight > 130 )
                DisplayBackground.Height = this.ActualHeight - 130;
        }

        private void StackPanel_SizeChanged(object sender , System.Windows.SizeChangedEventArgs e)
        {
            UpdateBackgroundSize();
            if( m_HasBitmap )
                m_Model.PrintBitmap();
        }

        public bool HasBitmap 
        { 
            get {return m_HasBitmap;}
            set { m_HasBitmap = value; }
        }


        public void SetOutOfScope(string watchName)
        {
            this.NameOfVariable.Content = "(Variable undefined in current context) " + watchName; 
        }
    }
}