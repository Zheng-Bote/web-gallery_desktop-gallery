/**
 * SPDX-FileComment: Header file for the OpenCage Data API adapter.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file adapter_opencage.hpp
 * @brief Adapter for the OpenCage Data Geocoding API.
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
 * @brief Adapter implementation for the OpenCage Data API.
 */
class OpenCageAdapter : public ApiAdapter {
public:
  /**
   * @brief Gets the name of the API adapter.
   * @return std::string "opencage".
   */
  std::string name() const override { return "opencage"; }

  /**
   * @brief Parses the JSON response from the OpenCage Data API.
   * @param response_body The raw JSON response string.
   * @return AddressResult The parsed address result.
   */
  AddressResult parse_response(const std::string &response_body) const override;
};

} // namespace regeocode
