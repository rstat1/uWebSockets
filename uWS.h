#ifndef UWS_H
#define UWS_H

#include <cstddef>
#include <functional>
#include <string>

namespace uWS {

enum OpCode : unsigned char {
    TEXT = 1,
    BINARY = 2,
    PING = 9,
    PONG = 10
};

class Parser;

class Socket {
    friend class Server;
    friend class Parser;
private:
    void *socket;
    Socket(void *p) : socket(p) {}
    void write(char *data, size_t length, bool transferOwnership);
public:
    void fail(); // aka close
    void send(char *data, size_t length, OpCode opCode, size_t fakedLength = 0);
    void sendFragment(char *data, size_t length, OpCode opCode, size_t remainingBytes);
};

class Server
{
    friend class Parser;
    friend class Socket;
private:
    // internal callbacks
    static void onReadable(void *vp, int status, int events);
    static void onWritable(void *vp, int status, int events);
    static void onAcceptable(void *vp, int status, int events);
    static void internalFragment(Socket socket, const char *fragment, size_t length, OpCode opCode, bool fin, size_t remainingBytes);

    // external callbacks
    void (*connectionCallback)(Socket);
    void (*disconnectionCallback)(Socket);
    void (*fragmentCallback)(Socket, const char *, size_t, OpCode, bool, size_t);
    void (*messageCallback)(Socket socket, const char *message, size_t length, OpCode opCode);

    // buffers
    char *receiveBuffer, *sendBuffer;
    static const int BUFFER_SIZE = 307200,
                     SHORT_SEND = 4096;

    // accept poll
    void *server;
    void *listenAddr;

public:
    Server(int port);
    ~Server();
    void onConnection(void (*connectionCallback)(Socket));
    void onDisconnection(void (*disconnectionCallback)(Socket));
    void onFragment(void (*fragmentCallback)(Socket, const char *, size_t, OpCode, bool, size_t));
    void onMessage(void (*messageCallback)(Socket, const char *, size_t, OpCode));
    void run();
    void close();
    void upgrade(int fd, const char *secKey);
    static bool isValidUtf8(std::string &str);
};

}

#endif // UWS_H
