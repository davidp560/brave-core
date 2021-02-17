/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_SECURITY_CRYPTO_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_SECURITY_CRYPTO_UTIL_H_

#include <stdint.h>

#include <map>
#include <string>
#include <vector>

namespace ads {

struct ConfirmationInfo;
struct VerifiableConversionInfo;

namespace security {

struct VerifiableConversionEnvelopeInfo;

std::string Sign(const std::map<std::string, std::string>& headers,
                 const std::string& key_id,
                 const std::string& secret_key);

std::vector<uint8_t> Sha256Hash(const std::string& value);

std::vector<uint8_t> Encrypt(const std::vector<uint8_t>& message,
                             const std::vector<uint8_t>& nonce,
                             const std::vector<uint8_t>& public_key,
                             const std::vector<uint8_t>& ephemeral_secret_key);

std::string Decrypt(const std::vector<uint8_t>& ciphertext,
                    const std::vector<uint8_t>& nonce,
                    const std::vector<uint8_t>& ephemeral_public_key,
                    const std::vector<uint8_t>& secret_key);

// TODO(Moritz Haller): privacy/wallet/wallet_util.cc
std::vector<uint8_t> GenerateSecretKeyFromSeed(const std::string& seed_base64);

// TODO(Moritz Haller): privacy/confirmations/confirmations_util.cc
bool Verify(const ConfirmationInfo& confirmation);

// TODO(Moritz Haller): conversions/conversions_util.cc
VerifiableConversionEnvelopeInfo EncryptAndEncodeVerifiableConversion(
    const VerifiableConversionInfo& verifiable_conversion);

std::string DecodeAndDecryptVerifiableConversionEnvelope(
    const VerifiableConversionEnvelopeInfo verifiable_conversion_envelope,
    const std::string& advertiser_sk_base64);

}  // namespace security
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_SECURITY_CRYPTO_UTIL_H_
