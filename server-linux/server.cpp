#include "client_handler.hpp"

constexpr const unsigned int port = 1337; // you can change this to whatever you want
                                          // just make sure it matches the clients port

const std::string get_ip(const uint32_t& addr) {
    std::string out = "";
    for (uint64_t i = 0; i < sizeof(addr); i++) {
        out += std::to_string((uint32_t)(*(uint8_t*)((uint64_t)&addr + i)));
        if (i != sizeof(addr) - 1) {
            out += ".";
        }
    }
    return out;
}

void handle_client(const int clientfd, struct sockaddr cli_addr, socklen_t addr_len) {
    const std::string client_ip = get_ip((*(struct sockaddr_in*)&cli_addr).sin_addr.s_addr);
    logger->info("new client connected: %s", client_ip.c_str());

    ClientHandler client_handler = ClientHandler(clientfd, client_ip, cli_addr, addr_len);

    try {
        if (!client_handler.login()) {
            close(clientfd);
            return;
        }
    }
    catch (std::exception& e) {
        logger->error("failed to check user %s because %s", client_handler.client_ip.c_str(), e.what());
        close(clientfd);
        return;
    }

    try {
        if (!client_handler.start_application()) {
            close(clientfd);
            return;
        }
    }
    catch (std::exception& e) {
        logger->error("failed to start application main on %s because %s", client_handler.client_ip.c_str(), e.what());
        close(clientfd);
        return;
    }

    close(clientfd);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0); // linux socket api documentation
    if (sockfd < 0) {
        logger->error("couldn't create socket");
        return -1;
    }
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        logger->error("couldn't bind to port"); // something is probably already running on this port
        return -1;
    }
    if (listen(sockfd, 100) < 0) { // allow 100 clients at the same time
        logger->error("couldn't start listener");
        return -1;
    }

    logger->info("listening on port %lu", port);

    while (true) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);
        int clientfd = accept(sockfd, (struct sockaddr*)&cli_addr, &clilen);
        if (clientfd < 0) {
            logger->warning("failed to accept new client");
            continue;
        }
        std::thread(handle_client, clientfd, *(struct sockaddr*)&cli_addr, clilen).detach();
    }
    close(sockfd);
    return 0;
}