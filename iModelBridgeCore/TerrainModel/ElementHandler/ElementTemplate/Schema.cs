using System;
using System.Collections.Generic;
using System.Text;
using Bentley.DgnPlatformNET.DgnEC;
using Bentley.DgnPlatformNET.XDataTree;
using Bentley.ECObjects.Schema;
using BECO = Bentley.ECObjects;

namespace Bentley.TerrainModel.ElementTemplate
    {
    /// <summary>
    /// DTMElementTemplateExtender partial class - deals with all schema related issues
    /// </summary>
    /// <author>James.Goode</author>
    /// <date>5/6/2008</date>
    public partial class DTMElementTemplateExtender
        {

        #region Private Fields

        /// <summary>
        /// Schema
        /// </summary>
        private BECO.Schema.IECSchema m_schema = null;

        private DisplayCalculatedProperties m_displayCalculatedProperties = null;

        private DisplaySourceProperties m_displaySourceProperties = null;

        private CategoryContainer m_calculatedCategoryContainer = null;

        private SourceFeatureProperties m_sourceFeatureProperties = null;

        /// <summary>
        /// Collection of categories
        /// </summary>
        private System.Collections.Hashtable m_categories = new System.Collections.Hashtable();
        
        #endregion

        #region Constants

        /// <summary>
        /// Schema name and Class constants
        /// </summary>
        private const string SCHEMA_NAME = "DTMElement_TemplateExtender_Schema";
        private const string CLASS_NAME = "DTMElement_TemplateExtender_Class";

        #endregion

        #region Private methods

        /// <summary>
        /// Create the schema for the new entry
        /// </summary>
        private void CreateSchema()
            {
            // Create new template extender schema
            m_displayCalculatedProperties = new DisplayCalculatedProperties();
            m_calculatedCategoryContainer = new CategoryContainer("CalculatedFeatureSettings");
            m_displaySourceProperties = new DisplaySourceProperties();
            m_sourceFeatureProperties = new SourceFeatureProperties();

            m_calculatedCategoryContainer.AddSubItem(Constants.CONTOURS, new ContourProperties());
            m_calculatedCategoryContainer.AddSubItem(Constants.TRIANGLES, new TriangleProperties());
            m_calculatedCategoryContainer.AddSubItem(Constants.TRIANGLEVERTICES, new TriangleVerticeProperties());
            m_calculatedCategoryContainer.AddSubItem(Constants.FLOWARROWS, new FlowArrowProperties());
            m_calculatedCategoryContainer.AddSubItem(Constants.LOWPOINTS, new LowPointProperties());
            m_calculatedCategoryContainer.AddSubItem(Constants.HIGHPOINTS, new HighPointProperties());
            }

        #endregion
        }
    }
