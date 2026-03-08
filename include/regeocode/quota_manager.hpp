/**
 * SPDX-FileComment: Header file for the Quota Manager.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file quota_manager.hpp
 * @brief Manages API usage quotas.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#pragma once
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace regeocode {

/**
 * @brief Class to manage API request quotas.
 */
class QuotaManager {
public:
  /**
   * @brief Constructor.
   * @param state_file_path Path to the file storing quota state. Default is
   * "quota_status.json".
   */
  explicit QuotaManager(
      const std::string &state_file_path = "quota_status.json");

  /**
   * @brief Tries to reserve a request slot for a specific API.
   *
   * Checks if the daily limit for the given API has been reached.
   * If not, increments the counter and returns true.
   *
   * @param api_name Name of the API.
   * @param limit Daily limit for the API.
   * @return true If a slot was reserved.
   * @return false If the limit has been reached.
   */
  bool try_consume(const std::string &api_name, long limit);

private:
  void load();
  void save();
  std::string get_current_date() const;

  std::string file_path_;
  std::mutex mutex_; // Important for batch processing!

  // Structure: { "google": { "date": "2023-10-27", "count": 150 } }
  nlohmann::json state_;
};

} // namespace regeocode