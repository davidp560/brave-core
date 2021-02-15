/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BUNDLE_CREATIVE_CONVERSION_INFO_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BUNDLE_CREATIVE_CONVERSION_INFO_H_

#include <stdint.h>

#include <string>
#include <vector>

namespace ads {

struct CreativeConversionInfo {
  ConversionInfo();
  ConversionInfo(const CreativeConversionInfo& info);
  ~ConversionInfo();

  bool operator==(const CreativeConversionInfo& rhs) const;
  bool operator!=(const CreativeConversionInfo& rhs) const;

  enum class SortType { kNone = 0, kAscendingOrder, kDescendingOrder };

  std::string creative_set_id;
  std::string type;
  std::string url_pattern;
  int observation_window = 0;
  int64_t expiry_timestamp = 0;
};

using CreativeConversionList = std::vector<CreativeConversionInfo>;

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BUNDLE_CREATIVE_CONVERSION_INFO_H_
