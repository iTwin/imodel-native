using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.Connect.Client.API.V1;

namespace UsageDataExtractor
    {
    public class ConnectAPI
        {
        /// <summary> The ConnectClientAPI instance to use in functions </summary>
        private ConnectClientAPI m_connectApi;
        private string m_relyingParty;

        public ConnectAPI (string relyingParty)
            {
            m_connectApi = new ConnectClientAPI();
            m_relyingParty = relyingParty;
            try
                {
                if ( !m_connectApi.IsRunning() )
                    {
                    m_connectApi.StartClientApp(true); // will throw if client app cannot be started
                    }
                }
            catch ( Exception ex )
                {
                Console.WriteLine(ex.Message);
                throw;
                }
            }

        public string GetTokenIfConnected()
            {
            try
                {
                if ( !m_connectApi.IsRunning() )
                    {
                    throw new Exception("Connection Client is not running.");
                    }

                if ( !m_connectApi.IsUserSessionActive() )
                    {
                    throw new Exception("User session is not active.");
                    }

                return m_connectApi.GetSerializedDelegateSecurityToken(m_relyingParty);
                }
            catch ( Exception ex )
                {
                Console.WriteLine(ex.Message);
                throw;
                }
            }
        }
    }
