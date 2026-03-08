/**
 * SPDX-FileComment: Implementation of the Bing Maps API adapter.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file adapter_bing.cpp
 * @brief Implementation of the Bing Maps Reverse Geocoding API adapter.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#include "regeocode/adapter_bing.hpp"
#include <iostream>

namespace regeocode {

AddressResult
BingAdapter::parse_response(const std::string &response_body) const {
  AddressResult result;

  // Default values
  result.address_english = "";
  result.address_local = "";
  result.country_code = "";

  try {
    auto root = nlohmann::json::parse(response_body);

    // Azure Maps / Bing Structure: { "addresses": [ { "address": { ... } } ] }
    if (root.contains("addresses") && root["addresses"].is_array() &&
        !root["addresses"].empty()) {

      const auto &first_hit = root["addresses"][0];

      if (first_hit.contains("address")) {
        const auto &addr = first_hit["address"];

        // 1. Main address (freeformAddress is usually best readable)
        if (addr.contains("freeformAddress")) {
          result.address_english = addr["freeformAddress"].get<std::string>();
          // For Azure Maps, the language is controlled via API parameter,
          // so the result is usually already localized.
          result.address_local = result.address_english;
        }

        // 2. Country code
        if (addr.contains("countryCode")) {
          result.country_code = addr["countryCode"].get<std::string>();
        }

        // 3. Fill attributes (Take all fields from "address")
        for (auto &[key, val] : addr.items()) {
          if (val.is_string()) {
            result.attributes[key] = val.get<std::string>();
          } else if (val.is_number()) {
            // Convert numbers to strings
            if (val.is_number_integer()) {
              result.attributes[key] = std::to_string(val.get<long>());
            } else {
              result.attributes[key] = std::to_string(val.get<double>());
            }
          } else if (val.is_boolean()) {
            result.attributes[key] = val.get<bool>() ? "true" : "false";
          }
        }
      }

      // Optional: Get position from root object of hit, if necessary
      if (first_hit.contains("position")) {
        result.attributes["position_raw"] =
            first_hit["position"].get<std::string>();
      }
    }

  } catch (const std::exception &e) {
    // Parsing error: return empty result or log
    // result.attributes["error"] = e.what();
  }

  return result;
}

} // namespace regeocode