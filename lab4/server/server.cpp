#include "server.h"
#include "protocol.h"
#include "matrix.h"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <memory>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

struct ClientState {
    std::atomic<int> status{static_cast<int>(ClientStatus::INIT)};
    std::vector<int> matrix;
    int num_threads = 1;
    int N = 0;
};

Server::Server(int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Socket creation failed\n";
        exit(1);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed\n";
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        exit(1);
    }

    std::cout << "Server listening on port " << port << "\n";
}

Server::~Server() {
    if (server_fd != -1) {
        close(server_fd);
    }
}

void Server::run() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd >= 0) {
            std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << "\n";
            std::thread(&Server::handle_client, this, client_fd).detach();
        }
    }
}

void Server::handle_client(int client_fd) {
    auto state = std::make_shared<ClientState>();

    while (true) {
        uint32_t len_n = 0;
        int n = recv(client_fd, &len_n, 4, MSG_WAITALL);
        if (n <= 0) break;
        uint32_t len = ntohl(len_n);

        if (len == 0) continue;

        std::vector<uint8_t> payload(len);
        n = recv(client_fd, payload.data(), len, MSG_WAITALL);
        if (n <= 0) break;

        uint8_t type = payload[0];

        switch (static_cast<CommandType>(type)) {
            case CommandType::CONFIG_AND_DATA:
                handle_config_and_data(client_fd, state, payload, len);
                break;
            case CommandType::START_COMPUTATION:
                handle_start_computation(client_fd, state);
                break;
            case CommandType::REQUEST_STATUS:
                handle_request_status(client_fd, state);
                break;
            default:
                std::cerr << "Unknown command type received: " << static_cast<int>(type) << "\n";
                break;
        }
    }
    close(client_fd);
}

void Server::handle_config_and_data(int client_fd, std::shared_ptr<ClientState> state, const std::vector<uint8_t>& payload, uint32_t len) {
    if (len >= 9) {
        uint32_t num_threads_n, N_n;
        std::memcpy(&num_threads_n, payload.data() + 1, 4);
        std::memcpy(&N_n, payload.data() + 5, 4);
        state->num_threads = ntohl(num_threads_n);
        state->N = ntohl(N_n);

        uint32_t expected_len = 9 + state->N * state->N * 4;
        if (len == expected_len) {
            state->matrix.resize(state->N * state->N);
            for (int i = 0; i < state->N * state->N; ++i) {
                uint32_t val_n;
                std::memcpy(&val_n, payload.data() + 9 + i * 4, 4);
                state->matrix[i] = ntohl(val_n);
            }
            uint32_t resp_len = htonl(1);
            uint8_t resp_type = static_cast<uint8_t>(CommandType::CONFIG_AND_DATA);
            send(client_fd, &resp_len, 4, 0);
            send(client_fd, &resp_type, 1, 0);
        } else {
            state->status = static_cast<int>(ClientStatus::ERROR);
        }
    }
}

void Server::handle_start_computation(int client_fd, std::shared_ptr<ClientState> state) {
    state->status = static_cast<int>(ClientStatus::RUNNING);
    std::thread([state]() {
        process_multi_thread(state->matrix, state->num_threads);
        state->status = static_cast<int>(ClientStatus::DONE);
    }).detach();

    uint32_t resp_len = htonl(1);
    uint8_t resp_type = static_cast<uint8_t>(CommandType::START_COMPUTATION);
    send(client_fd, &resp_len, 4, 0);
    send(client_fd, &resp_type, 1, 0);
}

void Server::handle_request_status(int client_fd, std::shared_ptr<ClientState> state) {
    int current_status = state->status.load();
    if (current_status == static_cast<int>(ClientStatus::DONE)) {
        uint32_t payload_size = 2 + 4 + state->matrix.size() * 4;
        uint32_t net_len = htonl(payload_size);
        send(client_fd, &net_len, 4, 0);

        uint8_t resp_type = static_cast<uint8_t>(CommandType::STATUS_RESPONSE);
        uint8_t status_byte = static_cast<int>(ClientStatus::DONE);
        uint32_t net_N = htonl(state->N);

        send(client_fd, &resp_type, 1, 0);
        send(client_fd, &status_byte, 1, 0);
        send(client_fd, &net_N, 4, 0);

        std::vector<uint32_t> net_matrix(state->matrix.size());
        for (size_t i = 0; i < state->matrix.size(); ++i) {
            net_matrix[i] = htonl(state->matrix[i]);
        }
        send(client_fd, net_matrix.data(), net_matrix.size() * 4, 0);
        state->status = static_cast<int>(ClientStatus::INIT);
    } else {
        uint32_t payload_size = 2;
        uint32_t net_len = htonl(payload_size);
        send(client_fd, &net_len, 4, 0);

        uint8_t resp_type = static_cast<uint8_t>(CommandType::STATUS_RESPONSE);
        uint8_t status_byte = current_status;
        send(client_fd, &resp_type, 1, 0);
        send(client_fd, &status_byte, 1, 0);
    }
}
