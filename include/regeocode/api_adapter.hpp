/**
 * SPDX-FileComment: Header file for the API Adapter interface.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file api_adapter.hpp
 * @brief Interface for all API adapters.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#pragma once

#include <map>
#include <memory>
#include <string>

namespace regeocode {

/**
 * @brief Structure to hold the result of an address lookup.
 */
struct AddressResult {
  std::string address_english; ///< Address in English.
  std::string address_local;   ///< Address in local language.
  std::string raw_json;        ///< Raw JSON response from the API.
  std::string country_code;    ///< ISO country code.
  std::map<std::string, std::string> attributes; ///< Additional attributes.
};

/**
 * @brief Abstract base class for API adapters.
 */
class ApiAdapter {
public:
  /**
   * @brief Virtual destructor.
   */
  virtual ~ApiAdapter() = default;

  /**
   * @brief Gets the name of the API adapter.
   * @return std::string The name of the adapter.
   */
  virtual std::string name() const = 0;

  /**
   * @brief Parses the JSON response from the API.
   * @param response_body The raw JSON response string.
   * @return AddressResult The parsed address result.
   */
  virtual AddressResult
  parse_response(const std::string &response_body) const = 0;
};

using ApiAdapterPtr = std::unique_ptr<ApiAdapter>;

} // namespace regeocode
