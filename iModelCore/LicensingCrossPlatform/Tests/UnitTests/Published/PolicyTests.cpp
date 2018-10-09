/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/PolicyTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PolicyTests.h"
#include "DummyPolicyHelper.h"

#include <Licensing/Utils/JWToken.h>
#include "../../../Licensing/Policy.h"

USING_NAMESPACE_BENTLEY_LICENSING

USING_NAMESPACE_BENTLEY_LICENSING_UNIT_TESTS

TEST_F(PolicyTests, Create_JWtokenWithoutPolicyClaim_ReturnsError)
{
	auto jwToken = JWToken::Create("ew0KCSJ0eXAiOiAiSldUIiwNCgkiYWxnIjogIlJTMjU2IiwNCgkieDV0IjogIkZzMUUwVTA1S1Y4S2V6aVBzVi1iMlk5YWk1WSINCn0=.ew0KCSJ0ZXN0Q2xhaW0iOiAiVGVzdENsYWltVmFsdWUiDQp9.pd_PZBiPq3JAP2gvHntX2yEoObhCSMLQ8xce_3_ulvDOxFADq3DoCMlbq6lxRlX4ZUaHsvL6Mf0ZI6FoRhbBZZvM0g2w-lUWp28CSeH4rzmF-S9DLfWr64JN8QhEmYQVRC3deloBpBKtFDIKLs5B4BSfFPnCzs2vc0vW8WUp3ZOZbP28V1rBlwcgPQr8ljWC2C4jj0q8O56-E1GDfULIpuvGJGetWV3tHu3Rs_0fgxESc8x1typK6t9dDWsy-MHP8dF95pJ57nTl_b2c2_D4WBQJQA0zzeinrwDAzalgNmg8g5M2M2G-kdoybcePtKkrqwIdQxJVYyKJw1Cguc_eCg");
	EXPECT_NE(jwToken, nullptr);

	auto policy = Policy::Create(jwToken);
	EXPECT_EQ(policy, nullptr);
}

