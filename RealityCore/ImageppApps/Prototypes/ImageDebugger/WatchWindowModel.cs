/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/WatchWindowModel.cs $
|    $RCSfile: WatchWindowModel.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Configuration;
using System.Collections.Generic;
using System.Drawing;

namespace Bentley.ImageViewer
{
    public struct Watch
    {
        public string Name;
        public PixelType PixelType;
        public int Width;
        public int Height;
        public byte[] BitmapBuffer;
        public byte[] Palette;
        public Bitmap BitmapThumbnail;
        public bool IsWatchOfArray;
        public uint VisualizerId;
        public bool IsIncompleteArray;

        public bool CheckIfComplete()
        {
            if(Width > 0 && Height > 0 && (PixelType != ImageViewer.PixelType.HRPPixelTypeI1R8G8B8A8RLE && PixelType != ImageViewer.PixelType.HRPPixelTypeI1R8G8B8RLE || BitmapBuffer.Length > 0))
                return !(IsIncompleteArray = false);
            else
                return !(IsIncompleteArray = true);
        }
    }
    public class WatchWindowModel
    {
        private VisualizerModel m_VisualizerModel;
        private ImageViewerPackage m_Parent;
        private List<Watch> m_WatchedVariable;
        private BitmapManipulator m_Manipulator;
        private WatchWindow m_View;
        private DebuggerConnector m_Connector;
        private PixelConverter m_PixelConverter;

        public WatchWindowModel(VisualizerModel imageDebugger , ImageViewerPackage imageViewerPackage)
        {
            this.m_VisualizerModel = imageDebugger;
            this.m_Parent = imageViewerPackage;
            imageDebugger.SetWatchModel(this);
            m_Manipulator = new BitmapManipulator();
            m_View = m_Parent.GetWatchControl();
            m_View.SetModel(this);
            m_WatchedVariable = new List<Watch>();
            m_PixelConverter = new PixelConverter();
            LoadAllWatch();
        }


        public void AddWatch(Watch watch)
        {
            try
            {
                watch.BitmapThumbnail = m_Manipulator.GetThumbnail(m_Manipulator.GetBitmapFromByteArray(watch.Width , watch.Height , m_PixelConverter.ConvertToV32B8G8R8A8(watch.BitmapBuffer , watch.PixelType , watch.Palette , watch.Width , watch.Height)), m_VisualizerModel.GetBackgroundColor());
            }
            catch
            {
            }
            m_WatchedVariable.Add(watch);
            m_View.AddWatch(watch);
        }

        public void DeleteWatch(Watch watch)
        {
            m_WatchedVariable.Remove(watch);
        }

        public void EvaluateWatch(string expression)
        {
            m_Connector.AddWatch(expression);
        }

        public void SetConnector(DebuggerConnector debuggerConnector)
        {
            m_Connector = debuggerConnector;
        }

        public void SendToVisualiser(Watch watch)
        {
            m_VisualizerModel.SetBitmap(watch);
        }

        public void UpdateAllWatch()
        {
            if( m_View.IsVisible )
            {
                for( int i = 0 ; i < m_WatchedVariable.Count ; i++ )
                {
                 Watch watch = m_Connector.UpdateWatch(m_WatchedVariable[i]);                    
                 watch.Name = m_WatchedVariable[i].Name;
                 watch.VisualizerId = m_WatchedVariable[i].VisualizerId;
                 watch.IsWatchOfArray = m_WatchedVariable[i].IsWatchOfArray;
                 if(watch.IsWatchOfArray)
                 {
                     watch.Width = m_WatchedVariable[i].Width;
                     watch.Height = m_WatchedVariable[i].Height;
                     watch.PixelType = m_WatchedVariable[i].PixelType;
                 }
                 if( watch.Width != 0 && watch.Height != 0 )
                 {
                     try
                     {
                         watch.BitmapThumbnail = m_Manipulator.GetThumbnail(m_Manipulator.GetBitmapFromByteArray(watch.Width , watch.Height , m_PixelConverter.ConvertToV32B8G8R8A8(watch.BitmapBuffer , watch.PixelType , watch.Palette , watch.Width , watch.Height)), m_VisualizerModel.GetBackgroundColor());
                     }
                     catch
                     {
                     }
                 }
                 m_WatchedVariable[i] = watch;
                 m_View.UpdateWatch(m_WatchedVariable[i] , i);
                }
            }
        }

        public void SaveAllWatch()
        {
            Configuration conf = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
            conf.AppSettings.Settings.Clear();
            int i = 0;
            foreach( Watch watch in m_WatchedVariable )
            {
                conf.AppSettings.Settings.Add("ImageViewer" + i.ToString() , watch.Name);
                if( watch.IsWatchOfArray )
                {
                    conf.AppSettings.Settings.Add("ImageViewer" + i.ToString()+"Width" , watch.Width.ToString());
                    conf.AppSettings.Settings.Add("ImageViewer" + i.ToString()+"Height" , watch.Height.ToString());
                    conf.AppSettings.Settings.Add("ImageViewer" + i.ToString()+"PixelType" , watch.PixelType.ToString());
                }
                i++;
            }
            conf.Save(ConfigurationSaveMode.Full);

        }
        private void LoadAllWatch()
        {
            Configuration conf = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
            for(int i = 0; true; i++)
            {
                Watch watch = new Watch();
                try
                {
                    watch.Name = conf.AppSettings.Settings["ImageViewer" + i.ToString()].Value;
                    try
                    {
                        watch.Width = Convert.ToInt32(conf.AppSettings.Settings["ImageViewer" + i.ToString()+"Width"].Value);
                        watch.Height = Convert.ToInt32(conf.AppSettings.Settings["ImageViewer" + i.ToString()+"Height"].Value);
                        watch.PixelType = PixelConverter.GetPixelType(conf.AppSettings.Settings["ImageViewer" + i.ToString()+"PixelType"].Value);
                        watch.IsWatchOfArray = true;
                    }
                    catch
                    {
                        watch.IsWatchOfArray = false;
                    }
                    AddWatch(watch);
                }
                catch
                {
                    break;
                }
            }
        }
    }
}
