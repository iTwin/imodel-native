/*--------------------------------------------------------------------------------------+
|
|     $Source: Prototypes/ImageDebugger/DebuggerConnector.cs $
|    $RCSfile: DebuggerConnector.cs, $
|   $Revision: 1 $
|       $Date: 2013/08/15 $
|     $Author: Julien Rossignol $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
using System;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Windows;
using Microsoft.VisualStudio.Debugger.Interop;
using Microsoft.VisualStudio.Shell.Interop;
using Microsoft.VisualStudio.Shell;


namespace Bentley.ImageViewer
{
    public struct ErrorReport
    {
        public string VersionName;
        public bool FailedWidth;
        public bool FailedHeight;
        public bool InvalidWidth;
        public bool InvalidHeight;
        public bool FailedPixelType;
        public bool FailedPalette;
        public bool UnmanagedPixelType;
        public bool FailedBuffer;
        public bool Failed; //is true if anything went wrong
    }

    /// <summary>
    /// This is the most important class of the package, here resides everything needed to communicate with visual studio's debugger, 
    /// </summary>
    /// <author>Julien Rossignol</author>
    //IvsCppDebugUIVisualizer is an interface allowing the package to be called when the magnifying glass icon for visualizers is pressed in the debugger. DisplayValue() is part of this interface
    //IBeImageDebuggerService is an iterface allowing the package to correctly register this class as a visualizer
    //IDebugEventCallback2 is an interface allowing this class to receive callback on debugger event, Event() method is part of this interface
    public class DebuggerConnector : IVsCppDebugUIVisualizer , IBeImageDebuggerService, IDebugEventCallback2
    {
        //model class managing both windows
        private VisualizerModel m_VisualizerModel = null; 
        private WatchWindowModel m_WatchModel = null;
        private TypedefXmlReader m_XmlReader; //parse the typedef.xml file
        private List<ErrorReport> m_ErrorReport;
        private Version m_typedefAttribute; //current typedef attribute
        private ImageViewerPackage m_Parent; //parent package, used to manage context menu button state
        private IDebugExpressionContext2 m_Context; // current debugguing context, the context is needed to evaluate expression and allow for watch
        private ErrorReport m_CurrentErrorReport;

        /// <summary>
        /// Constructor for the connector
        /// </summary>
        public DebuggerConnector(VisualizerModel visualizerModel , WatchWindowModel watchModel , ImageViewerPackage parent)
        {
            m_VisualizerModel = visualizerModel;
            m_WatchModel = watchModel;
            m_Parent = parent;
            m_XmlReader = new TypedefXmlReader();
            m_ErrorReport = new List<ErrorReport>();

            IVsDebugger debugService = Microsoft.VisualStudio.Shell.Package.GetGlobalService(typeof(SVsShellDebugger)) as IVsDebugger; //register the connector to debugger event so he can receive a callback on breakpoints
            if( debugService != null )
                debugService.AdviseDebugEventCallback(this);
        }

        /// <summary>
        /// Part of the IDebugEventCallback2 interface, it is called automatically by the debugger each time an event occurs
        /// </summary>
        /// <param name="pEngine">Interface corresponding to the debugger engine, it has no use for us</param>
        /// <param name="pProcess">Interface corresponding to the current debugger process, it has no use for us</param>
        /// <param name="pProgram">Interface corresponding to the current program attached to the debugger process, it has no use for us</param>
        /// <param name="pthread">Interface corresponding to the current debugger thread, this allow us to retrieve the debugger context, much need to evaluate expression</param>
        /// <param name="pEvent">Interface corresponding to the event, not used since the Guid gives us more info</param>
        /// <param name="eventGuid">Guid corresponding to the event, use to check if the event is a breakpoint stop</param>
        /// <param name="attrib">Flags indicating which type of event it is</param>
        /// <returns></returns>
        public int Event(IDebugEngine2 pEngine , IDebugProcess2 pProcess , IDebugProgram2 pProgram , IDebugThread2 pthread , IDebugEvent2 pEvent , ref Guid eventGuid , uint attrib)
        {
            THREADPROPERTIES[] threadProp = new THREADPROPERTIES[1];
            pthread.GetThreadProperties(enum_THREADPROPERTY_FIELDS.TPF_STATE , threadProp); //retrieve the properties of the thread to check the thread state

            if( threadProp[0].dwThreadState == 0x0002 )//check if thread is stopped because of a breakpoint
            {
                IEnumDebugFrameInfo2 frameInfoEnum; 
                pthread.EnumFrameInfo(enum_FRAMEINFO_FLAGS.FIF_FRAME , 10 , out frameInfoEnum); //retrieve frameInfoEnum
                FRAMEINFO[] frameInfo = new FRAMEINFO[1];
                uint celtFetched = 0;
                frameInfoEnum.Next(1 , frameInfo , ref celtFetched); //retrieve frameInfo from frameInfoEnum
                IDebugExpressionContext2 context;
                frameInfo[0].m_pFrame.GetExpressionContext(out context); //retrieve debugger context from frameInfo
                m_Context = context; 
                m_Parent.ChangeButtonEnabled(true); //tells visual studio that the context button can be pressed 

                if( eventGuid == new Guid("{ce6f92d3-4222-4b1e-830d-3ecff112bf22}") ) //this event GUID is called for every stop in the debugger, got it from a blog on the internet, never saw another reference to it, but it works perfectly
                {
                        m_WatchModel.UpdateAllWatch(); //since the debugger is stopped we need to update all watch
                        if( m_Parent.HasHexaDisplay == m_Parent.GetHexaDisplay() )
                        {
                            if( m_VisualizerModel.GetCurrentVariable() != "" && m_VisualizerModel.GetCurrentVariable() != null && m_Parent.VisualizerIsOpen() ) //if visualizer is open and has a variable
                            {
                                IDebugProperty3 prop3 = GetDebugProperty(m_VisualizerModel.GetCurrentVariable()); //retrieves the debugProperty corresponding to the visualizer variable
                                DEBUG_CUSTOM_VIEWER[] custom_Viewer = new DEBUG_CUSTOM_VIEWER[10];
                                uint nbFetched;
                                prop3.GetCustomViewerList(0 , 10 , custom_Viewer , out nbFetched); //retrives all visualizers for that variable
                                if( nbFetched > 0 )
                                {
                                    for( int i = 0 ; i < nbFetched ; i++ ) //iterate through all visualizer to find ours
                                    {
                                        if( custom_Viewer[i].bstrMetric == "{5452AFEA-3DF6-46BB-9177-C0B08F318025}" && m_VisualizerModel.GetWatch().VisualizerId == custom_Viewer[i].dwID ) // this GUID is specified in ImageViewerPackage.cs
                                        {
                                            if( custom_Viewer[i].dwID == 1 ) //if the variable is only a buffer
                                            {
                                                Watch watch =  UpdatePointer((IDebugProperty3) prop3 , m_VisualizerModel.GetWatch());
                                                m_VisualizerModel.SetBitmap(watch);
                                            }
                                            else
                                            {
                                                m_ErrorReport = new List<ErrorReport>();
                                                DisplayValue(0 , custom_Viewer[i].dwID , (IDebugProperty3) prop3);
                                                if( m_CurrentErrorReport.Failed )
                                                    OutputErrorReport();
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    m_VisualizerModel.OutOfScope();
                                }
                            }
                        }
                        else
                            m_Parent.HasHexaDisplay = m_Parent.GetHexaDisplay();
                }
            }
            else //if the program is running
            {
                m_Context = null;
                m_Parent.ChangeButtonEnabled(false);
            }
            return 0;
        }

        /// <summary>
        /// Update the buffer of the specified watch
        /// </summary>
        /// <param name="debugProperty">Debug property corresponding to the watch</param>
        /// <param name="watch">watch to update</param>
        /// <returns>watch updated</returns>
        private Watch UpdatePointer(IDebugProperty3 debugProperty , Watch watch)
        {
            int bufferSize = PixelConverter.GetNumberOfBitsPerPixel(watch.PixelType)/8*watch.Width*watch.Height;
            watch.BitmapBuffer =  GetBuffer(debugProperty , bufferSize, 1);
            return watch;
        }


        /// <summary>Part of the IVsCppDebugUIVisualizer, called automatically by visual studio
        /// Since we often need to do the same operation on watch, we also called it manually from time to time
        /// It fetches all needed information and fill the visualizer with the bitmap</summary>
        /// <param name="ownerHwnd">Correspond to the window of the debugger, it's an old framework that allowed to do pretty much anything on the window even from the outside of the program, we do not use it since it not really secure</param>
        /// <param name="visualizerId">the id of the visualiser, used to differentiate between variable types</param>
        /// <param name="debugProperty">the debugProperty corresponding to the variable</param>
        /// <returns></returns>
        public int DisplayValue(uint ownerHwnd , uint visualizerId , IDebugProperty3 debugProperty)
        {
            try
            {
                m_Parent.ShowControl(); //show the visualizer
                Watch watch = GetWatch(visualizerId , debugProperty); //retrieves the watch
                if( watch.Equals(new Watch()) ) //if the GetWatch operation failed
                    throw new Exception();
                m_VisualizerModel.SetBitmap(watch); 
                return 0;
            }
            catch //precise exception was already sent in the GetWatch() method
            {
                DEBUG_CUSTOM_VIEWER[] custom_Viewer = new DEBUG_CUSTOM_VIEWER[10];
                uint nbFetched;
                debugProperty.GetCustomViewerList(0 , 10 , custom_Viewer , out nbFetched); //retrives all visualizers for that variable
                for( int i = 0 ; i < nbFetched ; i++ )
                {
                    if( custom_Viewer[i].bstrMetric == "{5452AFEA-3DF6-46BB-9177-C0B08F318025}" )
                    {
                        if( custom_Viewer[i].dwID == visualizerId )
                            continue;
                        else
                        {
                            Watch watch = GetWatch(custom_Viewer[i].dwID , debugProperty); //retrieves the watch
                            if( watch.Equals(new Watch()) ) //if the GetWatch operation failed
                                return 0;
                            m_VisualizerModel.SetBitmap(watch); 
                        }
                    }
                }
                 return 0;
            }
        }

        /// <summary>
        /// Retrieves the watch from the specified IDebugProperty3
        /// </summary>
        /// <param name="visualizerId">the id of the visualiser, used to differentiate between variable types</param>
        /// <param name="debugProperty">the debugProperty corresponding to the variable</param>
        /// <returns>Watch containing the information needed to reconstitute a bitmap </returns>
        public Watch GetWatch(uint visualizerId , IDebugProperty3 debugProperty)
        {
            try
            {
                Watch watch = new Watch();

                if( visualizerId == 1 )
                    watch = ProcessPointer(debugProperty);
                else
                {
                    foreach( Version typedef in m_XmlReader.GetTypedef(visualizerId).version )
                    {
                        try
                        {
                            m_CurrentErrorReport = new ErrorReport();
                            m_CurrentErrorReport.VersionName = typedef.VersionName;
                            m_typedefAttribute = typedef;
                            watch = ProcessBitmap(debugProperty , visualizerId);
                            if( watch.Equals(new Watch()))
                                break;
                        }
                        catch
                        {
                            m_ErrorReport.Insert(0 , m_CurrentErrorReport);
                        }
                    }
                }

                watch.VisualizerId = visualizerId;
                DEBUG_PROPERTY_INFO[] info = new DEBUG_PROPERTY_INFO[1];
                debugProperty.GetPropertyInfo(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_FULLNAME , 10 , 10000 , null , 0 , info);
                watch.Name = info[0].bstrFullName; //get the name of the watched variable
                return watch;
            }
            catch
            {
                return new Watch(); 
            }   
        }

        /// <summary>
        /// Returns watch of a watched buffer, it shows a PtrDialog to allow the programmer to enter buffer bixel type, width and height
        /// </summary>
        private Watch ProcessPointer(IDebugProperty3 debugProperty)
        {
            Watch watch = new Watch();
            watch.IsWatchOfArray = true;
            watch.IsIncompleteArray = true;
            watch.PixelType = PixelType.HRPPixelTypeV24R8G8B8;
            return watch;
        }

        /// <summary>
        /// Returns watch of a complete image type
        /// </summary>
        private Watch ProcessBitmap(IDebugProperty3 debugProperty, uint id)
        {
            try
            {
                Watch watch = new Watch();     
               
                    string widthStr;
                    try
                    {
                        widthStr = FindValue(m_typedefAttribute.Width , debugProperty);

                        try
                        {
                            if( widthStr.IndexOf('.') > 0 )
                                watch.Width = Convert.ToInt32(widthStr.Remove(widthStr.IndexOf('.')));
                            else
                                watch.Width = Convert.ToInt32(widthStr);
                            if( watch.Width < 1 )
                                throw new Exception();
                        }
                        catch
                        {
                            m_CurrentErrorReport.InvalidWidth = true;
                            m_CurrentErrorReport.Failed = true;
                        }
                    }
                    catch
                    {
                        m_CurrentErrorReport.FailedWidth = true;
                        m_CurrentErrorReport.Failed = true;
                        throw;
                    }

                    string heightStr;
                    try
                    {
                        heightStr = FindValue(m_typedefAttribute.Height , debugProperty);

                        try
                        {
                            if( heightStr.IndexOf('.') > 0 )
                                watch.Height = Convert.ToInt32(heightStr.Remove(heightStr.IndexOf('.')));
                            else
                                watch.Height = Convert.ToInt32(heightStr);

                            if( watch.Height < 1 )
                                throw new Exception();
                        }
                        catch
                        {
                            m_CurrentErrorReport.InvalidHeight = true;
                            m_CurrentErrorReport.Failed = true;
                            throw;
                        }
                    }
                    catch
                    {
                        m_CurrentErrorReport.FailedHeight = true;
                        m_CurrentErrorReport.Failed = true;
                        throw;
                    }


                    string PixelTypeStr;
                    try
                    {
                        PixelTypeStr= GetPixelType(debugProperty);
                    }
                    catch
                    {
                        if( m_typedefAttribute.DefaultPixelType == null || m_typedefAttribute.DefaultPixelType == "" )
                        {
                            m_CurrentErrorReport.FailedPixelType = true;
                            m_CurrentErrorReport.Failed = true;
                            throw;
                        }
                        else
                            PixelTypeStr = m_typedefAttribute.DefaultPixelType;
                    }

                    try
                    {
                        watch.PixelType = PixelConverter.GetPixelType(PixelTypeStr);
                    }
                    catch
                    {
                        m_CurrentErrorReport.UnmanagedPixelType = true;
                        m_CurrentErrorReport.Failed = true;
                        throw;
                    }

                    watch.Palette = null;
                    if( PixelConverter.HasPalette(watch.PixelType) )
                    {
                        watch.Palette = GetPalette(debugProperty , watch.PixelType);
                    }
                    watch.IsWatchOfArray = false;

                    int bufferSize;
                    try
                    {
                        string bufferSizeStr = FindValue(m_typedefAttribute.BufferSize , debugProperty);
                        bufferSize = Convert.ToInt32(bufferSizeStr);
                    }
                    catch
                    {
                        if( watch.PixelType != PixelType.HRPPixelTypeI1R8G8B8RLE && watch.PixelType != PixelType.HRPPixelTypeI1R8G8B8A8RLE )
                            bufferSize = watch.Width*watch.Height*PixelConverter.GetNumberOfBitsPerPixel(watch.PixelType)/8;
                        else
                            bufferSize = 0;
                    }

                    watch.BitmapBuffer =  GetBuffer(debugProperty , bufferSize , id);

               return watch;
            }
            catch
            {
                throw;
            }
        }


        /// <summary>
        /// Get the byte[] corresponding to the palette inside the variable represented by the debugProperty
        /// </summary>
        private byte[] GetPalette(IDebugProperty3 debugProperty, PixelType pixelType)
        {
            try
            {
                int paletteSize;

                if( m_typedefAttribute.FixedPaletteSize == 0 ) //if the palette size is not directly specified in the typedef file
                {
                    try
                    {
                        string paletteSizeStr = FindValue(m_typedefAttribute.PaletteSize , debugProperty);
                        paletteSize = Convert.ToInt32(paletteSizeStr);
                    }
                    catch
                    {
                        paletteSize = PixelConverter.GetPaletteEntrySize(pixelType)*PixelConverter.GetNumberOfEntryForPalette(pixelType);
                    }
                }
                else
                    paletteSize = m_typedefAttribute.FixedPaletteSize;

                IDebugProperty2 paletteProperty = FindProperty(m_typedefAttribute.Palette , debugProperty).pProperty; //find the property corresponding to the palette

                IDebugMemoryContext2 paletteMemoryContext;
                paletteProperty.GetMemoryContext(out paletteMemoryContext); //retrieves the memory context of the palette

                IDebugMemoryBytes2 memoryBytes;
                paletteProperty.GetMemoryBytes(out memoryBytes); //retrieves an interface corresponding the the memory bytes of the palette

                byte[] palette = new byte[paletteSize];

                uint NbOfBytRead = 0;
                uint NbOfByteNotRead = 0;
                memoryBytes.ReadAt(paletteMemoryContext , (uint) paletteSize , palette , out NbOfBytRead , ref NbOfByteNotRead); //read the palette data  
                return palette;
            }
            catch
            {
                m_CurrentErrorReport.FailedPalette = true;
                m_CurrentErrorReport.Failed = true;
                throw;
            }
        }

        /// <summary>
        /// Get the pixelType from the debugProperty
        /// </summary>
        private string GetPixelType(IDebugProperty3 debugProperty)
        {
            if( m_typedefAttribute.FixedPixelType == null || m_typedefAttribute.FixedPixelType == "" ) //if no pixel type is directly specified in typedef.xml
            {
                return FindValue(m_typedefAttribute.PixelType , debugProperty);
            }
            else
                return m_typedefAttribute.FixedPixelType;
        }

        /// <summary>
        /// Get the pixel buffer from the given debugProperty
        /// </summary>
        private byte[] GetBuffer(IDebugProperty3 debugProperty , int bufferSize, uint VisualizerId)
        {
            try
            {
                IDebugProperty2 bufferProperty;
                if( VisualizerId > 1 ) //if the debugproperty is not directly the buffer
                    bufferProperty = FindProperty(m_typedefAttribute.Buffer , debugProperty).pProperty;
                else
                    bufferProperty = debugProperty;

                IDebugMemoryContext2 bufferMemoryContext;
                bufferProperty.GetMemoryContext(out bufferMemoryContext);

                IDebugMemoryBytes2 memoryBytes;
                bufferProperty.GetMemoryBytes(out memoryBytes);

                byte[] buffer = new byte[bufferSize];

                uint NbOfBytRead = 0;
                uint NbOfByteNotRead = 0;
                int memoryByteSucess = memoryBytes.ReadAt(bufferMemoryContext , (uint) bufferSize , buffer , out NbOfBytRead , ref NbOfByteNotRead); //read memory data directly and copy it into the buffer
                if( NbOfBytRead == 0 && bufferSize != 0 || memoryByteSucess != 0 )
                    throw new Exception();
                return buffer;
            }
            catch
            {
                m_CurrentErrorReport.FailedBuffer = true;
                m_CurrentErrorReport.Failed = true;
                throw;
            }
        }

        /// <summary>
        /// return the string representing the propertyName value
        /// </summary>
        /// <param name="propertyName">path of the property to get the value, to check in element class use the '.' operator, event for pointer. Parent classe must be put inside bracket[]. Ex: "m_class.m_Pointer.m_derivedClass.[baseClass].thepropertyiwant"</param>
        private string FindValue(string propertyName , IDebugProperty2 propertyToLookIn)
        {
            if(propertyName.EndsWith("%typename%", StringComparison.InvariantCultureIgnoreCase))
                return FindProperty(propertyName, propertyToLookIn).bstrName.Replace("[" , "").Replace("]" , "");
            else
                return FindProperty(propertyName, propertyToLookIn).bstrValue;
        }
        
        /// <summary>
        /// return the IDebugProperty corresponding to the propertyName string
        /// </summary>
        /// <param name="propertyName">path of the property to get the value, to check in element class use the '.' operator, event for pointer. Parent classe must be put inside bracket[]. Ex: "m_class.m_Pointer.m_derivedClass.[baseClass].thepropertyiwant"</param>
        private DEBUG_PROPERTY_INFO FindProperty(string propertyName , IDebugProperty2 propertyToLookIn)
        {
            string[] propertyList = propertyName.Split(new char[] { '.' } , 2); //retrieves the first element to get in the string

            Guid guid  = Guid.Empty;
            IEnumDebugPropertyInfo2 childProperty;

            propertyToLookIn.EnumChildren(enum_DEBUGPROP_INFO_FLAGS.DEBUGPROP_INFO_ALL , 10 , ref guid , enum_DBG_ATTRIB_FLAGS.DBG_ATTRIB_DATA , null , 10000 , out childProperty); //get an enum with all the children of the property

            uint count;
            childProperty.GetCount(out count);
            DEBUG_PROPERTY_INFO[] childInfo = new DEBUG_PROPERTY_INFO[1];
            
            uint nbFetched;
            childProperty.Next(1 , childInfo , out nbFetched);
            if( propertyList[0] == "%typename%" )
            {
                return childInfo[0];
            }
            else
            {
                for( uint i = 0 ; i < count && propertyList[0] != childInfo[0].bstrName ; i++ ) //cycle throught all children
                {
                    childProperty.Next(1 , childInfo , out nbFetched);
                }



                if( propertyList[0] == childInfo[0].bstrName ) //if children is found
                {
                    if( propertyList.GetLength(0) != 1 ) //if we need to go deeper into the property
                        return FindProperty(propertyList[1] , childInfo[0].pProperty);
                    else
                        return childInfo[0];
                }
                else
                {
                    return new DEBUG_PROPERTY_INFO();
                }
            }

        }


        /// <summary>
        /// Visualize the variable corresponding to the name
        /// </summary>
        /// <param name="name">Name of the variable to watch</param>
        public void Visualize(string name)
        {
            if( m_Context != null )
            {
                IDebugProperty3 prop3 = GetDebugProperty(name);
                DEBUG_CUSTOM_VIEWER[] custom_Viewer = new DEBUG_CUSTOM_VIEWER[10];
                uint nbFetched;
                prop3.GetCustomViewerList(0 , 10 , custom_Viewer ,out nbFetched);
                for( int i = 0 ; i < nbFetched ; i++ )
                {
                    if( custom_Viewer[i].bstrMetric == "{5452AFEA-3DF6-46BB-9177-C0B08F318025}" )
                    {
                        DisplayValue(0 , custom_Viewer[i].dwID , prop3);
                        return;
                    }
                }
            }
        }
        /// <summary>
        /// Add a watch of the variable corresponding to the bane
        /// </summary>
        /// <param name="name">Name of the variable to watch</param>
        public void AddWatch(string name)
        {
            if( m_Context != null )
            {
                IDebugProperty3 prop3 = GetDebugProperty(name);
                DEBUG_CUSTOM_VIEWER[] custom_Viewer = new DEBUG_CUSTOM_VIEWER[10];
                uint nbFetched;
                prop3.GetCustomViewerList(0 , 10 , custom_Viewer , out nbFetched);
                for( int i = 0 ; i < nbFetched ; i++ )
                {
                    if( custom_Viewer[i].bstrMetric == "{5452AFEA-3DF6-46BB-9177-C0B08F318025}"  )
                    {
                        Watch watch = GetWatch(custom_Viewer[i].dwID , prop3);
                        watch.Name = name;
                        m_Parent.ShowWatch();
                        m_WatchModel.AddWatch(watch);
                        return;
                    }
                }
            }
        }
        /// <summary>
        /// Return the IDebugProperty3 corresponding to the name
        /// </summary>
        /// <param name="name">Name of the variable</param>
        private IDebugProperty3 GetDebugProperty(string name)
        {
                IDebugExpression2 expression;
                string error;
                uint pichError;
                m_Context.ParseText(name , enum_PARSEFLAGS.PARSE_EXPRESSION , 10 , out expression , out error , out pichError);
                IDebugProperty2 property;
                expression.EvaluateSync(enum_EVALFLAGS.EVAL_NOSIDEEFFECTS , 3000 , null , out property);
                IntPtr pIUknown = Marshal.GetIUnknownForObject(property);
                Guid guidIDebugProperty3 = typeof(IDebugProperty3).GUID;
                IntPtr pIDebug3;
                Marshal.QueryInterface(pIUknown , ref guidIDebugProperty3 , out pIDebug3);
                IDebugProperty3 prop3 = (IDebugProperty3) Marshal.GetObjectForIUnknown(pIDebug3);
                return prop3;
        }

        /// <summary>
        /// Returns an updated version of the watch
        /// </summary>
        /// <param name="watch">Watch to update</param>
        /// <returns>updated watch</returns>
        public Watch UpdateWatch(Watch watch)
        {
            if( m_Context != null )
            {
                try
                {
                    IDebugProperty3 prop3 = GetDebugProperty(watch.Name);
                    DEBUG_CUSTOM_VIEWER[] custom_Viewer = new DEBUG_CUSTOM_VIEWER[10];
                    uint nbFetched;
                    prop3.GetCustomViewerList(0 , 10 , custom_Viewer , out nbFetched);
                    if( nbFetched == 0 )
                        return new Watch();
                    for( int i = 0 ; i < nbFetched ; i++ )
                    {
                        if( custom_Viewer[i].bstrMetric == "{5452AFEA-3DF6-46BB-9177-C0B08F318025}" && watch.VisualizerId == custom_Viewer[i].dwID )
                        {
                            if( watch.IsWatchOfArray )
                                return UpdatePointer(prop3, watch);
                            else
                                return GetWatch(custom_Viewer[i].dwID , prop3);
                        }
                    }
                }
                catch
                {
                    return new Watch();
                }
            }
            return watch;
        }

        /// <summary>
        /// Return a new pixel buffer
        /// </summary>
        /// <param name="watchName">Name of the variable to get the buffer</param>
        /// <param name="bufferSize">Size of the buffer to get</param>
        public byte[] GetNewBuffer(string watchName , int bufferSize)
        {
            if( m_Context != null )
            {
                IDebugProperty3 prop3 = GetDebugProperty(watchName);
                DEBUG_CUSTOM_VIEWER[] custom_Viewer = new DEBUG_CUSTOM_VIEWER[10];
                uint nbFetched;
                prop3.GetCustomViewerList(0 , 10 , custom_Viewer , out nbFetched);
                for( int i = 0 ; i < nbFetched ; i++ )
                {
                    if( custom_Viewer[i].bstrMetric == "{5452AFEA-3DF6-46BB-9177-C0B08F318025}"  )
                    {
                        return GetBuffer(prop3 , bufferSize , custom_Viewer[i].dwID);
                    }
                }
            }
            return new byte[1];
        }

        /// <summary>
        /// Output a message box indicating where to loading of the bitmap failed
        /// </summary>
        private void OutputErrorReport()
        {
            string MessageText = "Failed to retrieve bitmap information \n \n";
            foreach( ErrorReport report in m_ErrorReport )
            {
                if( report.Failed )
                {
                    MessageText += report.VersionName + "\n";
                    if(report.FailedBuffer)
                        MessageText += "Could not read buffer" + "\n";
                    if( report.FailedHeight )
                        MessageText += "Could not read height" + "\n";
                    if( report.FailedPalette )
                        MessageText += "Could not read palette" + "\n";
                    if( report.FailedPixelType )
                        MessageText += "Could not read pixel type" + "\n";
                    if( report.FailedWidth )
                        MessageText += "Invalid width value" + "\n";
                    if( report.InvalidHeight )
                        MessageText += "Invalid height value" + "\n";
                    if( report.UnmanagedPixelType )
                        MessageText += "Specified pixel type is not managed" + "\n";
                }
            }
            MessageBox.Show(MessageText , "Cannot retrieve information" , MessageBoxButton.OK , MessageBoxImage.Error);
        }

    }
}
