/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "GameStateAPI.h"
#include "HttpGameStateServer.h"
#include "Log.h"
#include "Config.h"

GameStateAPI::GameStateAPI() : WorldScript("GameStateAPI"), _enabled(false), _port(8080)
{
}

GameStateAPI::~GameStateAPI()
{
    if (_httpServer)
    {
        _httpServer->Stop();
    }
}

void GameStateAPI::OnAfterConfigLoad(bool /*reload*/)
{
    _enabled = sConfigMgr->GetOption<bool>("GameStateAPI.Enable", false);
    _host = sConfigMgr->GetOption<std::string>("GameStateAPI.Host", "127.0.0.1");
    _port = static_cast<uint16>(sConfigMgr->GetOption<int32>("GameStateAPI.Port", 8080));
    _allowedOrigin = sConfigMgr->GetOption<std::string>("GameStateAPI.AllowedOrigin", "*");

    LOG_INFO("module.gamestate_api", "Game State API Module Configuration:");
    LOG_INFO("module.gamestate_api", "  Enabled: {}", _enabled ? "Yes" : "No");
    if (_enabled)
    {
        LOG_INFO("module.gamestate_api", "  Host: {}", _host);
        LOG_INFO("module.gamestate_api", "  Port: {}", _port);
        LOG_INFO("module.gamestate_api", "  Allowed Origin: {}", _allowedOrigin);
    }
}

void GameStateAPI::OnStartup()
{
    if (!_enabled)
    {
        LOG_INFO("module.gamestate_api", "Game State API Module is disabled");
        return;
    }

    LOG_INFO("module.gamestate_api", "Starting Game State API HTTP Server...");

    _httpServer = std::make_unique<HttpGameStateServer>(_host, _port, _allowedOrigin);

    if (_httpServer->Start())
    {
        LOG_INFO("module.gamestate_api", "Game State API HTTP Server started successfully on {}:{}", _host, _port);
    }
    else
    {
        LOG_ERROR("module.gamestate_api", "Failed to start Game State API HTTP Server");
        _httpServer.reset();
    }
}

void GameStateAPI::OnShutdown()
{
    if (_httpServer)
    {
        LOG_INFO("module.gamestate_api", "Stopping Game State API HTTP Server...");
        _httpServer->Stop();
        _httpServer.reset();
        LOG_INFO("module.gamestate_api", "Game State API HTTP Server stopped");
    }
}

// Register the script
void AddGameStateAPIScripts()
{
    new GameStateAPI();
}
