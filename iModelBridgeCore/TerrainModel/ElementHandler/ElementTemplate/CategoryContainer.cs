using System;
using System.Collections.Generic;
using System.Text;

namespace Bentley.TerrainModel.ElementTemplate
{
    public class CategoryContainer
    {
        public const string CATEGORY_CALCULATEDFEATURES = "DTMElementExtender_FeatureSettings";

        private string m_categoryName;
        private Bentley.ECObjects.Instance.IECInstance m_category = null;
        private System.Collections.Generic.IDictionary<string, ElementTemplateExtenderPropertyProvider> m_items;
        
        public CategoryContainer(string categoryName)
        {
            m_categoryName = categoryName;
            m_items = new Dictionary<string, ElementTemplateExtenderPropertyProvider>();
        }

        public void CreateCategory(System.Collections.Hashtable categories)
        {
            // Create categories
            m_category = Bentley.ECObjects.UI.ECPropertyPane.CreateCategory(
                CATEGORY_CALCULATEDFEATURES,
                StringLocalizer.Instance.GetLocalizedString(m_categoryName),
                StringLocalizer.Instance.GetLocalizedString(m_categoryName),
                Bentley.ECObjects.UI.ECPropertyPane.CategorySortPriorityHigh - 200);
            categories.Add(CATEGORY_CALCULATEDFEATURES, m_category);
        }

        public void AddSubItem (string name, ElementTemplateExtenderPropertyProvider item)
        {
            m_items.Add(name, item);
        }

        public void Activate(Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties)
        {
        foreach (KeyValuePair<string, ElementTemplateExtenderPropertyProvider> keyValuePair in m_items)
            {
            keyValuePair.Value.IsActivated(instance, properties);
            }
        }

        public void Activate(Bentley.ECObjects.Instance.IECInstance instance, List<Bentley.ECObjects.Instance.IECPropertyValue> properties, string name)
            {
            ElementTemplateExtenderPropertyProvider elementTemplateExtenderPropertyProvider;
            m_items.TryGetValue(name, out elementTemplateExtenderPropertyProvider);
            if(elementTemplateExtenderPropertyProvider != null)
                elementTemplateExtenderPropertyProvider.IsActivated(instance, properties);
            }

        public void ApplyProperties (Bentley.TerrainModelNET.Element.DTMElement elem, Bentley.ECObjects.Instance.IECInstance templateInstance)
            {
            foreach (KeyValuePair<string, ElementTemplateExtenderPropertyProvider> keyValuePair in m_items)
                {
                ICanApplyProperties applyProperties = keyValuePair.Value as ICanApplyProperties;
                if (applyProperties != null)
                    applyProperties.ApplyProperties (elem, templateInstance);
                }
            }
    }
}
