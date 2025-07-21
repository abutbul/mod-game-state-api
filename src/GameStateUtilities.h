/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#ifndef GAMESTATEAPI_GAMESTATESUTILITIES_H
#define GAMESTATEAPI_GAMESTATESUTILITIES_H

#include <nlohmann/json.hpp>

class Player;
class Item;

namespace GameStateUtilities
{
    // Get detailed item information as JSON
    nlohmann::json GetItemData(Item* item);

    // Get player equipment information as JSON (with detailed item stats)
    nlohmann::json GetPlayerEquipment(Player* player);

    // Get player statistics (health, mana, stats, resistances, etc.)
    nlohmann::json GetPlayerStats(Player* player);

    // Get comprehensive player data as JSON
    nlohmann::json GetPlayerData(Player* player, bool includeEquipment = false);

    // Get server state information as JSON
    nlohmann::json GetServerData();

    // Get all online players data as JSON array
    nlohmann::json GetAllPlayersData(bool includeEquipment = false);

    // Find a player by name
    Player* FindPlayerByName(const std::string& name);

    // Get player's talent specialization info
    nlohmann::json GetPlayerTalentInfo(Player* player);

    // Get player's skills and talents information
    nlohmann::json GetPlayerSkills(Player* player);

    // Get player's full skills and talents information (includes passive skills)
    nlohmann::json GetPlayerSkillsFull(Player* player);

    // Get player's active quests information
    nlohmann::json GetPlayerQuests(Player* player);
}

#endif // GAMESTATEAPI_GAMESTATESUTILITIES_H
