using System.Windows;
using System.Collections.Generic;
using System;

namespace Bentley.ImageViewer
{
    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class PtrDialog : Window
    {
        private string m_PixelType;
        private int m_BufferSize;
        private int m_Width;
        private int m_Height;

        public string PixelType 
        {
            get {return m_PixelType;}
        }
        public int BufferSize
        {
            get { return m_BufferSize;}
            set { m_BufferSize = value; }
        }
        public new int Width
        {
            get { return m_Width; }
            set { m_Width = value; }
        }
        public new int Height 
        {
            get { return m_Height; }
            set { m_Height = value; }
        }

        public PtrDialog()
        {
            InitializeComponent();
            string[] listPixelType = System.Enum.GetNames(typeof(PixelType));
            List<string> listPixelTypeNoPalette = new List<string>();
            foreach( string pixeltype in listPixelType )
            {
                if( PixelConverter.HasPalette(PixelConverter.GetPixelType(pixeltype)) )
                    continue;

                listPixelTypeNoPalette.Add(pixeltype);
            }
            pixelTypeList.ItemsSource = listPixelTypeNoPalette;
            pixelTypeList.SelectedIndex = 0;
        }

        private void Button_Click(object sender , RoutedEventArgs e)
        {
            try
            {
                if( this.pixelTypeList.Text != "" && this.pixelTypeList.Text != null )
                {
                    m_PixelType = pixelTypeList.Text;
                    m_Height = Convert.ToInt32(heightValue.Text);
                    m_Width = Convert.ToInt32(widthValue.Text);
                    if( bufferSizeValue.Text == "" )
                        m_BufferSize =(int) (PixelConverter.GetNumberOfBitsPerPixel(PixelConverter.GetPixelType(m_PixelType))/8.0*m_Width*m_Height);
                    else
                        m_BufferSize=Convert.ToInt32(bufferSizeValue.Text);
                    if( m_Height > 0 && m_Width > 0 && m_BufferSize > 0 )
                        this.DialogResult = true;
                    else
                        throw new Exception();
                }
                else
                    throw new Exception();
            }
            catch
            {
                MessageBox.Show("Specified values are invalid" , "Invalid value" , MessageBoxButton.OK , MessageBoxImage.Error);
            }
        }

        private void heightValue_TextChanged(object sender , System.Windows.Controls.TextChangedEventArgs e)
        {
            uint height;
            try
            {
                height = Convert.ToUInt32(heightValue.Text);
            }
            catch
            {
                heightValue.Text = heightValue.Text.Remove(heightValue.Text.Length-1);
                height = Convert.ToUInt32(heightValue.Text);
            }

            if( widthValue.Text != null && widthValue.Text != "" && height != 0 )
            {
                uint width = Convert.ToUInt32(widthValue.Text);
                bufferSizeValue.Text = (width*height*PixelConverter.GetNumberOfBitsPerPixel(PixelConverter.GetPixelType(pixelTypeList.Text))/8).ToString();
            }

        }

        private void widthValue_TextChanged(object sender , System.Windows.Controls.TextChangedEventArgs e)
        {
            uint width;
            try
            {
                width = Convert.ToUInt32(widthValue.Text);
            }
            catch
            {
                widthValue.Text = widthValue.Text.Remove(widthValue.Text.Length-1);
                width = Convert.ToUInt32(widthValue.Text);
            }

            if( heightValue.Text != null && heightValue.Text != "" && width != 0 )
            {
                uint height = Convert.ToUInt32(heightValue.Text);
                bufferSizeValue.Text = ( width*height*PixelConverter.GetNumberOfBitsPerPixel(PixelConverter.GetPixelType(pixelTypeList.Text))/8 ).ToString();
            }
        }

        private void pixelTypeList_SelectionChanged(object sender , System.Windows.Controls.SelectionChangedEventArgs e)
        {
           if( heightValue.Text != null && heightValue.Text != "" && widthValue.Text != null && widthValue.Text != "" )
            {
               uint width = Convert.ToUInt32(widthValue.Text);
               uint height = Convert.ToUInt32(heightValue.Text);
               bufferSizeValue.Text = ( width*height*PixelConverter.GetNumberOfBitsPerPixel(PixelConverter.GetPixelType(pixelTypeList.SelectedValue.ToString()))/8 ).ToString();
            }
        }
    }
}
