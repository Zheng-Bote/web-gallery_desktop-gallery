// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2026 Zheng-Bote
//
// qt_gh-update-checker.hpp - GitHub Update Checker Library
//
// A lightweight, header-only C++ library for checking GitHub repository updates.
// Provides semantic versioning (SemVer) comparison and GitHub API integration.
//
// Features:
// - Parse and compare semantic versions (major.minor.patch)
// - Convert GitHub repository URLs to GitHub API endpoints
// - Synchronous HTTP GET requests for GitHub API
// - JSON parsing of GitHub release information
// - Automatic update detection
//
// Usage:
//   #include "qt_gh-update-checker.hpp"
//   auto info = qtgh::check_github_update(
//       "https://github.com/owner/repo", "1.0.0"
//   );
//   if (info.hasUpdate) {
//       std::cout << "Update available: " << info.latestVersion.toStdString();
//   }

#pragma once
#include <QString>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <stdexcept>

namespace qtgh {

// ---------------------------------------------------------
// SemVer - Semantic Versioning
// ---------------------------------------------------------
/// @brief Semantic version structure (major.minor.patch)
///
/// Represents a semantic version following the SemVer 2.0.0 specification.
/// Supports parsing from strings and provides comparison operators.
///
/// @example
///   SemVer v = SemVer::parse("1.2.3");
///   assert(v.major == 1 && v.minor == 2 && v.patch == 3);
struct SemVer {
    int major = 0;  ///< Major version number
    int minor = 0;  ///< Minor version number
    int patch = 0;  ///< Patch version number

    /// @brief Parse semantic version from string
    /// @param v Version string (e.g., "1.2.3", "v1.2", "1")
    /// @return Parsed SemVer structure
    /// @throws std::runtime_error if the version string is invalid
    /// @example
    ///   auto v = SemVer::parse("1.2.3");      // Works
    ///   auto v = SemVer::parse("v1.2");       // Works (patch defaults to 0)
    ///   auto v = SemVer::parse("invalid");    // Throws std::runtime_error
    static SemVer parse(const QString& v) {
        static QRegularExpression re(R"(v?(\d+)\.(\d+)(?:\.(\d+))?)");
        auto m = re.match(v);
        if (!m.hasMatch())
            throw std::runtime_error(("Invalid SemVer: " + v).toStdString());

        SemVer sv;
        sv.major = m.captured(1).toInt();
        sv.minor = m.captured(2).toInt();
        sv.patch = m.captured(3).isEmpty() ? 0 : m.captured(3).toInt();
        return sv;
    }

    /// @brief Three-way comparison operator
    /// Enables full comparison semantics: ==, <, >, <=, >=, !=
    auto operator<=>(const SemVer&) const = default;
};

// ---------------------------------------------------------
// GitHub URL Conversion
// ---------------------------------------------------------
/// @brief Convert GitHub repository URL to GitHub API endpoint
/// @param url Repository URL
///   - Web URL: https://github.com/owner/repo or https://github.com/owner/repo.git
///   - API URL: https://api.github.com/repos/owner/repo (returned as-is)
/// @return GitHub API URL for accessing the latest release
/// @throws std::runtime_error if the URL format is invalid
/// @example
///   auto api = toGithubApiUrl("https://github.com/nlohmann/json");
///   // Returns: "https://api.github.com/repos/nlohmann/json/releases/latest"
inline QString toGithubApiUrl(const QString& url) {
    if (url.contains("api.github.com"))
        return url;

    static QRegularExpression re(R"(https://github\.com/([^/]+)/([^/]+))");
    auto m = re.match(url);
    if (!m.hasMatch())
        throw std::runtime_error(("Invalid GitHub URL: " + url).toStdString());

    QString owner = m.captured(1);
    QString repo  = m.captured(2);

    if (repo.endsWith(".git"))
        repo.chop(4);

    return QStringLiteral("https://api.github.com/repos/%1/%2/releases/latest")
        .arg(owner, repo);
}

// ---------------------------------------------------------
// HTTP Utilities
// ---------------------------------------------------------
/// @brief Perform synchronous HTTP GET request
/// @param url Request URL
/// @return Response body as QByteArray
/// @throws std::runtime_error if the network request fails
///
/// This function performs a blocking HTTP GET request without requiring
/// QObject inheritance. It uses QEventLoop internally for synchronous operation.
/// The User-Agent header is set to "Qt-gh-update-checker".
///
/// @warning This blocks the current thread until the response is received.
/// Use asynchronous networking for GUI applications.
inline QByteArray http_get(const QString& url) {
    QNetworkAccessManager mgr;
    QNetworkRequest req{QUrl(url)};
    req.setHeader(QNetworkRequest::UserAgentHeader, "Qt-gh-update-checker");

    QEventLoop loop;
    QObject::connect(&mgr, &QNetworkAccessManager::finished,
                     &loop, &QEventLoop::quit);

    QNetworkReply* reply = mgr.get(req);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        auto msg = reply->errorString();
        reply->deleteLater();
        throw std::runtime_error(("Network error: " + msg).toStdString());
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();
    return data;
}

// ---------------------------------------------------------
// Update Information
// ---------------------------------------------------------
/// @brief Result of an update check operation
///
/// Contains information about the latest available version and whether
/// an update is recommended compared to the local version.
struct UpdateInfo {
    bool hasUpdate;           ///< True if a newer version is available
    QString latestVersion;    ///< Latest version tag from GitHub releases
};

// ---------------------------------------------------------
// Main API Function
// ---------------------------------------------------------
/// @brief Check for updates on a GitHub repository
/// @param repoUrl GitHub repository URL (https://github.com/owner/repo)
/// @param localVersion Current version string (e.g., "1.0.0")
/// @return UpdateInfo with latest version and update status
/// @throws std::runtime_error if:
///   - The repository URL is invalid
///   - The version strings cannot be parsed
///   - The GitHub API request fails
///   - The API response is invalid
///
/// This is the main entry point for update checking. It performs all
/// necessary operations: URL conversion, HTTP request, JSON parsing,
/// and version comparison.
///
/// @example
///   try {
///       auto info = qtgh::check_github_update(
///           "https://github.com/nlohmann/json",
///           "3.0.0"
///       );
///       if (info.hasUpdate) {
///           std::cout << "Update available: " << info.latestVersion.toStdString();
///       }
///   } catch (const std::exception& e) {
///       std::cerr << "Error: " << e.what();
///   }
inline UpdateInfo check_github_update(const QString& repoUrl,
                                      const QString& localVersion)
{
    QString apiUrl = toGithubApiUrl(repoUrl);

    QByteArray data = http_get(apiUrl);
    auto doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
        throw std::runtime_error("GitHub API returned non-object JSON");

    auto obj = doc.object();

    if (!obj.contains("tag_name") || !obj["tag_name"].isString()) {
        if (obj.contains("message") && obj["message"].isString()) {
            throw std::runtime_error(
                ("GitHub API error: " + obj["message"].toString()).toStdString()
            );
        }
        throw std::runtime_error("GitHub API returned no valid tag_name");
    }

    QString latest = obj["tag_name"].toString();

    SemVer local  = SemVer::parse(localVersion);
    SemVer remote = SemVer::parse(latest);

    return { remote > local, latest };
}

} // namespace qtgh
