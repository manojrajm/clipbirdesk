#pragma once  // Header guard see https://en.wikipedia.org/wiki/Include_guard

// Copyright (c) 2023 Sri Lakshmi Kanthan P
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

// C++ headers
#include <algorithm>
#include <vector>

// Qt headers
#include <QByteArray>
#include <QHostAddress>
#include <QHostInfo>
#include <QObject>
#include <QTimer>
#include <QUdpSocket>
#include <QtLogging>

// KDE headers
#include <KDNSSD/PublicService>

// Local headers
#include "constants/constants.hpp"
#include "types/enums/enums.hpp"
#include "utility/functions/ipconv/ipconv.hpp"

namespace srilakshmikanthanp::clipbirdesk::network::service {
/**
 * @brief Abstract Discovery server that Listens for the client that send
 * the broadcast message The user of this class should implement the
 * getIpType(), getIPAddress() and getPort() functions to return the
 * IP type, IP address and port number respectively
 */
class Register : public QObject {
 private:  // typedefs for this class

  using IPType = types::enums::IPType;

 signals:  // signals for this class
  /// @brief On Error Occurred
  void OnErrorOccurred(QString error);

 signals:  // signals for this class
  /// @brief On Service Registered
  void OnServiceRegistered();

 private:  // Just for Qt

  Q_OBJECT

 private:  // variables

  KDNSSD::PublicService* service;

 private:  // disable copy and move

  Q_DISABLE_COPY_MOVE(Register)

 private:  // private functions

 public:  // public functions

  /**
   * @brief Construct a new Discovery Register object
   *
   * @param parent Parent object
   */
  explicit Register(QObject* parent = nullptr);

 protected:  // protected functions

  /**
   * @brief Destroy the Discovery Register object
   */
  virtual ~Register()                            = default;

  /**
   * @brief Get the IP Type of the server it can be IPv4 or IPv6
   * the IP type is used to determine the length of the IP address
   * if the IP type is IPv4 then the IP address is 4 bytes long if
   * the IP type is IPv6 then the IP address is 16 bytes long
   *
   * @note UNUSED ON Current Implementation
   * @return types::IPType IP type
   * @throw Any Exception If any error occurs
   */
  virtual types::enums::IPType getIPType() const = 0;

  /**
   * @brief Get the Port number of the server it can be any port number
   * from 0 to 65535 but the port number should be greater than 1024
   * because the port number less than 1024 are reserved for the system
   * services
   *
   * @return types::Port Port number
   * @throw Any Exception If any error occurs
   */
  virtual quint16 getPort() const                = 0;

  /**
   * @brief Get the IP Address of the server it can be IPv4 or IPv6
   * if the IP type is IPv4 then the IP address is 4 bytes long if
   * the IP type is IPv6 then the IP address is 16 bytes long
   *
   * @note UNUSED ON Current Implementation
   * @return types::IPAddress IP address
   * @throw Any Exception If any error occurs
   */
  virtual QHostAddress getIPAddress() const      = 0;

 public:

  /**
   * @brief Register the service
   *
   * @param callback Callback function to be called
   * when service Registered
   */
  virtual void registerServiceAsync();

  /**
   * @brief Stop the server
   */
  virtual void unregisterService();
};
}  // namespace srilakshmikanthanp::clipbirdesk::network::service