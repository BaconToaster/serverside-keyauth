#pragma once
#include "keyauth.hpp"

class ClientHandler {
    private:
    int clientfd{};
    struct sockaddr cli_addr = {};
    socklen_t addr_len{};
    struct packet_data packet = {};

    public:
    std::string client_ip{};
    ClientHandler(const int& _clientfd, const std::string& _client_ip, const struct sockaddr& _cli_addr, const socklen_t& _addr_len) {
        clientfd = _clientfd;
        client_ip = _client_ip;
        cli_addr = _cli_addr;
        addr_len = _addr_len;
    }

    bool handle_packet() {
        uint64_t timestamp = get_timestamp();
        packet.timestamp = timestamp;
        // encrypt packet here
        if (sendto(clientfd, &packet, sizeof(packet_data), 0, &cli_addr, addr_len) < 0) {
            logger->warning("failed to send a packet to %s", client_ip.c_str());
            return false;
        }
        do {
            if (recvfrom(clientfd, &packet, sizeof(packet_data), 0, &cli_addr, &addr_len) < 0) {
                logger->warning("failed to receive a packet from %s", client_ip.c_str());
                return false;
            }
            // decrypt packet here
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } while (packet.timestamp == timestamp || packet.timestamp == 0); // ignore own and NULL packets
        if (packet.action != packet_action::CLIENT_SUCCESS) {
            // either an error occured while transferring the data or the packet has been modified
            logger->warning("didn't receive expected packet from %s", client_ip.c_str());
            return false;
        }
        return true;
    }

    bool login() {
        packet = create_packet(packet_action::SEND_KEY_HWID);
        if (!handle_packet()) {
            logger->warning("failed to receive key packet from %s", client_ip.c_str());
            return false;
        }
        struct key_hwid_packet_buffer buf = *(struct key_hwid_packet_buffer*)(packet.buffer);
        keyauth_result result = keyauth::check_user(buf.key, buf.hwid, buf.version);
        switch (result) {
            case keyauth_result::VALID: {
                client_ip += " (" + std::string(buf.key) + ")";
                logger->info("%s logged in", client_ip.c_str());
                return true;
            }

            default: {
                packet = create_packet(packet_action::KEY_CHECK_FAILED);
                struct invalid_key_packet_buffer out_buf = {};
                out_buf.result = result;
                *(struct invalid_key_packet_buffer*)(packet.buffer) = out_buf;
                uint64_t timestamp = get_timestamp();
                packet.timestamp = timestamp;
                // encrypt packet here
                if (sendto(clientfd, &packet, sizeof(packet_data), 0, &cli_addr, addr_len) < 0) {
                    logger->warning("failed to send key invalid packet to %s", client_ip.c_str());
                    return false;
                }
                return false;
            }
        }
        return false; // how did we get here?
    }

    bool start_application() {
        packet = create_packet(packet_action::OPEN_MESSAGEBOX);
        open_messagebox_packet_buffer buf{};
        buf.hWnd = 0;
        strcpy(buf.text, "Hello, World!");
        strcpy(buf.caption, "MessageBox");
        buf.type = 0; // MB_OK = 0
        *(open_messagebox_packet_buffer*)(packet.buffer) = buf;
        if (!handle_packet()) {
            logger->warning("failed to receive messagebox packet from %s", client_ip.c_str());
            return false;
        }
        return true;
    }
};