TEST_F(PolicyTests, Create_Policy)
	{
	Utf8String cert = "MIIFbzCCBFegAwIBAgIQAVxqd5S2CUbiu85DkHkIODANBgkqhkiG9w0BAQsFADBNMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5EaWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTYwODE5MDAwMDAwWhcNMTgwODI0MTIwMDAwWjCBoTELMAkGA1UEBhMCVVMxFTATBgNVBAgTDFBlbm5zeWx2YW5pYTEOMAwGA1UEBxMFRXh0b24xJjAkBgNVBAoTHUJlbnRsZXkgU3lzdGVtcywgSW5jb3Jwb3JhdGVkMQswCQYDVQQLEwJJVDE2MDQGA1UEAxMtQkNTLUVudGl0bGVtZW50LVBvbGljeS1TaWduaW5nLVFBLmJlbnRsZXkuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw9VMFKZB24W4EJhRNeR0nw/P32wFPPncKP0BdvrHY+CAeeKy5ySHNAblKfocw6Z3k2uW+Ct31l4q65j/clnM8qAmn8oQSiXBAACUFfgelenEakZIhsIGOWK9Jh9I+aSfkZrykToCvXb4tVIXbFVJb7chMcquehCGpGaiYmhWPUbppRgQHWol+FmwG9Pm0IR8jIUErlTgQ8cTVv9Uvf6g2HbipAwhTorg6CyW7Srky8+fO5jUYysiENuo1sdEPbaQbB80/gGM/QgzseK3GcLEf22R+aidpP9j6wUSA6duZlkclEsgy/fHycG8iPiwr0X/ZSloG4XISAFQqGVLH0JqDQIDAQABo4IB9DCCAfAwHwYDVR0jBBgwFoAUD4BhHIIxYdUvKOeNRji0LOHG2eIwHQYDVR0OBBYEFKRY7fr7RIxF9dxkOiBlcYQy/t3cMDgGA1UdEQQxMC+CLUJDUy1FbnRpdGxlbWVudC1Qb2xpY3ktU2lnbmluZy1RQS5iZW50bGV5LmNvbTAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMGsGA1UdHwRkMGIwL6AtoCuGKWh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9zc2NhLXNoYTItZzUuY3JsMC+gLaArhilodHRwOi8vY3JsNC5kaWdpY2VydC5jb20vc3NjYS1zaGEyLWc1LmNybDBMBgNVHSAERTBDMDcGCWCGSAGG/WwBATAqMCgGCCsGAQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5jb20vQ1BTMAgGBmeBDAECAjB8BggrBgEFBQcBAQRwMG4wJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBGBggrBgEFBQcwAoY6aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0U0hBMlNlY3VyZVNlcnZlckNBLmNydDAMBgNVHRMBAf8EAjAAMA0GCSqGSIb3DQEBCwUAA4IBAQBx89qhpkT54QRsZOR58+wrh1flxlV6FXGO2py62JebpQzjS5c4k/krnaxPUPe3VLja1WQFwhTBuyve9QAcfrz3dxNJ8V4OFwKu7BI2BaJmRn4FyUl1ela1AN7KKfgnFYeRfNOMeiQPCLYjrfsg9f0k0kzk9AuwOurL8jbzlOYo6s5KeEn3VaII50xEHu0E9/nvJkduIFrPug1Thg2HNLt4c9M2ivRfdhLzvALYLOpIUhYVvQvXKuioQ5WuGejNOOvVvX6uDrrXW1VofRUgMK/rJ+ZMr4TNg0+fHKo88qnpgpN1tJJd48ekFIZl1YmWVNrd1mP5sWV+D+shkoCPRduA";
	Utf8String token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6IkZzMUUwVTA1S1Y4S2V6aVBzVi1iMlk5YWk1WSJ9.eyJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcy9wb2xpY3kiOiJleUpRYjJ4cFkzbEpaQ0k2SW1VMk5HTmhPRFkwTFRZNU56TXROREF4WlMwNVlqRTNMVE0zTUdFek1HTmhORGt6WmlJc0lsQnZiR2xqZVZabGNuTnBiMjRpT2pFdU1Dd2lVRzlzYVdONVEzSmxZWFJsWkU5dUlqb2lNakF4Tnkwd05pMHhORlF4TURvME56bzBPUzQzT1RBMk9UZzNXaUlzSWxCdmJHbGplVVY0Y0dseVpYTlBiaUk2SWpJd01UY3RNRFl0TWpGVU1UQTZORGM2TkRrdU56a3dOams0TjFvaUxDSlNaWEYxWlhOMFJHRjBZU0k2ZXlKU1pYRjFaWE4wWldSVFpXTjFjbUZpYkdWeklqcGJleUpRY205a2RXTjBTV1FpT2pFd01EQXNJa1psWVhSMWNtVlRkSEpwYm1jaU9pSWlmVjBzSWxWelpYSkpaQ0k2SWpFeE1tUmtaVFExTFdZNE5XVXROREZsTmkxaE1ERXlMVEk0WmpWaVkyUTBOR05sTVNJc0lrMWhZMmhwYm1WT1lXMWxJam9pZEdWemRHbHVaeUlzSWtOc2FXVnVkRVJoZEdWVWFXMWxJam9pTWpBeE55MHdOQzB5TkZReU1Ub3dNVG94TkM0NU5UZGFJaXdpVEc5allXeGxJam9pWlc0dFZWTWlMQ0pCY0hCc2FXVnpWRzhpT2lKb2RIUndjem92TDNWc1lYTmtaWFpsZFhNeWMyWmpNREV1WldGemRIVnpNaTVqYkc5MVpHRndjQzVoZW5WeVpTNWpiMjB2SWl3aVEyaGxZMnRsWkU5MWRFUmhkR1VpT201MWJHd3NJazFoWTJocGJtVlRTVVFpT2lKNlJXTXdhMkpZWTAxclVFSllVMnBsU2tSVkszQkZOVFJ1TDBrOUlpd2lRV05qWlhOelMyVjVJanB1ZFd4c2ZTd2lUV0ZqYUdsdVpWTnBaMjVoZEhWeVpTSTZJak5GUlVoMFZ6SjZTazA0U3pjeFJXOU9NSEJZYWsxQk5FMVlORDBpTENKQmNIQnNhV1Z6Vkc5VmMyVnlTV1FpT2lJeE1USmtaR1UwTlMxbU9EVmxMVFF4WlRZdFlUQXhNaTB5T0dZMVltTmtORFJqWlRFaUxDSkJjSEJzYVdWelZHOVRaV04xY21GaWJHVkpaSE1pT201MWJHd3NJa0ZEVEhNaU9tNTFiR3dzSWxObFkzVnlZV0pzWlVSaGRHRWlPbTUxYkd3c0lsVnpaWEpFWVhSaElqcDdJbFZ6WlhKSlpDSTZJakV4TW1Sa1pUUTFMV1k0TldVdE5ERmxOaTFoTURFeUxUSTRaalZpWTJRME5HTmxNU0lzSWs5eVoyRnVhWHBoZEdsdmJrbGtJam9pTm1VMk1tRmtNekl0TlRneVppMDBNR000TFdGbE9EWXRPV1ExTUdZMllqRm1ZV1ExSWl3aVZYTmhaMlZEYjNWdWRISjVTVk5QSWpvaVZWTWlMQ0pWYkhScGJXRjBaVk5CVUVsa0lqb2lNVEF3TVRRMk5EZzRNU0lzSWxWc2RHbHRZWFJsU1dRaU9pSmlPV1EwTURFNE1DMDFNVGxrTFRRd09USXRPV05pWXkxak9HVTNOR0kzTXpObU0yVWlMQ0pWYkhScGJXRjBaVU52ZFc1MGNubEpaQ0k2SWpBd01EQXdNREF3TFRBd01EQXRNREF3TUMwd01EQXdMVEF3TURBd01EQXdNREF3TUNKOUxDSkVaV1poZFd4MFVYVmhiR2xtYVdWeWN5STZXM3NpVG1GdFpTSTZJa0ZzYkc5M1QyWm1iR2x1WlZWellXZGxJaXdpVm1Gc2RXVWlPaUpVVWxWRklpd2lWSGx3WlNJNkltSnZiMndpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJa1p5WlhGMVpXNWplVlJ2VTJWdVpDSXNJbFpoYkhWbElqb2lNalFpTENKVWVYQmxJam9pYVc1MElpd2lVSEp2YlhCMElqcHVkV3hzZlN4N0lrNWhiV1VpT2lKSVpXRnlkR0psWVhSSmJuUmxjblpoYkNJc0lsWmhiSFZsSWpvaU1TSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJa2h2ZEhCaGRHaFNaWFJ5ZVVGMGRHVnRjSFJ6SWl3aVZtRnNkV1VpT2lJMElpd2lWSGx3WlNJNkltbHVkQ0lzSWxCeWIyMXdkQ0k2Ym5Wc2JIMHNleUpPWVcxbElqb2lTWE5EYUdWamEyVmtUM1YwSWl3aVZtRnNkV1VpT2lKMGNuVmxJaXdpVkhsd1pTSTZJbUp2YjJ3aUxDSlFjbTl0Y0hRaU9tNTFiR3g5TEhzaVRtRnRaU0k2SWt4dloybHVSM0poWTJWUVpYSnBiMlFpTENKV1lXeDFaU0k2SWpjaUxDSlVlWEJsSWpvaWFXNTBJaXdpVUhKdmJYQjBJanB1ZFd4c2ZTeDdJazVoYldVaU9pSk1iMmRTWldOdmNtUnpWRzlUWlc1a1JsUXlJaXdpVm1Gc2RXVWlPaUkzTlRBd01DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJazFoZUV4dlowWnBiR1ZFZFhKaGRHbHZia2x1VFdsdWRYUmxjeUlzSWxaaGJIVmxJam9pTXpZd0lpd2lWSGx3WlNJNkltbHVkQ0lzSWxCeWIyMXdkQ0k2Ym5Wc2JIMHNleUpPWVcxbElqb2lUMlptYkdsdVpVUjFjbUYwYVc5dUlpd2lWbUZzZFdVaU9pSTNJaXdpVkhsd1pTSTZJbWx1ZENJc0lsQnliMjF3ZENJNmJuVnNiSDBzZXlKT1lXMWxJam9pVUc5c2FXTjVTVzUwWlhKMllXd2lMQ0pXWVd4MVpTSTZJall3SWl3aVZIbHdaU0k2SW1sdWRDSXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVZHbHRaVlJ2UzJWbGNGVnVVMlZ1ZEV4dlozTWlMQ0pXWVd4MVpTSTZJall3SWl3aVZIbHdaU0k2SW1sdWRDSXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVZYTmhaMlZVZVhCbElpd2lWbUZzZFdVaU9pSlFjbTlrZFdOMGFXOXVJaXdpVkhsd1pTSTZJbk4wY21sdVp5SXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVZYTmxRV3hsY25ScGJtZFRaWEoyYVdObElpd2lWbUZzZFdVaU9pSkVhWE5oWW14bFpDSXNJbFI1Y0dVaU9pSmxiblZ0SWl3aVVISnZiWEIwSWpvaVdXOTFJR2hoZG1VZ2NtVmhZMmhsWkNCaElHeHBZMlZ1YzJVZ2RHaHlaWE5vYjJ4a0lHUmxabWx1WldRZ1lua2dlVzkxY2lCdmNtZGhibWw2WVhScGIyN2lnSmx6SUdGa2JXbHVhWE4wY21GMGIzSXVJRlJ2SUhCeWIyTmxaV1FnZVc5MUlHMTFjM1FnNG9DY1FXTnJibTkzYkdWa1oyWGlnSnd1TGk0dUxqRXhNU0o5WFgwPSIsImlzcyI6Imh0dHA6Ly9wb2xpY3lzZXJ2aWNlLmJlbnRsZXkuY29tIiwiYXVkIjoiaHR0cHM6Ly91bGFzZGV2ZXVzMnNmYzAxLmVhc3R1czIuY2xvdWRhcHAuYXp1cmUuY29tLyIsImV4cCI6MTQ5ODA0MjA2OSwibmJmIjoxNDk3NDM3MjY5fQ.C002y5BwwhFQFRAiPg55wnfLzPW6wARCl-0kozCQY_i0NbFA5BBwHGQ5IjiLPYz-2MgevkrcHGFM5duQgQo9734RVQbPkecglE1xb3VeDME0ANTbvpxOL7zqhqGjN-1kZQnP8PtsZxU8mmPbEKncEoaA7U7uJEWFxHvjWeuc4-aYoamatUFqEyq8DQsxdsDzeDndXfDJYY_w1DIn7g-SUoVyBlRaX8LG2rmdz0Rf75IpbSm4vCZT06TNlpAwreZ5Z82pj8Wqi1DLRUd7fQ-CQ12ditEF_qFs9JDFaMWx-SVge2MbStQtGixNqML2LqkMZT454kIEpj642CdQ8DX4XA";

	auto jwToken = JWToken::Create(token, cert);
	auto policy = Policy::Create(jwToken);
	ASSERT_NE(policy,nullptr);
	}

