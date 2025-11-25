function FindProxyForURL(url, host) {
  if (isPlainHostName(host) || host == "127.0.0.1") {
    return "DIRECT";
  }
  // Explicit port numbers are needed to prevent default to 80.
  // On Windows, WinHTTP does not like whitespace after semi-colons.
  if (host == "simple.com") {
    return "PROXY no-such-proxy:80";
  } else if (host == "multi.com") {
    // Return proxy list with Firefox proxy types.
    return "HTTPS some-such-proxy:443;HTTPS any-such-proxy:41";
  } else if (host == "multi-legacy.com") {
    // Older implementations don't support HTTP/HTTPS/SOCKS proxy types.
    return "PROXY some-such-proxy:443;PROXY any-such-proxy:41";
  } else if (host == "multi-legacy-direct.com") {
    // Connect using direct connection if no proxy is available.
    return "PROXY some-such-proxy:443;PROXY any-such-proxy:41;DIRECT";
  }
  return "DIRECT";
}
