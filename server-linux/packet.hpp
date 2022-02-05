// share this file between client and server
#pragma once
#include "defs.hpp"

enum class packet_action : uint8_t {
    NONE = 0,
    SEND_KEY_HWID,
    KEY_CHECK_FAILED,
    CLIENT_SUCCESS,
    OPEN_MESSAGEBOX
};

enum class keyauth_result : uint8_t {
    VALID = 0,
    OUTDATED,
    INVALID,
    FAILED
};

struct packet_data { // max packet size is PAGE which is 0x1000 = 4096 bytes
    packet_action action{};
    uint64_t timestamp{}; // to make sure that the server or client didn't just receive its own packet
    char buffer[4087]{};
};

struct key_hwid_packet_buffer {
    char key[255];
    char hwid[255];
    char version[25];
};

struct invalid_key_packet_buffer {
    keyauth_result result;
};

struct open_messagebox_packet_buffer {
    void* hWnd{};
    char text[255]{};
    char caption[255]{};
    std::uint32_t type{};
};

inline const struct packet_data create_packet(const packet_action& action) {
    struct packet_data packet = {};
    packet.action = action;
    return packet;
}

inline uint64_t get_timestamp() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}