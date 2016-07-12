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
        public enum ServerState : int
            {
            Reject = -1,
            New = 0,
            Valid = 1,
            Modify = 2,
            Old = 3
            }

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

        //! Destructor
        ~FtpData()
            {}

        //! Get / Set
        public FtpDataWrapper Get() { return m_extractedData; }
        public void Set(FtpDataWrapper data) { m_extractedData = data; }

        //! Save to database
        public void Save()
            {
            using (RealityDB context = new RealityDB())
                {
                // Don't add data if it already exists.
                string currentUrl = m_extractedData.GetUrl();
                SpatialDataSource source = context.SpatialDataSources.FirstOrDefault(SpatialDataSource => SpatialDataSource.MainURL.Equals(currentUrl));
                if(null != source)
                    return;

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
            // &&JFC Patch: rawMetadata is too huge to save, keep this field empty for now.
            // metadata.RawMetadata = m_extractedData.GetMetadata().GetData();
            metadata.RawMetadata = "";

            context.Metadatas.Add(metadata);

            // Thumbnail.
            Thumbnail thumbnail = new Thumbnail();
            thumbnail.ThumbnailProvenance = m_extractedData.GetThumbnail().GetProvenance();
            thumbnail.ThumbnailFormat = m_extractedData.GetThumbnail().GetFormat();
            thumbnail.ThumbnailWidth = m_extractedData.GetThumbnail().GetWidth().ToString();
            thumbnail.ThumbnailHeight = m_extractedData.GetThumbnail().GetHeight().ToString();
            thumbnail.ThumbnailStamp = m_extractedData.GetThumbnail().GetStamp();
            thumbnail.ThumbnailGenerationDetails = m_extractedData.GetThumbnail().GetGenerationDetails();

            List<byte> dataList = m_extractedData.GetThumbnail().GetData();
            byte[] dataArray = new byte[dataList.Count];
            for (int i = 0; i < dataList.Count; ++i)
                {
                dataArray[i] = dataList[i];
                }
            thumbnail.ThumbnailData = dataArray;

            context.Thumbnails.Add(thumbnail);

            // Server.
            string currentServer = m_extractedData.GetServer().GetUrl();
            Server ftpServer = context.Servers.FirstOrDefault(Server => Server.URL.Equals((currentServer)));
            if (null == ftpServer)
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
                ftpServer.State = ((int)ServerState.New).ToString();
                ftpServer.Type = m_extractedData.GetServer().GetServerType();

                context.Servers.Add(ftpServer);
                }           

            // Spatial Entity Base.
            SpatialEntityBas entityBase = new SpatialEntityBas();
            entityBase.Name = m_extractedData.GetName();
            entityBase.ResolutionInMeters = m_extractedData.GetResolution();
            entityBase.DataProvider = m_extractedData.GetProvider();
            entityBase.DataProviderName = m_extractedData.GetProvider();
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
            entityBase.Footprint = DbGeometry.PolygonFromText("POLYGON((" +
                                                              xMax.ToString() + " " + yMax.ToString() + "," +
                                                              xMax.ToString() + " " + yMin.ToString() + "," +
                                                              xMin.ToString() + " " + yMin.ToString() + "," +
                                                              xMin.ToString() + " " + yMax.ToString() + "," +
                                                              xMax.ToString() + " " + yMax.ToString() +
                                                              "))", 4326);
            entityBase.Date = Convert.ToDateTime(m_extractedData.GetDate());
            entityBase.Metadata_Id = metadata.Id;
            entityBase.Thumbnail_Id = thumbnail.Id;

            context.SpatialEntityBases.Add(entityBase);

            // Spatial Data Source.
            SpatialDataSource source = new SpatialDataSource();
            source.MainURL = m_extractedData.GetUrl();
            source.CompoundType = m_extractedData.GetCompoundType();
            source.FileSize = Convert.ToInt64(m_extractedData.GetSize());
            source.DataSourceType = m_extractedData.GetDataType();
            source.LocationInCompound = m_extractedData.GetLocationInCompound();
            source.Server_Id = ftpServer.Id;

            context.SpatialDataSources.Add(source);

            //Spatial Entities.
            SpatialEntity entity = new SpatialEntity();
            entity.Id = entityBase.Id;
            entity.IdStr = entityBase.IdStr;
            entity.SpatialEntityBas = entityBase;
            List<SpatialDataSource> spatialDataSources = new List<SpatialDataSource>();
            spatialDataSources.Add(source);
            entity.SpatialDataSources = spatialDataSources;
            
            context.SpatialEntities.Add(entity);

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

            Console.WriteLine("Status: Saved " + m_extractedData.GetName() + " to database.");
            }

        //! Update database.
        public void Update()
            {
            using (RealityDB context = new RealityDB())
                {
                // Update data only if it already exists.
                string currentUrl = m_extractedData.GetUrl();
                SpatialDataSource source = context.SpatialDataSources.FirstOrDefault(SpatialDataSource => SpatialDataSource.MainURL.Equals(currentUrl));
                if (null == source)
                    return;

                Update(context, source);
                }            
            }

        //! Update database.
        public void Update(RealityDB context, SpatialDataSource source)
            {
            // Spatial Data Source.
            source.MainURL = m_extractedData.GetUrl();
            source.CompoundType = m_extractedData.GetCompoundType();
            source.FileSize = Convert.ToInt64(m_extractedData.GetSize());
            source.DataSourceType = m_extractedData.GetDataType();
            source.LocationInCompound = m_extractedData.GetLocationInCompound();

            // Server.
            Server server = context.Servers.FirstOrDefault(Server => Server.Id == source.Server_Id);
            if (null != server)
                {
                server.CommunicationProtocol = m_extractedData.GetServer().GetProtocol();
                server.Name = m_extractedData.GetServer().GetName();
                server.URL = m_extractedData.GetServer().GetUrl();
                server.ServerContactInformation = m_extractedData.GetServer().GetContactInfo();
                server.Legal = m_extractedData.GetServer().GetLegal();
                server.Online = m_extractedData.GetServer().IsOnline();
                server.LastCheck = Convert.ToDateTime(m_extractedData.GetServer().GetLastCheck());
                server.LastTimeOnline = Convert.ToDateTime(m_extractedData.GetServer().GetLastTimeOnline());
                server.Latency = m_extractedData.GetServer().GetLatency();
                server.State = ((int)ServerState.New).ToString();
                server.Type = m_extractedData.GetServer().GetServerType();
                }

            // Retrieve spatialEntityBase from spatialDataSource.
            SpatialEntityBas entityBase = null;
            SpatialEntity entity = context.SpatialEntities.FirstOrDefault(SpatialEntity => SpatialEntity.SpatialDataSources.Select(SpatialDataSource => SpatialDataSource.Id).Contains(source.Id));            
            if (null != entity)
                entityBase = entity.SpatialEntityBas;

            // Spatial Entity Base.
            if(null != entityBase)
                {
                entityBase.Name = m_extractedData.GetName();
                entityBase.ResolutionInMeters = m_extractedData.GetResolution();
                entityBase.DataProvider = m_extractedData.GetProvider();
                entityBase.DataProviderName = m_extractedData.GetProvider();
                List<double> footprint = m_extractedData.GetFootprint();
                double xMin = double.MaxValue;
                double xMax = double.MinValue;
                double yMin = double.MaxValue;
                double yMax = double.MinValue;
                for (int i = 0; i < footprint.Count; ++i)
                    {
                    if (0 == i % 2)
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
                entityBase.Footprint = DbGeometry.PolygonFromText("POLYGON((" +
                                                                    xMax.ToString() + " " + yMax.ToString() + "," +
                                                                    xMax.ToString() + " " + yMin.ToString() + "," +
                                                                    xMin.ToString() + " " + yMin.ToString() + "," +
                                                                    xMin.ToString() + " " + yMax.ToString() + "," +
                                                                    xMax.ToString() + " " + yMax.ToString() +
                                                                    "))", 4326);
                entityBase.Date = Convert.ToDateTime(m_extractedData.GetDate());
                }

            // Retrieve metadata and thumbnail from spatialEntityBase.                
            Metadata metadata = null;
            Thumbnail thumbnail = null;
            if(null != entityBase)
                {
                metadata = context.Metadatas.FirstOrDefault(Metadata => Metadata.Id == entityBase.Metadata_Id);
                thumbnail = context.Thumbnails.FirstOrDefault(Thumbnail => Thumbnail.Id == entityBase.Thumbnail_Id);
                }

            // Metadata.
            if (null != metadata)
                {
                metadata.Provenance = m_extractedData.GetMetadata().GetProvenance();
                metadata.Description = m_extractedData.GetMetadata().GetDescription();
                metadata.ContactInformation = m_extractedData.GetMetadata().GetContactInfo();
                metadata.Legal = m_extractedData.GetMetadata().GetLegal();
                metadata.RawMetadataFormat = m_extractedData.GetMetadata().GetFormat();
                // &&JFC Patch: rawMetadata is too huge to save, keep this field empty for now.
                // metadata.RawMetadata = m_extractedData.GetMetadata().GetData();
                metadata.RawMetadata = "";
                }            

            // Thumbnail.
            if(null != thumbnail)
                {
                thumbnail.ThumbnailProvenance = m_extractedData.GetThumbnail().GetProvenance();
                thumbnail.ThumbnailFormat = m_extractedData.GetThumbnail().GetFormat();
                thumbnail.ThumbnailWidth = m_extractedData.GetThumbnail().GetWidth().ToString();
                thumbnail.ThumbnailHeight = m_extractedData.GetThumbnail().GetHeight().ToString();
                thumbnail.ThumbnailStamp = m_extractedData.GetThumbnail().GetStamp();
                thumbnail.ThumbnailGenerationDetails = m_extractedData.GetThumbnail().GetGenerationDetails();

                List<byte> dataList = m_extractedData.GetThumbnail().GetData();
                byte[] dataArray = new byte[dataList.Count];
                for (int i = 0; i < dataList.Count; ++i)
                    {
                    dataArray[i] = dataList[i];
                    }
                thumbnail.ThumbnailData = dataArray;
                }       
                        
            // Update.
            try
                {
                context.SaveChanges();
                }
            catch (System.Exception ex)
                {
                Console.WriteLine("Status: Exception occurred, " + ex.Message);
                return;
                }

            Console.WriteLine("Status: Updated " + m_extractedData.GetName() + " in database.");
            }

        //! Private members
        private FtpDataWrapper m_extractedData;
        }


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    public class FtpTraversalObserver : RealityServicesCli.IFtpTraversalObserverWrapper
        {
        public bool OnFileListed_AddToQueue(string file)
            {
            // File is null, don't add.
            if (null == file)
                {
                Console.WriteLine("Status: Failed, file is null.");
                return false;
                }

            if (FtpTraversalEngine.UpdateMode())
                {
                // Look for duplicates. If file don't already exists, there is nothing to update, don't add.
                if (!IsDuplicate(file))
                    {
                    Console.WriteLine("Status: Skipped " + file);
                    return false;
                    }
                }
            else
                {
                // Look for duplicates. If file already exists, don't add.
                if (IsDuplicate(file))
                    {
                    Console.WriteLine("Status: Skipped " + file);
                    return false;
                    }
                }           
            
            Console.WriteLine("Status: Added " + file + " to queue.");
            return true;
            }

        public void OnFileDownloaded(string file)
            {
            if (null == file)
                return;

            // Show status in console.
            Console.WriteLine("Status: Downloaded " + file);
            }

        public void OnDataExtracted(RealityServicesCli.FtpDataWrapper extractedData)
            {
            if (null == extractedData)
                return;

            FtpData data = new FtpData(extractedData);
            if(FtpTraversalEngine.UpdateMode())
                {
                // Update entry in database.                
                data.Update();
                }            
            else
                {
                // Save entry to database.                
                data.Save();
                }            
            }

        private bool IsDuplicate(string file)
            {
            // Look for duplicates. Compare full url.
            using (RealityDB context = new RealityDB())
                {
                SpatialDataSource source = context.SpatialDataSources.FirstOrDefault(SpatialDataSource => SpatialDataSource.MainURL.Contains(file));
                return (null != source);
                }
            }

        private bool IsMirror(string file)
            {
            // Get filename.
            string filename = file;
            int pos = file.LastIndexOf("/");
            if (-1 != pos)
                filename = file.Substring(pos + 1);
        
            // Look for duplicates. Compare filename. 
            // &&JFC WIP: Compare size too.
            using (RealityDB context = new RealityDB())
                {
                SpatialDataSource source = context.SpatialDataSources.FirstOrDefault(SpatialDataSource => SpatialDataSource.MainURL.Contains(file));
                return (null != source);
                }
            }
        }


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    class FtpTraversalEngine
        {
        static void ShowUsage()
            {
            Console.WriteLine("Usage: ftptraversalenginenet.exe [options] FtpUrl [ftpargs] [DualFtpUrl] [ftpargs]");
            Console.WriteLine();
            Console.WriteLine("Options:");
            Console.WriteLine("  -h, --help             Show this help message and exit");
            Console.WriteLine("  -u, --update           Enable update mode");
            Console.WriteLine();
            Console.WriteLine("FtpArgs:");
            Console.WriteLine("  -provider:PROVIDER     Set provider name");
            }

        static void Main(string[] args)
            {
            Console.Title = "FTP Traversal Engine";           

            // Validate parameters.
            int ftpUrlCount = args.Count(arg => arg.Contains("ftp://"));
            if (1 > ftpUrlCount || 2 < ftpUrlCount)
                {
                ShowUsage();

                // Terminate.
                Console.ReadKey();
                return;
                }

            // Get parameters.
            m_dualMode = (2 == ftpUrlCount);
            List<string> ftpUrls = new List<string>(ftpUrlCount);
            List<string> ftpProviders = new List<string>(ftpUrlCount);            
            for (int i = 0; i < args.Count(); ++i)
                {
                // Show usage.
                if (args[i].Equals("-h") || args[i].Equals("--help"))
                    {
                    ShowUsage();

                    // Terminate.
                    Console.ReadKey();
                    return;
                    }
                // Update mode.
                else if (args[i].Equals("-u") || args[i].Equals("--update"))
                    {
                    m_updateMode = true;
                    }
                // Ftp url.
                else if (args[i].Contains("ftp://"))
                    {
                    ftpUrls.Add(args[i]);

                    // Get ftp parameters, if there is any.
                    if((i+1) < args.Count())
                        {
                        // Provider.
                        if (args[i+1].Contains("-provider:"))
                            ftpProviders.Add(args[i+1].Substring(args[i+1].IndexOf(":") + 1));
                        else
                            ftpProviders.Add(null);
                        }
                    else
                        ftpProviders.Add(null);
                    }                
                }

            // FTP traversal.
            FtpStatusWrapper status = FtpStatusWrapper.UnknownError;
            FtpClientWrapper client = null;
            for (int i = 0; i < ftpUrls.Count(); ++i)
                {                
                try
                    {
                    // Connect to FTP.
                    Console.WriteLine();
                    Console.WriteLine("*****************");
                    Console.WriteLine("Connecting to FTP");
                    Console.WriteLine("*****************");
                    Console.WriteLine();

                    client = FtpClientWrapper.ConnectTo(ftpUrls[i], ftpProviders[i]);
                    if (null == client)
                        {
                        Console.WriteLine("Status: Could not connect to " + ftpUrls[i]);
                        continue;
                        }
                    Console.WriteLine("Status: Connected to " + ftpUrls[i]);

                    // Retrieve data.
                    Console.WriteLine();
                    Console.WriteLine("***************");
                    Console.WriteLine("Retrieving data");
                    Console.WriteLine("***************");
                    Console.WriteLine();

                    client.SetObserver(new FtpTraversalObserver());
                    status = client.GetData();
                    if (FtpStatusWrapper.Success != status)
                        {
                        Console.WriteLine("Status: Failed, " + status);
                        continue;
                        }
                    }
                catch (System.Exception ex)
                    {
                    Console.WriteLine("Status: Exception occurred, " + ex.Message);
                    continue;
                    }            
                }

            // Terminate.
            Console.WriteLine("Press any key to exit.");
            Console.ReadKey();
            }

        //! Get/Set parameters.
        public static bool UpdateMode() { return m_updateMode; }
        public static bool DualMode() { return m_dualMode; }

        // Members.
        private static bool m_updateMode = false;
        private static bool m_dualMode = false;
        }
    }
