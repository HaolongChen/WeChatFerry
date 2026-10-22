#pragma once

#include <atomic>
#include <functional>
#include <thread>
#include <unordered_map>

#include <nng/nng.h>

#include "wcf.pb.h"

#include "message_handler.h"
#include "message_sender.h"

class RpcServer
{
public:
    static RpcServer &getInstance();
    static void destroyInstance();
    static void shutdown();

    int start(int port = RPC_DEFAULT_PORT, int *status = nullptr);
    int stop();

private:
    RpcServer(int port = RPC_DEFAULT_PORT, int *status = nullptr);
    ~RpcServer();
    RpcServer(const RpcServer &)            = delete;
    RpcServer &operator=(const RpcServer &) = delete;

    void run_rpc_server();
    void on_message_callback();
    bool start_message_listener(bool pyq, uint8_t *out, size_t *len);
    bool stop_message_listener(uint8_t *out, size_t *len);
    bool dispatcher(uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len);

    static std::string build_url(int port);

    using FunctionHandler = std::function<bool(const Request &, uint8_t *, size_t *)>;

    // 服务器默认端口号和绑定地址
    static constexpr int RPC_DEFAULT_PORT           = 10086;
    static constexpr const char *RPC_SERVER_ADDRESS = "tcp://0.0.0.0";

    // Status signal
    static constexpr const int START = 1 << 0;
    static constexpr const int RUN = 1 << 1;
    static constexpr const int STOP = 1 << 2;
    static constexpr const int STOP_MESSAGE_LISTENER = 1 << 3;
    static constexpr const int SHUTDOWN = 1 << 4;
    static constexpr const int DESTROY = 1 << 5;
    static constexpr const int ERRORS = 1 << 6;

    static void on_start() { if (status_) *status_ |= START; }
    static void on_run() { if (status_) *status_ |= RUN; }
    static void on_stop() { if (status_) *status_ |= STOP; }
    static void on_stop_message_listener() { if (status_) *status_ |= STOP_MESSAGE_LISTENER; }
    static void on_shutdown() { if (status_) *status_ |= SHUTDOWN; }
    static void on_destroy() { if (status_) *status_ |= DESTROY; }
    static void on_error() { if (status_) *status_ |= ERRORS; }

    int port_ = RPC_DEFAULT_PORT;
    static int* status_;
    std::atomic<bool> isRunning_ { false };
    std::thread cmdThread_;
    std::thread msgThread_;

    message::Handler &handler_;
    message::Sender &sender_;

    struct Deleter {
        void operator()(RpcServer *server) const { delete server; }
    };

    static std::unique_ptr<RpcServer, Deleter> instance_;
    static const std::unordered_map<Functions, FunctionHandler> rpcFunctionMap;
};
