/**
 * SPDX-FileComment: Header file for the GeoNames Timezone API adapter.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file adapter_geonames_timezone.hpp
 * @brief Adapter for the GeoNames Timezone API.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#pragma once

#include "api_adapter.hpp"

namespace regeocode {

/**
 * @brief Adapter implementation for the GeoNames Timezone API.
 */
class GeoNamesTimezoneAdapter : public ApiAdapter {
public:
  /**
   * @brief Gets the name of the API adapter.
   *
   * The name must match the entry "Adapter = timezone" in the INI file.
   *
   * @return std::string "timezone".
   */
  std::string name() const override { return "timezone"; }

  /**
   * @brief Parses the JSON response from the GeoNames Timezone API.
   * @param response_body The raw JSON response string.
   * @return AddressResult The parsed address result (containing timezone info).
   */
  AddressResult parse_response(const std::string &response_body) const override;
};

} // namespace regeocode