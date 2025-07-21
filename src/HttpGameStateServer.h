/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#ifndef HTTP_GAME_STATE_SERVER_H
#define HTTP_GAME_STATE_SERVER_H

#include "Define.h"
#include <yhirose/httplib.h>
#include <string>
#include <memory>
#include <thread>
#include <atomic>

// Modern HTTP server using httplib.h
class HttpGameStateServer
{
public:
    HttpGameStateServer(const std::string& host, uint16 port, const std::string& allowedOrigin);
    ~HttpGameStateServer();

    bool Start();
    void Stop();
    bool IsRunning() const { return _running.load(); }

private:
    // REST API endpoint handlers
    void HandlePlayerInfo(const httplib::Request& req, httplib::Response& res);
    void HandlePlayerStats(const httplib::Request& req, httplib::Response& res);
    void HandlePlayerEquipment(const httplib::Request& req, httplib::Response& res);
    void HandlePlayerSkills(const httplib::Request& req, httplib::Response& res);
    void HandlePlayerSkillsFull(const httplib::Request& req, httplib::Response& res);
    void HandlePlayerQuests(const httplib::Request& req, httplib::Response& res);
    void HandleServerInfo(const httplib::Request& req, httplib::Response& res);
    void HandleOnlinePlayers(const httplib::Request& req, httplib::Response& res);
    void HandleHealthCheck(const httplib::Request& req, httplib::Response& res);

    // Utility methods
    void SetCorsHeaders(httplib::Response& res);
    void SendJsonResponse(httplib::Response& res, const std::string& json, int status = 200);
    void SendErrorResponse(httplib::Response& res, const std::string& message, int status = 400);

    std::string _host;
    uint16 _port;
    std::string _allowedOrigin;

    std::unique_ptr<httplib::Server> _server;
    std::unique_ptr<std::thread> _serverThread;
    std::atomic<bool> _running;
};

#endif // HTTP_GAME_STATE_SERVER_H
