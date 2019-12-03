/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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

int getch();
std::string getpass(bool show_asterisk = true);

void Initialize();

void ProcessClientCommand(std::vector<std::string> input);

// client functions
void CreateClient();

void CreateAccessKeyClient();

void CreateUltimateLevelAccessKeyClient();

void CreateSaasClient();

void TrackUsage();

void MarkFeature();

void AccessKeyMarkFeature();

void SaasMarkFeature();

void Import(BeFileNameCR filepath);

void LicenseStatus();