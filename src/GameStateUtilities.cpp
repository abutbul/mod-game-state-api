/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "GameStateUtilities.h"
#include "WorldSessionMgr.h"
#include "GameTime.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SharedDefines.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "ObjectMgr.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "World.h"
#include "Group.h"
#include "QuestDef.h"
#include "DBCStores.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <fmt/format.h>

namespace GameStateUtilities
{
    nlohmann::json GetItemData(Item* item)
    {
        nlohmann::json itemData = nlohmann::json::object();

        if (!item)
            return itemData;

        const ItemTemplate* itemTemplate = item->GetTemplate();
        if (!itemTemplate)
        {
            // Basic item data without template
            itemData = {
                {"entry", item->GetEntry()},
                {"count", item->GetCount()},
                {"durability", item->GetUInt32Value(ITEM_FIELD_DURABILITY)},
                {"max_durability", item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY)}
            };
            return itemData;
        }

        // Basic item information
        itemData["entry"] = item->GetEntry();
        itemData["count"] = item->GetCount();
        itemData["name"] = itemTemplate->Name1;
        itemData["quality"] = itemTemplate->Quality;
        itemData["item_level"] = itemTemplate->ItemLevel;
        itemData["required_level"] = itemTemplate->RequiredLevel;
        itemData["class"] = itemTemplate->Class;
        itemData["subclass"] = itemTemplate->SubClass;
        itemData["inventory_type"] = itemTemplate->InventoryType;
        itemData["durability"] = item->GetUInt32Value(ITEM_FIELD_DURABILITY);
        itemData["max_durability"] = item->GetUInt32Value(ITEM_FIELD_MAXDURABILITY);

        // Item stats
        nlohmann::json stats = nlohmann::json::array();
        for (uint32 i = 0; i < itemTemplate->StatsCount && i < MAX_ITEM_PROTO_STATS; ++i)
        {
            if (itemTemplate->ItemStat[i].ItemStatValue != 0)
            {
                stats.push_back({
                    {"type", itemTemplate->ItemStat[i].ItemStatType},
                    {"value", itemTemplate->ItemStat[i].ItemStatValue}
                });
            }
        }
        itemData["stats"] = stats;

        // Resistances
        itemData["resistances"] = {
            {"armor", itemTemplate->Armor},
            {"holy", itemTemplate->HolyRes},
            {"fire", itemTemplate->FireRes},
            {"nature", itemTemplate->NatureRes},
            {"frost", itemTemplate->FrostRes},
            {"shadow", itemTemplate->ShadowRes},
            {"arcane", itemTemplate->ArcaneRes}
        };

        // Weapon data if applicable
        if (itemTemplate->Class == ITEM_CLASS_WEAPON)
        {
            nlohmann::json weaponData = nlohmann::json::object();
            weaponData["delay"] = itemTemplate->Delay;
            weaponData["dps"] = itemTemplate->getDPS();

            nlohmann::json damages = nlohmann::json::array();
            for (uint32 i = 0; i < MAX_ITEM_PROTO_DAMAGES; ++i)
            {
                if (itemTemplate->Damage[i].DamageMin > 0 || itemTemplate->Damage[i].DamageMax > 0)
                {
                    damages.push_back({
                        {"min", itemTemplate->Damage[i].DamageMin},
                        {"max", itemTemplate->Damage[i].DamageMax},
                        {"type", itemTemplate->Damage[i].DamageType}
                    });
                }
            }
            weaponData["damages"] = damages;
            itemData["weapon_data"] = weaponData;
        }

        // Gem sockets
        nlohmann::json sockets = nlohmann::json::array();
        for (uint32 i = 0; i < MAX_ITEM_PROTO_SOCKETS; ++i)
        {
            if (itemTemplate->Socket[i].Color)
            {
                sockets.push_back({
                    {"color", itemTemplate->Socket[i].Color},
                    {"content", itemTemplate->Socket[i].Content}
                });
            }
        }
        if (!sockets.empty())
        {
            itemData["sockets"] = sockets;
            if (itemTemplate->socketBonus)
            {
                itemData["socket_bonus"] = itemTemplate->socketBonus;
            }
        }

