/**
 * SPDX-FileComment: Implementation of the HTTP Client.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file http_client.cpp
 * @brief Implementation of the HTTP Client using libcurl.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#include "regeocode/http_client.hpp"
#include <curl/curl.h>
#include <string>

namespace regeocode {

// --- FIX START: Implement Constructor & Destructor ---
// These were missing and caused the linker error.
HttpClient::HttpClient() {
  // We could call curl_global_init here, but it's often optional
}

HttpClient::~HttpClient() {
  // We could do cleanup here
}
// --- FIX END ----------------------------------------------

// Callback must be defined before use
static size_t WriteCallback(void *contents, size_t size, size_t nmemb,
                            void *userp) {
  size_t total_size = size * nmemb;
  std::string *str = static_cast<std::string *>(userp);
  str->append(static_cast<char *>(contents), total_size);
  return total_size;
}

HttpResponse HttpClient::get(const std::string &url, long timeout) const {
  CURL *curl = curl_easy_init();
  HttpResponse response;
  response.status_code = 0;

  if (curl) {
    std::string response_string;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

    // Timeouts
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

    // User Agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "regeocode-cpp/1.0");

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
      response.status_code = 599; // 599 = Network Connect Timeout Error
      response.body = curl_easy_strerror(res);
    } else {
      long http_code = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
      response.status_code = static_cast<int>(http_code);
      response.body = response_string;
    }

    curl_easy_cleanup(curl);
  }
  return response;
}

} // namespace regeocode