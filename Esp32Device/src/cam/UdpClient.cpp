#include "UdpClient.hpp"
#include <Arduino.h>
#include <errno.h>
#include <lwip/netdb.h>
#include <lwip/sockets.h>

simonCAMUDP::simonCAMUDP()
        : udp_server(-1), server_port(0), remote_port(0), remote_ip_int(0) {}

void simonCAMUDP::begin(IPAddress address, uint16_t port) {
    server_port = port;

    if ((udp_server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        Serial.printf("could not create socket: %d", errno);
        return;
    }

    int yes = 1;
    if (setsockopt(udp_server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) <
        0) {
        Serial.printf("could not set socket option: %d\n", errno);

        return;
    }

    struct sockaddr_in addr;
    memset((char *) &addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = (in_addr_t) address;
    if (bind(udp_server, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        Serial.printf("could not bind socket: %d\n", errno);
        return;
    }
    fcntl(udp_server, F_SETFL, O_NONBLOCK);
}

void simonCAMUDP::setServer(const char *host, uint16_t port) {
    struct hostent *server;
    server = gethostbyname(host);
    if (server == NULL) {
        Serial.printf("could not get host from dns: %d\n", errno);
        return;
    }
    setServer(IPAddress((const uint8_t *) (server->h_addr_list[0])), port);
}

void simonCAMUDP::setServer(IPAddress address, uint16_t port) {
    this->remote_ip = address;
    this->remote_ip_int = (uint32_t) remote_ip;
    this->remote_port = port;
}

void simonCAMUDP::send(uint8_t *buf, size_t len) {
    if (udp_server == -1) {
        if ((udp_server = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            Serial.printf("could not create socket: %d\n", errno);
            return;
        }
        fcntl(udp_server, F_SETFL, O_NONBLOCK);
    }

    struct sockaddr_in recipient;
    recipient.sin_addr.s_addr = remote_ip_int;
    recipient.sin_family = AF_INET;
    recipient.sin_port = htons(remote_port);
    int sent = sendto(udp_server, buf, len, 0, (struct sockaddr *) &recipient,
                      sizeof(recipient));
    if (sent < 0) {
        Serial.printf("could not send data: %d\n", errno);
    }
}


