#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

# This file is intended for stuff that needs a bit more than the stuff in util; it can include middle-level files.
import os, re, threading
from . import azurecli, cmdutil, compat, globalvars, utils

s_httpAuth = os.environ.get('BB_NUGET_AUTH', None) # may specify Authorization header (e.g., for jfrog)
s_winhttpLock = None

#---------------------------------------------------------------------------------------
# bsimethod
#---------------------------------------------------------------------------------------
def ChooseAuthenticationHeader (serverToUse, serverName, serverAddress):
    header = globalvars.buildStrategy.GetAuthenticationToken(serverName) if globalvars.buildStrategy else None
    if header:
        serverToUse.m_provider = globalvars.CREDENTIAL_PROVIDER_TOKEN
        return header
    
    header = azurecli.GetAuthenticationHeader()
    if header:
        serverToUse.m_provider = globalvars.CREDENTIAL_PROVIDER_AZ
        return header
     
    # Shouldn't get here. We assume people have Azure CLI now.
    if not utils.isSubnetPrg() and not 'AVOID_NUGET_MSCP' in os.environ:
        header = globalvars.buildStrategy.GetMicrosoftProviderToken(serverAddress.replace ('feeds', 'nuget'))
        if header:
            serverToUse.m_provider = globalvars.CREDENTIAL_PROVIDER_AUTO
            return header

    utils.showInfoMsg ("Unable to determine credential provider automatically.\n", utils.INFO_LEVEL_Interesting)
    return None   

#-------------------------------------------------------------------------------------------
#-------------------------------------------------------------------------------------------
s_hgHttpProxy = [False, None]  # Store this since currently we call GetHgHttpProxy a lot.
s_hgHttpProxyEnv = None  # Don't want to look through env with every URL call

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetHgHttpProxy():
    if s_hgHttpProxy[0]:
        return s_hgHttpProxy[1]
    
    s_hgHttpProxy[0] = True
    
    httpProxyList = []
    def ParseHgConfigOutput (outputLine):
        outputLine = outputLine.strip()
        if len(outputLine) > 0:
            httpProxyList.append(outputLine)
    hgCmd = ["hg", "config", "http_proxy.host"]
    localDir = os.path.dirname(os.path.realpath(__file__))
    cmdutil.runAndWait (hgCmd, localDir, ParseHgConfigOutput)
    if len(httpProxyList) > 0:
        httpMatch = re.match('^[hH][tT][tT][pP]([sS])*://.*', httpProxyList[0])
        if httpMatch:
            s_hgHttpProxy[1] = httpProxyList[0]
        else:
            s_hgHttpProxy[1] = 'http://{0}'.format(httpProxyList[0])
        
    return s_hgHttpProxy[1]

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def GetHttpProxyDict(url):
    global s_hgHttpProxyEnv
    # If we've already decided there is no proxy, fastest just to say so first.
    if s_hgHttpProxyEnv == {}:
        return None

    # If it's https, no proxy
    if url.startswith('https'):
        return None

    # If it's http, we get the proxy information from Mercurial
    if s_hgHttpProxyEnv == None:
        s_hgHttpProxyEnv = {}
        httpProxy = GetHgHttpProxy()
        if httpProxy:
            s_hgHttpProxyEnv = {'http': httpProxy}
            utils.showInfoMsg ("Choosing proxy {0}\n".format (httpProxy), utils.INFO_LEVEL_RarelyUseful)
    return s_hgHttpProxyEnv

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def CheckForHttps(address):
    proxy = GetHgHttpProxy()
    if proxy:
        #                     1         2
        urlMatch = re.match( '(https)://(.*)$', address) 
        if urlMatch:
            address = 'http://{}'.format(urlMatch.group(2))
            utils.showInfoMsg (address, utils.INFO_LEVEL_SomewhatInteresting)
                
    return address

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def urllib2Get(url, httpAuth = None):
    # s_httpAuth may specify Authorization header in BB_NUGET_AUTH (e.g., for jfrog)
    if s_httpAuth:
        return compat.getUrlWithData(url, None, {}, s_httpAuth, proxyDict=GetHttpProxyDict(url))
    if httpAuth:
        return compat.getUrlWithData(url, None, {}, httpAuth, proxyDict=GetHttpProxyDict(url))
    return compat.getUrl(url, proxyDict=GetHttpProxyDict(url))

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def httpGet(url, httpAuth = None):
    global s_winhttpLock
    numRetries = 3
    while numRetries > 0:
        numRetries = numRetries - 1
        try:
            httpCode = 200 # assume success
            httpContent = None
            if os.name == 'nt' and not (s_httpAuth or httpAuth):
                import pythoncom
                pythoncom.CoInitialize() # pylint: disable=no-member
                import win32com.client
                if not s_winhttpLock:
                    s_winhttpLock = threading.RLock()
                with s_winhttpLock:
                    winhttp = win32com.client.Dispatch('WinHTTP.WinHTTPRequest.5.1')
                    winhttp.SetAutoLogonPolicy(0)
                    winhttp.Open('GET', url, False)
                    winhttp.Send()
                    httpCode = winhttp.status
                    utils.showInfoMsg ('Status {0} returned from pulling URL {1}\n'.format (httpCode, url), utils.INFO_LEVEL_RarelyUseful)
                    if 200 == httpCode:
                        if compat.py3:
                            httpContent = winhttp.responseBody.tobytes()
                        else:
                            httpContent = winhttp.responseBody
                        break
                    elif 401 == httpCode:
                        httpCode, httpContent = urllib2Get(url, httpAuth)
                        if httpCode == 200:
                            break
            else: # unix?
                httpCode, httpContent = urllib2Get(url, httpAuth)
                if httpCode == 200:
                    break
        except Exception as ex:
            utils.showInfoMsg ('Exception when pulling URL {0}\n  {1}\n'.format (url, ex), utils.INFO_LEVEL_RarelyUseful)
            if numRetries <= 0:
                raise ex
    return httpCode, httpContent

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def IsRemoteAddress(address):
    return address.startswith("http")

