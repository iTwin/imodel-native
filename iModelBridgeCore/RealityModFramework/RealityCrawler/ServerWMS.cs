using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using RealityPlatformWMSCapabilities;
using System.Net;
using System.Xml;
using System.IO;
using System.Diagnostics;
using Bentley.RealityPlatform.RealityCrawlerDB;
using System.Data.Entity.Spatial;

using System.Data.Entity.Validation;

namespace Bentley.RealityPlatform.RealityCrawler
    {
    /// <summary>
    /// WMS server object to get the paremeters of the servers and save them to the database.
    /// </summary>
    class ServerWMS
        {
        #region Constants
        // Constants
        /// <summary>
        /// List of the default coordinate systems to find first in the WMS server.
        /// </summary>
        public readonly static String[] DefaultCSArray = { "EPSG:4326",
                                                           "EPSG:4269",
                                                           "EPSG:4258",
                                                           "EPSG:4612",
                                                           "EPSG:4148",
                                                           "EPSG:4126",
                                                           "EPSG:4140",
                                                           "EPSG:4152",
                                                           "EPSG:4167",
                                                           "EPSG:4283",
                                                           "EPSG:4674",
                                                           "EPSG:4171",
                                                           "CRS:84",
                                                           "CRS:83",
                                                           "EPSG:3857",
                                                           "EPSG:900913" };
        /// <summary>
        /// Enum of the URL response status.
        /// </summary>
        public enum ResponseStatus
            {
            InvalidURL,
            NoResponse,
            NotXML,
            NotCapabilities,
            Success
            };
        #endregion

        #region Private Members
        private Boolean m_validWMSServer = true;
        private WMSCapabilitiesNet m_capabilities;
        private String m_xmlContent;
        #endregion

        #region Constructor
        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Server URL to get the capabilities</param>
        public ServerWMS (String url)
            {
            try
                {
                SetServerURL (url);
                GetCapabilities ();
                }
            catch ( Exception )
                {
                // Invalid URL.
                m_validWMSServer = false;
                Status = ResponseStatus.InvalidURL;
                }
            }

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="url">Server URL to get the capabilities</param>
        public ServerWMS (Uri url)
            {
            try
                {

                SetServerURL (url);
                GetCapabilities ();
                }
            catch ( Exception )
                {
                // Invalid URL.
                m_validWMSServer = false;
                Status = ResponseStatus.InvalidURL;
                }
            }
        #endregion

        #region Public Properties
        /// <summary>
        /// Server URL to get the capabilities.
        /// </summary>
        public Uri ServerURL
            {
            get;
            private set;
            }

        /// <summary>
        /// Latency of the WMS server in Milliseconds.
        /// </summary>
        public double Latency
            {
            get;
            private set;
            }

        /// <summary>
        /// Status of the URL request.
        /// </summary>
        public ResponseStatus Status
            {
            get;
            private set;
            }
        #endregion

        #region Methods
        #region Public Methods
        /// <summary>
        /// Allow to know if the URL is a valid WMS server URL.
        /// </summary>
        /// <returns>True if the URL is a valid WMS server URL, False elsewhere.</returns>
        public Boolean isValid ()
            {
            return m_validWMSServer;
            }

        /// <summary>
        /// Save the valid WMS server capabilities to an SQL database.
        /// </summary>
        public void Save ()
            {
            if ( this.isValid () )
                {
                using ( RealityCrawlerDBContainer context = new RealityCrawlerDBContainer () )
                    {
                    String currentServerURL;
                     if (String.IsNullOrWhiteSpace(m_capabilities.GetCapabilityGroup ().GetRequest ().GetCapabilities ().GetDcpType ().GetHttpGet ().GetHref ()))
                        currentServerURL = ServerURL.AbsoluteUri;
                    else
                         currentServerURL = m_capabilities.GetCapabilityGroup ().GetRequest ().GetCapabilities ().GetDcpType ().GetHttpGet ().GetHref ();

                    Server wmsServer = context.ServerSet.FirstOrDefault (Server => Server.URL.Equals (currentServerURL));
                    if ( wmsServer == null )
                        {
                        Save (context, ServerState.New, 0);
                        }
                    }
                }
            }

        /// <summary>
        /// Save the valid WMS server capabilities to an SQL database.
        /// </summary>
        /// <param name="context">The current database context.</param>
        /// <param name="State">Default state to set for the new WMS server entry in the databse.</param>
        public void Save (RealityCrawlerDBContainer context, ServerState State, int OldServerId)
            {
            // Trick to speedup the database operations by stopping the Change detection.               
            context.Configuration.AutoDetectChangesEnabled = false;
   
            Server wmsServer = new Server ();
            wmsServer.Latency = Latency;
            wmsServer.Online = true; // ToDo: manage offline server
            wmsServer.LastCheck = DateTime.UtcNow; // ToDo: manage offline server
            wmsServer.LastTimeOnline = DateTime.UtcNow; // ToDo: manage offline server
            wmsServer.State = State;
            if ( String.IsNullOrWhiteSpace (m_capabilities.GetCapabilityGroup ().GetRequest ().GetCapabilities ().GetDcpType ().GetHttpGet ().GetHref ()) )
                wmsServer.URL = ServerURL.AbsoluteUri; // ToDo: Determine which URL should be saved
            else
                wmsServer.URL = m_capabilities.GetCapabilityGroup ().GetRequest ().GetCapabilities ().GetDcpType ().GetHttpGet ().GetHref (); // ToDo: Determine which URL should be saved

            wmsServer.Name = m_capabilities.GetServiceGroup ().GetTitle ();
            wmsServer.Legal = "AccessConstraints: " + m_capabilities.GetServiceGroup ().GetAccessConstraints () + ", Fees: " + m_capabilities.GetServiceGroup ().GetFees ();
            wmsServer.Type = ServerType.WMS;
            if (OldServerId > 0)
                wmsServer.OldServerId = OldServerId;

            // Get the first coordinate system found if default one are not found.
            if ( m_capabilities.GetCapabilityGroup ().GetLayerList ().Count > 0 )
                {
                wmsServer.CoordinateSystem = m_capabilities.GetCapabilityGroup ().GetLayerList ().First<WMSLayerNet>().GetCRSList ().First<String> ();
                foreach ( String DefaultCS in DefaultCSArray )
                    {
                    if ( m_capabilities.GetCapabilityGroup ().GetLayerList ().First<WMSLayerNet> ().GetCRSList ().Contains (DefaultCS) )
                        {
                        wmsServer.CoordinateSystem = DefaultCS;
                        break;
                        }
                    }
                }

            SaveExtendedProperties (context, wmsServer);

            context.ServerSet.Add (wmsServer);
            
            try
                {
                context.SaveChanges ();
                }
            catch ( DbEntityValidationException e )
                {
                foreach ( var eve in e.EntityValidationErrors )
                    {
                    Debug.WriteLine ("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
                        eve.Entry.Entity.GetType ().Name, eve.Entry.State);
                    foreach ( var ve in eve.ValidationErrors )
                        {
                        Debug.WriteLine ("- Property: \"{0}\", Error: \"{1}\"",
                            ve.PropertyName, ve.ErrorMessage);
                        }
                    }
                throw;
                }
                    
            // Setback the Change detection state.
            context.Configuration.AutoDetectChangesEnabled = true;
            }

        /// <summary>
        /// Update the status of the server depending on the response if it is already in the database, create a new entry if not.
        /// </summary>
        public void UpdateStatus ()
            {
            if ( this.isValid () )
                {
                using ( RealityCrawlerDBContainer context = new RealityCrawlerDBContainer () )
                    {
                    Server wmsServer = context.ServerSet.FirstOrDefault (Server => Server.URL.Equals (m_capabilities.GetCapabilityGroup ().GetRequest ().GetCapabilities ().GetDcpType ().GetHttpGet ().GetHref ()));
                    if ( wmsServer != null )
                        UpdateStatus (wmsServer.ServerId);
                    else
                        Save (context, ServerState.New, 0);
                    }
                }
            }

        #endregion
        #region Private Methods
        /// <summary>
        /// Set the WMS server Url without the REQUEST and SERVICE parameters.
        /// </summary>
        /// <param name="url">WMS Server URL</param>
        private void SetServerURL(Uri url)
            {
            SetServerURL(url.AbsoluteUri);
            }

        /// <summary>
        /// Set the WMS server Url without the REQUEST and SERVICE parameters.
        /// </summary>
        /// <param name="url">WMS Server URL</param>
        private void SetServerURL(String url)
            {
                String StrURL = url.Split ('?')[0] + "?";
                StrURL = StrURL.Replace("&","");
                String[] QueryArray = url.Split ('?').Length > 1 ? url.Split ('?')[1].Split ('&') : null;
                foreach ( String Query in QueryArray )
                    {
                    if ( !Query.ToLowerInvariant ().Contains ("request=".ToLowerInvariant ()) && !Query.ToLowerInvariant ().Contains ("service=".ToLowerInvariant ()) && !String.IsNullOrWhiteSpace(Query))
                        StrURL += Query + "&";
                    }
                ServerURL = new Uri (StrURL);
            }

        /// <summary>
        /// Fill the WMSServer objet with the properties from the URL of that WMS Server.
        /// </summary>
        private void GetCapabilities ()
            {
            GetCapabilities (ServerURL);
            }

        /// <summary>
        /// Fill the WMSServer objet with the properties from the provided URL.
        /// </summary>
        /// <param name="url">Server URL to get the capabilities.</param>
        private void GetCapabilities (Uri url)
            {
            try
                {
                Stopwatch timer = new Stopwatch ();
                timer.Start ();

                WebRequest request = WebRequest.Create (url.AbsoluteUri + "SERVICE=WMS&REQUEST=GetCapabilities");
                // ToDo: Manage error here
                WebResponse response = request.GetResponse ();
                timer.Stop ();
                // ToDo : Mesure the download speed.
                TimeSpan timeTaken = timer.Elapsed;

                //Get the Response Stream from the URL 
                System.IO.Stream responseStream = response.GetResponseStream ();

                // Store the stream in a String
                Encoding encode = Encoding.GetEncoding ("utf-8");
                StreamReader readStream = new StreamReader (responseStream, encode);
                m_xmlContent = readStream.ReadToEnd ();

                m_capabilities = new WMSCapabilitiesNet (m_xmlContent);

                // Set the Response Time
                Latency = timeTaken.TotalMilliseconds;


                Status = ResponseStatus.Success;
                }
            catch ( WebException )
                {
                // No response from the server.
                m_validWMSServer = false;
                Status = ResponseStatus.NoResponse;
                }
            catch ( XmlException )
                {
                // Not an XML file.
                m_validWMSServer = false;
                Status = ResponseStatus.NotXML;
                }
            catch ( NullReferenceException )
                {
                // Not a valid getCapabilities file.
                m_validWMSServer = false;
                Status = ResponseStatus.NotCapabilities;
                }
            catch ( Exception )
                {
                // No response from the server.
                m_validWMSServer = false;
                Status = ResponseStatus.NoResponse;
                }

            }

        /// <summary>
        /// Update the status of the existing server depending on the response.
        /// </summary>
        /// <param name="ServerId">ServerId of the server to update.</param>
        private void UpdateStatus (int ServerId)
            {
            using ( var context = new RealityCrawlerDBContainer () )
                {
                Server ServerToUpdate = context.ServerSet.FirstOrDefault (Server => Server.ServerId == ServerId);                
                if ( ServerToUpdate == null )
                    {
                    // Manage the case where the serverId didn't exist in the database.
                    this.Save ();
                    }
                else
                    {
                    if ( this.isValid () )
                        {
                        if ( ServerToUpdate.ServerExtendedProperties != null)
                            {
                            if ( ServerToUpdate.ServerExtendedProperties.GetCapabilitiesData.Equals (m_xmlContent) )
                                {
                                ServerToUpdate.Online = true;
                                ServerToUpdate.LastCheck = DateTime.UtcNow;
                                ServerToUpdate.LastTimeOnline = DateTime.UtcNow;
                                ServerToUpdate.Latency = Latency;

                                try
                                    {
                                    context.SaveChanges ();
                                    }
                                catch ( DbEntityValidationException e )
                                    {
                                    foreach ( var eve in e.EntityValidationErrors )
                                        {
                                        Debug.WriteLine ("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
                                            eve.Entry.Entity.GetType ().Name, eve.Entry.State);
                                        foreach ( var ve in eve.ValidationErrors )
                                            {
                                            Debug.WriteLine ("- Property: \"{0}\", Error: \"{1}\"",
                                                ve.PropertyName, ve.ErrorMessage);
                                            }
                                        }
                                    throw;
                                    }
                                }
                            else
                                {
                                ServerToUpdate.State = ServerState.Old;
                                try
                                    {
                                    context.SaveChanges ();
                                    }
                                catch ( DbEntityValidationException e )
                                    {
                                    foreach ( var eve in e.EntityValidationErrors )
                                        {
                                        Debug.WriteLine ("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
                                            eve.Entry.Entity.GetType ().Name, eve.Entry.State);
                                        foreach ( var ve in eve.ValidationErrors )
                                            {
                                            Debug.WriteLine ("- Property: \"{0}\", Error: \"{1}\"",
                                                ve.PropertyName, ve.ErrorMessage);
                                            }
                                        }
                                    throw;
                                    }
                                this.Save (context, ServerState.Modify, ServerId);
                                }
                            }
                        else
                            {
                            ServerToUpdate.State = ServerState.Old;
                            try
                                {
                                context.SaveChanges ();
                                }
                            catch ( DbEntityValidationException e )
                                {
                                foreach ( var eve in e.EntityValidationErrors )
                                    {
                                    Debug.WriteLine ("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
                                        eve.Entry.Entity.GetType ().Name, eve.Entry.State);
                                    foreach ( var ve in eve.ValidationErrors )
                                        {
                                        Debug.WriteLine ("- Property: \"{0}\", Error: \"{1}\"",
                                            ve.PropertyName, ve.ErrorMessage);
                                        }
                                    }
                                throw;
                                }
                            this.Save (context, ServerState.Modify, ServerId);
                            }
                        }
                    else
                        {
                        // Trick to speedup the database operations by stopping the Change detection.               
                        context.Configuration.AutoDetectChangesEnabled = false;

                        ServerToUpdate.Online = false;
                        ServerToUpdate.LastCheck = DateTime.UtcNow;

                        try
                            {
                            context.SaveChanges ();
                            }
                        catch ( DbEntityValidationException e )
                            {
                            foreach ( var eve in e.EntityValidationErrors )
                                {
                                Debug.WriteLine ("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
                                    eve.Entry.Entity.GetType ().Name, eve.Entry.State);
                                foreach ( var ve in eve.ValidationErrors )
                                    {
                                    Debug.WriteLine ("- Property: \"{0}\", Error: \"{1}\"",
                                        ve.PropertyName, ve.ErrorMessage);
                                    }
                                }
                            throw;
                            }

                        // Setback the Change detection state.
                        context.Configuration.AutoDetectChangesEnabled = true;
                        }
                    }
                }
            }

        /// <summary>
        /// Save the extended properties of the WMS server to the WMSServerExtendedProperties table.
        /// </summary>
        /// <param name="context">The current database context.</param>
        /// <param name="wmsServer">Parent Server of the extended propertie.</param>
        private void SaveExtendedProperties (RealityCrawlerDBContainer context, Server wmsServer)
            {
            WMSServerExtendedProperties extendedProperties = new WMSServerExtendedProperties ();

            extendedProperties.Version = m_capabilities.GetVersion ();
            if ( m_capabilities.GetServiceGroup () != null )
                {
                extendedProperties.Name = m_capabilities.GetServiceGroup ().GetName ();
                extendedProperties.Title = m_capabilities.GetServiceGroup ().GetTitle ();
                extendedProperties.Description = m_capabilities.GetServiceGroup ().GetAbstract ();
                if ( m_capabilities.GetServiceGroup ().GetContactInformation () != null )
                    {
                    if ( m_capabilities.GetServiceGroup ().GetContactInformation ().GetPerson () != null )
                        {
                        extendedProperties.ContactPerson = m_capabilities.GetServiceGroup ().GetContactInformation ().GetPerson ().GetName ();
                        extendedProperties.ContactOrganization = m_capabilities.GetServiceGroup ().GetContactInformation ().GetPerson ().GetOrganization ();
                        }
                    extendedProperties.ContactPosition = m_capabilities.GetServiceGroup ().GetContactInformation ().GetPosition ();
                    if ( m_capabilities.GetServiceGroup ().GetContactInformation ().GetAddress () != null )
                        {
                        extendedProperties.ContactAddressType = m_capabilities.GetServiceGroup ().GetContactInformation ().GetAddress ().GetTypeNet ();
                        extendedProperties.ContactAddress = m_capabilities.GetServiceGroup ().GetContactInformation ().GetAddress ().GetAddress ();
                        extendedProperties.ContactCity = m_capabilities.GetServiceGroup ().GetContactInformation ().GetAddress ().GetCity ();
                        extendedProperties.ContactStateOrProvince = m_capabilities.GetServiceGroup ().GetContactInformation ().GetAddress ().GetStateOrProvince ();
                        extendedProperties.ContactPostCode = m_capabilities.GetServiceGroup ().GetContactInformation ().GetAddress ().GetPostCode ();
                        extendedProperties.ContactCountry = m_capabilities.GetServiceGroup ().GetContactInformation ().GetAddress ().GetCountry ();
                        }
                    extendedProperties.ContactVoiceTelephone = m_capabilities.GetServiceGroup ().GetContactInformation ().GetVoiceTelephone ();
                    extendedProperties.ContactFacsimileTelephone = m_capabilities.GetServiceGroup ().GetContactInformation ().GetFacsimileTelephone ();
                    extendedProperties.ContactElectronicMailAddress = m_capabilities.GetServiceGroup ().GetContactInformation ().GetEmailAddress ();
                    }
                extendedProperties.Fees = m_capabilities.GetServiceGroup ().GetFees ();
                extendedProperties.AccessConstraints = m_capabilities.GetServiceGroup ().GetAccessConstraints ();
                }
            extendedProperties.GetCapabilitiesData = m_xmlContent;
            extendedProperties.Server = wmsServer;
            if ( m_capabilities.GetCapabilityGroup () != null )
                {
                extendedProperties.GetMapURL = m_capabilities.GetCapabilityGroup ().GetRequest ().GetMap ().GetDcpType ().GetHttpGet ().GetHref ().Split ('?')[0];
                extendedProperties.GetMapURLQuery = m_capabilities.GetCapabilityGroup ().GetRequest ().GetMap ().GetDcpType ().GetHttpGet ().GetHref ().Split ('?').Length > 1 ? m_capabilities.GetCapabilityGroup ().GetRequest ().GetMap ().GetDcpType ().GetHttpGet ().GetHref ().Split ('?')[1] : "";

                // Save all the supported format in one string separated by ','.
                foreach ( String Format in m_capabilities.GetCapabilityGroup ().GetRequest ().GetMap ().GetFormatList () )
                    {
                    extendedProperties.SupportedFormats += Format + ",";
                    }
                extendedProperties.SupportedFormats = extendedProperties.SupportedFormats.TrimEnd (',');

                // Save all the layers in the databases.
                //extendedProperties.LayerNumber = SaveLayers (context, m_capabilities.RootLayer, extendedProperties);

                int currtentLayerCount = 0;
                foreach ( WMSLayerNet Layer in m_capabilities.GetCapabilityGroup ().GetLayerList () )
                    {
                    currtentLayerCount = SaveLayers (context, Layer, extendedProperties, null, currtentLayerCount);
                    }
                }

            context.WMSServerExtendedPropertiesSet.Add (extendedProperties);
            }

        /// <summary>
        /// Save all the sublayers of the root layer to the SQL database.
        /// </summary>
        /// <param name="context">The current database context.</param>
        /// <param name="Layer">Root WMSLayer to add to the database.</param>
        /// <param name="extendedProperties">Parent WMSServerExtendedProperties of the layer.</param>
        private int SaveLayers (RealityCrawlerDBContainer context, WMSLayerNet Layer, WMSServerExtendedProperties extendedProperties)
            {
            return SaveLayers (context, Layer, extendedProperties, null, 0);
            }

        /// <summary>
        /// Sort the coordinates and generate a lonlat geometry.
        /// </summary>
        /// <param name="Xmin">West longitude.</param>
        /// <param name="Xmax">East longitude.</param>
        /// <param name="Ymin">South latitude.</param>
        /// <param name="Ymax">North latitude.</param>
        /// <returns>A LonLat DbGeometry</returns>
        private DbGeometry FilterBoundingBox (double Xmin, double Xmax, double Ymin, double Ymax)
            {
            if(Xmin > Xmax)
                Swap (ref Xmin, ref Xmax);

            if (Ymin > Ymax)
                Swap (ref Ymin, ref Ymax);

            // This method will on detect that the latitude and longitude have been inverted if the longitude is outside -90 and 90. 
            // If the longitude is in the interval of -90 and 90 there no way to know if the latitude and longitude had been inverted.
            // Note: Latitude horizontal line between -90 and 90 (ex. Equator).
            //       Longitude vertical line between -180 and 180 (ex. Greenwich).
            if ( Ymin > 90 || Ymin < -90 || Ymax > 90 || Ymax < -90 )
                {
                Swap (ref Xmin, ref Ymin);
                Swap (ref Xmax, ref Ymax);
                }

            return DbGeometry.PolygonFromText ("POLYGON((" +
                    Xmax.ToString () + " " + Ymax.ToString () + "," +
                    Xmax.ToString () + " " + Ymin.ToString () + "," +
                    Xmin.ToString () + " " + Ymin.ToString () + "," +
                    Xmin.ToString () + " " + Ymax.ToString () + "," +
                    Xmax.ToString () + " " + Ymax.ToString () +
                    "))", 4326);
            }

        /// <summary>
        /// Save all the sublayers of the current layer to the SQL database.
        /// </summary>
        /// <param name="context">The current database context.</param>
        /// <param name="Layer">WMSLayer to add to the database.</param>
        /// <param name="extendedProperties">Parent WMSServerExtendedProperties of the layer.</param>
        /// <param name="ParentLayer">Parent layer of the current layer need to keep the hierachy.</param>
        private int SaveLayers (RealityCrawlerDBContainer context, WMSLayerNet Layer, WMSServerExtendedProperties extendedProperties, WMSServerData ParentLayer, int LayersCount)
            {
            try
                {
                var layerData = new WMSServerData ();
                layerData.Name = Layer.GetName ();
                layerData.Title = Layer.GetTitle ();
                layerData.Description = Layer.GetAbstract ();
                if ( Layer.GetGeoBBox () != null )
                    {
                    layerData.BoundingBox = FilterBoundingBox (
                        Layer.GetGeoBBox ().GetWestBoundLong (), 
                        Layer.GetGeoBBox ().GetEastBoundLong (), 
                        Layer.GetGeoBBox ().GetSouthBoundLat (), 
                        Layer.GetGeoBBox ().GetNorthBoundLat ());
                    }
                else if ( Layer.GetLatLonBBox () != null )
                    {
                    layerData.BoundingBox = FilterBoundingBox (
                        Layer.GetLatLonBBox ().GetMinX (), 
                        Layer.GetLatLonBBox ().GetMaxX (), 
                        Layer.GetLatLonBBox ().GetMinY (), 
                        Layer.GetLatLonBBox ().GetMaxY ());
                    }                
                else if ( !ParentLayer.BoundingBox.IsEmpty )
                    layerData.BoundingBox = ParentLayer.BoundingBox;
                else
                    layerData.BoundingBox = FilterBoundingBox (0, 0, 0, 0);                

                // Save all the availiable coordinate systems for the layer in one string separated by ','.
                if ( Layer.GetCRSList ().Count > 0 )
                    {
                    foreach ( String cs in Layer.GetCRSList () )
                        {
                        layerData.CoordinateSystems += cs + ",";
                        }
                    layerData.CoordinateSystems = layerData.CoordinateSystems.TrimEnd (',');
                    }

                if ( Layer.GetStyle() != null)
                    {
                    layerData.Style = Layer.GetStyle ().GetName ();
                    }

                layerData.WMSServerExtendedProperties = extendedProperties;

                if ( ParentLayer != null )
                    layerData.WMSDataParent = ParentLayer;

                int currtentLayerCount = LayersCount;
                foreach ( WMSLayerNet ChildLayer in Layer.GetLayerList () )
                    {
                    currtentLayerCount = SaveLayers (context, ChildLayer, extendedProperties, layerData, currtentLayerCount);
                    }

                // ToDo: Generer l'éritage des layers.
                context.WMSServerDataSet.Add (layerData);
                return currtentLayerCount++;
                }
            catch ( Exception e)
                {
                Console.WriteLine ("Invalid layer: {0}.", e.Message);
                return LayersCount;
                }
            }        
        #endregion
        #region Static Methods
        #region Public Static Methods
        /// <summary>
        /// Update all the MS server status in the database.
        /// </summary>
        public static void Update ()
            {
            using ( var context = new RealityCrawlerDBContainer () )
                {
                Dictionary<int, string> ServerURLToUpdate = GenerateCrawledServerList (context, true);
                // Remove the modifyed server from the database before updating the server status.
                var ModifyedServers = context.ServerSet.Where (Server => Server.Type == ServerType.WMS).Where (Server => Server.State == ServerState.Modify);
                foreach (var ModifiedServer in  ModifyedServers)
                    {
                    if ( ModifiedServer.ServerExtendedProperties != null )
                        context.WMSServerExtendedPropertiesSet.Remove (ModifiedServer.ServerExtendedProperties);
                    }
                context.ServerSet.RemoveRange (ModifyedServers);
                try
                    {
                    context.SaveChanges ();
                    }
                catch ( DbEntityValidationException e )
                    {
                    foreach ( var eve in e.EntityValidationErrors )
                        {
                        Debug.WriteLine ("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
                            eve.Entry.Entity.GetType ().Name, eve.Entry.State);
                        foreach ( var ve in eve.ValidationErrors )
                            {
                            Debug.WriteLine ("- Property: \"{0}\", Error: \"{1}\"",
                                ve.PropertyName, ve.ErrorMessage);
                            }
                        }
                    throw;
                    }
                int ServerURLToUpdateOrderNo = 1; // Used only to display info to the user.
                foreach ( KeyValuePair<int, string> kvp in ServerURLToUpdate )
                    {
                    // For debug print the URL to the console.
                    System.Console.WriteLine ("Updating ({0}/{1}): {2}", ServerURLToUpdateOrderNo, ServerURLToUpdate.Count, kvp.Value);
                    ServerURLToUpdateOrderNo++;
                    // ToDo: Verify the way to update and test the server. Need to modify getCapability().
                    ServerWMS UpdatedWMSServer = new ServerWMS (kvp.Value);
                    UpdatedWMSServer.UpdateStatus (kvp.Key);
                    }
                }
            }

        /// <summary>
        /// Generate a Dictionary of the WMS server ID and URL from the database.
        /// </summary>
        /// <returns>Return a dictionary containing the ServerId and the URL.</returns>
        public static Dictionary<int, string> GenerateCrawledServerList ()
            {
            using ( var context = new RealityCrawlerDBContainer () )
                {
                return GenerateCrawledServerList(context, false);
                }
            }        

        #endregion
        #region Private Static Methods
        /// <summary>
        /// Generate a Dictionary of the WMS server ID and URL from the database.
        /// </summary>
        /// <param name="context">The current database context.</param>
        /// <param name="ToUpdate">If the ToUpdate flag is true exclude the rejected server from the list.</param>
        /// <returns>Return a dictionary containing the ServerId and the URL.</returns>
        private static Dictionary<int, string> GenerateCrawledServerList (RealityCrawlerDBContainer context, Boolean ToUpdate)
            {
            Dictionary<int, string> ServerURL;    
            if (ToUpdate)
                ServerURL = context.ServerSet.Where (Server => Server.Type == ServerType.WMS).Where(Server => Server.State > ServerState.Reject).Where(Server => Server.State != ServerState.Modify).ToDictionary (Server => Server.ServerId, Server => Server.URL);
            else
                ServerURL = context.ServerSet.Where (Server => Server.Type == ServerType.WMS).Where (Server => Server.State != ServerState.Modify).ToDictionary (Server => Server.ServerId, Server => Server.URL.Split('?')[0]);
            return ServerURL;
            }

        /// <summary>
        /// Swap the content of the var1 and var2.
        /// </summary>
        /// <typeparam name="T">Type of the vars.</typeparam>
        /// <param name="var1">First var to be swapped.</param>
        /// <param name="var2">Second var to be swapped.</param>
        private static void Swap<T> (ref T var1, ref T var2)
            {
            T temp = var1;
            var1 = var2;
            var2 = temp;
            }
        #endregion
        #endregion
        #endregion
        }
    }
