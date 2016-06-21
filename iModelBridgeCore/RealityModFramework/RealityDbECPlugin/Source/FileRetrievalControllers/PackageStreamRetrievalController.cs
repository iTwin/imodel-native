/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/FileRetrievalControllers/PackageStreamRetrievalController.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using Bentley.EC.Persistence;
using Bentley.EC.Persistence.Operations;
using Bentley.ECObjects.Instance;
using Bentley.Exceptions;
using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.FileRetrievalControllers
    {
    internal static class PackageStreamRetrievalController
        {
        //IECInstance m_instance;
        //string m_connectionString;

        //public PackageStreamRetrievalController(IECInstance instance,
        //                                        string connectionString)
        //{
        //    m_instance = instance;
        //    m_connectionString = connectionString;
        //}

        public static void SetStreamRetrieval (IECInstance instance,
                                       string connectionString)
            {
            Log.Logger.trace("Retrieving package " + instance.InstanceId);
            using ( DbConnection dbConnection = new SqlConnection(connectionString) )
                {
                dbConnection.Open();
                using ( DbCommand dbCommand = dbConnection.CreateCommand() )
                    {
                    dbCommand.CommandText = "SELECT FileContent FROM dbo.Packages WHERE Name = @param0";

                    DbParameter param0 = dbCommand.CreateParameter();
                    param0.DbType = DbType.String;
                    param0.ParameterName = "@param0";
                    param0.Value = instance.InstanceId;
                    dbCommand.Parameters.Add(param0);

                    // We read the file from the database and write it on disk
                    using ( DbDataReader reader = dbCommand.ExecuteReader(CommandBehavior.SequentialAccess) )
                        {
                        //We read only the first row, since the instance is supposed to be unique
                        if ( reader.Read() == false )
                            {
                            Log.Logger.error("Package retrieval aborted. There is no package named " + instance.InstanceId);
                            throw new InstanceDoesNotExistException("There is no package named " + instance.InstanceId);
                            }

                        MemoryStream mStream = new MemoryStream();

                        //using (BinaryWriter writer = new BinaryWriter(mStream, Encoding., true))
                        //{
                        int bufferSize = 4096;
                        byte[] outByte = new byte[bufferSize];
                        long startIndex = 0;

                        long retval = reader.GetBytes(0, startIndex, outByte, 0, bufferSize);

                        while ( retval == bufferSize )
                            {
                            //    writer.Write(outByte);
                            //    writer.Flush();

                            mStream.Write(outByte, 0, bufferSize);
                            startIndex += bufferSize;
                            retval = reader.GetBytes(0, startIndex, outByte, 0, bufferSize);
                            }
                        //writer.Write(outByte, 0, (int)retval - 1);
                        //writer.Flush();
                        mStream.Write(outByte, 0, (int) retval - 1);
                        StreamBackedDescriptor desc = new StreamBackedDescriptor(mStream, instance.InstanceId, mStream.Length, DateTime.UtcNow);
                        StreamBackedDescriptorAccessor.SetIn(instance, desc);

                        //}


                        }
                    }
                dbConnection.Close();
                }
            }
        }
    }
