/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/FtpTraversalEngineNet.cs $
| 
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Data.Entity.Spatial;
using System.Data.Entity.Validation;
using System.Diagnostics;
using System.Linq;

using FtpTraversalEngineNet;
using RealityServicesCli;


namespace FtpTraversalEngineNet
    {
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    class FtpData
        {
        //! Constructor
        public FtpData()
            {
            m_extractedData = null;
            }

        //! Constructor
        public FtpData(FtpDataWrapper extractedData)
            {
            m_extractedData = extractedData;
            }

        //! Get
        public FtpDataWrapper GetExtractedData() { return m_extractedData; }

        //! Set
        public void SetExtractedData(FtpDataWrapper data) { m_extractedData = data; }

        //! Save to database
        public void Save()
            {
            using (RealityDB context = new RealityDB())
                {
                // Don't add data if it already exists.
                string currentUrl = m_extractedData.GetUrl();
                SpatialDataSource source = context.SpatialDataSources.FirstOrDefault(SpatialDataSource => SpatialDataSource.MainURL.Equals(currentUrl));
                if(null != source)
                    {
                    Console.WriteLine("ALREADY EXISTS");
                    return;
                    }

                Save(context);
                }
            }

        //! Save to database
        public void Save(RealityDB context)
            {
            // Metadata.
            Metadata metadata = new Metadata();
            metadata.Provenance = m_extractedData.GetMetadata().GetProvenance();
            metadata.Description = m_extractedData.GetMetadata().GetDescription();
            metadata.ContactInformation = m_extractedData.GetMetadata().GetContactInfo();
            metadata.Legal = m_extractedData.GetMetadata().GetLegal();
            metadata.RawMetadataFormat = m_extractedData.GetMetadata().GetFormat();
            metadata.RawMetadata = m_extractedData.GetMetadata().GetData();

            context.Metadatas.Add(metadata);

            // Thumbnail.
            Thumbnail thumbnail = new Thumbnail();
            thumbnail.ThumbnailProvenance = m_extractedData.GetThumbnail().GetProvenance();
            thumbnail.ThumbnailFormat = m_extractedData.GetThumbnail().GetFormat();
            thumbnail.ThumbnailWidth = m_extractedData.GetThumbnail().GetWidth().ToString();
            thumbnail.ThumbnailHeight = m_extractedData.GetThumbnail().GetHeight().ToString();
            thumbnail.ThumbnailStamp = m_extractedData.GetThumbnail().GetStamp();
            thumbnail.ThumbnailData = null; //m_extractedData.GetThumbnail().GetData();
            thumbnail.ThumbnailGenerationDetails = m_extractedData.GetThumbnail().GetGenerationDetails();

            context.Thumbnails.Add(thumbnail);

            // Spatial Entity Base.
            SpatialEntityBas entity = new SpatialEntityBas();
            entity.Name = m_extractedData.GetName();
            List<double> footprint = m_extractedData.GetFootprint();
            double xMin = double.MaxValue;
            double xMax = double.MinValue;
            double yMin = double.MaxValue;
            double yMax = double.MinValue;
            for (int i = 0; i < footprint.Count; ++i)
                {
                if(0 == i % 2)
                    {
                    xMin = xMin < footprint[i] ? xMin : footprint[i];
                    xMax = xMax > footprint[i] ? xMax : footprint[i];
                    }
                else
                    {
                    yMin = yMin < footprint[i] ? yMin : footprint[i];
                    yMax = yMax > footprint[i] ? yMax : footprint[i];
                    }
                }
            entity.Footprint = DbGeometry.PolygonFromText("POLYGON((" +
                                                              xMax.ToString() + " " + yMax.ToString() + "," +
                                                              xMax.ToString() + " " + yMin.ToString() + "," +
                                                              xMin.ToString() + " " + yMin.ToString() + "," +
                                                              xMin.ToString() + " " + yMax.ToString() + "," +
                                                              xMax.ToString() + " " + yMax.ToString() +
                                                              "))", 4326);
            entity.Date = Convert.ToDateTime(m_extractedData.GetDate());
            entity.Metadata_Id = metadata.Id;
            entity.Thumbnail_Id = thumbnail.Id;

            context.SpatialEntityBases.Add(entity);

            // Server.
            string currentServer = m_extractedData.GetServer().GetUrl();
            Server ftpServer = context.Servers.FirstOrDefault(Server => Server.URL.Equals((currentServer)));
            if(null == ftpServer)
                {
                //Add new server.
                ftpServer = new Server();
                ftpServer.CommunicationProtocol = m_extractedData.GetServer().GetProtocol();
                ftpServer.Name = m_extractedData.GetServer().GetName();
                ftpServer.URL = m_extractedData.GetServer().GetUrl();
                ftpServer.ServerContactInformation = m_extractedData.GetServer().GetContactInfo();
                ftpServer.Legal = m_extractedData.GetServer().GetLegal();
                ftpServer.Online = m_extractedData.GetServer().IsOnline();
                ftpServer.LastCheck = Convert.ToDateTime(m_extractedData.GetServer().GetLastCheck());
                ftpServer.LastTimeOnline = Convert.ToDateTime(m_extractedData.GetServer().GetLastTimeOnline());
                ftpServer.Latency = m_extractedData.GetServer().GetLatency();
                ftpServer.State = m_extractedData.GetServer().GetState();
                ftpServer.Type = m_extractedData.GetServer().GetServerType();

                context.Servers.Add(ftpServer);
                }           

            // Spatial Data Source.
            SpatialDataSource source = new SpatialDataSource();
            source.MainURL = m_extractedData.GetUrl();
            source.CompoundType = m_extractedData.GetCompoundType();
            source.FileSize = Convert.ToInt64(m_extractedData.GetSize());
            source.DataSourceType = m_extractedData.GetDataType();
            source.LocationInCompound = m_extractedData.GetLocationInCompound();
            source.Server_Id = ftpServer.Id;

            context.SpatialDataSources.Add(source);

            // Spatial Entity Spatial Data Source
            // ???

            try
                {
                context.SaveChanges();
                }
            catch (DbEntityValidationException e)
                {
                foreach (var eve in e.EntityValidationErrors)
                    {
                    Debug.WriteLine("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
                        eve.Entry.Entity.GetType().Name, eve.Entry.State);
                    foreach (var ve in eve.ValidationErrors)
                        {
                        Debug.WriteLine("- Property: \"{0}\", Error: \"{1}\"",
                            ve.PropertyName, ve.ErrorMessage);
                        }
                    }
                throw;
                }


            //// Trick to speedup the database operations by stopping the Change detection.               
            //context.Configuration.AutoDetectChangesEnabled = false;
            //
            //Server wmsServer = new Server();
            //wmsServer.Latency = Latency;
            //wmsServer.Online = true; // ToDo: manage offline server
            //wmsServer.LastCheck = DateTime.UtcNow; // ToDo: manage offline server
            //wmsServer.LastTimeOnline = DateTime.UtcNow; // ToDo: manage offline server
            //wmsServer.State = State;
            //if (String.IsNullOrWhiteSpace(m_capabilities.GetCapabilityGroup().GetRequest().GetCapabilities().GetDcpType().GetHttpGet().GetHref()))
            //    wmsServer.URL = ServerURL.AbsoluteUri; // ToDo: Determine which URL should be saved
            //else
            //    wmsServer.URL = m_capabilities.GetCapabilityGroup().GetRequest().GetCapabilities().GetDcpType().GetHttpGet().GetHref(); // ToDo: Determine which URL should be saved
            //
            //wmsServer.Name = m_capabilities.GetServiceGroup().GetTitle();
            //wmsServer.Legal = "AccessConstraints: " + m_capabilities.GetServiceGroup().GetAccessConstraints() + ", Fees: " + m_capabilities.GetServiceGroup().GetFees();
            //wmsServer.Type = ServerType.WMS;
            //if (OldServerId > 0)
            //    wmsServer.OldServerId = OldServerId;
            //
            //// Get the first coordinate system found if default one are not found.
            //wmsServer.CoordinateSystem = null;
            //if (m_capabilities.GetCapabilityGroup().GetLayerList().Count > 0)
            //    {
            //    if (m_capabilities.GetCapabilityGroup().GetLayerList().First<WMSLayerNet>().GetCRSList().Count > 0)
            //        {
            //        wmsServer.CoordinateSystem = m_capabilities.GetCapabilityGroup().GetLayerList().First<WMSLayerNet>().GetCRSList().First<String>();
            //        foreach (String DefaultCS in DefaultCSArray)
            //            {
            //            if (m_capabilities.GetCapabilityGroup().GetLayerList().First<WMSLayerNet>().GetCRSList().Contains(DefaultCS))
            //                {
            //                wmsServer.CoordinateSystem = DefaultCS;
            //                break;
            //                }
            //            }
            //        }
            //    }
            //
            //SaveExtendedProperties(context, wmsServer);
            //
            //context.ServerSet.Add(wmsServer);
            //
            //try
            //    {
            //    context.SaveChanges();
            //    }
            //catch (DbEntityValidationException e)
            //    {
            //    foreach (var eve in e.EntityValidationErrors)
            //        {
            //        Debug.WriteLine("Entity of type \"{0}\" in state \"{1}\" has the following validation errors:",
            //            eve.Entry.Entity.GetType().Name, eve.Entry.State);
            //        foreach (var ve in eve.ValidationErrors)
            //            {
            //            Debug.WriteLine("- Property: \"{0}\", Error: \"{1}\"",
            //                ve.PropertyName, ve.ErrorMessage);
            //            }
            //        }
            //    throw;
            //    }
            //
            //// Setback the Change detection state.
            //context.Configuration.AutoDetectChangesEnabled = true;

            Console.WriteLine("SUCCESS");
            }

        //! Private members
        private FtpDataWrapper m_extractedData;
        }

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    class FtpExplorerObserver : RealityServicesCli.IFtpTraversalObserverWrapper
        {
        public void OnDataExtracted(RealityServicesCli.FtpDataWrapper extractedData)
            {
            if (null == extractedData)
                return;

            Console.Write("Saving " + extractedData.GetName() + " to database... ");           

            // Save to database.
            FtpData data = new FtpData(extractedData);
            data.Save();
            }
        }


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    class FtpExplorer
        {
        static void Main(string[] args)
            {
            Console.Title = "FTP Traversal Engine";

            // Validate parameters.
            if (args.Length != 1)
                {
                Console.WriteLine("Invalid parameters.");
                Console.ReadKey();

                return;
                }

            FtpStatusWrapper status = FtpStatusWrapper.UnknownError;

            // Connect.
            FtpClientWrapper client = FtpClientWrapper.ConnectTo(args[0]);
            if(client == null)            
                {
                    Console.WriteLine("Could not connect to" + args[0]);
                    Console.WriteLine("Press any key to exit.");
                    Console.ReadKey();

                    return;
                }
            Console.WriteLine("Connected to " + args[0] + ". Retrieving data...");
              
            // Get data.
            FtpExplorerObserver observer = new FtpExplorerObserver();
            client.SetObserver(observer);  
            status = client.GetData();
            if (status != FtpStatusWrapper.Success)
                {
                Console.WriteLine("FAILED: " + status);
                Console.WriteLine("Press any key to exit.");
                Console.ReadKey();
            
                return;
                }

            // Terminate.
            Console.WriteLine("Press any key to exit.");
            Console.ReadKey();
            }
        }
    }
