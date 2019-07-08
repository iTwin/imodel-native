
#include <string>
#include <vector>
#include <BeJsonCpp/BeJsonUtilities.h>

#include <Licensing/Client.h>
#include <Licensing/AccessKeyClient.h>
#include <Licensing/SaasClient.h>

bool m_isClientCreated;
bool m_heartbeatRunning;

std::string m_fullPath;

Licensing::ClientPtr m_client;
Licensing::AccessKeyClientPtr m_accessKeyClient;
Licensing::SaasClientPtr m_saasClient;

JsonLocalState* m_localState;

// command line processing functions
std::vector<std::string> ParseInput(std::string input);

void Initialize();

void ProcessClientCommand(std::vector<std::string> input);

// client functions
void CreateClient();

void CreateAccessKeyClient();

void CreateSaasClient();

void TrackUsage();

void MarkFeature();

void AccessKeyMarkFeature();

void SaasMarkFeature();

void LicenseStatus();