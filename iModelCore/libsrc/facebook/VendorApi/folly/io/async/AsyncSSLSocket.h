/*
 * Copyright 2016 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <iomanip>

#include <folly/Optional.h>
#include <folly/String.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/AsyncTimeout.h>
#include <folly/io/async/SSLContext.h>
#include <folly/io/async/TimeoutManager.h>
#include <folly/io/async/ssl/OpenSSLPtrTypes.h>
#include <folly/io/async/ssl/OpenSSLUtils.h>
#include <folly/io/async/ssl/SSLErrors.h>
#include <folly/io/async/ssl/TLSDefinitions.h>

#include <folly/Bits.h>
#include <folly/io/IOBuf.h>
#include <folly/io/Cursor.h>
#include <folly/portability/Sockets.h>

namespace folly {

/**
 * A class for performing asynchronous I/O on an SSL connection.
 *
 * AsyncSSLSocket allows users to asynchronously wait for data on an
 * SSL connection, and to asynchronously send data.
 *
 * The APIs for reading and writing are intentionally asymmetric.
 * Waiting for data to read is a persistent API: a callback is
 * installed, and is notified whenever new data is available.  It
 * continues to be notified of new events until it is uninstalled.
 *
 * AsyncSSLSocket does not provide read timeout functionality,
 * because it typically cannot determine when the timeout should be
 * active.  Generally, a timeout should only be enabled when
 * processing is blocked waiting on data from the remote endpoint.
 * For server connections, the timeout should not be active if the
 * server is currently processing one or more outstanding requests for
 * this connection.  For client connections, the timeout should not be
 * active if there are no requests pending on the connection.
 * Additionally, if a client has multiple pending requests, it will
 * ususally want a separate timeout for each request, rather than a
 * single read timeout.
 *
 * The write API is fairly intuitive: a user can request to send a
 * block of data, and a callback will be informed once the entire
 * block has been transferred to the kernel, or on error.
 * AsyncSSLSocket does provide a send timeout, since most callers
 * want to give up if the remote end stops responding and no further
 * progress can be made sending the data.
 */
class AsyncSSLSocket : public virtual AsyncSocket {
 public:
  typedef std::unique_ptr<AsyncSSLSocket, Destructor> UniquePtr;
  using X509_deleter = folly::static_function_deleter<X509, &X509_free>;

  class HandshakeCB {
   public:
    virtual ~HandshakeCB() = default;

    /**
     * handshakeVer() is invoked during handshaking to give the
     * application chance to validate it's peer's certificate.
     *
     * Note that OpenSSL performs only rudimentary internal
     * consistency verification checks by itself. Any other validation
     * like whether or not the certificate was issued by a trusted CA.
     * The default implementation of this callback mimics what what
     * OpenSSL does internally if SSL_VERIFY_PEER is set with no
     * verification callback.
     *
     * See the passages on verify_callback in SSL_CTX_set_verify(3)
     * for more details.
     */
    virtual bool handshakeVer(AsyncSSLSocket* /*sock*/,
                                 bool preverifyOk,
                                 X509_STORE_CTX* /*ctx*/) noexcept {
      return preverifyOk;
    }

    /**
     * handshakeSuc() is called when a new SSL connection is
     * established, i.e., after SSL_accept/connect() returns successfully.
     *
     * The HandshakeCB will be uninstalled before handshakeSuc()
     * is called.
     *
     * @param sock  SSL socket on which the handshake was initiated
     */
    virtual void handshakeSuc(AsyncSSLSocket *sock) noexcept = 0;

    /**
     * handshakeErr() is called if an error occurs while
     * establishing the SSL connection.
     *
     * The HandshakeCB will be uninstalled before handshakeErr()
     * is called.
     *
     * @param sock  SSL socket on which the handshake was initiated
     * @param ex  An exception representing the error.
     */
    virtual void handshakeErr(
      AsyncSSLSocket *sock,
      const AsyncSocketException& ex)
      noexcept = 0;
  };

  class HandshakeTimeout : public AsyncTimeout {
   public:
    HandshakeTimeout(AsyncSSLSocket* sslSocket, EventBase* eventBase)
      : AsyncTimeout(eventBase)
      , sslSocket_(sslSocket) {}

    virtual void timeoutExpired() noexcept {
      sslSocket_->timeoutExpired();
    }

