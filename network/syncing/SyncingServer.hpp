#pragma once  // Header guard see https://en.wikipedia.org/wiki/Include_guard

// Copyright (c) 2023 Sri Lakshmi Kanthan P
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QSslConfiguration>
#include <QSslServer>
#include <QSslSocket>
#include <QVector>

#include "network/discovery/DiscoveryServer.hpp"
#include "types/enums/enums.hpp"
#include "utility/functions/ipconv.hpp"

namespace srilakshmikanthanp::clipbirdesk::network::syncing {
/**
 * @brief Syncing server that syncs the clipboard data between
 * the clients
 */
class SyncingServer : public discovery::DiscoveryServer {
 signals:    // signals
  /// @brief On client state changed
  void OnCLientStateChanged(QSslSocket *client, bool connected);

 signals:    // signals for this class
  /// @brief On Error Occurred
  void OnErrorOccurred(QString error);

 signals:    // signals for this class
  /// @brief On Sync Request
  void OnClintListChanged(QList<QSslSocket *> clients);

 signals:    // signals
  /// @brief On Sync Request
  void OnSyncRequest(QVector<QPair<QString, QByteArray>> items);

 private:    // just for Qt
  /// @brief Qt meta object
  Q_OBJECT

 public:     // Authenticator Type
  /// @brief Authenticator
  using Authenticator = bool (*)(QSslSocket *client);

 private:    // members of the class
  /// @brief SSL server
  QSslServer m_ssl_server = QSslServer(this);

  /// @brief List of clients
  QList<QSslSocket *> m_clients;

 private:    // Authenticator Instance
  /// @brief Authenticator
  Authenticator m_authenticator = nullptr;

 private:    // some typedefs
  using MalformedPacket = types::except::MalformedPacket;

 private:    // member functions

  /**
   * @brief Create the packet and send it to the client
   *
   * @param client Client to send
   * @param packet Packet to send
   */
  template <typename Packet>
  void sendPacket(QSslSocket *client, const Packet& pack) {
    client->write(utility::functions::toQByteArray(pak));
  }

  /**
   * @brief Create the packet and send it to all the
   * clients
   *
   * @param packet Packet to send
   */
  template <typename Packet>
  void sendPacket(const Packet& pack) {
    for (auto client: m_clients) {
      this->sendPacket(client, pack);
    }
  }

  /**
   * @brief Process the SyncingPacket from the client
   *
   * @param packet SyncingPacket
   */
  void processSyncingPacket(const packets::SyncingPacket& packet) {
    // Make the vector of QPair<QString, QByteArray>
    QVector<QPair<QString, QByteArray>> items;

    // Get the items from the packet
    #define STR(item) item.getMimeType().toStdString().c_str()
    for (auto item : packet.getItems()) {
      items.append({STR(item), item.getPayload()});
    }
    #undef  STR // just used to avoid the long line

    // Notify the listeners to sync the data
    emit OnSyncRequest(items);

    // send the packet to all the clients
    this->sendPacket(packet);
  }

  /**
   * @brief Callback function that process the ready
   * read from the client
   */
  void processReadyRead() {
    // Get the client that was ready to read
    auto client = qobject_cast<QSslSocket *>(sender());

    // Get the data from the client
    const auto data = client->readAll();

    // using the fromQByteArray from namespace
    using utility::functions::fromQByteArray;
    using utility::functions::createPacket;

    // Deserialize the data to SyncingPacket
    try {
      this->processSyncingPacket(fromQByteArray<packets::SyncingPacket>(data));
      return;
    } catch (const types::except::MalformedPacket &e) {
      const auto packType = packets::InvalidRequest::PacketType::RequestFailed;
      this->sendPacket(client, createPacket({packType, e.getCode(), e.what()}));
      return;
    } catch (const std::exception &e) {
      emit OnErrorOccurred(e.what());
      return;
    } catch (...) {
      emit OnErrorOccurred("Unknown Error");
      return;
    }
  }

