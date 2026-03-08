/**
 * SPDX-FileComment: Header file for the Bing Maps API adapter.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file adapter_bing.hpp
 * @brief Adapter for the Bing Maps Reverse Geocoding API.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#pragma once

#include "api_adapter.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace regeocode {

/**
 * @brief Adapter implementation for the Bing Maps API.
 */
class BingAdapter : public ApiAdapter {
public:
  /**
   * @brief Gets the name of the API adapter.
   * @return std::string "bing".
   */
  std::string name() const override { return "bing"; }

  /**
   * @brief Parses the JSON response from the Bing Maps API.
   * @param response_body The raw JSON response string.
   * @return AddressResult The parsed address result.
   */
  AddressResult parse_response(const std::string &response_body) const override;
};

} // namespace regeocode