   private:
    AsyncSSLSocket* sslSocket_;
  };

  /**
   * Create a client AsyncSSLSocket
   */
  AsyncSSLSocket(const std::shared_ptr<folly::SSLContext> &ctx,
                 EventBase* evb, bool deferSecurityNegotiation = false);

  /**
   * Create a server/client AsyncSSLSocket from an already connected
   * socket file descriptor.
   *
   * Note that while AsyncSSLSocket enables TCP_NODELAY for sockets it creates
   * when connecting, it does not change the socket options when given an
   * existing file descriptor.  If callers want TCP_NODELAY enabled when using
   * this version of the constructor, they need to explicitly call
   * setNoDelay(true) after the constructor returns.
   *
   * @param ctx             SSL context for this connection.
   * @param evb EventBase that will manage this socket.
   * @param fd  File descriptor to take over (should be a connected socket).
   * @param server Is socket in server mode?
   * @param deferSecurityNegotiation
   *          unencrypted data can be sent before sslConn/Accept
   */
  AsyncSSLSocket(const std::shared_ptr<folly::SSLContext>& ctx,
                 EventBase* evb, int fd,
                 bool server = true, bool deferSecurityNegotiation = false);


  /**
   * Helper function to create a server/client shared_ptr<AsyncSSLSocket>.
   */
  static std::shared_ptr<AsyncSSLSocket> newSocket(
    const std::shared_ptr<folly::SSLContext>& ctx,
    EventBase* evb, int fd, bool server=true,
    bool deferSecurityNegotiation = false) {
    return std::shared_ptr<AsyncSSLSocket>(
      new AsyncSSLSocket(ctx, evb, fd, server, deferSecurityNegotiation),
      Destructor());
  }

  /**
   * Helper function to create a client shared_ptr<AsyncSSLSocket>.
   */
  static std::shared_ptr<AsyncSSLSocket> newSocket(
    const std::shared_ptr<folly::SSLContext>& ctx,
    EventBase* evb, bool deferSecurityNegotiation = false) {
    return std::shared_ptr<AsyncSSLSocket>(
      new AsyncSSLSocket(ctx, evb, deferSecurityNegotiation),
      Destructor());
  }


#if OPENSSL_VERSION_NUMBER >= 0x1000105fL && !defined(OPENSSL_NO_TLSEXT)
  /**
   * Create a client AsyncSSLSocket with tlsext_servername in
   * the Client Hello message.
   */
  AsyncSSLSocket(const std::shared_ptr<folly::SSLContext> &ctx,
                  EventBase* evb,
                 const std::string& serverName,
                bool deferSecurityNegotiation = false);

  /**
   * Create a client AsyncSSLSocket from an already connected
   * socket file descriptor.
   *
   * Note that while AsyncSSLSocket enables TCP_NODELAY for sockets it creates
   * when connecting, it does not change the socket options when given an
   * existing file descriptor.  If callers want TCP_NODELAY enabled when using
   * this version of the constructor, they need to explicitly call
   * setNoDelay(true) after the constructor returns.
   *
   * @param ctx  SSL context for this connection.
   * @param evb  EventBase that will manage this socket.
   * @param fd   File descriptor to take over (should be a connected socket).
   * @param serverName tlsext_hostname that will be sent in ClientHello.
   */
  AsyncSSLSocket(const std::shared_ptr<folly::SSLContext>& ctx,
                  EventBase* evb,
                  int fd,
                 const std::string& serverName,
                bool deferSecurityNegotiation = false);

  static std::shared_ptr<AsyncSSLSocket> newSocket(
    const std::shared_ptr<folly::SSLContext>& ctx,
    EventBase* evb,
    const std::string& serverName,
    bool deferSecurityNegotiation = false) {
    return std::shared_ptr<AsyncSSLSocket>(
      new AsyncSSLSocket(ctx, evb, serverName, deferSecurityNegotiation),
      Destructor());
  }
#endif

  /**
   * TODO: implement support for SSL renegotiation.
   *
   * This involves proper handling of the SSL_ERROR_WANT_READ/WRITE
   * code as a result of SSL_write/read(), instead of returning an
   * error. In that case, the READ/WRITE event should be registered,
   * and a flag (e.g., writeBlockedOnRead) should be set to indiciate
   * the condition. In the next invocation of read/write callback, if
   * the flag is on, performWrite()/performRead() should be called in
   * addition to the normal call to performRead()/performWrite(), and
   * the flag should be reset.
   */