        // Spells on item
        nlohmann::json spells = nlohmann::json::array();
        for (uint32 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (itemTemplate->Spells[i].SpellId > 0)
            {
                spells.push_back({
                    {"spell_id", itemTemplate->Spells[i].SpellId},
                    {"trigger", itemTemplate->Spells[i].SpellTrigger},
                    {"charges", itemTemplate->Spells[i].SpellCharges},
                    {"cooldown", itemTemplate->Spells[i].SpellCooldown}
                });
            }
        }
        if (!spells.empty())
        {
            itemData["spells"] = spells;
        }

        // Additional properties
        itemData["item_set"] = itemTemplate->ItemSet;
        itemData["bonding"] = itemTemplate->Bonding;
        itemData["stack_size"] = itemTemplate->GetMaxStackSize();
        itemData["sell_price"] = itemTemplate->SellPrice;
        itemData["buy_price"] = itemTemplate->BuyPrice;

        return itemData;
    }

    nlohmann::json GetPlayerEquipment(Player* player)
    {
        nlohmann::json equipment = nlohmann::json::object();

        if (!player)
            return equipment;

        // Equipment slot mappings using actual AzerothCore slot constants
        const std::map<uint8, std::string> slotNames = {
            {EQUIPMENT_SLOT_HEAD, "head"},
            {EQUIPMENT_SLOT_NECK, "neck"},
            {EQUIPMENT_SLOT_SHOULDERS, "shoulders"},
            {EQUIPMENT_SLOT_BODY, "body"},
            {EQUIPMENT_SLOT_CHEST, "chest"},
            {EQUIPMENT_SLOT_WAIST, "waist"},
            {EQUIPMENT_SLOT_LEGS, "legs"},
            {EQUIPMENT_SLOT_FEET, "feet"},
            {EQUIPMENT_SLOT_WRISTS, "wrists"},
            {EQUIPMENT_SLOT_HANDS, "hands"},
            {EQUIPMENT_SLOT_FINGER1, "finger1"},
            {EQUIPMENT_SLOT_FINGER2, "finger2"},
            {EQUIPMENT_SLOT_TRINKET1, "trinket1"},
            {EQUIPMENT_SLOT_TRINKET2, "trinket2"},
            {EQUIPMENT_SLOT_BACK, "back"},
            {EQUIPMENT_SLOT_MAINHAND, "mainhand"},
            {EQUIPMENT_SLOT_OFFHAND, "offhand"},
            {EQUIPMENT_SLOT_RANGED, "ranged"},
            {EQUIPMENT_SLOT_TABARD, "tabard"}
        };

        for (const auto& [slot, name] : slotNames)
        {
            if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
            {
                equipment[name] = GetItemData(item);
            }
            else
            {
                equipment[name] = nullptr;
            }
        }

        return equipment;
    }

    nlohmann::json GetPlayerStats(Player* player)
    {
        nlohmann::json stats = nlohmann::json::object();

        if (!player)
            return stats;

        // Basic character stats
        stats["level"] = player->GetLevel();
        stats["experience"] = {
            {"current", player->GetUInt32Value(PLAYER_XP)},
            {"next_level", player->GetUInt32Value(PLAYER_NEXT_LEVEL_XP)}
        };

        // Primary stats
        stats["attributes"] = {
            {"strength", player->GetStat(STAT_STRENGTH)},
            {"agility", player->GetStat(STAT_AGILITY)},
            {"stamina", player->GetStat(STAT_STAMINA)},
            {"intellect", player->GetStat(STAT_INTELLECT)},
            {"spirit", player->GetStat(STAT_SPIRIT)}
        };

        // Health and power
        stats["health"] = {
            {"current", player->GetHealth()},
            {"max", player->GetMaxHealth()}
        };

        // All power types
        stats["power"] = {
            {"mana", {
                {"current", player->GetPower(POWER_MANA)},
                {"max", player->GetMaxPower(POWER_MANA)}
            }},
            {"rage", {
                {"current", player->GetPower(POWER_RAGE)},
                {"max", player->GetMaxPower(POWER_RAGE)}
            }},
            {"energy", {
                {"current", player->GetPower(POWER_ENERGY)},
                {"max", player->GetMaxPower(POWER_ENERGY)}
            }},
            {"focus", {
                {"current", player->GetPower(POWER_FOCUS)},
                {"max", player->GetMaxPower(POWER_FOCUS)}
            }},
            {"happiness", {
                {"current", player->GetPower(POWER_HAPPINESS)},
                {"max", player->GetMaxPower(POWER_HAPPINESS)}
            }},
            {"runes", {
                {"current", player->GetPower(POWER_RUNE)},
                {"max", player->GetMaxPower(POWER_RUNE)}
            }},
            {"runic_power", {
                {"current", player->GetPower(POWER_RUNIC_POWER)},
                {"max", player->GetMaxPower(POWER_RUNIC_POWER)}
            }}
        };

        // Combat stats
        stats["combat"] = {
            {"attack_power", player->GetTotalAttackPowerValue(BASE_ATTACK)},
            {"ranged_attack_power", player->GetTotalAttackPowerValue(RANGED_ATTACK)},
            {"spell_power", player->GetBaseSpellPowerBonus()},
            {"critical_chance", {
                {"melee", player->GetFloatValue(PLAYER_CRIT_PERCENTAGE)},
                {"ranged", player->GetFloatValue(PLAYER_RANGED_CRIT_PERCENTAGE)},
                {"spell", player->GetFloatValue(PLAYER_SPELL_CRIT_PERCENTAGE1)}
            }},
            {"hit_chance", {
                {"melee", player->GetFloatValue(PLAYER_FIELD_MOD_TARGET_PHYSICAL_RESISTANCE)},
                {"spell", player->GetFloatValue(PLAYER_FIELD_MOD_TARGET_RESISTANCE)}
            }}
        };

        // Resistances
        stats["resistances"] = {
            {"armor", player->GetArmor()},
            {"holy", player->GetResistance(SPELL_SCHOOL_HOLY)},
            {"fire", player->GetResistance(SPELL_SCHOOL_FIRE)},
            {"nature", player->GetResistance(SPELL_SCHOOL_NATURE)},
            {"frost", player->GetResistance(SPELL_SCHOOL_FROST)},
            {"shadow", player->GetResistance(SPELL_SCHOOL_SHADOW)},
            {"arcane", player->GetResistance(SPELL_SCHOOL_ARCANE)}
        };

        // Status information
        stats["status"] = {
            {"alive", player->IsAlive()},
            {"in_combat", player->IsInCombat()},
            {"resting", player->HasPlayerFlag(PLAYER_FLAGS_RESTING)},
            {"ghost", player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)},
            {"player_vs_player", player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_PVP_TIMER)},
            {"away", player->isAFK()},
            {"dnd", player->isDND()}
        };

        // Average item level
        stats["average_item_level"] = player->GetAverageItemLevel();

        return stats;
    }

    nlohmann::json GetPlayerTalentInfo(Player* player)
    {
        nlohmann::json talents = nlohmann::json::object();

        if (!player)
            return talents;

        talents["active_spec"] = player->GetActiveSpec();
        talents["specs_count"] = player->GetSpecsCount();

        // Get talent points spent in each tree
        // Note: This is a simplified version, full talent tree data would require more complex implementation
        uint32 totalTalentPoints = player->CalculateTalentsPoints();
        uint32 freeTalentPoints = player->GetFreeTalentPoints();
        uint32 usedTalentPoints = (totalTalentPoints >= freeTalentPoints) ? (totalTalentPoints - freeTalentPoints) : 0;

        talents["talent_points"] = {
            {"available", freeTalentPoints},
            {"used", usedTalentPoints},
            {"total", totalTalentPoints}
        };

        return talents;
    }

    nlohmann::json GetPlayerData(Player* player, bool includeEquipment)
    {
        nlohmann::json data = nlohmann::json::object();

        if (!player)
            return data;

        WorldSession* session = player->GetSession();

        data["name"] = player->GetName();
        data["level"] = player->GetLevel();
        data["class"] = player->getClass();
        data["race"] = player->getRace();
        data["gender"] = player->getGender();
        data["guid"] = player->GetGUID().GetCounter();
        data["zone_id"] = player->GetZoneId();
        data["area_id"] = player->GetAreaId();
        data["map_id"] = player->GetMapId();
        data["online"] = true; // If we have a Player object, they're online

        // Account and session info
        if (session)
        {
            data["account_id"] = session->GetAccountId();
            data["account_name"] = session->GetPlayerName();
            data["latency"] = session->GetLatency();
            data["security_level"] = static_cast<uint32>(session->GetSecurity());
        }

        // Guild information
        if (Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId()))
        {
            data["guild"] = {
                {"id", player->GetGuildId()},
                {"name", guild->GetName()},
                {"rank", player->GetRank()}
            };
        }
        else
        {
            data["guild"] = nullptr;
        }

        // Additional character data
        data["money"] = player->GetMoney();
        data["played_time"] = {
            {"total", player->GetTotalPlayedTime()},
            {"level", player->GetLevelPlayedTime()}
        };
        data["honor_points"] = player->GetHonorPoints();
        data["arena_points"] = player->GetArenaPoints();

        // Position information
        data["position"] = {
            {"x", player->GetPositionX()},
            {"y", player->GetPositionY()},
            {"z", player->GetPositionZ()},
            {"orientation", player->GetOrientation()}
        };

        // Health and power
        data["health"] = {
            {"current", player->GetHealth()},
            {"max", player->GetMaxHealth()}
        };

        Powers primaryPower = player->getPowerType();
        data["power"] = {
            {"type", static_cast<uint32>(primaryPower)},
            {"current", player->GetPower(primaryPower)},
            {"max", player->GetMaxPower(primaryPower)}
        };

        // Group information
        if (Group* group = player->GetGroup())
        {
            data["group"] = {
                {"id", group->GetGUID().GetCounter()},
                {"leader_guid", group->GetLeaderGUID().GetCounter()},
                {"members_count", group->GetMembersCount()},
                {"is_leader", group->IsLeader(player->GetGUID())},
                {"is_assistant", group->IsAssistant(player->GetGUID())},
                {"loot_method", static_cast<uint32>(group->GetLootMethod())},
                {"is_raid", group->isRaidGroup()},
                {"is_bg_group", group->isBGGroup()},
                {"is_lfg_group", group->isLFGGroup()}
            };
        }
        else
        {
            data["group"] = nullptr;
        }

        // Get basic stats without detailed breakdown
        data["stats"] = {
            {"strength", player->GetStat(STAT_STRENGTH)},
            {"agility", player->GetStat(STAT_AGILITY)},
            {"stamina", player->GetStat(STAT_STAMINA)},
            {"intellect", player->GetStat(STAT_INTELLECT)},
            {"spirit", player->GetStat(STAT_SPIRIT)},
            {"average_item_level", player->GetAverageItemLevel()}
        };

        // Status flags
        data["status"] = {
            {"alive", player->IsAlive()},
            {"in_combat", player->IsInCombat()},
            {"ghost", player->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_GHOST)},
            {"resting", player->HasPlayerFlag(PLAYER_FLAGS_RESTING)},
            {"away", player->isAFK()},
            {"dnd", player->isDND()},
            {"gm", player->IsGameMaster()}
        };

        if (includeEquipment)
        {
            data["equipment"] = GetPlayerEquipment(player);
        }

        return data;
    }

    nlohmann::json GetServerData()
    {
        nlohmann::json data = nlohmann::json::object();

        Seconds uptime = GameTime::GetUptime();
        Seconds currentTime = GameTime::GetGameTime();
        Seconds startTime = GameTime::GetStartTime();

        data["uptime_seconds"] = uptime.count();
        data["current_time"] = currentTime.count();
        data["start_time"] = startTime.count();
        data["player_count"] = sWorldSessionMgr->GetPlayerCount();
        data["max_player_count"] = sWorldSessionMgr->GetMaxPlayerCount();
        data["active_sessions"] = sWorldSessionMgr->GetActiveSessionCount();
        data["queued_sessions"] = sWorldSessionMgr->GetQueuedSessionCount();
        data["total_sessions"] = sWorldSessionMgr->GetActiveAndQueuedSessionCount();

        // Format uptime as human-readable
        uint32 uptimeSeconds = static_cast<uint32>(uptime.count());
        uint32 days = uptimeSeconds / 86400;
        uint32 hours = (uptimeSeconds % 86400) / 3600;
        uint32 minutes = (uptimeSeconds % 3600) / 60;
        uint32 seconds = uptimeSeconds % 60;

        data["uptime_formatted"] = fmt::format("{}d {}h {}m {}s", days, hours, minutes, seconds);

        return data;
    }

    nlohmann::json GetAllPlayersData(bool includeEquipment)
    {
        nlohmann::json players = nlohmann::json::array();

        const auto& sessions = sWorldSessionMgr->GetAllSessions();
        for (const auto& [accountId, session] : sessions)
        {
            if (Player* player = session->GetPlayer())
            {
                // Only include players that are actually in world
                if (player->IsInWorld())
                {
                    players.push_back(GetPlayerData(player, includeEquipment));
                }
            }
        }

        return players;
    }

    Player* FindPlayerByName(const std::string& name)
    {
        // Use AzerothCore's ObjectAccessor for efficient player lookup
        return ObjectAccessor::FindPlayerByName(name);
    }

    nlohmann::json GetPlayerSkills(Player* player)
    {
        nlohmann::json skills = nlohmann::json::object();

        if (!player)
            return skills;

        // Get castable spells (non-passive spells)
        nlohmann::json castableSpells = nlohmann::json::array();

        const PlayerSpellMap& spellMap = player->GetSpellMap();
        // Get castable spells (non-passive spells)
        for (const auto& spellPair : spellMap)
        {
            uint32 spellId = spellPair.first;
            const PlayerSpell* playerSpell = spellPair.second;

            // Skip only removed spells
            if (playerSpell->State == PLAYERSPELL_REMOVED)
                continue;

            // Get spell info
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            // Skip passive spells - we only want castable abilities
            if (spellInfo->IsPassive())
                continue;

            nlohmann::json spellData = {
                {"spell_id", spellId},
                {"name", spellInfo->SpellName[0] ? spellInfo->SpellName[0] : "Unknown"},
                {"rank", spellInfo->Rank[0] ? spellInfo->Rank[0] : ""},
                {"school", spellInfo->SchoolMask},
                {"cast_time", spellInfo->CastTimeEntry ? spellInfo->CastTimeEntry->CastTime : 0},
                {"cooldown", spellInfo->RecoveryTime},
                {"range", spellInfo->RangeEntry ? spellInfo->RangeEntry->RangeMax[0] : 0.0f}
            };

            castableSpells.push_back(spellData);
        }

        skills["castable_spells"] = castableSpells;
        skills["spell_count"] = castableSpells.size();

        return skills;
    }

    nlohmann::json GetPlayerSkillsFull(Player* player)
    {
        nlohmann::json skills = nlohmann::json::object();

        if (!player)
            return skills;

        // Get all learned skills (the original implementation)
        nlohmann::json skillsArray = nlohmann::json::array();

        // Iterate through all skill lines that the player has
        for (uint32 i = 0; i < PLAYER_MAX_SKILLS; ++i)
        {
            uint32 skill = player->GetUInt32Value(PLAYER_SKILL_INFO_1_1 + i * 3);
            if (skill == 0)
                continue;

            uint16 skillId = SKILL_VALUE(skill);
            uint16 skillStep = SKILL_MAX(skill);

            if (skillId == 0)
                continue;

            // Get skill values
            uint16 skillValue = player->GetSkillValue(skillId);
            uint16 maxSkillValue = player->GetMaxSkillValue(skillId);
            uint16 pureSkillValue = player->GetPureSkillValue(skillId);
            int16 permBonus = player->GetSkillPermBonusValue(skillId);
            int16 tempBonus = player->GetSkillTempBonusValue(skillId);

            nlohmann::json skillData = {
                {"skill_id", skillId},
                {"skill_step", skillStep},
                {"current_value", skillValue},
                {"max_value", maxSkillValue},
                {"pure_value", pureSkillValue},
                {"permanent_bonus", permBonus},
                {"temporary_bonus", tempBonus}
            };

            // Add skill line name if available
            if (SkillLineEntry const* skillLineEntry = sSkillLineStore.LookupEntry(skillId))
            {
                skillData["name"] = skillLineEntry->name[0]; // Default locale
            }

            skillsArray.push_back(skillData);
        }

        skills["passive_skills"] = skillsArray;

        // Get talent information
        nlohmann::json talentsInfo = GetPlayerTalentInfo(player);
        skills["talents"] = talentsInfo;

        // Get castable spells
        nlohmann::json castableData = GetPlayerSkills(player);
        skills["castable_spells"] = castableData["castable_spells"];
        skills["spell_count"] = castableData["spell_count"];

        return skills;
    }

    nlohmann::json GetPlayerQuests(Player* player)
    {
        nlohmann::json questsData = nlohmann::json::object();

        if (!player)
            return questsData;

        nlohmann::json activeQuests = nlohmann::json::array();
        nlohmann::json completedQuests = nlohmann::json::array();

        // Get quest status map
        QuestStatusMap& questStatusMap = player->getQuestStatusMap();

        for (const auto& questStatusPair : questStatusMap)
        {
            uint32 questId = questStatusPair.first;
            const QuestStatusData& questStatus = questStatusPair.second;

            // Get quest template
            Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
            if (!quest)
                continue;

            nlohmann::json questData = {
                {"quest_id", questId},
                {"title", quest->GetTitle()},
                {"description", quest->GetDetails()},
                {"level", quest->GetQuestLevel()},
                {"min_level", quest->GetMinLevel()},
                {"quest_type", quest->GetType()},
                {"suggested_players", quest->GetSuggestedPlayers()},
                {"time_limit", quest->GetTimeAllowed()},
                {"is_daily", quest->IsDaily()},
                {"is_weekly", quest->IsWeekly()},
                {"is_repeatable", quest->IsRepeatable()},
                {"status", static_cast<uint32>(questStatus.Status)}
            };

            // Add quest objectives progress if quest is in progress
            if (questStatus.Status == QUEST_STATUS_INCOMPLETE)
            {
                nlohmann::json objectives = nlohmann::json::array();

                // Item objectives
                nlohmann::json itemObjectives = nlohmann::json::array();
                for (uint32 i = 0; i < QUEST_ITEM_OBJECTIVES_COUNT; ++i)
                {
                    if (quest->RequiredItemId[i] > 0)
                    {
                        itemObjectives.push_back({
                            {"item_id", quest->RequiredItemId[i]},
                            {"required_count", quest->RequiredItemCount[i]},
                            {"current_count", questStatus.ItemCount[i]}
                        });
                    }
                }

                // Creature/GameObject objectives
                nlohmann::json creatureObjectives = nlohmann::json::array();
                for (uint32 i = 0; i < QUEST_OBJECTIVES_COUNT; ++i)
                {
                    if (quest->RequiredNpcOrGo[i] != 0)
                    {
                        creatureObjectives.push_back({
                            {"npc_or_go_id", quest->RequiredNpcOrGo[i]},
                            {"required_count", quest->RequiredNpcOrGoCount[i]},
                            {"current_count", questStatus.CreatureOrGOCount[i]}
                        });
                    }
                }

                questData["item_objectives"] = itemObjectives;
                questData["creature_objectives"] = creatureObjectives;
                questData["explored"] = questStatus.Explored;
                questData["timer"] = questStatus.Timer;

                activeQuests.push_back(questData);
            }
            else if (questStatus.Status == QUEST_STATUS_COMPLETE)
            {
                // Quest is complete but not yet turned in
                questData["ready_to_turn_in"] = true;
                activeQuests.push_back(questData);
            }
            else if (questStatus.Status == QUEST_STATUS_REWARDED)
            {
                // Quest has been completed and turned in
                completedQuests.push_back(questData);
            }
        }

        questsData["active_quests"] = activeQuests;
        questsData["completed_quests"] = completedQuests;
        questsData["active_count"] = activeQuests.size();
        questsData["completed_count"] = completedQuests.size();

        return questsData;
    }
}
