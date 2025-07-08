/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#ifndef GAME_STATE_API_H
#define GAME_STATE_API_H

#include "ScriptMgr.h"
#include "Config.h"
#include <string>
#include <memory>

class HttpGameStateServer;

// Game State API Module
class GameStateAPI : public WorldScript
{
public:
    GameStateAPI();
    ~GameStateAPI();

    void OnAfterConfigLoad(bool reload) override;
    void OnStartup() override;
    void OnShutdown() override;

private:
    std::unique_ptr<HttpGameStateServer> _httpServer;
    bool _enabled;
    std::string _host;
    uint16 _port;
    std::string _allowedOrigin;
};

#endif // GAME_STATE_API_H