  // Inherit TAsyncTransport methods from AsyncSocket except the
  // following.
  // See the documentation in TAsyncTransport.h
  // TODO: implement graceful shutdown in close()
  // TODO: implement detachSSL() that returns the SSL connection
  virtual void closeNow() override;
  virtual void shutdownWrite() override;
  virtual void shutdownWriteNow() override;
  virtual bool good() const override;
  virtual bool connecting() const override;
  virtual std::string getApplicationProtocol() noexcept override;

  virtual std::string getSecurityProtocol() const override { return "TLS"; }

  bool isEorTrackingEnabled() const override;
  virtual void setEorTracking(bool track) override;
  virtual size_t getRawBytesWritten() const override;
  virtual size_t getRawBytesReceived() const override;
  void enableClientHelloParsing();

  /**
   * Accept an SSL connection on the socket.
   *
   * The callback will be invoked and uninstalled when an SSL
   * connection has been established on the underlying socket.
   * The value of verifyPeer determines the client verification method.
   * By default, its set to use the value in the underlying context
   *
   * @param callback callback object to invoke on success/failure
   * @param timeout timeout for this function in milliseconds, or 0 for no
   *                timeout
   * @param verifyPeer  SSLVerifyPeerEnum uses the options specified in the
   *                context by default, can be set explcitly to override the
   *                method in the context
   */
  virtual void sslAccept(HandshakeCB* callback, uint32_t timeout = 0,
      const folly::SSLContext::SSLVerifyPeerEnum& verifyPeer =
            folly::SSLContext::SSLVerifyPeerEnum::USE_CTX);

  /**
   * Invoke SSL accept following an asynchronous session cache lookup
   */
  void restartSSLAccept();

  /**
   * Connect to the given address, invoking callback when complete or on error
   *
   * Note timeout applies to TCP + SSL connection time
   */
  void connect(ConnectCallback* callback,
               const folly::SocketAddress& address,
               int timeout = 0,
               const OptionMap &options = emptyOptionMap,
               const folly::SocketAddress& bindAddr = anyAddress())
               noexcept override;

  using AsyncSocket::connect;

  /**
   * Initiate an SSL connection on the socket
   * The callback will be invoked and uninstalled when an SSL connection
   * has been establshed on the underlying socket.
   * The verification option verifyPeer is applied if it's passed explicitly.
   * If it's not, the options in SSLContext set on the underlying SSLContext
   * are applied.
   *
   * @param callback callback object to invoke on success/failure
   * @param timeout timeout for this function in milliseconds, or 0 for no
   *                timeout
   * @param verifyPeer  SSLVerifyPeerEnum uses the options specified in the
   *                context by default, can be set explcitly to override the
   *                method in the context. If verification is turned on sets
   *                SSL_VERIFY_PEER and invokes
   *                HandshakeCB::handshakeVer().
   */
  virtual void sslConn(HandshakeCB *callback, uint64_t timeout = 0,
            const folly::SSLContext::SSLVerifyPeerEnum& verifyPeer =
                  folly::SSLContext::SSLVerifyPeerEnum::USE_CTX);

  enum SSLStateEnum {
    STATE_UNINIT,
    STATE_UNENCRYPTED,
    STATE_ACCEPTING,
    STATE_CACHE_LOOKUP,
    STATE_ASYNC_PENDING,
    STATE_CONNECTING,
    STATE_ESTABLISHED,
    STATE_REMOTE_CLOSED, /// remote end closed; we can still write
    STATE_CLOSING,       ///< close() called, but waiting on writes to complete
    /// close() called with pending writes, before connect() has completed
    STATE_CONNECTING_CLOSING,
    STATE_CLOSED,
    STATE_ERROR
  };

  SSLStateEnum getSSLState() const { return sslState_;}

  /**
   * Get a handle to the negotiated SSL session.  This increments the session
   * refcount and must be deallocated by the caller.
   */
  SSL_SESSION *getSSLSession();

  /**
   * Get a handle to the SSL struct.
   */
  const SSL* getSSL() const;

