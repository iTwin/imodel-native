using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    internal static class DownloadReportHelper
        {
        public static void InsertInDatabase (Stream stream, string packageName, string connectionString, IDbConnectionCreator dbConnectionCreator)
            {
            using ( IDbConnection sqlConnection = dbConnectionCreator.CreateDbConnection(connectionString) )
                {
                sqlConnection.Open();
                using ( IDbCommand dbCommand = sqlConnection.CreateCommand() )
                    {
                    dbCommand.CommandText = "IF 0 = (SELECT COUNT(*) FROM dbo.DownloadReports WHERE Id = @param0) INSERT INTO dbo.DownloadReports (Id, CreationTime, ReportContent) VALUES (@param0, @param1, @param2)";
                    dbCommand.CommandType = CommandType.Text;

                    IDbDataParameter param0 = dbCommand.CreateParameter();
                    param0.DbType = DbType.String;
                    param0.ParameterName = "@param0";
                    param0.Value = packageName;
                    dbCommand.Parameters.Add(param0);

                    IDbDataParameter param1 = dbCommand.CreateParameter();
                    param1.DbType = DbType.DateTime;
                    param1.ParameterName = "@param1";
                    param1.Value = DateTime.UtcNow;
                    dbCommand.Parameters.Add(param1);

                    long longLength = stream.Length;
                    int intLength;
                    if ( longLength > int.MaxValue )
                        {
                        //Log.Logger.error("Package requested is too large.");
                        throw new Bentley.Exceptions.UserFriendlyException("DownloadReports is too large. Please reduce the size of the order.");
                        }
                    intLength = Convert.ToInt32(longLength);
                    byte[] fileBytes = new byte[stream.Length];
                    if ( stream.CanSeek )
                        {
                        stream.Seek(0, SeekOrigin.Begin);
                        }
                    stream.Read(fileBytes, 0, intLength);

                    IDbDataParameter param2 = dbCommand.CreateParameter();
                    param2.DbType = DbType.Binary;
                    param2.ParameterName = "@param2";
                    param2.Value = fileBytes;
                    dbCommand.Parameters.Add(param2);

                    try
                        {
                        dbCommand.ExecuteNonQuery();
                        }
                    catch ( SqlException e )
                        {
                        if ( e.Number == 547 )
                            {
                            throw new Bentley.Exceptions.UserFriendlyException("Please use the Id of an existing package.");
                            }
                        else
                            {
                            throw;
                            }
                        }
                    }
                sqlConnection.Close();
                }
            return;
            }
        }
    }
