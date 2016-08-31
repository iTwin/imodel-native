/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/HttpTraversalEngineNet/HttpTraversalEngineNet.cs $
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

using HttpTraversalEngineNet;
using RealityServicesCli;


namespace HttpTraversalEngineNet
    {
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    class HttpData
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
        public HttpData()
            {
            m_extractedData = null;
            }

        //! Constructor
        public HttpData(HttpDataWrapper extractedData)
            {
            m_extractedData = extractedData;
            }

        //! Destructor
        ~HttpData()
            {}

        //! Get / Set
        public HttpDataWrapper Get() { return m_extractedData; }
        public void Set(HttpDataWrapper data) { m_extractedData = data; }

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

            // Spatial Entities.
            // Dual Mode:
            // Check if an entity representing the same source already exists.
            // If an entity already exists, add this alternate source. Else, create a new entity with only one source.
            SpatialEntityBas existingEntityBase = context.SpatialEntityBases.FirstOrDefault(SpatialEntityBas => SpatialEntityBas.Name.Equals(entityBase.Name));
            if(HttpTraversalEngine.DualMode() && null != existingEntityBase)
                {
                // Retrieve correspond entity.
                SpatialEntity existingEntity = context.SpatialEntities.FirstOrDefault(SpatialEntity => SpatialEntity.Id == existingEntityBase.Id);
                if (null != existingEntity)
                    existingEntity.SpatialDataSources.Add(source);
                }
            else
                {
                SpatialEntity entity = new SpatialEntity();
                entity.Id = entityBase.Id;
                entity.IdStr = entityBase.IdStr;
                entity.SpatialEntityBas = entityBase;
                List<SpatialDataSource> spatialDataSources = new List<SpatialDataSource>();
                spatialDataSources.Add(source);
                entity.SpatialDataSources = spatialDataSources;

                context.SpatialEntities.Add(entity);
                }           

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
        private HttpDataWrapper m_extractedData;
        }


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              4/2016
    //=====================================================================================
    public class HttpTraversalObserver : RealityServicesCli.IHttpTraversalObserverWrapper
        {
        public bool OnFileListed_AddToQueue(string file)
            {
            // File is null, don't add.
            if (null == file)
                {
                Console.WriteLine("Status: Failed, file is null.");
                return false;
                }

            if (HttpTraversalEngine.UpdateMode())
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

        public void OnDataExtracted(RealityServicesCli.HttpDataWrapper extractedData)
            {
            if (null == extractedData)
                return;

            HttpData data = new HttpData(extractedData);
            if(HttpTraversalEngine.UpdateMode())
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
    class HttpTraversalEngine
        {
        static void ShowUsage()
            {
            Console.WriteLine("Usage: httptraversalenginenet.exe [options] HttpUrl [DualHttpUrl]");
            Console.WriteLine();
            Console.WriteLine("Options:");
            Console.WriteLine("  -h, --help             Show this help message and exit");
            Console.WriteLine("  -u, --update           Enable update mode");
            Console.WriteLine("  -provider:PROVIDER     Set provider name");
            }

        static void Main(string[] args)
            {
            Console.Title = "HTTP Traversal Engine";           

            // Validate parameters.
            int httpUrlCount = args.Count(arg => arg.Contains("http://") || arg.Contains("https://"));
            if (1 > httpUrlCount || 2 < httpUrlCount || 5 < args.Length)
                {
                ShowUsage();

                // Terminate.
                Console.WriteLine();
                Console.WriteLine("Press any key to exit.");
                Console.ReadKey();
                return;
                }

            // Get parameters.
            m_dualMode = (2 == httpUrlCount);
            string provider = null;
            List<string> httpUrls = new List<string>(httpUrlCount);            
            for (int i = 0; i < args.Count(); ++i)
                {
                // Show usage.
                if (args[i].Equals("-h") || args[i].Equals("--help"))
                    {
                    ShowUsage();

                    // Terminate.
                    Console.WriteLine();
                    Console.WriteLine("Press any key to exit.");
                    Console.ReadKey();
                    return;
                    }
                // Update mode.
                else if (args[i].Equals("-u") || args[i].Equals("--update"))
                    m_updateMode = true;
                // Provider.
                else if (args[i].Contains("-provider:"))
                    provider = args[i].Substring(args[i].IndexOf(":") + 1); // For now, all data are considered to be from the same http provider.
                // Http url.
                else if (args[i].Contains("http://") || args[i].Contains("https://"))
                    httpUrls.Add(args[i]);            
                }

            // HTTP traversal.
            HttpStatusWrapper status = HttpStatusWrapper.UnknownError;
            HttpClientWrapper client = null;
            for (int i = 0; i < httpUrls.Count(); ++i)
                {                
                try
                    {
                    // Connect to HTTP.
                    Console.WriteLine();
                    Console.WriteLine("******************");
                    Console.WriteLine("Connecting to HTTP");
                    Console.WriteLine("******************");
                    Console.WriteLine();

                    client = HttpClientWrapper.ConnectTo(httpUrls[i], provider);
                    if (null == client)
                        {
                        Console.WriteLine("Status: Could not connect to " + httpUrls[i]);
                        continue;
                        }
                    Console.WriteLine("Status: Connected to " + httpUrls[i]);

                    // Retrieve data.
                    Console.WriteLine();
                    Console.WriteLine("***************");
                    Console.WriteLine("Retrieving data");
                    Console.WriteLine("***************");
                    Console.WriteLine();

                    client.SetObserver(new HttpTraversalObserver());
                    status = client.GetData();
                    if (HttpStatusWrapper.Success != status)
                        {
                        Console.WriteLine("Status: Failed, " + status);
                        continue;
                        }

                    client.Dispose();
                    }
                catch (System.Exception ex)
                    {
                    Console.WriteLine("Status: Exception occurred, " + ex.Message);
                    continue;
                    }            
                }

            // Terminate.
            Console.WriteLine();
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
