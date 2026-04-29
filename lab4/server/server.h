#pragma once

#include <memory>
#include <vector>
#include <cstdint>

struct ClientState;

class Server {
public:
    Server(int port);
    ~Server();
    void run();

private:
    int server_fd;

    void handle_client(int client_fd);
    void handle_config_and_data(int client_fd, std::shared_ptr<ClientState> state, const std::vector<uint8_t>& payload, uint32_t len);
    void handle_start_computation(int client_fd, std::shared_ptr<ClientState> state);
    void handle_request_status(int client_fd, std::shared_ptr<ClientState> state);
};