TEST_F(PolicyTests, Create_PolicyNullInputs)
	{
	//auto policyNullToken = Policy::Create(PolicyToken::Create(Json::Value::GetNull()));
	auto policyNullToken = Policy::Create(JWToken::Create("", ""));
	auto policyNullJson = Policy::Create(Json::Value::GetNull());
	ASSERT_EQ(policyNullToken, nullptr);
	ASSERT_EQ(policyNullJson, nullptr);
	}

TEST_F(PolicyTests, Validate_ProperRead)
	{
	Utf8String cert = "MIIFbzCCBFegAwIBAgIQAVxqd5S2CUbiu85DkHkIODANBgkqhkiG9w0BAQsFADBNMQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMScwJQYDVQQDEx5EaWdpQ2VydCBTSEEyIFNlY3VyZSBTZXJ2ZXIgQ0EwHhcNMTYwODE5MDAwMDAwWhcNMTgwODI0MTIwMDAwWjCBoTELMAkGA1UEBhMCVVMxFTATBgNVBAgTDFBlbm5zeWx2YW5pYTEOMAwGA1UEBxMFRXh0b24xJjAkBgNVBAoTHUJlbnRsZXkgU3lzdGVtcywgSW5jb3Jwb3JhdGVkMQswCQYDVQQLEwJJVDE2MDQGA1UEAxMtQkNTLUVudGl0bGVtZW50LVBvbGljeS1TaWduaW5nLVFBLmJlbnRsZXkuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw9VMFKZB24W4EJhRNeR0nw/P32wFPPncKP0BdvrHY+CAeeKy5ySHNAblKfocw6Z3k2uW+Ct31l4q65j/clnM8qAmn8oQSiXBAACUFfgelenEakZIhsIGOWK9Jh9I+aSfkZrykToCvXb4tVIXbFVJb7chMcquehCGpGaiYmhWPUbppRgQHWol+FmwG9Pm0IR8jIUErlTgQ8cTVv9Uvf6g2HbipAwhTorg6CyW7Srky8+fO5jUYysiENuo1sdEPbaQbB80/gGM/QgzseK3GcLEf22R+aidpP9j6wUSA6duZlkclEsgy/fHycG8iPiwr0X/ZSloG4XISAFQqGVLH0JqDQIDAQABo4IB9DCCAfAwHwYDVR0jBBgwFoAUD4BhHIIxYdUvKOeNRji0LOHG2eIwHQYDVR0OBBYEFKRY7fr7RIxF9dxkOiBlcYQy/t3cMDgGA1UdEQQxMC+CLUJDUy1FbnRpdGxlbWVudC1Qb2xpY3ktU2lnbmluZy1RQS5iZW50bGV5LmNvbTAOBgNVHQ8BAf8EBAMCBaAwHQYDVR0lBBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMGsGA1UdHwRkMGIwL6AtoCuGKWh0dHA6Ly9jcmwzLmRpZ2ljZXJ0LmNvbS9zc2NhLXNoYTItZzUuY3JsMC+gLaArhilodHRwOi8vY3JsNC5kaWdpY2VydC5jb20vc3NjYS1zaGEyLWc1LmNybDBMBgNVHSAERTBDMDcGCWCGSAGG/WwBATAqMCgGCCsGAQUFBwIBFhxodHRwczovL3d3dy5kaWdpY2VydC5jb20vQ1BTMAgGBmeBDAECAjB8BggrBgEFBQcBAQRwMG4wJAYIKwYBBQUHMAGGGGh0dHA6Ly9vY3NwLmRpZ2ljZXJ0LmNvbTBGBggrBgEFBQcwAoY6aHR0cDovL2NhY2VydHMuZGlnaWNlcnQuY29tL0RpZ2lDZXJ0U0hBMlNlY3VyZVNlcnZlckNBLmNydDAMBgNVHRMBAf8EAjAAMA0GCSqGSIb3DQEBCwUAA4IBAQBx89qhpkT54QRsZOR58+wrh1flxlV6FXGO2py62JebpQzjS5c4k/krnaxPUPe3VLja1WQFwhTBuyve9QAcfrz3dxNJ8V4OFwKu7BI2BaJmRn4FyUl1ela1AN7KKfgnFYeRfNOMeiQPCLYjrfsg9f0k0kzk9AuwOurL8jbzlOYo6s5KeEn3VaII50xEHu0E9/nvJkduIFrPug1Thg2HNLt4c9M2ivRfdhLzvALYLOpIUhYVvQvXKuioQ5WuGejNOOvVvX6uDrrXW1VofRUgMK/rJ+ZMr4TNg0+fHKo88qnpgpN1tJJd48ekFIZl1YmWVNrd1mP5sWV+D+shkoCPRduA";
	Utf8String token = "eyJ0eXAiOiJKV1QiLCJhbGciOiJSUzI1NiIsIng1dCI6IkZzMUUwVTA1S1Y4S2V6aVBzVi1iMlk5YWk1WSJ9.eyJodHRwOi8vc2NoZW1hcy5iZW50bGV5LmNvbS93cy8yMDExLzAzL2lkZW50aXR5L2NsYWltcy9wb2xpY3kiOiJleUpRYjJ4cFkzbEpaQ0k2SW1VMk5HTmhPRFkwTFRZNU56TXROREF4WlMwNVlqRTNMVE0zTUdFek1HTmhORGt6WmlJc0lsQnZiR2xqZVZabGNuTnBiMjRpT2pFdU1Dd2lVRzlzYVdONVEzSmxZWFJsWkU5dUlqb2lNakF4Tnkwd05pMHhORlF4TURvME56bzBPUzQzT1RBMk9UZzNXaUlzSWxCdmJHbGplVVY0Y0dseVpYTlBiaUk2SWpJd01UY3RNRFl0TWpGVU1UQTZORGM2TkRrdU56a3dOams0TjFvaUxDSlNaWEYxWlhOMFJHRjBZU0k2ZXlKU1pYRjFaWE4wWldSVFpXTjFjbUZpYkdWeklqcGJleUpRY205a2RXTjBTV1FpT2pFd01EQXNJa1psWVhSMWNtVlRkSEpwYm1jaU9pSWlmVjBzSWxWelpYSkpaQ0k2SWpFeE1tUmtaVFExTFdZNE5XVXROREZsTmkxaE1ERXlMVEk0WmpWaVkyUTBOR05sTVNJc0lrMWhZMmhwYm1WT1lXMWxJam9pZEdWemRHbHVaeUlzSWtOc2FXVnVkRVJoZEdWVWFXMWxJam9pTWpBeE55MHdOQzB5TkZReU1Ub3dNVG94TkM0NU5UZGFJaXdpVEc5allXeGxJam9pWlc0dFZWTWlMQ0pCY0hCc2FXVnpWRzhpT2lKb2RIUndjem92TDNWc1lYTmtaWFpsZFhNeWMyWmpNREV1WldGemRIVnpNaTVqYkc5MVpHRndjQzVoZW5WeVpTNWpiMjB2SWl3aVEyaGxZMnRsWkU5MWRFUmhkR1VpT201MWJHd3NJazFoWTJocGJtVlRTVVFpT2lKNlJXTXdhMkpZWTAxclVFSllVMnBsU2tSVkszQkZOVFJ1TDBrOUlpd2lRV05qWlhOelMyVjVJanB1ZFd4c2ZTd2lUV0ZqYUdsdVpWTnBaMjVoZEhWeVpTSTZJak5GUlVoMFZ6SjZTazA0U3pjeFJXOU9NSEJZYWsxQk5FMVlORDBpTENKQmNIQnNhV1Z6Vkc5VmMyVnlTV1FpT2lJeE1USmtaR1UwTlMxbU9EVmxMVFF4WlRZdFlUQXhNaTB5T0dZMVltTmtORFJqWlRFaUxDSkJjSEJzYVdWelZHOVRaV04xY21GaWJHVkpaSE1pT201MWJHd3NJa0ZEVEhNaU9tNTFiR3dzSWxObFkzVnlZV0pzWlVSaGRHRWlPbTUxYkd3c0lsVnpaWEpFWVhSaElqcDdJbFZ6WlhKSlpDSTZJakV4TW1Sa1pUUTFMV1k0TldVdE5ERmxOaTFoTURFeUxUSTRaalZpWTJRME5HTmxNU0lzSWs5eVoyRnVhWHBoZEdsdmJrbGtJam9pTm1VMk1tRmtNekl0TlRneVppMDBNR000TFdGbE9EWXRPV1ExTUdZMllqRm1ZV1ExSWl3aVZYTmhaMlZEYjNWdWRISjVTVk5QSWpvaVZWTWlMQ0pWYkhScGJXRjBaVk5CVUVsa0lqb2lNVEF3TVRRMk5EZzRNU0lzSWxWc2RHbHRZWFJsU1dRaU9pSmlPV1EwTURFNE1DMDFNVGxrTFRRd09USXRPV05pWXkxak9HVTNOR0kzTXpObU0yVWlMQ0pWYkhScGJXRjBaVU52ZFc1MGNubEpaQ0k2SWpBd01EQXdNREF3TFRBd01EQXRNREF3TUMwd01EQXdMVEF3TURBd01EQXdNREF3TUNKOUxDSkVaV1poZFd4MFVYVmhiR2xtYVdWeWN5STZXM3NpVG1GdFpTSTZJa0ZzYkc5M1QyWm1iR2x1WlZWellXZGxJaXdpVm1Gc2RXVWlPaUpVVWxWRklpd2lWSGx3WlNJNkltSnZiMndpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJa1p5WlhGMVpXNWplVlJ2VTJWdVpDSXNJbFpoYkhWbElqb2lNalFpTENKVWVYQmxJam9pYVc1MElpd2lVSEp2YlhCMElqcHVkV3hzZlN4N0lrNWhiV1VpT2lKSVpXRnlkR0psWVhSSmJuUmxjblpoYkNJc0lsWmhiSFZsSWpvaU1TSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJa2h2ZEhCaGRHaFNaWFJ5ZVVGMGRHVnRjSFJ6SWl3aVZtRnNkV1VpT2lJMElpd2lWSGx3WlNJNkltbHVkQ0lzSWxCeWIyMXdkQ0k2Ym5Wc2JIMHNleUpPWVcxbElqb2lTWE5EYUdWamEyVmtUM1YwSWl3aVZtRnNkV1VpT2lKMGNuVmxJaXdpVkhsd1pTSTZJbUp2YjJ3aUxDSlFjbTl0Y0hRaU9tNTFiR3g5TEhzaVRtRnRaU0k2SWt4dloybHVSM0poWTJWUVpYSnBiMlFpTENKV1lXeDFaU0k2SWpjaUxDSlVlWEJsSWpvaWFXNTBJaXdpVUhKdmJYQjBJanB1ZFd4c2ZTeDdJazVoYldVaU9pSk1iMmRTWldOdmNtUnpWRzlUWlc1a1JsUXlJaXdpVm1Gc2RXVWlPaUkzTlRBd01DSXNJbFI1Y0dVaU9pSnBiblFpTENKUWNtOXRjSFFpT201MWJHeDlMSHNpVG1GdFpTSTZJazFoZUV4dlowWnBiR1ZFZFhKaGRHbHZia2x1VFdsdWRYUmxjeUlzSWxaaGJIVmxJam9pTXpZd0lpd2lWSGx3WlNJNkltbHVkQ0lzSWxCeWIyMXdkQ0k2Ym5Wc2JIMHNleUpPWVcxbElqb2lUMlptYkdsdVpVUjFjbUYwYVc5dUlpd2lWbUZzZFdVaU9pSTNJaXdpVkhsd1pTSTZJbWx1ZENJc0lsQnliMjF3ZENJNmJuVnNiSDBzZXlKT1lXMWxJam9pVUc5c2FXTjVTVzUwWlhKMllXd2lMQ0pXWVd4MVpTSTZJall3SWl3aVZIbHdaU0k2SW1sdWRDSXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVZHbHRaVlJ2UzJWbGNGVnVVMlZ1ZEV4dlozTWlMQ0pXWVd4MVpTSTZJall3SWl3aVZIbHdaU0k2SW1sdWRDSXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVZYTmhaMlZVZVhCbElpd2lWbUZzZFdVaU9pSlFjbTlrZFdOMGFXOXVJaXdpVkhsd1pTSTZJbk4wY21sdVp5SXNJbEJ5YjIxd2RDSTZiblZzYkgwc2V5Sk9ZVzFsSWpvaVZYTmxRV3hsY25ScGJtZFRaWEoyYVdObElpd2lWbUZzZFdVaU9pSkVhWE5oWW14bFpDSXNJbFI1Y0dVaU9pSmxiblZ0SWl3aVVISnZiWEIwSWpvaVdXOTFJR2hoZG1VZ2NtVmhZMmhsWkNCaElHeHBZMlZ1YzJVZ2RHaHlaWE5vYjJ4a0lHUmxabWx1WldRZ1lua2dlVzkxY2lCdmNtZGhibWw2WVhScGIyN2lnSmx6SUdGa2JXbHVhWE4wY21GMGIzSXVJRlJ2SUhCeWIyTmxaV1FnZVc5MUlHMTFjM1FnNG9DY1FXTnJibTkzYkdWa1oyWGlnSnd1TGk0dUxqRXhNU0o5WFgwPSIsImlzcyI6Imh0dHA6Ly9wb2xpY3lzZXJ2aWNlLmJlbnRsZXkuY29tIiwiYXVkIjoiaHR0cHM6Ly91bGFzZGV2ZXVzMnNmYzAxLmVhc3R1czIuY2xvdWRhcHAuYXp1cmUuY29tLyIsImV4cCI6MTQ5ODA0MjA2OSwibmJmIjoxNDk3NDM3MjY5fQ.C002y5BwwhFQFRAiPg55wnfLzPW6wARCl-0kozCQY_i0NbFA5BBwHGQ5IjiLPYz-2MgevkrcHGFM5duQgQo9734RVQbPkecglE1xb3VeDME0ANTbvpxOL7zqhqGjN-1kZQnP8PtsZxU8mmPbEKncEoaA7U7uJEWFxHvjWeuc4-aYoamatUFqEyq8DQsxdsDzeDndXfDJYY_w1DIn7g-SUoVyBlRaX8LG2rmdz0Rf75IpbSm4vCZT06TNlpAwreZ5Z82pj8Wqi1DLRUd7fQ-CQ12ditEF_qFs9JDFaMWx-SVge2MbStQtGixNqML2LqkMZT454kIEpj642CdQ8DX4XA";

	auto jwToken = JWToken::Create(token, cert);
	auto policy = Policy::Create(jwToken);
	// check all parameters
	ASSERT_NE(policy, nullptr);
	ASSERT_TRUE(policy->GetPolicyId().Equals("e64ca864-6973-401e-9b17-370a30ca493f"));
	ASSERT_EQ(policy->GetPolicyVersion(), 1.0);
	ASSERT_EQ(DateHelper::TimeToString(policy->GetPolicyCreatedOn()), "2017-06-14T10:47:49");
	ASSERT_EQ(DateHelper::TimeToString(policy->GetPolicyExpiresOn()), "2017-06-21T10:47:49");
	auto requestData = policy->GetRequestData();
	ASSERT_NE(requestData, nullptr);
	ASSERT_TRUE(requestData->GetMachineSID().Equals("zEc0kbXcMkPBXSjeJDU+pE54n/I="));
	ASSERT_TRUE(std::string(requestData->GetAccessKey().c_str()).empty());
	ASSERT_TRUE(requestData->GetUserId().Equals("112dde45-f85e-41e6-a012-28f5bcd44ce1"));
	ASSERT_EQ(requestData->GetCheckedOutDate(), -1); // no time
	auto requestedSecurables = requestData->GetRequestedSecurables();
	ASSERT_EQ(requestedSecurables.size(), 1);
	ASSERT_EQ(requestedSecurables.front()->GetProductId(), 1000);
	ASSERT_EQ(std::to_string(requestedSecurables.front()->GetProductId()).c_str(), Utf8String("1000"));
	ASSERT_TRUE(std::string(requestedSecurables.front()->GetFeatureString().c_str()).empty());
	ASSERT_TRUE(std::string(requestedSecurables.front()->GetVersion().c_str()).empty());
	ASSERT_EQ(requestData->GetMachineName(), "testing");
	ASSERT_EQ(DateHelper::TimeToString(requestData->GetClientDateTime()), "2017-04-24T21:01:14");
	ASSERT_EQ(requestData->GetLocale(), "en-US");
	ASSERT_EQ(requestData->GetAppliesTo(), "https://ulasdeveus2sfc01.eastus2.cloudapp.azure.com/");
	ASSERT_TRUE(policy->GetMachineSignature().Equals("3EEHtW2zJM8K71EoN0pXjMA4MX4="));
	ASSERT_TRUE(policy->GetAppliesToUserId().Equals("112dde45-f85e-41e6-a012-28f5bcd44ce1"));
	ASSERT_EQ(policy->GetAppliesToSecurableIds().size(), 0);
	ASSERT_EQ(policy->GetACLs().size(), 0);
	ASSERT_EQ(policy->GetSecurableData().size(), 0);
	auto userData = policy->GetUserData();
	ASSERT_NE(userData, nullptr);
	ASSERT_TRUE(userData->GetUserId().Equals("112dde45-f85e-41e6-a012-28f5bcd44ce1"));
	ASSERT_TRUE(userData->GetOrganizationId().Equals("6e62ad32-582f-40c8-ae86-9d50f6b1fad5"));
	ASSERT_TRUE(userData->GetUsageCountryISO().Equals("US"));
	ASSERT_TRUE(userData->GetUltimateSAPId() == 1001464881);
	ASSERT_TRUE(userData->GetUltimateId().Equals("b9d40180-519d-4092-9cbc-c8e74b733f3e"));
	ASSERT_TRUE(userData->GetUltimateCountryId().Equals("00000000-0000-0000-0000-000000000000"));
	auto defaultQualifiers = policy->GetDefaultQualifiers();
	ASSERT_EQ(defaultQualifiers.size(), 13);

	}

