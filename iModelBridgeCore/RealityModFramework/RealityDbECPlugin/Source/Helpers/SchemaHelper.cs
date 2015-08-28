using Bentley.EC.PluginBuilder.Modules;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Repository;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace IndexECPlugin.Source.Helpers
{
    internal class SchemaHelper
    {
        private SchemaModule m_schemaModule;
        private RepositoryConnection m_connection;
        private string m_schemaName;

        public SchemaHelper(SchemaModule schemaModule, RepositoryConnection connection, string schemaName)
        {
            m_schemaModule = schemaModule;
            m_connection = connection;
            m_schemaName = schemaName;
        }

        public IECClass GetClass(string className)
        {
            return m_schemaModule.FindECClass(m_connection, m_schemaName, className);
        }
    }
}