  /**
   * Set the SSL session to be used during sslConn.  AsyncSSLSocket will
   * hold a reference to the session until it is destroyed or released by the
   * underlying SSL structure.
   *
   * @param takeOwnership if true, AsyncSSLSocket will assume the caller's
   *                      reference count to session.
   */
  void setSSLSession(SSL_SESSION *session, bool takeOwnership = false);

  /**
   * Get the name of the protocol selected by the client during
   * Next Protocol Negotiation (NPN) or Application Layer Protocol Negotiation
   * (ALPN)
   *
   * Throw an exception if openssl does not support NPN
   *
   * @param protoName      Name of the protocol (not guaranteed to be
   *                       null terminated); will be set to nullptr if
   *                       the client did not negotiate a protocol.
   *                       Note: the AsyncSSLSocket retains ownership
   *                       of this string.
   * @param protoNameLen   Length of the name.
   * @param protoType      Whether this was an NPN or ALPN negotiation
   */
  virtual void getSelectedNextProtocol(
      const unsigned char** protoName,
      unsigned* protoLen,
      SSLContext::NextProtocolType* protoType = nullptr) const;

  /**
   * Get the name of the protocol selected by the client during
   * Next Protocol Negotiation (NPN) or Application Layer Protocol Negotiation
   * (ALPN)
   *
   * @param protoName      Name of the protocol (not guaranteed to be
   *                       null terminated); will be set to nullptr if
   *                       the client did not negotiate a protocol.
   *                       Note: the AsyncSSLSocket retains ownership
   *                       of this string.
   * @param protoNameLen   Length of the name.
   * @param protoType      Whether this was an NPN or ALPN negotiation
   * @return false if openssl does not support NPN
   */
  virtual bool getSelectedNextProtocolNoThrow(
      const unsigned char** protoName,
      unsigned* protoLen,
      SSLContext::NextProtocolType* protoType = nullptr) const;

  /**
   * Determine if the session specified during setSSLSession was reused
   * or if the server rejected it and issued a new session.
   */
  virtual bool getSSLSessionReused() const;

  /**
   * true if the session was resumed using session ID
   */
  bool sessionIDResumed() const { return sessionIDResumed_; }

  void setSessionIDResumed(bool resumed) {
    sessionIDResumed_ = resumed;
  }

  /**
   * Get the negociated cipher name for this SSL connection.
   * Returns the cipher used or the constant value "NONE" when no SSL session
   * has been established.
   */
  virtual const char* getNegotiatedCipherName() const;

  /**
   * Get the server name for this SSL connection.
   * Returns the server name used or the constant value "NONE" when no SSL
   * session has been established.
   * If openssl has no SNI support, throw TTransportException.
   */
  const char *getSSLServerName() const;

  /**
   * Get the server name for this SSL connection.
   * Returns the server name used or the constant value "NONE" when no SSL
   * session has been established.
   * If openssl has no SNI support, return "NONE"
   */
  const char *getSSLServerNameNoThrow() const;

  /**
   * Get the SSL version for this connection.
   * Possible return values are SSL2_VERSION, SSL3_VERSION, TLS1_VERSION,
   * with hexa representations 0x200, 0x300, 0x301,
   * or 0 if no SSL session has been established.
   */
  int getSSLVersion() const;

  /**
   * Get the signature algorithm used in the cert that is used for this
   * connection.
   */
  const char *getSSLCertSigAlgName() const;

  /**
   * Get the certificate size used for this SSL connection.
   */
  int getSSLCertSize() const;

  virtual void attachEventBase(EventBase* eventBase) override {
    AsyncSocket::attachEventBase(eventBase);
    handshakeTimeout_.attachEventBase(eventBase);
  }

  virtual void detachEventBase() override {
    AsyncSocket::detachEventBase();
    handshakeTimeout_.detachEventBase();
  }

  virtual bool isDetachable() const override {
    return AsyncSocket::isDetachable() && !handshakeTimeout_.isScheduled();
  }

  virtual void attachTimeoutManager(TimeoutManager* manager) {
    handshakeTimeout_.attachTimeoutManager(manager);
  }

  virtual void detachTimeoutManager() {
    handshakeTimeout_.detachTimeoutManager();
  }

#if OPENSSL_VERSION_NUMBER >= 0x009080bfL
  /**
   * This function will set the SSL context for this socket to the
   * argument. This should only be used on client SSL Sockets that have
   * already called detachSSLContext();
   */
  void attachSSLContext(const std::shared_ptr<folly::SSLContext>& ctx);

