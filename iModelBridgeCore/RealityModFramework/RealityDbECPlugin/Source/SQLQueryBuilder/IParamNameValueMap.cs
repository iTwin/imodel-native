using System;
using System.Collections;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;

namespace IndexECPlugin.Source
    {
    /// <summary>
    /// Interface for parameterized query helper. Its purpose is to set the parameters values.
    /// </summary>
    public interface IParamNameValueMap
        {

        /// <summary>
        /// Adds a parameter with its value and type in the map
        /// </summary>
        /// <param name="paramName">The parameter's name</param>
        /// <param name="paramValue">The parameter's value</param>
        /// <param name="paramType">The parameter's type</param>
        void AddParamNameValue (string paramName, object paramValue, IECType paramType);

        }

    /// <summary>
    /// GenericParamNameValueMap sets the parameters values to a generic SQL command using DbTypes data types.
    /// </summary>
    public class GenericParamNameValueMap : Dictionary<string, Tuple<object, DbType>>, IParamNameValueMap
        {

        /// <summary>
        /// Constructor for GenericParamNameValueMap
        /// </summary>
        public GenericParamNameValueMap()
            : base()
            {
            //m_paramNameValueMap = new Dictionary<string, Tuple<string, DbType>>();
            }

        /// <summary>
        /// Adds a parameter with its value and type in the map
        /// </summary>
        /// <param name="paramName">The parameter's name</param>
        /// <param name="paramValue">The parameter's value</param>
        /// <param name="paramType">The parameter's type</param>
        public void AddParamNameValue (string paramName, object paramValue, DbType paramType)
            {
            base.Add(paramName, new Tuple<object, DbType>(paramValue, paramType));
            }

        /// <summary>
        /// Adds a parameter with its value and type in the map
        /// </summary>
        /// <param name="paramName">The parameter's name</param>
        /// <param name="paramValue">The parameter's value</param>
        /// <param name="paramType">The parameter's type</param>
        public void AddParamNameValue (string paramName, object paramValue, IECType paramType)
            {
            base.Add(paramName, new Tuple<object, DbType>(paramValue, ECToSQLMap.ECTypeToDbType(paramType)));
            }
        }

    /// <summary>
    /// SqlServerParamNameValueMap sets the parameters values to a generic SQL command using SqlDbType data types.
    /// </summary>
    public class SqlServerParamNameValueMap : Dictionary<string, Tuple<object, SqlDbType>>, IParamNameValueMap
        {

        /// <summary>
        /// Constructor for SqlServerParamNameValueMap
        /// </summary>
        public SqlServerParamNameValueMap()
            :base()
            {
            }

        /// <summary>
        /// Adds a parameter with its value and type in the map
        /// </summary>
        /// <param name="paramName">The parameter's name</param>
        /// <param name="paramValue">The parameter's value</param>
        /// <param name="paramType">The parameter's type</param>
        public void AddParamNameValue (string paramName, object paramValue, SqlDbType paramType)
            {
            base.Add(paramName, new Tuple<object, SqlDbType>(paramValue, paramType));
            }

        /// <summary>
        /// Adds a parameter with its value and type in the map
        /// </summary>
        /// <param name="paramName">The parameter's name</param>
        /// <param name="paramValue">The parameter's value</param>
        /// <param name="paramType">The parameter's type</param>
        public void AddParamNameValue (string paramName, object paramValue, IECType paramType)
            {
            base.Add(paramName, new Tuple<object, SqlDbType>(paramValue, ECToSQLMap.ECTypeToSqlDbType(paramType)));
            }

        }

    }
