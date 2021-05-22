#include "network.h"

SteamNetworkingMicroseconds g_logTimeZero;

static void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType,
                        const char *pszMsg) {
  SteamNetworkingMicroseconds time =
      SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
  printf("%10.6f %s\n", time * 1e-6, pszMsg);
  fflush(stdout);
  if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
    fflush(stdout);
    fflush(stderr);
  }
}

static void FatalError(const char *fmt, ...) {
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char *nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

static void Printf(const char *fmt, ...) {
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char *nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
}

void InitSteamDatagramConnectionSockets() {
#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg))
    FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);
#else
  SteamDatagramClient_SetAppID(570); // Just set something, doesn't matter what
  // SteamDatagramClient_SetUniverse( k_EUniverseDev );

  SteamDatagramErrMsg errMsg;
  if (!SteamDatagramClient_Init(true, errMsg))
    FatalError("SteamDatagramClient_Init failed.  %s", errMsg);

  // Disable authentication when running with Steam, for this
  // example, since we're not a real app.
  //
  // Authentication is disabled automatically in the open-source
  // version since we don't have a trusted third party to issue
  // certs.
  SteamNetworkingUtils()->SetGlobalConfigValueInt32(
      k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1);
#endif

  g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

  SteamNetworkingUtils()->SetDebugOutputFunction(
      k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

void ShutdownSteamDatagramConnectionSockets() {
  // Give connections time to finish up.  This is an application layer protocol
  // here, it's not TCP.  Note that if you have an application and you need to
  // be more sure about cleanup, you won't be able to do this.  You will need to
  // send a message and then either wait for the peer to close the connection,
  // or you can pool the connection to see if any reliable data is pending.
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
  GameNetworkingSockets_Kill();
#else
  SteamDatagramClient_Kill();
#endif
}

static GameServer *s_pCallbackServerInstance;

static void SteamNetConnectionStatusChangedServerCallback(
    SteamNetConnectionStatusChangedCallback_t *pInfo) {
  s_pCallbackServerInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

void GameServer::Start(uint16 nPort) {
  s_pCallbackServerInstance = this;

  // Select instance to use.  For now we'll always use the default.
  // But we could use SteamGameServerNetworkingSockets() on Steam.
  m_pInterface = SteamNetworkingSockets();

  // Start listening
  m_serverLocalAddr.Clear();
  m_serverLocalAddr.m_port = nPort;
  m_opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void *)SteamNetConnectionStatusChangedServerCallback);
  m_hListenSock =
      m_pInterface->CreateListenSocketIP(m_serverLocalAddr, 1, &m_opt);
  if (m_hListenSock == k_HSteamListenSocket_Invalid)
    FatalError("Failed to listen on port %d", nPort);
  m_hPollGroup = m_pInterface->CreatePollGroup();
  if (m_hPollGroup == k_HSteamNetPollGroup_Invalid)
    FatalError("Failed to listen on port %d", nPort);
  Printf("Server listening on port %d\n", nPort);
  m_running = true;
}

void GameServer::Update() {
  if (!m_running) {
    return;
  }
  PollIncomingMessages();
  PollConnectionStateChanges();
}

void GameServer::Stop() {
  if (!m_running) {
    return;
  }

  // Close all the connections
  Printf("Closing connections...\n");
  for (auto it : m_mapClients) {
    // Send them one more goodbye message.  Note that we also have the
    // connection close reason as a place to send final data.  However,
    // that's usually best left for more diagnostic/debug text not actual
    // protocol strings.
    SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

    // Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
    // to flush this out and close gracefully.
    m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
  }
  m_mapClients.clear();

  m_pInterface->CloseListenSocket(m_hListenSock);
  m_hListenSock = k_HSteamListenSocket_Invalid;

  m_pInterface->DestroyPollGroup(m_hPollGroup);
  m_hPollGroup = k_HSteamNetPollGroup_Invalid;
}

void GameServer::SendStringToClient(HSteamNetConnection conn, const char *str) {
  m_pInterface->SendMessageToConnection(
      conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
}

void GameServer::SendStringToAllClients(const char *str,
                                        HSteamNetConnection except) {
  for (auto &c : m_mapClients) {
    if (c.first != except)
      SendStringToClient(c.first, str);
  }
}

void GameServer::PollIncomingMessages() {
  ISteamNetworkingMessage *pIncomingMsgs[32];
  int numMsgs =
      m_pInterface->ReceiveMessagesOnPollGroup(m_hPollGroup, pIncomingMsgs, 32);

  if (numMsgs == 0) {
    return;
  }

  if (numMsgs < 0) {
    FatalError("Error checking for messages");
    return;
  }
  Printf("Server: Got %d messages", numMsgs);

  for (auto i = 0; i < numMsgs; i++) {
    auto pIncomingMsg = pIncomingMsgs[i];
    for (auto &c : m_mapClients) {
      if (c.first != pIncomingMsg->m_conn) {
        m_pInterface->SendMessageToConnection(
            c.first, pIncomingMsg->m_pData, pIncomingMsg->m_cbSize,
            k_nSteamNetworkingSend_Reliable, nullptr);
      }
    }

    // We don't need this anymore.
    pIncomingMsg->Release();
  }
}

void GameServer::OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t *pInfo) {
  char temp[1024];

  // What's the state of the connection?
  switch (pInfo->m_info.m_eState) {
  case k_ESteamNetworkingConnectionState_None:
    // NOTE: We will get callbacks here when we destroy connections.  You can
    // ignore these.
    break;

  case k_ESteamNetworkingConnectionState_ClosedByPeer:
  case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
    // Ignore if they were not previously connected.  (If they disconnected
    // before we accepted the connection.)
    if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {

      // Locate the client.  Note that it should have been found, because this
      // is the only codepath where we remove clients (except on shutdown),
      // and connection change callbacks are dispatched in queue order.
      auto itClient = m_mapClients.find(pInfo->m_hConn);
      assert(itClient != m_mapClients.end());

      // Select appropriate log messages
      const char *pszDebugLogAction;
      if (pInfo->m_info.m_eState ==
          k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
        pszDebugLogAction = "problem detected locally";
        // sprintf(temp, "Alas, %s hath fallen into shadow.  (%s)",
        // itClient->second.m_sNick.c_str(),
        //        pInfo->m_info.m_szEndDebug);
      } else {
        // Note that here we could check the reason code to see if
        // it was a "usual" connection or an "unusual" one.
        pszDebugLogAction = "closed by peer";
        // sprintf(temp, "%s hath departed", itClient->second.m_sNick.c_str());
      }

      // Spew something to our own log.  Note that because we put their nick
      // as the connection description, it will show up, along with their
      // transport-specific data (e.g. their IP address)
      Printf("Connection %s %s, reason %d: %s\n",
             pInfo->m_info.m_szConnectionDescription, pszDebugLogAction,
             pInfo->m_info.m_eEndReason, pInfo->m_info.m_szEndDebug);

      m_mapClients.erase(itClient);

      // TODO: figure out what to do in this case for our game
      // Send a message so everybody else knows what happened
      // SendStringToAllClients(temp);
    } else {
      assert(pInfo->m_eOldState ==
             k_ESteamNetworkingConnectionState_Connecting);
    }

    // Clean up the connection.  This is important!
    // The connection is "closed" in the network sense, but
    // it has not been destroyed.  We must close it on our end, too
    // to finish up.  The reason information do not matter in this case,
    // and we cannot linger because it's already closed on the other end,
    // so we just pass 0's.
    m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
    break;
  }

  case k_ESteamNetworkingConnectionState_Connecting: {
    // This must be a new connection
    assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());

    Printf("Connection request from %s",
           pInfo->m_info.m_szConnectionDescription);

    // A client is attempting to connect
    // Try to accept the connection.
    if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
      // This could fail.  If the remote host tried to connect, but then
      // disconnected, the connection may already be half closed.  Just
      // destroy whatever we have on our side.
      m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
      Printf("Can't accept connection.  (It was already closed?)");
      break;
    }

    // Assign the poll group
    if (!m_pInterface->SetConnectionPollGroup(pInfo->m_hConn, m_hPollGroup)) {
      m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
      Printf("Failed to set poll group?");
      break;
    }

    // Add them to the client list, using std::map wacky syntax
    m_mapClients[pInfo->m_hConn];
    break;
  }

  case k_ESteamNetworkingConnectionState_Connected:
    // We will get a callback immediately after accepting the connection.
    // Since we are the server, we can ignore this, it's not news to us.
    break;

  default:
    // Silences -Wswitch
    break;
  }
}

