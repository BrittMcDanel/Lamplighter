#ifndef NETWORK_H
#define NETWORK_H

#include <algorithm>
#include <assert.h>
#include <cctype>
#include <chrono>
#include <map>
#include <mutex>
#include <queue>
#include <random>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <thread>

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

void InitSteamDatagramConnectionSockets();
void ShutdownSteamDatagramConnectionSockets();

struct ClientData {};

class GameServer {
public:
  void Start(uint16 nPort);
  void Update();
  void Stop();
  void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t *pInfo);

private:
  HSteamListenSocket m_hListenSock;
  HSteamNetPollGroup m_hPollGroup;
  ISteamNetworkingSockets *m_pInterface;
  std::map<HSteamNetConnection, ClientData> m_mapClients;
  SteamNetworkingConfigValue_t m_opt;
  SteamNetworkingIPAddr m_serverLocalAddr;
  bool m_running;

  void SendStringToClient(HSteamNetConnection conn, const char *str);
  void SendStringToAllClients(
      const char *str,
      HSteamNetConnection except = k_HSteamNetConnection_Invalid);
  void PollIncomingMessages();
  void PollConnectionStateChanges();
};

class GameClient {
public:
  void Start(const SteamNetworkingIPAddr &serverAddr);
  void Update();
  void Stop();
  void SendMessage(const std::string &message);
  void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t *pInfo);
  std::vector<std::string> GetMessages();
  uint32_t GetId() const { return m_hConnection; }

private:
  HSteamNetConnection m_hConnection;
  ISteamNetworkingSockets *m_pInterface;
  SteamNetworkingConfigValue_t m_opt;
  std::vector<std::string> m_local_messages;

  void PollConnectionStateChanges();
};

#endif // NETWORK_H
