/*---------------------------------------------------------------------------------------------
 * Adapted from https://github.com/atom/node-keytar
 * See LICENSE.md in the libsrc/keytar folder for license information. 
 *--------------------------------------------------------------------------------------------*/
#pragma once
#include <Napi/napi.h>
#include <BeSecurity/keytar/credentials.h>
#include <string>

USING_NAMESPACE_KEYTAR

//=======================================================================================
// Add the password for the service and account to the keychain.
//! @bsiclass
//=======================================================================================
struct SetPasswordWorker : public Napi::AsyncWorker {
  public:
    SetPasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string &service, const std::string &account, const std::string &password);
    ~SetPasswordWorker();

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    static Napi::Value SetPasswordAsync(Napi::CallbackInfo const &info);

  private : 
    Napi::Promise::Deferred deferred;
    const std::string service;
    const std::string account;
    const std::string password;
};

//=======================================================================================
// Get the stored password for the service and account.
//! @bsiclass
//=======================================================================================
struct GetPasswordWorker : public Napi::AsyncWorker {
  public:
    GetPasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string& service, const std::string& account);
    ~GetPasswordWorker();

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    static Napi::Value GetPasswordAsync(Napi::CallbackInfo const &info); 

  private:
    Napi::Promise::Deferred deferred;
    const std::string service;
    const std::string account;
    std::string password;
    bool success;
};

//=======================================================================================
// Delete the stored password for the service and account.
//! @bsiclass
//=======================================================================================
struct DeletePasswordWorker : public Napi::AsyncWorker {
  public:
    DeletePasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string& service, const std::string& account);
    ~DeletePasswordWorker();

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    static Napi::Value DeletePasswordAsync(Napi::CallbackInfo const &info); 

  private:
    Napi::Promise::Deferred deferred;
    const std::string service;
    const std::string account;
    bool success;
};

//=======================================================================================
// Find a password for the service in the keychain.
//! @bsiclass
//=======================================================================================
struct FindPasswordWorker : public Napi::AsyncWorker {
  public:
    FindPasswordWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string& service);
    ~FindPasswordWorker();

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    static Napi::Value FindPasswordAsync(Napi::CallbackInfo const &info); 

  private:
    Napi::Promise::Deferred deferred;
    const std::string service;
    std::string password;
    bool success;
};

//=======================================================================================
// Find all accounts and passwords for `service` in the keychain.
//! @bsiclass
//=======================================================================================
struct FindCredentialsWorker : public Napi::AsyncWorker {
  public:
    FindCredentialsWorker(Napi::Function &callback, Napi::Promise::Deferred& deferred, const std::string& service);
    ~FindCredentialsWorker();

    void Execute() override;
    void OnOK() override;
    void OnError(Napi::Error const &error) override;

    static Napi::Value FindCredentialsAsync(Napi::CallbackInfo const &info); 

  private:
    Napi::Promise::Deferred deferred;
    const std::string service;
    std::vector<Credentials> credentials;
    bool success;
};