void GameServer::PollConnectionStateChanges() { m_pInterface->RunCallbacks(); }

static GameClient *s_pCallbackClientInstance;

static void SteamNetConnectionStatusChangedClientCallback(
    SteamNetConnectionStatusChangedCallback_t *pInfo) {
  s_pCallbackClientInstance->OnSteamNetConnectionStatusChanged(pInfo);
}

void GameClient::Start(const SteamNetworkingIPAddr &serverAddr) {
  s_pCallbackClientInstance = this;

  // Select instance to use.  For now we'll always use the default.
  m_pInterface = SteamNetworkingSockets();

  // Start connecting
  char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
  serverAddr.ToString(szAddr, sizeof(szAddr), true);
  Printf("Connecting to game server at %s", szAddr);
  m_opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged,
               (void *)SteamNetConnectionStatusChangedClientCallback);
  m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr, 1, &m_opt);
  if (m_hConnection == k_HSteamNetConnection_Invalid)
    FatalError("Failed to create connection");
}

void GameClient::Update() { PollConnectionStateChanges(); }

void GameClient::Stop() {}

std::vector<std::string> GameClient::GetMessages() {
  std::vector<std::string> messages;

  // Handle local messages first
  for (auto& message : m_local_messages) {
    messages.push_back(message);
  }
  m_local_messages.clear();

  // Handle server messages
  ISteamNetworkingMessage *pIncomingMsgs[32];
  int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection,
                                                          pIncomingMsgs, 32);

  if (numMsgs == 0) {
    return messages;
  }

  if (numMsgs < 0) {
    FatalError("Error checking for messages");
    return messages;
  }

  for (auto i = 0; i < numMsgs; i++) {
    auto pIncomingMsg = pIncomingMsgs[i];
    std::string msg;
    msg.assign((const char *)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
    messages.push_back(msg);
    pIncomingMsg->Release();
  }

  return messages;
}

