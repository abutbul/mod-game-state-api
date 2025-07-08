/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "HttpGameStateServer.h"
#include "GameStateAPI.h"
#include "GameStateUtilities.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "WorldSession.h"
#include "WorldSessionMgr.h"
#include "World.h"
#include "GameTime.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

HttpGameStateServer::HttpGameStateServer(const std::string& host, uint16 port, const std::string& allowedOrigin)
    : _host(host), _port(port), _allowedOrigin(allowedOrigin), _running(false)
{
    _server = std::make_unique<httplib::Server>();

    // Set up CORS middleware for all requests
    _server->set_pre_routing_handler([this](const httplib::Request& /*req*/, httplib::Response& res) {
        SetCorsHeaders(res);
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // Handle OPTIONS requests for CORS preflight
    _server->Options(".*", [this](const httplib::Request& /*req*/, httplib::Response& res) {
        SetCorsHeaders(res);
        res.status = 200;
    });

    // API endpoints
    _server->Get("/api/health", [this](const httplib::Request& req, httplib::Response& res) {
        HandleHealthCheck(req, res);
    });

    _server->Get("/api/server", [this](const httplib::Request& req, httplib::Response& res) {
        HandleServerInfo(req, res);
    });

    _server->Get("/api/players", [this](const httplib::Request& req, httplib::Response& res) {
        HandleOnlinePlayers(req, res);
    });

    _server->Get("/api/player/([^/]+)", [this](const httplib::Request& req, httplib::Response& res) {
        HandlePlayerInfo(req, res);
    });

    _server->Get("/api/player/([^/]+)/stats", [this](const httplib::Request& req, httplib::Response& res) {
        HandlePlayerStats(req, res);
    });

    _server->Get("/api/player/([^/]+)/equipment", [this](const httplib::Request& req, httplib::Response& res) {
        HandlePlayerEquipment(req, res);
    });

    // Set up CORS and error handling
    _server->set_pre_routing_handler([](const httplib::Request& /*req*/, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return httplib::Server::HandlerResponse::Unhandled;
    });
}

HttpGameStateServer::~HttpGameStateServer()
{
    Stop();
}

bool HttpGameStateServer::Start()
{
    if (_running.load())
    {
        LOG_WARN("module.gamestate_api", "HTTP server is already running");
        return false;
    }

    _serverThread = std::make_unique<std::thread>([this]() {
        LOG_INFO("module.gamestate_api", "Starting HTTP server on {}:{}", _host, _port);
        _running.store(true);

        if (!_server->listen(_host, _port))
        {
            LOG_ERROR("module.gamestate_api", "Failed to start HTTP server on {}:{}", _host, _port);
            _running.store(false);
        }
        else
        {
            LOG_INFO("module.gamestate_api", "HTTP server stopped");
            _running.store(false);
        }
    });

    // Give the server a moment to start
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (_running.load())
    {
        LOG_INFO("module.gamestate_api", "Game State API HTTP server started successfully on {}:{}", _host, _port);
        return true;
    }
    else
    {
        LOG_ERROR("module.gamestate_api", "Failed to start Game State API HTTP server");
        if (_serverThread && _serverThread->joinable())
        {
            _serverThread->join();
        }
        return false;
    }
}

void HttpGameStateServer::Stop()
{
    if (!_running.load())
    {
        return;
    }

    LOG_INFO("module.gamestate_api", "Stopping HTTP server...");

    if (_server)
    {
        _server->stop();
    }

    if (_serverThread && _serverThread->joinable())
    {
        _serverThread->join();
    }

    _running.store(false);
    LOG_INFO("module.gamestate_api", "HTTP server stopped");
}

void HttpGameStateServer::HandleHealthCheck(const httplib::Request& /*req*/, httplib::Response& res)
{
    json response = {
        {"status", "ok"},
        {"timestamp", std::time(nullptr)},
        {"uptime_seconds", GameTime::GetUptime().count()}
    };

    SendJsonResponse(res, response.dump(2));
}

void HttpGameStateServer::HandleServerInfo(const httplib::Request& /*req*/, httplib::Response& res)
{
    try
    {
        json serverData = GameStateUtilities::GetServerData();
        SendJsonResponse(res, serverData.dump(2));
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("module.gamestate_api", "Error getting server info: {}", e.what());
        json error = {{"error", "Internal server error"}, {"status", 500}};
        SendJsonResponse(res, error.dump(2), 500);
    }
}

void HttpGameStateServer::HandleOnlinePlayers(const httplib::Request& req, httplib::Response& res)
{
    try
    {
        // Check for equipment parameter
        bool includeEquipment = req.has_param("equipment") && req.get_param_value("equipment") == "true";

        json playersData = GameStateUtilities::GetAllPlayersData(includeEquipment);

        json response = {
            {"count", playersData.size()},
            {"players", playersData}
        };

        SendJsonResponse(res, response.dump(2));
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("module.gamestate_api", "Error getting players list: {}", e.what());
        json error = {{"error", "Internal server error"}, {"status", 500}};
        SendJsonResponse(res, error.dump(2), 500);
    }
}

void HttpGameStateServer::HandlePlayerInfo(const httplib::Request& req, httplib::Response& res)
{
    std::string playerName = req.matches[1];

    if (playerName.empty())
    {
        SendErrorResponse(res, "Player name is required", 400);
        return;
    }

    // Find the player using GameStateUtilities
    Player* player = GameStateUtilities::FindPlayerByName(playerName);
    if (!player || !player->IsInWorld())
    {
        SendErrorResponse(res, "Player not found or not online", 404);
        return;
    }

    // Check if equipment should be included
    bool includeEquipment = req.has_param("include") &&
                           req.get_param_value("include").find("equipment") != std::string::npos;

    // Get player data using GameStateUtilities
    json playerJson = GameStateUtilities::GetPlayerData(player, includeEquipment);

    SendJsonResponse(res, playerJson.dump());
}

void HttpGameStateServer::HandlePlayerStats(const httplib::Request& req, httplib::Response& res)
{
    std::string playerName = req.matches[1];

    if (playerName.empty())
    {
        SendErrorResponse(res, "Player name is required", 400);
        return;
    }

    Player* player = GameStateUtilities::FindPlayerByName(playerName);
    if (!player || !player->IsInWorld())
    {
        SendErrorResponse(res, "Player not found or not online", 404);
        return;
    }

    json statsJson = GameStateUtilities::GetPlayerStats(player);
    SendJsonResponse(res, statsJson.dump());
}

void HttpGameStateServer::HandlePlayerEquipment(const httplib::Request& req, httplib::Response& res)
{
    std::string playerName = req.matches[1];

    if (playerName.empty())
    {
        SendErrorResponse(res, "Player name is required", 400);
        return;
    }

    Player* player = GameStateUtilities::FindPlayerByName(playerName);
    if (!player || !player->IsInWorld())
    {
        SendErrorResponse(res, "Player not found or not online", 404);
        return;
    }

    json equipmentJson = GameStateUtilities::GetPlayerEquipment(player);
    SendJsonResponse(res, equipmentJson.dump());
}

void HttpGameStateServer::SetCorsHeaders(httplib::Response& res)
{
    res.set_header("Access-Control-Allow-Origin", _allowedOrigin);
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    res.set_header("Access-Control-Max-Age", "86400");
}

void HttpGameStateServer::SendJsonResponse(httplib::Response& res, const std::string& json, int status)
{
    res.status = status;
    res.set_content(json, "application/json");
}

void HttpGameStateServer::SendErrorResponse(httplib::Response& res, const std::string& message, int status)
{
    json error = {
        {"error", message},
        {"timestamp", std::time(nullptr)}
    };
    SendJsonResponse(res, error.dump(), status);
}

