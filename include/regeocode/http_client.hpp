/**
 * SPDX-FileComment: Header file for the HTTP Client.
 * SPDX-FileType: SOURCE
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file http_client.hpp
 * @brief Simple HTTP client Wrapper.
 * @version 0.1.0
 * @date 2026-02-14
 *
 * @author ZHENG Robert
 * @license MIT License
 */

#pragma once

#include <string>

namespace regeocode {

/**
 * @brief Structure to hold HTTP response data.
 */
struct HttpResponse {
  long status_code{}; ///< HTTP status code (e.g., 200, 404).
  std::string body;   ///< Response body content.
};

/**
 * @brief Simple HTTP client class.
 */
class HttpClient {
public:
  /**
   * @brief Constructor.
   */
  HttpClient();

  /**
   * @brief Destructor.
   */
  ~HttpClient();

  /**
   * @brief Performs an HTTP GET request.
   *
   * @param url The URL to request.
   * @param timeout Timeout in seconds. Default is 10.
   * @return HttpResponse The response including status code and body.
   */
  virtual HttpResponse get(const std::string &url, long timeout = 10) const;
};

} // namespace regeocode
