#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os, json, sys, threading, time
# This needs to be the lowest-level import; do not import any parts of bb here.

py3 = (int(sys.version[0]) > 2)

perfCounter = time.perf_counter if py3 else time.clock # pylint: disable=no-member

if py3:
    import configparser
    from urllib.parse import urlparse, unquote, quote
    from urllib.request import urlopen, URLopener, Request, ProxyHandler, build_opener
    from urllib.error import HTTPError, URLError
else:
    import thread
    import ConfigParser as configparser
    from future.moves.urllib.parse import urlparse, unquote, quote
    from future.moves.urllib.request import urlopen, URLopener, Request, ProxyHandler, build_opener
    from future.moves.urllib.error import HTTPError, URLError

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getThreadId ():
    if py3:
        return threading.get_ident()    # pylint: disable=no-member
    else:
        return thread.get_ident()       # pylint: disable=no-member

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getCurrentRunningThreadCount (semaphore):
    if py3:
        return semaphore._value             # pylint: disable=protected-access
    else:
        return semaphore._Semaphore__value  # pylint: disable=protected-access,no-member

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def openLog (logFileName, options):
    # raw_input renamed to input in py3.
    if py3:
        return open (logFileName, options, encoding='utf-8')
    else:
        return open (logFileName, options)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getInput (inputStr):
    # raw_input renamed to input in py3.
    if py3:
        return input(inputStr)
    else:
        return raw_input(inputStr) # pylint: disable=undefined-variable

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUnicode (inputStr):
    if py3:
        return str(inputStr)
    else:
        return unicode(inputStr) # pylint: disable=undefined-variable

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def isUnicode (inputStr):
    if py3:
        return isinstance (inputStr, str)
    else:
        return isinstance (inputStr, unicode) # pylint: disable=undefined-variable

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getAscii (inputStr):
    if py3:
        if isinstance(inputStr, str): # pylint: disable=undefined-variable
            return inputStr.encode('ascii', 'ignore')
    else:
        return inputStr

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getStringForEnv (inputStr):
    if not py3:
        if isinstance(inputStr, unicode): # pylint: disable=undefined-variable
            inputStr = inputStr.encode('ascii', 'ignore')

    return inputStr

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getStringForHeader (inputStr):
    if py3:
        if not isinstance(inputStr, str): # pylint: disable=undefined-variable
            return inputStr.decode()
    else:
        return inputStr

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getEncodableString (inputStr):
    if py3:
        return inputStr.encode('ascii', 'ignore').decode()
    else:
        return inputStr

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def assertItemIsString (inputStr):
    if py3:
        assert isinstance(inputStr, str)
    else:
        assert isinstance(inputStr, unicode) # pylint: disable=undefined-variable

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUrlWithHeaders (url, headers, proxyDict=None):
    httpCode = 200 # assume success
    httpContent = None
    try:
        httpRequest = Request(url, headers=headers)
        if proxyDict:
            proxy_support = ProxyHandler(proxyDict)
            opener = build_opener(proxy_support)
            httpResponse = opener.open(httpRequest)
        else:
            httpResponse = urlopen(httpRequest)
    except HTTPError as e:
        httpCode = e.code
    if 200 == httpCode:
        httpContent = httpResponse.read()
    return httpCode, httpContent

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUrl (url, proxyDict=None):
    return getUrlWithHeaders (url, {}, proxyDict=proxyDict)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUrlWithJson (url, jsonData, proxyDict=None):
    httpCode = 200 # assume success
    jsonContent = None

    try:
        httpRequest = Request(url)
        httpRequest.add_header("Content-Type", "application/json")
        jsonStr = json.dumps(jsonData)
        if py3:
            jsonStr = jsonStr.encode('UTF-8')
        if proxyDict:
            proxy_support = ProxyHandler(proxyDict)
            opener = build_opener(proxy_support)
            httpResponse = opener.open(httpRequest, jsonStr)
        else:
            httpResponse = urlopen(httpRequest, jsonStr)
    except HTTPError as e:
        httpCode = e.code
    if 200 == httpCode:
        jsonContent = json.load(httpResponse)
    return httpCode, jsonContent

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUrlWithData (url, data, statusheaders, unredirectedAuthHeader = None, proxyDict=None, showInfoMsg=None, LogLevel=None):
    httpCode = 200 # assume success
    httpContent = None
    startTime = perfCounter()

    try:
        httpRequest = Request(url, data, statusheaders)
        if unredirectedAuthHeader:
            httpRequest.add_unredirected_header("Authorization", unredirectedAuthHeader)
        if proxyDict:
            proxy_support = ProxyHandler(proxyDict)
            opener = build_opener(proxy_support)
            httpResponse = opener.open(httpRequest)
        else:
            httpResponse = urlopen(httpRequest)
        httpContent = httpResponse.read()
    except HTTPError as e:
        httpCode = e.code
    except URLError:
        httpCode = 503

    if (showInfoMsg and LogLevel):
        showInfoMsg ("getUrlWithData '{0}' took {1:0.2f} seconds\n".format(url, perfCounter() - startTime), LogLevel)

    return httpCode, httpContent

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getUrlFile (url, fileName, authHeader = None):
    opener = URLopener()
    if authHeader:
        opener.addheader("Authorization", authHeader)

    dirName = os.path.dirname (fileName)
    if not os.path.exists (dirName):
        os.makedirs (dirName)

    httpCode = 200 # assume success
    try:
        opener.retrieve(url, fileName)
    except HTTPError as e:
        httpCode = e.code
    return httpCode


