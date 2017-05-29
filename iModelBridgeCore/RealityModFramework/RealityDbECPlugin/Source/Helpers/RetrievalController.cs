using System;
using System.Data;
using System.IO;
using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Operations;
using Bentley.ECObjects.Instance;

namespace IndexECPlugin.Source.Helpers
    {
    internal static class RetrievalController
        {
        public static void RetrievePackage (IECInstance instance, string connectionString, IDbConnectionCreator dbConnectionCreator)
            {
            RetrieveFile(instance, connectionString, dbConnectionCreator, "Package", "SELECT FileContent FROM dbo.Packages WHERE Name = @param0");
            }

        public static void RetrieveDownloadReport (IECInstance instance, string connectionString, IDbConnectionCreator dbConnectionCreator)
            {
            RetrieveFile(instance, connectionString, dbConnectionCreator, "Download Report",
                "SELECT ReportContent FROM dbo.DownloadReports WHERE Id = @param0");
            }

        private static void RetrieveFile (IECInstance instance, string connectionString, IDbConnectionCreator dbConnectionCreator,
                                         string fileType, string dbCommandText)
            {
            Log.Logger.trace("Retrieving " + fileType + " " + instance.InstanceId);
            using ( IDbConnection dbConnection = dbConnectionCreator.CreateDbConnection(connectionString) )
                {
                dbConnection.Open();
                using ( IDbCommand dbCommand = dbConnection.CreateCommand() )
                    {
                    dbCommand.CommandText = dbCommandText;

                    IDbDataParameter param0 = dbCommand.CreateParameter();
                    param0.DbType = DbType.String;
                    param0.ParameterName = "@param0";
                    param0.Value = instance.InstanceId;
                    dbCommand.Parameters.Add(param0);

                    // We read the file from the database and write it on disk
                    using ( IDataReader reader = dbCommand.ExecuteReader(CommandBehavior.SequentialAccess) )
                        {
                        //We read only the first row, since the instance is supposed to be unique
                        if ( reader.Read() == false )
                            {
                            Log.Logger.error(fileType + " retrieval aborted. There is no " + fileType + " named " + instance.InstanceId);
                            throw new InstanceDoesNotExistException("There is no such " + fileType + ".");
                            }

                        MemoryStream mStream = new MemoryStream();

                        int bufferSize = 4096;
                        byte[] outByte = new byte[bufferSize];
                        long startIndex = 0;

                        long retval = reader.GetBytes(0, startIndex, outByte, 0, bufferSize);

                        while ( retval == bufferSize )
                            {
                            mStream.Write(outByte, 0, bufferSize);
                            startIndex += bufferSize;
                            retval = reader.GetBytes(0, startIndex, outByte, 0, bufferSize);
                            }

                        mStream.Write(outByte, 0, (int) retval);
                        StreamBackedDescriptor desc = new StreamBackedDescriptor(mStream, instance.InstanceId, mStream.Length, DateTime.UtcNow);
                        StreamBackedDescriptorAccessor.SetIn(instance, desc);
                        }
                    }
                dbConnection.Close();
                }
            }
        }
    }
