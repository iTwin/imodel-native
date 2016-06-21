/*-------------------------------------------------------------------------------------
|
|     $Source: RealityDbECPlugin/Source/Helpers/DataReadingHelper.cs $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;

namespace IndexECPlugin.Source.Helpers
    {
    /// <summary>
    /// Helper used to indicate the location of the data in the DataReader. The class keeps track of the order and arguments 
    /// the AddColumn method was called with. This class is low level and must be used correctly by the SQLQueryBuilder classes.
    /// These classes must make sure the data will be output as indicated by the helper
    /// </summary>
    public class DataReadingHelper
        {
        private int m_currentIndex = 0;

        private int? m_streamDataColumn = null;
        private Dictionary<IECProperty, int> instanceDataColumnList = new Dictionary<IECProperty, int>();
        private int? m_relatedInstanceIdColumn = null;

        private Dictionary<string, int> nonInstanceDataColumnList = new Dictionary<string, int>();

        /// <summary>
        /// Gets the collection of all properties requested. This includes instanceData and SpatialInstanceData.
        /// </summary>
        /// <returns>Collection of all properties</returns>
        public IEnumerable<IECProperty> GetProperties()
            {
            return instanceDataColumnList.Keys;
            }

        /// <summary>
        /// This method must be called to add a new "property related" column to the sql query.
        /// </summary>
        /// <param name="columnCategory">The category of the column. This will decide which rows will be accessible by which Get methods</param>
        /// <param name="property">The property associated with the data. Only used for instanceData and spatialInstanceData categories. Otherwise, can be null</param>
        /// <param name="numberOfColumn">The effective number of column the data requested will be using. Most of the time, 1 is the adequate value to use, but it is imperative to give the good value.
        ///   As an example, for SQL Server, the spatial query is returned in two columns, one for the WKT, and one for the SRID</param>
        public void AddColumn (ColumnCategory columnCategory, IECProperty property, int numberOfColumn = 1)
            {
            if ( (columnCategory == ColumnCategory.instanceData) || (columnCategory == ColumnCategory.spatialInstanceData) )
                {
                instanceDataColumnList.Add(property, m_currentIndex);
                }
            else if ( columnCategory == ColumnCategory.streamData )
                {
                if ( m_streamDataColumn == null )
                    {
                    m_streamDataColumn = m_currentIndex;
                    }
                else
                    {
                    throw new ProgrammerException("This method should be called only once for the streamData category.");
                    }
                }
            else if ( columnCategory == ColumnCategory.relatedInstanceId )
                {
                if ( m_relatedInstanceIdColumn == null )
                    {
                    m_relatedInstanceIdColumn = m_currentIndex;
                    }
                else
                    {
                    throw new ProgrammerException("This method should be called only once for the relatedInstanceId category.");
                    }
                }
            m_currentIndex += numberOfColumn;
            }

        /// <summary>
        /// This method must be called to add a new non-"property related" column to the sql query.
        /// </summary>
        /// <param name="columnName">The name of the column in the database</param>
        public void AddNonPropertyDataColumn(string columnName)
            {
            nonInstanceDataColumnList.Add(columnName, m_currentIndex);
            m_currentIndex++;
            }

        /// <summary>
        /// Returns the index of the stream data column, if such data was requested.
        /// </summary>
        /// <returns>The index of the stream data column, if such data was requested. If not, null is returned</returns>
        public int? getStreamDataColumn ()
            {
            return m_streamDataColumn;
            }

        /// <summary>
        /// Returns the index of the data linked to a property requested in the ECQuery.
        /// </summary>
        /// <param name="property">The requested property</param>
        /// <returns>The index of the data in the DataReader</returns>
        public int? getInstanceDataColumn (IECProperty property)
            {
            if ( instanceDataColumnList.ContainsKey(property) )
                {
                return instanceDataColumnList[property];
                }
            return null;
            }

        /// <summary>
        /// Returns the index of the related Instance Id column.
        /// </summary>
        /// <returns>The index of the related Instance Id column, if it was requested. If not, null is returned. </returns>
        public int? getRelatedInstanceIdColumn ()
            {
            return m_relatedInstanceIdColumn;
            }

        /// <summary>
        /// Returns the index of a column that is not linked to an ECProperty
        /// </summary>
        /// <param name="columnName">The name of the column</param>
        /// <returns>The index of the related column, if it was requested. If not, null is returned.</returns>
        public int? getNonPropertyDataColumn(string columnName)
            {
            return nonInstanceDataColumnList[columnName];
            }
        }
    }