  /**
   * @brief Process the connections that are pending
   */
  void processConnections() {
    while (m_ssl_server.hasPendingConnections()) {
      // Get the client that has been connected
      auto client_tcp = (m_ssl_server.nextPendingConnection());

      // Convert the client to SSL client
      auto client_tls = qobject_cast<QSslSocket *>(client_tcp);

      // If the client is not SSL client then disconnect
      if (!client_tls) {
        client_tcp->disconnectFromHost(); continue;
      }

      // Authenticate the client
      if (!m_authenticator(client_tls)) {
        client_tls->disconnectFromHost(); continue;
      }

      // Connect the client to the callback function that process
      // the disconnection when the client is disconnected
      // so the listener can be notified
      const auto signal_d = &QSslSocket::disconnected;
      const auto slot_d = &SyncingServer::processDisconnection;
      QObject::connect(client_tls, signal_d, this, slot_d);

      // Connect the socket to the callback function that
      // process the ready read when the socket is ready
      // to read so the listener can be notified
      const auto signal_r = &QSslSocket::readyRead;
      const auto slot_r = &SyncingServer::processReadyRead;
      QObject::connect(client_tls, signal_r, this, slot_r);

      // Notify the listeners that the client is connected
      emit OnCLientStateChanged(client_tls, true);

      // Add the client to the list of clients
      m_clients.append(client_tls);

      // Notify the listeners that the client list is changed
      emit OnClintListChanged(m_clients);
    }
  }

  /**
   * @brief Process the disconnection from the client
   */
  void processDisconnection() {
    // Get the client that was disconnected
    auto client = qobject_cast<QSslSocket *>(sender());

    // Remove the client from the list of clients
    m_clients.removeOne(client);

    // Notify the listeners that the client list is changed
    emit OnClintListChanged(m_clients);

    // Notify the listeners that the client is disconnected
    emit OnCLientStateChanged(client, false);
  }

 public:   // constructors and destructors
  /**
   * @brief Construct a new Syncing Server object and
   * bind to any available port and any available
   * IP address
   *
   * @param config SSL configuration
   * @param parent Parent object
   */
  #define PARAMS const QSslConfiguration &config, Authenticator auth, QObject *p = nullptr
  explicit SyncingServer(PARAMS) : DiscoveryServer(p) {
  #undef PARAMS // just used to avoid the long line
    // Connect the socket to the callback function that
    // process the connections when the socket is ready
    // to read so the listener can be notified
    const auto signal_c = &QSslServer::pendingConnectionAvailable;
    const auto slot_c = &SyncingServer::processConnections;
    QObject::connect(&m_ssl_server, signal_c, this, slot_c);

    // set the configuration For the SSL server
    m_ssl_server.setSslConfiguration(config);

    // Set the authenticator
    m_authenticator = auth;

    // bind the server to any available port and any available
    // IP address and start listening for connections
    m_ssl_server.listen(QHostAddress::Any);
  }

  /**
   * @brief Destroy the Syncing Server object
   */
  virtual ~SyncingServer() = default;

  /**
   * @brief Get the Clients that are connected to the server
   *
   * @return QList<QSslSocket*> List of clients
   */
  const QList<QSslSocket *> getConnectedClientsList() const {
    return m_clients;
  }

  /**
   * @brief Disconnect the client from the server and delete
   * the client
   *
   * @param client Client to disconnect
   */
  void disconnectClient(QSslSocket *client) {
    client->disconnectFromHost();
  }

  /**
   * @brief Request the clients to sync the clipboard items
   *
   * @param data QVector<QPair<QString, QByteArray>>
   */
  void syncItems(QVector<QPair<QString, QByteArray>> items) {
    const auto packType = packets::SyncingPacket::PacketType::SyncPacket;
    this->sendPacket(utility::functions::createPacket(packType, items));
  }

 protected:  // override functions from the base class

  /**
   * @brief Get the IP Type of the SyncingServer it can be IPv4 or
   * IPv6 the IP type is used to determine the length of the IP address
   * if the IP type is IPv4 then the IP address is 4 bytes long if
   * the IP type is IPv6 then the IP address is 16 bytes long
   *
   * @note The IP Type used in SyncingServer is IPv4
   *
   * @return types::IPType IP type
   * @throw Any Exception If any error occurs
   */
  virtual types::enums::IPType getIPType() const override {
    return types::enums::IPType::IPv4;
  }

  /**
   * @brief Get the Port number of the SyncingServer it can be any port
   * number from 0 to 65535 but the port number should be greater than 1024
   * because the port number less than 1024 are reserved for the system
   * services
   *
   * @return types::Port Port number
   * @throw Any Exception If any error occurs
   */
  virtual quint16 getPort() const override {
    return m_ssl_server.serverPort();
  }

  /**
   * @brief Get the IP Address of the SyncingServer it can be IPv4 or
   * IPv6 if the IP type is IPv4 then the IP address is 4 bytes long if
   * the IP type is IPv6 then the IP address is 16 bytes long
   *
   * @return types::IPAddress IP address
   * @throw Any Exception If any error occurs
   */
  virtual QHostAddress getIPAddress() const override {
    return m_ssl_server.serverAddress();
  }
};
}  // namespace srilakshmikanthanp::clipbirdesk::network::syncing