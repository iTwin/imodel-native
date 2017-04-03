using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Interface for IDbConnectionCreator classes, used for generating IDbConnection objects
    /// </summary>
    public interface IDbConnectionCreator
        {
        /// <summary>
        /// Returns a IDbConnection object constructed with the given connectionString
        /// </summary>
        /// <param name="connectionString">The connection string</param>
        /// <returns>The IDbConnection object</returns>
        IDbConnection CreateDbConnection (string connectionString);
        }

    /// <summary>
    /// Class used for generating SqlConnection objects
    /// </summary>
    public class SqlDbConnectionCreator : IDbConnectionCreator
        {
        /// <summary>
        /// Returns a SqlConnection object constructed with the given connectionString
        /// </summary>
        /// <param name="connectionString">The connection string</param>
        /// <returns>The SqlConnection object</returns>
        public IDbConnection CreateDbConnection (string connectionString)
            {
            return new SqlConnection(connectionString);
            }
        }
    }
