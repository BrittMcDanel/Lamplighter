#include <asio.hpp>
#include <iostream>

#include "network.h"

using asio::ip::tcp;

int main() {
  PeerConnection p = PeerConnection::createPeerConnnection("174.59.31.92", "6112");

  /*
  asio::error_code error;
  for (int i = 0; i < 20; i++) {
    std::array<uint8_t, 3> buf;
    buf.fill(0);
    buf[0] |= 1 << 7;
    asio::write(socket, asio::buffer(buf), error);

    size_t len = socket.read_some(asio::buffer(buf), error);
    uint16_t response = *reinterpret_cast<uint16_t*>(&buf.data()[0]);
    std::cout << response << std::endl;
  }
   */

  return 0;
}