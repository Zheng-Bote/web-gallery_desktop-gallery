/**
 * SPDX-FileComment: Singleton class to manage the SQLite database connection
 * SPDX-FileType: HEADER
 * SPDX-FileContributor: ZHENG Robert
 * SPDX-FileCopyrightText: 2026 ZHENG Robert
 * SPDX-License-Identifier: MIT
 *
 * @file DatabaseManager.hpp
 * @brief Singleton class to manage the SQLite database connection.
 * @version 1.0.0
 * @date 2026-03-07
 *
 * @author ZHENG Robert (robert@hase-zheng.net)
 * @copyright Copyright (c) 2026 ZHENG Robert
 *
 * @license MIT License
 */
#pragma once
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString> // Important

/**
 * @class DatabaseManager
 * @brief Singleton class to manage the SQLite database connection.
 *
 * This class handles the initialization, access, and clean-up of the
 * database connection used throughout the application.
 */
class DatabaseManager {
public:
  /**
   * @brief Get the singleton instance of the DatabaseManager.
   * @return Reference to the singleton instance.
   */
  static DatabaseManager &instance();

  /**
   * @brief Initialize the database connection.
   * @return true if initialization was successful, false otherwise.
   */
  bool init();

  /**
   * @brief Get the current database connection.
   * @return Reference to the QSqlDatabase object.
   */
  QSqlDatabase &getDb();

  /**
   * @brief Get the file path of the database.
   * @return The absolute path to the database file.
   */
  QString getDatabasePath() const;

  /**
   * @brief Clear all data from the database.
   *
   * CAUTION: This operation is irreversible.
   */
  void clearDatabase();

private:
  DatabaseManager() = default;
  ~DatabaseManager() = default;

  QString m_dbPath;  ///< Internal storage for the database path
  QSqlDatabase m_db; ///< The Qt SQL database object
};