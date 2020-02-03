/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/WatchWindowToolWindow.cs $
|    $RCSfile: WatchWindowToolWindow.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/22 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Runtime.InteropServices;
using Microsoft.VisualStudio.Shell;

namespace Bentley.ImageViewer
{
    /// <summary>
    /// This class implements the tool window exposed by this package and hosts a user control.
    ///
    /// In Visual Studio tool windows are composed of a frame (implemented by the shell) and a pane, 
    /// usually implemented by the package implementer.
    ///
    /// This class derives from the ToolWindowPane class provided from the MPF in order to use its 
    /// implementation of the IVsUIElementPane interface.
    /// </summary>
    [Guid("fde8f819-d231-4ff1-b343-d83e313cb6ee")]
    public class WatchWindowToolWindow : ToolWindowPane
    {
        /// <summary>
        /// Standard constructor for the tool window.
        /// </summary>
        public WatchWindowToolWindow() :
            base(null)
        {
            // Set the window title reading it from the resources.
            this.Caption = Resources.WatchWindowTitle;
            // Set the image that will appear on the tab of the window frame
            // when docked with an other window
            // The resource ID correspond to the one defined in the resx file
            // while the Index is the offset in the bitmap strip. Each image in
            // the strip being 16x16.
            this.BitmapResourceID = 301;
            this.BitmapIndex = 1;

            // This is the user control hosted by the tool window; Note that, even if this class implements IDisposable,
            // we are not calling Dispose on this object. This is because ToolWindowPane calls Dispose on 
            // the object returned by the Content property.
            base.Content = new WatchWindow();
        }
    }
}
