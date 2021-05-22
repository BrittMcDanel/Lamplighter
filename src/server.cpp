#include <stdio.h>
#include <inttypes.h>
#include <iostream>
#include <chrono>

#include "network.h"

int main(int argc, const char *argv[]) {
    GameClient client;
    GameServer server;

    int nPort = 6112;
    std::string serverIP;
    SteamNetworkingIPAddr addrServer;
    addrServer.Clear();

    if (argc > 1 && strcmp(argv[1], "server")) {
        serverIP = "127.0.0.1:6112";
    } else {
        //Hardcoded for now
        serverIP = "174.59.31.92:6112";
    }

    if (!addrServer.ParseString(serverIP.c_str())) {
        std::cout << "Invalid server address: " << serverIP << std::endl;
        return 1;
    }

    // Create client and server sockets
    InitSteamDatagramConnectionSockets();
    server.Start(nPort);
    client.Start(addrServer);

//    server.Update();
//    client.Update();
//    client.SendMessage("Sending: ");
//
//    while (true) {
//        server.Update();
//        client.Update();
//        std::this_thread::sleep_for(std::chrono::milliseconds(500));
//        for (const auto &message : client.GetMessages()) {
//            std::cout << message << std::endl;
//        }
//    }


    for (int i = 0; i < 10; i++) {
        server.Update();
        client.Update();

        client.SendMessage("Sending: " + std::to_string(i));

        for (const auto &message : client.GetMessages()) {
            std::cout << message << std::endl;
        }

        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    client.Stop();
    server.Stop();
    ShutdownSteamDatagramConnectionSockets();
    return 0;
}