#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def parseUrl (remoteUrl):
    return urlparse (remoteUrl)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def unquoteUrl (remoteUrl):
    return unquote (remoteUrl)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def quoteUrl (remoteUrl):
    return quote (remoteUrl)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getConfigParser ():
    return configparser.ConfigParser ()

#--------------------------------------------------------------------------------
# @bsimethod
#--------------------------------------------------------------------------------
def getRegistryEntry (regPath, regEntry, force32bitRegistry=False, force64bitRegistry=False):
    if py3:
        import winreg as reg
    else:
        import _winreg as reg
    entry = None
    regtype = None
    msg = None

    assert not (force32bitRegistry and force64bitRegistry)

    accessMask = reg.KEY_READ
    if force32bitRegistry:
        accessMask |= reg.KEY_WOW64_32KEY
    elif force64bitRegistry:
        accessMask |= reg.KEY_WOW64_64KEY

    try:
        root_key = reg.OpenKey (reg.HKEY_LOCAL_MACHINE, regPath, 0, accessMask)
        entry, regtype = reg.QueryValueEx (root_key, regEntry)
    except:
        # no such value? ignore it
        msg = "Warning: failed to read from registry: {0}\\@{1}\n".format(regPath, regEntry)

    return (entry, regtype, msg)

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def getRegistrySubKeys (regPath, force32bitRegistry=False, force64bitRegistry=False):
    if py3:
        import winreg as reg
    else:
        import _winreg as reg

    assert not (force32bitRegistry and force64bitRegistry)

    accessMask = reg.KEY_READ
    if force32bitRegistry:
        accessMask |= reg.KEY_WOW64_32KEY
    elif force64bitRegistry:
        accessMask |= reg.KEY_WOW64_64KEY

    i = 0
    try:
        root_key = reg.OpenKey (reg.HKEY_LOCAL_MACHINE, regPath, 0, accessMask)
        while True:
            subkey = reg.EnumKey(root_key, i)
            yield subkey
            i+=1
    except WindowsError:
        pass

#-------------------------------------------------------------------------------------------
# bsimethod
#-------------------------------------------------------------------------------------------
def readJsonFile (filepath):
    if py3:
        with open(filepath, 'rt', encoding='utf-8') as licFile:
            return json.load(licFile)
    else:
        with open(filepath, 'rt') as licFile:
            return json.loads(licFile.read().decode('utf-8'))
