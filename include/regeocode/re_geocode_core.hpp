/**
 * SPDX-FileComment: Header file for the Core Geocoding logic.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file re_geocode_core.hpp
 * @brief Core classes and structures for Reverse Geocoding.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>

#include "regeocode/api_adapter.hpp"
#include "regeocode/http_client.hpp"
#include "regeocode/quota_manager.hpp"

namespace regeocode {

/**
 * @brief Structure representing geographic coordinates.
 */
struct Coordinates {
  double latitude;  ///< Latitude.
  double longitude; ///< Longitude.
};

/**
 * @brief Configuration for a single API.
 */
struct ApiConfig {
  std::string name;         ///< API name.
  std::string uri_template; ///< URI template for requests.
  std::string api_key;      ///< API Key.
  std::string adapter;      ///< Adapter name.
  std::string type;         ///< API type.
  long timeout = 10;        ///< Request timeout in seconds.
  long daily_limit = 0;     ///< Daily request limit.
};

// NEW: Container for the entire config result
struct Configuration {
  std::unordered_map<std::string, ApiConfig> apis;
  std::string quota_file_path = "quota_status.json"; // Default
};

class ConfigLoader {
public:
  explicit ConfigLoader(std::string ini_path);
  // CHANGE: Returns Configuration struct instead of just the map
  Configuration load() const;

private:
  std::string ini_path_;
};

class ReverseGeocoder {
public:
  // CHANGE: Constructor now takes quota_file_path
  ReverseGeocoder(
      std::unordered_map<std::string, ApiConfig> configs,
      std::vector<ApiAdapterPtr> adapters,
      std::unique_ptr<HttpClient> http_client = std::make_unique<HttpClient>(),
      const std::string &quota_file_path = "quota_status.json");

  // ... (Rest of methods remain the same) ...

  /**
   * @brief Performs reverse geocoding using a specific API.
   * @param coords Coordinates to lookup.
   * @param api_name Name of the API to use.
   * @param language_code Language code (default: en).
   * @return AddressResult The result.
   */
  AddressResult reverse_geocode(const Coordinates &coords,
                                const std::string &api_name,
                                const std::string &language_code = "en") const;

  /**
   * @brief Performs reverse geocoding with dual language support.
   * @param coords Coordinates to lookup.
   * @param api_name Name of the API to use.
   * @param user_lang User's preferred language.
   * @return AddressResult The result with dual language info.
   */
  AddressResult
  reverse_geocode_dual_language(const Coordinates &coords,
                                const std::string &api_name,
                                const std::string &user_lang) const;

  /**
   * @brief Performs reverse geocoding and returns JSON.
   * @param coords Coordinates to lookup.
   * @param api_name Name of the API to use.
   * @param lang_override Language override.
   * @return nlohmann::json JSON result.
   */
  nlohmann::json
  reverse_geocode_json(const Coordinates &coords, const std::string &api_name,
                       const std::string &lang_override = "") const;

  /**
   * @brief Performs reverse geocoding with fallback strategy.
   * @param coords Coordinates to lookup.
   * @param priority_list List of APIs to try in order.
   * @param lang_override Language override.
   * @return nlohmann::json JSON result.
   */
  nlohmann::json
  reverse_geocode_fallback(const Coordinates &coords,
                           const std::vector<std::string> &priority_list,
                           const std::string &lang_override = "") const;

  /**
   * @brief Performs batch reverse geocoding.
   * @param coords_list List of coordinates.
   * @param priority_list Priority list of APIs.
   * @param lang_override Language override.
   * @return std::vector<nlohmann::json> List of JSON results.
   */
  std::vector<nlohmann::json>
  batch_reverse_geocode(const std::vector<Coordinates> &coords_list,
                        const std::vector<std::string> &priority_list,
                        const std::string &lang_override = "") const;

private:
  std::unordered_map<std::string, ApiConfig> configs_;
  std::unordered_map<std::string, ApiAdapterPtr> adapters_;
  std::unique_ptr<HttpClient> http_client_;

  mutable QuotaManager quota_manager_;
};

} // namespace regeocode