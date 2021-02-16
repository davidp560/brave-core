/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "third_party/blink/renderer/platform/wtf/text/string_builder.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"

namespace {

const char* kIpfs[] = {"ipfs", ".ipfs.localhost"};
const char* kIpns[] = {"ipns", ".ipns.localhost"};

bool BuildRawIPFSIfApplicable(const String& host,
                              const String& scheme,
                              const String& domain,
                              StringBuilder* builder) {
  if (!host.EndsWith(domain) || !builder)
    return false;
  wtf_size_t cid_length = host.Find(domain);
  if (!cid_length || cid_length == kNotFound)
    return false;
  builder->Append(scheme);
  builder->Append("://");
  builder->Append(host.Left(cid_length));
  return true;
}

}  // namespace

#define BRAVE_IPFS_ORIGINS_CHECK                                       \
  if (BuildRawIPFSIfApplicable(host_, kIpfs[0], kIpfs[1], &builder) || \
      BuildRawIPFSIfApplicable(host_, kIpns[0], kIpns[1], &builder))   \
    return;

#include "../../../../../../../third_party/blink/renderer/platform/weborigin/security_origin.cc"  // NOLINT
#undef BRAVE_IPFS_ORIGINS_CHECK
