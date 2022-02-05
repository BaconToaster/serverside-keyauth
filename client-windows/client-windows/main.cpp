#include "packet.hpp"
#include "logger.hpp"

constexpr const u_short port = 1337; // replace with your port
const std::string version = "1.0";
SOCKADDR sock_addr{};
int sock_len = sizeof(SOCKADDR);
uint64_t last_timestamp = 1337;
packet_data packet{};

std::string get_hwid() {
	ATL::CAccessToken access_token;
	ATL::CSid current_user_sid;
	if (access_token.GetProcessToken(TOKEN_READ | TOKEN_QUERY) && access_token.GetUser(&current_user_sid)) {
		return std::string(CT2A(current_user_sid.Sid()));
	}
	return "";
}

SOCKET connect_to_server() {
	SOCKADDR_IN address{};
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	sock_addr = *(SOCKADDR*)&sock_addr;
	inet_pton(AF_INET, "127.0.0.1", &address.sin_addr.s_addr); // replace with your server ip

	const SOCKET connection = socket(AF_INET, SOCK_STREAM, 0);
	if (connection == INVALID_SOCKET) {
		return INVALID_SOCKET;
	}

	if (connect(connection, (SOCKADDR*)&address, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		closesocket(connection);
		return INVALID_SOCKET;
	}

	return connection;
}

int main() {
	WSADATA wsa_data;
	if (WSAStartup(MAKEWORD(2, 2), &wsa_data)) { // we wanna use WinSock 2.2
		logger->error("failed to init WinSock 2.2");
		return -1;
	}

	const SOCKET sock = connect_to_server();
	if (sock == INVALID_SOCKET) {
		logger->error("failed to connect to server");
		return -1;
	}

	while (true) {
		memset(&packet, 0, sizeof(packet_data));
		if (recvfrom(sock, (char*)&packet, sizeof(packet_data), 0, &sock_addr, &sock_len) < 0) {
			logger->error("failed to receive packet from server");
			return -1;
		}

		// decrypt packet here

		if (packet.timestamp == last_timestamp || packet.timestamp == 0) {
			continue; // ignore own or NULL packet
		}

		switch (packet.action) {
		case packet_action::SEND_KEY_HWID: {
			std::string hwid = get_hwid();
			std::string key{};
			logger->info("please enter your key");
			std::cin >> key;

			packet = create_packet(packet_action::CLIENT_SUCCESS);
			last_timestamp = get_timestamp();
			packet.timestamp = last_timestamp;
			key_hwid_packet_buffer buf{};
			strcpy_s(buf.key, key.c_str());
			strcpy_s(buf.hwid, hwid.c_str());
			strcpy_s(buf.version, version.c_str());
			*(key_hwid_packet_buffer*)(packet.buffer) = buf;
			// encrypt packet here

			if (sendto(sock, (const char*)&packet, sizeof(packet_data), 0, &sock_addr, sock_len) < 0) {
				return -1;
			}

			break;
		}

		case packet_action::KEY_CHECK_FAILED: {
			invalid_key_packet_buffer buf = *(invalid_key_packet_buffer*)(packet.buffer);

			switch (buf.result) {
			case keyauth_result::OUTDATED: {
				logger->warning("outdated version");
				break;
			}

			case keyauth_result::INVALID: {
				logger->warning("invalid key");
				break;
			}

			case keyauth_result::FAILED: {
				logger->warning("internal server error");
				break;
			}

			default:
				logger->error("how did we get here?");
				return -1;
			}

			return -1;
		}

		case packet_action::OPEN_MESSAGEBOX: {
			MessageBoxA((HWND)(*(open_messagebox_packet_buffer*)(packet.buffer)).hWnd, (*(open_messagebox_packet_buffer*)(packet.buffer)).text,
				(*(open_messagebox_packet_buffer*)(packet.buffer)).caption, (*(open_messagebox_packet_buffer*)(packet.buffer)).type);

			packet = create_packet(packet_action::CLIENT_SUCCESS);
			last_timestamp = get_timestamp();
			packet.timestamp = last_timestamp;
			// encrypt packet here
			if (sendto(sock, (const char*)&packet, sizeof(packet_data), 0, &sock_addr, sock_len) < 0) {
				return -1;
			}
			break;
		}

		default: {
			logger->error("received invalid packet");
			return -1;
		}
		}
	}

	return 0;
}