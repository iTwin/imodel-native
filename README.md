# imodel-native

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/iTwin/imodel-native)

This is the public portion of the iTwin native add-on.

## System Proxies

The imodel-native backend detects the system proxy configuration on Windows, macOS, and iOS, and it automatically uses this when doing SQLite downloads from remote servers. This includes support for Proxy Auto-Config (PAC) scripts. None of this is supported on the Linux and Android backends.
