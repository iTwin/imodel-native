/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/WatchWindow.xaml.cs $
|    $RCSfile: WatchWindow.xaml.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace Bentley.ImageViewer
{
    /// <summary>
    /// Interaction logic for WatchWindow.xaml
    /// </summary>
    /// 
    public partial class WatchWindow : UserControl
    {
        private WatchWindowModel m_Model;

        public WatchWindow()
        {
            InitializeComponent();
        }
        public void SetModel(WatchWindowModel model)
        {
            m_Model = model;
        }
        public void AddWatch(Watch watch)
        {
            WatchBox box = new WatchBox(this);
            box.SetWatch(watch);
            WatchBoxPanel.Children.Insert(WatchBoxPanel.Children.Count , box);
        }

        private void Expression_KeyDown(object sender , KeyEventArgs e)
        {
            if( e.Key == Key.Enter )
            {
                m_Model.EvaluateWatch(Expression.Text);
            }
        }

        public void DeleteWatch(Watch watch)
        {
            m_Model.DeleteWatch(watch);
        }

        public void SendToVisualiser(Watch watch)
        {
            m_Model.SendToVisualiser(watch);
        }

        public void RemoveAllWatch()
        {
            WatchBoxPanel.Children.Clear();
        }

        private void UserControl_IsVisibleChanged(object sender , DependencyPropertyChangedEventArgs e)
        {
            if( this.IsVisible == true )
                m_Model.UpdateAllWatch();
            else
                m_Model.SaveAllWatch();
        }

        public void UpdateWatch(Watch watch , int index)
        {
            WatchBoxPanel.Children.RemoveAt(index);
            WatchBox box = new WatchBox(this);
            box.SetWatch(watch);
            WatchBoxPanel.Children.Insert(index , box);
        }
    }
}
