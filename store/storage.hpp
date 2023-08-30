#pragma once  // Header guard see https://en.wikipedia.org/wiki/Include_guard

// Copyright (c) 2023 Sri Lakshmi Kanthan P
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <QObject>
#include <QSettings>

namespace srilakshmikanthanp::clipbirdesk::storage {
class Storage : public QObject {
 private:  // settings

  QSettings *settings = new QSettings("srilakshmikanthanp", "clipbird");

 private: // groups
  const char* clientGroup  = "client";
  const char* generalGroup = "general";
  const char* serverGroup  = "server";

 private: // keys
  const char* hostStateKey = "hostState";

 private:  // qt

  Q_OBJECT

 private:  // constructor

  /**
   * @brief Construct a new SQLStore object
   */
  Storage(QObject *parent = nullptr);

 private:  // constructor

  Q_DISABLE_COPY_MOVE(Storage)

 public:  // methods

  /**
   * @brief Destroy the SQLStore object
   */
  virtual ~Storage() = default;

  /**
   * @brief Store Client name and JWT cert
   */
  void setClientCert(const QString &name, const QByteArray &cert);

  /**
   * @brief has the cert for the name
   */
  bool hasClientCert(const QString &name);

  /**
  * @brief Clear the client cert
  */
  void clearClientCert(const QString &name);

  /**
   * @brief Clear the client cert
   */
  void clearAllClientCert();

  /**
   * @brief Get the JWT cert for the name
   */
  QByteArray getClientCert(const QString &name);

  /**
   * @brief Store the server name and JWT cert
   */
  void setServerCert(const QString &name, const QByteArray &cert);

  /**
   * @brief has the cert for the name
   */
  bool hasServerCert(const QString &name);

  /**
   * @brief Clear the server cert
   */
  void clearServerCert(const QString &name);

  /**
   * @brief Clear the server cert
   */
  void clearAllServerCert();

  /**
   * @brief Get the JWT cert for the name
   */
  QByteArray getServerCert(const QString &name);

  /**
   * @brief Set the current state of the server or client
   */
  void setHostIsServer(bool isServer);

  /**
   * @brief Get the current state of the server or client
   */
  bool getHostIsServer();

  /**
   * @brief Instance of the storage
   */
  static Storage& instance();
};

}  // namespace srilakshmikanthanp::clipbirdesk::storage
