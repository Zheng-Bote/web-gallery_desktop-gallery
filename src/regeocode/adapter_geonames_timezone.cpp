/**
 * SPDX-FileComment: Implementation of the GeoNames Timezone API adapter.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file adapter_geonames_timezone.cpp
 * @brief Implementation of the GeoNames Timezone API adapter.
 * @version 0.2.0
 * @date 2026-03-10
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#include "regeocode/adapter_geonames_timezone.hpp"
#include <QDebug>
#include <nlohmann/json.hpp>
#include <sstream>

namespace regeocode {

AddressResult GeoNamesTimezoneAdapter::parse_response(
    const std::string &response_body) const {
  AddressResult res;
  res.raw_json = response_body;

  try {
    auto j = nlohmann::json::parse(response_body);

    // Error handling: GeoNames often returns a "status" object on errors
    if (j.contains("status")) {
      const auto &status = j["status"];
      std::string msg = status.value("message", "Unknown GeoNames error");
      int code = status.value("value", -1);
      qDebug() << "[ERROR] GeoNames API error (code " << code << "): " << msg;
      return res; // Return empty result on error
    }

    // Mapping fields
    if (j.contains("countryCode")) {
      res.country_code = j["countryCode"].get<std::string>();
    }

    // We use address_english for the timezone ID
    if (j.contains("timezoneId")) {
      res.address_english = j["timezoneId"].get<std::string>();
      res.attributes["timezone_id"] = res.address_english;
    }
    if (j.contains("time")) {
      res.address_local = j["time"].get<std::string>(); // For display
      res.attributes["local_time"] = res.address_local; // For JSON Data
    }
    if (j.contains("gmtOffset")) {
      // res.attributes["gmt_offset"] =
      //    std::to_string(j["gmtOffset"].get<double>());

      double offset = j["gmtOffset"].get<double>();
      // Format without trailing zeros: 5.5 → "5.5", 3.0 → "3"
      std::ostringstream oss;
      oss << offset;
      res.attributes["gmt_offset"] = oss.str();
    }

    // Optional: If countryName exists and we don't have an "address" yet,
    // we could append it. Here we append it to the timezone so the user
    // has more info.
    if (j.contains("countryName")) {
      std::string cname = j["countryName"].get<std::string>();
      if (!res.address_english.empty()) {
        res.address_english += " (" + cname + ")";
      } else {
        res.address_english = cname;
      }
    }

  } catch (const nlohmann::json::exception &) {
    // Ignore JSON errors, return empty result (or raw_json remains set)
  }

  return res;
}

} // namespace regeocode