using Bentley.EC.Persistence;
using Bentley.EC.Persistence.FileSystemResource;
using Bentley.EC.Persistence.Operations;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.FileRetrievalControllers
{
    internal class PackageRetrievalController : FileResourceRetrievalController
    {
        string m_packagesLocation;
        string m_connectionString;

        public PackageRetrievalController
            (
            IECInstance instance,
            FileResourceManager manager,
            RetrieveBackingFileOperation operation,
            string packagesLocation,
            string connectionString
            )
            : base(instance, manager, operation)
        {
            m_packagesLocation = packagesLocation;
            m_connectionString = connectionString;
        }

        public override void DoRetrieveFile(bool transferFile)
        {
            Log.Logger.trace("Retrieving package " + Instance.InstanceId);
            string location = Path.Combine(m_packagesLocation + Instance.InstanceId);
            DateTime creationTime;
            using (DbConnection dbConnection = new SqlConnection(m_connectionString))
            {
                using (DbCommand dbCommand = dbConnection.CreateCommand())
                {
                    dbCommand.CommandText = "SELECT CreationTime, File FROM dbo.Packages WHERE Name = @param0";

                    DbParameter param0 = dbCommand.CreateParameter();
                    param0.DbType = DbType.String;
                    param0.ParameterName = "@param0";
                    param0.Value = Instance.InstanceId;
                    dbCommand.Parameters.Add(param0);

                    // We read the file from the database and write it on disk
                    using (DbDataReader reader = dbCommand.ExecuteReader(CommandBehavior.SequentialAccess))
                    {
                        //We read only the first row, since the instance is supposed to be unique
                        if (reader.Read() == false)
                        {
                            Log.Logger.error("There is no package named " + Instance.InstanceId);
                            throw new InstanceDoesNotExistException("There is no package named " + Instance.InstanceId);
                        }

                        creationTime = reader.GetDateTime(0);

                        using (FileStream fStream = new FileStream(location, FileMode.Create, FileAccess.Write))
                        {
                            using (BinaryWriter writer = new BinaryWriter(fStream))
                            {
                                int bufferSize = 4096;
                                byte[] outByte = new byte[bufferSize];
                                long startIndex = 0;

                                long retval = reader.GetBytes(1, startIndex, outByte, 0, bufferSize);

                                while (retval == bufferSize)
                                {
                                    writer.Write(outByte);
                                    writer.Flush();
                                    startIndex += bufferSize;
                                    retval = reader.GetBytes(1, startIndex, outByte, 0, bufferSize);
                                }
                                writer.Write(outByte, 0, (int)retval - 1);
                                writer.Flush();

                            }
                        }

                    }
                }
            }

            //FileInfo fileInfo = new FileInfo(location);
            //if (!fileInfo.Exists)
            //{
            //    Log.Logger.error("There is no package named " + Instance.InstanceId);
            //    throw new UserFriendlyException("There is no such file associated to this instance");
            //}

            //DateTime WriteTime = fileInfo.LastWriteTimeUtc;

            var fileBackedDescriptor = new FileBackedDescriptor(ResourceManager.Connection.RepositoryIdentifier.ECPluginID, location, creationTime);

            FileBackedDescriptorAccessor.SetIn(Instance, fileBackedDescriptor);
            ResourceManager.SetAsSynchronizedWithRepository(Instance, ResourceManager.Connection.RepositoryIdentifier.ECPluginID, fileBackedDescriptor.RelativePath, creationTime, transferFile);

        }

        public override bool InstanceRequiresLockForLocalFileModifications
        {
            get
            {
                return false;
            }
        }

        public override bool ObtainingLockDuringRetrieval
        {
            get
            {
                return false;
            }
        }
    }
}