TEST_F(PolicyTests, InvalidJson_Robustness)
	{
	auto policy = Policy::Create(DummyPolicyHelper::CreatePolicyMissingFields());
	// verify Utf8Strings are empty and don't cause SEH exception
	ASSERT_NE(policy, nullptr);
	ASSERT_NE(policy->GetPolicyId(), "");
	ASSERT_EQ(policy->GetMachineSignature(), "");
	ASSERT_EQ(policy->GetAppliesToUserId(), "");
	}

TEST_F(PolicyTests, GetACLsQualifierOverride_HeartbeatInterval_Success)
    {
    bvector<QualifierOverride> aclsQualifierOverrides;
    bvector<QualifierOverride> securedataQualifierOverrides;
    QualifierOverride aclsQualifierOverride;

    aclsQualifierOverride.qualifierName = "HeartbeatInterval";
    aclsQualifierOverride.qualifierValue = "5";
    aclsQualifierOverride.qualifierType = "int";
    aclsQualifierOverride.qualifierPrompt = "";
    aclsQualifierOverrides.push_back(aclsQualifierOverride);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto policy = Policy::Create(DummyPolicyHelper::CreatePolicyQuailifierOverrides(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7), 
                                                                                    DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false,
                                                                                    aclsQualifierOverrides, securedataQualifierOverrides));

    EXPECT_NE(policy, nullptr);

    int heartbeatInterval = policy->GetHeartbeatInterval("9900", "");
    EXPECT_EQ(5*60*1000, heartbeatInterval);
    }

