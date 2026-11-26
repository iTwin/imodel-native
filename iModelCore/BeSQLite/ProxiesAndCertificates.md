# Proxies and certificates

CloudSqlite downloads iModel and workspace data from cloud storage with libcurl. `BeSQLiteCurlHandleConfig.cpp` sets the system proxy (see [README](../../README.md#system-proxies)) and the TLS options below on each curl handle.

## Transparent proxies

Some corporate networks route traffic through a transparent (man-in-the-middle) proxy that re-signs TLS traffic with its own root certificate. IT marks that certificate as trusted on users' machines. Our code works in these networks only if it trusts the system certificate store.

A briefcase download makes network calls from two places: itwinjs-core fetches metadata through Node networking, and CloudSqlite downloads the data through libcurl. Both have to trust the proxy's certificate.

### libcurl TLS backend per platform

| Platform | TLS backend | System certificate store |
|---|---|---|
| Windows | Schannel | used by default |
| macOS, iOS | OpenSSL with Apple SecTrust | used, through `CURLSSLOPT_NATIVE_CA` |
| Linux, Android | OpenSSL | `CURLSSLOPT_NATIVE_CA` is not set; Linux has not been reviewed in detail |

`iModelCore/libsrc/curl/BeCurl.mke` selects the backends. The curl version string names the TLS backend, which is the first thing to check when debugging a certificate problem.

### Unreachable revocation endpoints

By default libcurl with Schannel refuses to connect when the CRL or OCSP endpoint of a certificate is unreachable, which transparent proxies often cause. We set `CURLSSLOPT_REVOKE_BEST_EFFORT` on every platform: a certificate that is reported as revoked still fails, but an unreachable revocation endpoint does not block the connection.

### Node and Electron

- Browsers, including Chromium in Electron frontends, use the system certificate store.
- Node 23 and later trust the system store when started with `--use-system-ca`.
- Older Node versions need a polyfill that adds the system certificates to requests, for example [`@vscode/proxy-agent`](https://github.com/microsoft/vscode-proxy-agent).
- `electron.net.fetch` uses the system store in most cases, but some calls still need the polyfill.

## Testing

### Through a transparent proxy

[mitmproxy](https://mitmproxy.org/) generates its own root certificate. [Install and trust it](https://docs.mitmproxy.org/stable/concepts/certificates/), then proxy all traffic:

```sh
mitmproxy --mode=local
```

An HTTPS site opened in a browser should now show a certificate issued by mitmproxy.

### With unreachable CRL and OCSP endpoints

Pick a host whose certificate lists `CRL Distribution Points` or `Authority Information Access` URLs, and block those hosts in `/etc/hosts` (`C:\Windows\System32\drivers\etc\hosts` on Windows):

```text
127.0.0.1  e6.c.lencr.org
```

The connection should still succeed.

## Debugging TLS on Windows

Show the issuer of a server certificate in Windows PowerShell 5.x (the snippet does not work reliably in PowerShell 7). Behind a transparent proxy the issuer is the proxy's certificate:

```powershell
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12; $req = [Net.HttpWebRequest]::Create("https://www.itwinjs.org"); try { $req.GetResponse().Close() } catch { $_.Exception.InnerException }; $req.ServicePoint.Certificate | Format-List Subject, Issuer
```

The CAPI2 event log shows every step of certificate validation, including CRL checks. Enable it and clear the certificate caches:

```sh
wevtutil sl Microsoft-Windows-CAPI2/Operational /e:true
certutil -urlcache * delete
certutil -flushCache 0 0x01
certutil -setreg chain\ChainCacheResyncFiletime @now
```

Then read the log in Event Viewer under Applications and Services Logs > Microsoft > Windows > CAPI2 > Operational.
