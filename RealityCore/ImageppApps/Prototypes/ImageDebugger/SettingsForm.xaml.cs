/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/SettingsForm.xaml.cs $
|    $RCSfile: SettingForm.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Windows;
using System.Windows.Forms;
using System.Windows.Input;
using System.Windows.Media;
using EnvDTE;
using EnvDTE80;

namespace Bentley.ImageViewer
{

    /// <summary>
    /// Interaction logic for Window1.xaml
    /// </summary>
    public partial class SettingsForm : System.Windows.Window
    {
        private Color m_ColorWpf;
        private System.Drawing.Color m_ColorBitmap;

        public Color ColorWpf
        {
            get { return m_ColorWpf; }
        }
        public System.Drawing.Color ColorBitmap
        {
            get { return m_ColorBitmap; }
        }

        public SettingsForm()
        {
            InitializeComponent();
            LoadSettings();
        }

        private void Rectangle_MouseLeftButtonDown(object sender , MouseButtonEventArgs e)
        {
            ColorDialog colorDialog = new ColorDialog();
            colorDialog.ShowDialog();
            Color color = new Color();
            color.A = colorDialog.Color.A;
            color.B = colorDialog.Color.B;
            color.G = colorDialog.Color.G;
            color.R = colorDialog.Color.R;

            m_ColorBitmap = colorDialog.Color;
            m_ColorWpf = color;
            ColorRect.Fill = new SolidColorBrush(m_ColorWpf);
        }

        private void Button_Click(object sender , RoutedEventArgs e)
        {
            DialogResult = true;
        }

        private void LoadSettings()
        {           
            System.Drawing.Color colorBitmap = Properties.Settings.Default.BackgroundColor;
            System.Windows.Media.Color color = new System.Windows.Media.Color();
            color.A = colorBitmap.A;
            color.B = colorBitmap.B;
            color.G = colorBitmap.G;
            color.R = colorBitmap.R;
            ColorRect.Fill = new SolidColorBrush(color);
            m_ColorBitmap = colorBitmap;
            m_ColorWpf = color;
        }
    }
}