void GameClient::SendMessage(const std::string &message) {
  m_local_messages.push_back(message);
  m_pInterface->SendMessageToConnection(
      m_hConnection, message.c_str(), (uint32)message.length(),
      k_nSteamNetworkingSend_Reliable, nullptr);
}

void GameClient::OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t *pInfo) {
  assert(pInfo->m_hConn == m_hConnection ||
         m_hConnection == k_HSteamNetConnection_Invalid);

  // What's the state of the connection?
  switch (pInfo->m_info.m_eState) {
  case k_ESteamNetworkingConnectionState_None:
    // NOTE: We will get callbacks here when we destroy connections.  You can
    // ignore these.
    break;

  case k_ESteamNetworkingConnectionState_ClosedByPeer:
  case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
    // TODO: Signal failure somehow
    // g_bQuit = true;

    // Clean up the connection.  This is important!
    // The connection is "closed" in the network sense, but
    // it has not been destroyed.  We must close it on our end, too
    // to finish up.  The reason information do not matter in this case,
    // and we cannot linger because it's already closed on the other end,
    // so we just pass 0's.
    m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
    m_hConnection = k_HSteamNetConnection_Invalid;
    break;
  }

  case k_ESteamNetworkingConnectionState_Connecting:
    // We will get this callback when we start connecting.
    // We can ignore this.
    break;

  case k_ESteamNetworkingConnectionState_Connected:
    Printf("Connected to server OK");
    break;

  default:
    // Silences -Wswitch
    break;
  }
}

void GameClient::PollConnectionStateChanges() { m_pInterface->RunCallbacks(); }