  /**
   * Detaches the SSL context for this socket.
   */
  void detachSSLContext();
#endif

#if OPENSSL_VERSION_NUMBER >= 0x1000105fL && !defined(OPENSSL_NO_TLSEXT)
  /**
   * Switch the SSLContext to continue the SSL handshake.
   * It can only be used in server mode.
   */
  void switchServerSSLContext(
    const std::shared_ptr<folly::SSLContext>& handshakeCtx);

  /**
   * Did server recognize/support the tlsext_hostname in Client Hello?
   * It can only be used in client mode.
   *
   * @return true - tlsext_hostname is matched by the server
   *         false - tlsext_hostname is not matched or
   *                 is not supported by server
   */
  bool isServerNameMatch() const;

  /**
   * Set the SNI hostname that we'll advertise to the server in the
   * ClientHello message.
   */
  void setServerName(std::string serverName) noexcept;
#endif

  void timeoutExpired() noexcept;

  /**
   * Get the list of supported ciphers sent by the client in the client's
   * preference order.
   */
  void getSSLClientCiphers(
      std::string& clientCiphers,
      bool convertToString = true) const {
    std::stringstream ciphersStream;
    std::string cipherName;

    if (parseClientHello_ == false
        || clientHelloInfo_->clientHelloCipherSuites_.empty()) {
      clientCiphers = "";
      return;
    }

    for (auto originalCipherCode : clientHelloInfo_->clientHelloCipherSuites_)
    {
      const SSL_CIPHER* cipher = nullptr;
      if (convertToString) {
        // OpenSSL expects code as a big endian char array
        auto cipherCode = htons(originalCipherCode);

#if defined(SSL_OP_NO_TLSv1_2)
        cipher =
            TLSv1_2_method()->get_cipher_by_char((unsigned char*)&cipherCode);
#elif defined(SSL_OP_NO_TLSv1_1)
        cipher =
            TLSv1_1_method()->get_cipher_by_char((unsigned char*)&cipherCode);
#elif defined(SSL_OP_NO_TLSv1)
        cipher =
            TLSv1_method()->get_cipher_by_char((unsigned char*)&cipherCode);
#else
        cipher =
            SSLv3_method()->get_cipher_by_char((unsigned char*)&cipherCode);
#endif
      }

      if (cipher == nullptr) {
        ciphersStream << std::setfill('0') << std::setw(4) << std::hex
                      << originalCipherCode << ":";
      } else {
        ciphersStream << SSL_CIPHER_get_name(cipher) << ":";
      }
    }

    clientCiphers = ciphersStream.str();
    clientCiphers.erase(clientCiphers.end() - 1);
  }

  /**
   * Get the list of compression methods sent by the client in TLS Hello.
   */
  std::string getSSLClientComprMethods() const {
    if (!parseClientHello_) {
      return "";
    }
    return folly::join(":", clientHelloInfo_->clientHelloCompressionMethods_);
  }

  /**
   * Get the list of TLS extensions sent by the client in the TLS Hello.
   */
  std::string getSSLClientExts() const {
    if (!parseClientHello_) {
      return "";
    }
    return folly::join(":", clientHelloInfo_->clientHelloExtensions_);
  }

  std::string getSSLClientSigAlgs() const {
    if (!parseClientHello_) {
      return "";
    }

    std::string sigAlgs;
    sigAlgs.reserve(clientHelloInfo_->clientHelloSigAlgs_.size() * 4);
    for (size_t i = 0; i < clientHelloInfo_->clientHelloSigAlgs_.size(); i++) {
      if (i) {
        sigAlgs.push_back(':');
      }
      sigAlgs.append(folly::to<std::string>(
          clientHelloInfo_->clientHelloSigAlgs_[i].first));
      sigAlgs.push_back(',');
      sigAlgs.append(folly::to<std::string>(
          clientHelloInfo_->clientHelloSigAlgs_[i].second));
    }

    return sigAlgs;
  }

  std::string getSSLAlertsReceived() const {
    std::string ret;

    for (const auto& alert : alertsReceived_) {
      if (!ret.empty()) {
        ret.append(",");
      }
      ret.append(folly::to<std::string>(alert.first, ": ", alert.second));
    }

    return ret;
  }

