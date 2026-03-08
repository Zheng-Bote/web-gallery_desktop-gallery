/**
 * SPDX-FileComment: Implementation of the Quota Manager.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file quota_manager.cpp
 * @brief Implementation of the Quota Manager.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#include "regeocode/quota_manager.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace regeocode {

QuotaManager::QuotaManager(const std::string &state_file_path)
    : file_path_(state_file_path) {
  load();
}

std::string QuotaManager::get_current_date() const {
  auto now = std::chrono::system_clock::now();
  auto in_time_t = std::chrono::system_clock::to_time_t(now);

  std::stringstream ss;
  ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
  return ss.str();
}

void QuotaManager::load() {
  std::ifstream f(file_path_);
  if (f.good()) {
    try {
      f >> state_;
    } catch (...) {
      state_ = nlohmann::json::object();
    }
  } else {
    state_ = nlohmann::json::object();
  }
}

void QuotaManager::save() {
  std::ofstream f(file_path_);
  f << state_.dump(4);
}

bool QuotaManager::try_consume(const std::string &api_name, long limit) {
  if (limit <= 0)
    return true; // 0 = unlimited

  std::lock_guard<std::mutex> lock(mutex_); // Make thread-safe

  std::string today = get_current_date();

  // Check if entry exists and if date is still correct
  if (!state_.contains(api_name) || state_[api_name]["date"] != today) {
    // New day or new entry -> Reset
    state_[api_name] = {{"date", today}, {"count", 0}};
  }

  long current_count = state_[api_name]["count"].get<long>();

  if (current_count >= limit) {
    return false; // Limit reached
  }

  // Increment and save immediately
  state_[api_name]["count"] = current_count + 1;
  save(); // Performance tip: With very high load maybe not save *every* time,
          // but ok for CLI.

  return true;
}

} // namespace regeocode