TEST_F(PolicyTests, GetSecureDataQualifierOverride_PolicyInterval_Success)
    {
    bvector<QualifierOverride> aclsQualifierOverrides;
    bvector<QualifierOverride> securedataQualifierOverrides;
    QualifierOverride securedataQualifierOverride;

    securedataQualifierOverride.qualifierName = "PolicyInterval";
    securedataQualifierOverride.qualifierValue = "120";
    securedataQualifierOverride.qualifierType = "int";
    securedataQualifierOverride.qualifierPrompt = "";
    securedataQualifierOverrides.push_back(securedataQualifierOverride);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto policy = Policy::Create(DummyPolicyHelper::CreatePolicyQuailifierOverrides(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7),
                                                                                    DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false,
                                                                                    aclsQualifierOverrides, securedataQualifierOverrides));

    EXPECT_NE(policy, nullptr);

    int policyInterval = policy->GetPolicyInterval("9900", "");
    EXPECT_EQ(120*60*1000, policyInterval);
    }

TEST_F(PolicyTests, GetACLsQualifierOverrideWithSecureDataQualifierOverride_TimeToKeepUnSentLogs_Success)
    {
    bvector<QualifierOverride> aclsQualifierOverrides;
    bvector<QualifierOverride> securedataQualifierOverrides;
    QualifierOverride aclsQualifierOverride;
    QualifierOverride securedataQualifierOverride;

    aclsQualifierOverride.qualifierName = "TimeToKeepUnSentLogs";
    aclsQualifierOverride.qualifierValue = "90";
    aclsQualifierOverride.qualifierType = "int";
    aclsQualifierOverride.qualifierPrompt = "";
    aclsQualifierOverrides.push_back(aclsQualifierOverride);

    securedataQualifierOverride.qualifierName = "TimeToKeepUnSentLogs";
    securedataQualifierOverride.qualifierValue = "120";
    securedataQualifierOverride.qualifierType = "int";
    securedataQualifierOverride.qualifierPrompt = "";
    securedataQualifierOverrides.push_back(securedataQualifierOverride);

    Utf8String userId = "ca1cc6ca-2af1-4efd-8876-fd5910a3a7fa";

    auto policy = Policy::Create(DummyPolicyHelper::CreatePolicyQuailifierOverrides(DateHelper::GetCurrentTime(), DateHelper::AddDaysToCurrentTime(7),
                                                                                    DateHelper::AddDaysToCurrentTime(7), userId, 9900, "", 1, false,
                                                                                    aclsQualifierOverrides, securedataQualifierOverrides));

    EXPECT_NE(policy, nullptr);

    int logPostingInterval = policy->GetTimeToKeepUnSentLogs("9900", "");
    EXPECT_EQ(90*60*1000, logPostingInterval);
    }