  /**
   * Get the list of shared ciphers between the server and the client.
   * Works well for only SSLv2, not so good for SSLv3 or TLSv1.
   */
  void getSSLSharedCiphers(std::string& sharedCiphers) const {
    char ciphersBuffer[1024];
    ciphersBuffer[0] = '\0';
    SSL_get_shared_ciphers(ssl_, ciphersBuffer, sizeof(ciphersBuffer) - 1);
    sharedCiphers = ciphersBuffer;
  }

  /**
   * Get the list of ciphers supported by the server in the server's
   * preference order.
   */
  void getSSLServerCiphers(std::string& serverCiphers) const {
    serverCiphers = SSL_get_cipher_list(ssl_, 0);
    int i = 1;
    const char *cipher;
    while ((cipher = SSL_get_cipher_list(ssl_, i)) != nullptr) {
      serverCiphers.append(":");
      serverCiphers.append(cipher);
      i++;
    }
  }

  static int getSSLExDataIndex();
  static AsyncSSLSocket* getFromSSL(const SSL *ssl);
  static int bioWrite(BIO* b, const char* in, int inl);
  void resetClientHelloParsing(SSL *ssl);
  static void clientHelloParsingCallback(int write_p, int version,
      int content_type, const void *buf, size_t len, SSL *ssl, void *arg);
  static const char* getSSLServerNameFromSSL(SSL* ssl);

  // For unit-tests
  ssl::ClientHelloInfo* getClientHelloInfo() const {
    return clientHelloInfo_.get();
  }

  /**
   * Returns the time taken to complete a handshake.
   */
  virtual std::chrono::nanoseconds getHandshakeTime() const {
    return handshakeEndTime_ - handshakeStartTime_;
  }

  void setMinWriteSize(size_t minWriteSize) {
    minWriteSize_ = minWriteSize;
  }

  size_t getMinWriteSize() const {
    return minWriteSize_;
  }

  void setReadCB(ReadCallback* callback) override;

  /**
   * Tries to enable the buffer movable experimental feature in openssl.
   * This is not guaranteed to succeed in case openssl does not have
   * the experimental feature built in.
   */
  void setBufferMovableEnabled(bool enabled);

  /**
   * Returns the peer certificate, or nullptr if no peer certificate received.
   */
  virtual ssl::X509UniquePtr getPeerCert() const override {
    if (!ssl_) {
      return nullptr;
    }

    X509* cert = SSL_get_peer_certificate(ssl_);
    return ssl::X509UniquePtr(cert);
  }

  /**
   * Force AsyncSSLSocket object to cache local and peer socket addresses.
   * If called with "true" before connect() this function forces full local
   * and remote socket addresses to be cached in the socket object and available
   * through getLocalAddress()/getPeerAddress() methods even after the socket is
   * closed.
   */
  void forceCacheAddrOnFailure(bool force) { cacheAddrOnFailure_ = force; }

  const std::string& getServiceIdentity() const { return serviceIdentity_; }

  void setServiceIdentity(std::string serviceIdentity) {
    serviceIdentity_ = std::move(serviceIdentity);
  }

 private:

  void init();

 protected:

  /**
   * Protected destructor.
   *
   * Users of AsyncSSLSocket must never delete it directly.  Instead, invoke
   * destroy() instead.  (See the documentation in DelayedDestruction.h for
   * more details.)
   */
  ~AsyncSSLSocket();

  // Inherit event notification methods from AsyncSocket except
  // the following.
  void prepareReadBuffer(void** buf, size_t* buflen) noexcept override;
  void handleRead() noexcept override;
  void handleWrite() noexcept override;
  void handleAccept() noexcept;
  void handleConnect() noexcept override;

  void invalidState(HandshakeCB* callback);
  bool willBlock(int ret,
                 int* sslErrorOut,
                 unsigned long* errErrorOut) noexcept;

  virtual void checkForImmediateRead() noexcept override;
  // AsyncSocket calls this at the wrong time for SSL
  void handleInitialReadWrite() noexcept override {}

  WriteResult interpretSSLError(int rc, int error);
  ReadResult performRead(void** buf, size_t* buflen, size_t* offset) override;
  WriteResult performWrite(
      const iovec* vec,
      uint32_t count,
      WriteFlags flags,
      uint32_t* countWritten,
      uint32_t* partialWritten) override;

  ssize_t performWriteIovec(const iovec* vec, uint32_t count,
                            WriteFlags flags, uint32_t* countWritten,
                            uint32_t* partialWritten);

  // This virtual wrapper around SSL_write exists solely for testing/mockability
  virtual int sslWriteImpl(SSL *ssl, const void *buf, int n) {
    return SSL_write(ssl, buf, n);
  }

  /**
   * Apply verification options passed to sslConn/sslAccept or those set
   * in the underlying SSLContext object.
   *
   * @param ssl pointer to the SSL object on which verification options will be
   * applied. If verifyPeer_ was explicitly set either via sslConn/sslAccept,
   * those options override the settings in the underlying SSLContext.
   */
  void applyVerificationOptions(SSL * ssl);

  /**
   * Sets up SSL with a custom write bio which intercepts all writes.
   *
   * @return true, if succeeds and false if there is an error creating the bio.
   */
  bool setupSSLBio();

  /**
   * A SSL_write wrapper that understand EOR
   *
   * @param ssl: SSL* object
   * @param buf: Buffer to be written
   * @param n:   Number of bytes to be written
   * @param eor: Does the last byte (buf[n-1]) have the app-last-byte?
   * @return:    The number of app bytes successfully written to the socket
   */
  int eorAwareSSLWrite(SSL *ssl, const void *buf, int n, bool eor);

  // Inherit error handling methods from AsyncSocket, plus the following.
  void failHandshake(const char* fn, const AsyncSocketException& ex);

  void invokeHandshakeErr(const AsyncSocketException& ex);
  void invokeHandshakeCB();

  void cacheLocalPeerAddr();

  static void sslInfoCallback(const SSL *ssl, int type, int val);

  // Whether we've applied the TCP_CORK option to the socket
  bool corked_{false};
  // SSL related members.
  bool server_{false};
  // Used to prevent client-initiated renegotiation.  Note that AsyncSSLSocket
  // doesn't fully support renegotiation, so we could just fail all attempts
  // to enforce this.  Once it is supported, we should make it an option
  // to disable client-initiated renegotiation.
  bool handshakeComplete_{false};
  bool renegotiateAttempted_{false};
  SSLStateEnum sslState_{STATE_UNINIT};
  std::shared_ptr<folly::SSLContext> ctx_;
  // Callback for SSL_accept() or SSL_connect()
  HandshakeCB* handshakeCallback_{nullptr};
  SSL* ssl_{nullptr};
  SSL_SESSION *sslSession_{nullptr};
  HandshakeTimeout handshakeTimeout_;
  // whether the SSL session was resumed using session ID or not
  bool sessionIDResumed_{false};

  // Whether to track EOR or not.
  bool trackEor_{false};

  // The app byte num that we are tracking for the MSG_EOR
  // Only one app EOR byte can be tracked.
  size_t appEorByteNo_{0};

  // Try to avoid calling SSL_write() for buffers smaller than this.
  // It doesn't take effect when it is 0.
  size_t minWriteSize_{1500};

  // When openssl is about to sendmsg() across the minEorRawBytesNo_,
  // it will pass MSG_EOR to sendmsg().
  size_t minEorRawByteNo_{0};
#if OPENSSL_VERSION_NUMBER >= 0x1000105fL && !defined(OPENSSL_NO_TLSEXT)
  std::shared_ptr<folly::SSLContext> handshakeCtx_;
  std::string tlsextHostname_;
#endif

  // a service identity that this socket/connection is associated with
  std::string serviceIdentity_;

  folly::SSLContext::SSLVerifyPeerEnum
    verifyPeer_{folly::SSLContext::SSLVerifyPeerEnum::USE_CTX};

  // Callback for SSL_CTX_set_verify()
  static int sslVerifyCallback(int preverifyOk, X509_STORE_CTX* ctx);

  bool parseClientHello_{false};
  bool cacheAddrOnFailure_{false};
  bool bufferMovableEnabled_{false};
  std::unique_ptr<ssl::ClientHelloInfo> clientHelloInfo_;
  std::vector<std::pair<char, StringPiece>> alertsReceived_;

  // Time taken to complete the ssl handshake.
  std::chrono::steady_clock::time_point handshakeStartTime_;
  std::chrono::steady_clock::time_point handshakeEndTime_;
};

} // namespace
