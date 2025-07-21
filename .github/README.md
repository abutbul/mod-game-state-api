# AzerothCore Game State API Module

A modern HTTP REST API module for AzerothCore that provides real-time game state information using industry-standard libraries.

## Features

- **Modern HTTP Server**: Uses `httplib.h` for robust HTTP handling
- **JSON API**: Uses `nlohmann/json` for type-safe JSON operations
- **CORS Support**: Proper CORS headers for web applications
- **RESTful Endpoints**: Clean, documented API endpoints
- **Self-contained**: No external dependencies required
- **Detailed Equipment**: Full item information including stats, sockets, and spells
- **Comprehensive Stats**: Player attributes, combat stats, resistances, and more

## API Endpoints

### Health Check
```
GET /api/health
```
Returns server health and basic status information.

### Server Information
```
GET /api/server
```
Returns detailed server information including uptime and player counts.

### Online Players List
```
GET /api/players
```
Returns a list of all currently online players with basic information.

**Query Parameters:**
- `equipment=true` - Include detailed equipment information for all players

### Individual Player Information
```
GET /api/player/{playerName}
```
Returns detailed information about a specific player including stats, guild, group, and status.

**Query Parameters:**
- `include=equipment` - Include detailed player equipment information

### Player Statistics
```
GET /api/player/{playerName}/stats
```
Returns comprehensive player statistics including attributes, combat stats, resistances, and status information.

### Player Equipment
```
GET /api/player/{playerName}/equipment
```
Returns detailed player equipment information including item stats, durability, sockets, spells, and more.

---

### Player Skills and Talents
```
GET /api/player/{playerName}/skills
```
Returns detailed information about a player's learned skills and talents.

**Response Format:**
```json
{
  "skills": [
    {
      "skill_id": 171,
      "skill_step": 1,
      "current_value": 285,
      "max_value": 300,
      "pure_value": 285,
      "permanent_bonus": 0,
      "temporary_bonus": 0,
      "name": "Alchemy"
    }
    // ... more skills
  ],
  "talents": {
    "active_spec": 0,
    "specs_count": 1,
    "talent_points": {
      "available": 5,
      "used": 66,
      "total": 71
    }
  }
}
```

**Skills Data Fields:**
- `skill_id`: The unique identifier for the skill line
- `skill_step`: The skill step/tier
- `current_value`: Current skill value (including bonuses)
- `max_value`: Maximum skill value for current level (including bonuses)
- `pure_value`: Base skill value without bonuses
- `permanent_bonus`: Permanent bonus from items/talents
- `temporary_bonus`: Temporary bonus from buffs/effects
- `name`: Skill line name (if available)

**Talents Data Fields:**
- `active_spec`: Currently active talent specialization
- `specs_count`: Number of available talent specializations
- `talent_points`: Information about talent point allocation

---

### Player Quests
```
GET /api/player/{playerName}/quests
```
Returns detailed information about a player's active and completed quests.

**Response Format:**
```json
{
  "active_quests": [
    {
      "quest_id": 12345,
      "title": "The Test Quest",
      "description": "This is a test quest description.",
      "level": 70,
      "min_level": 68,
      "quest_type": 0,
      "suggested_players": 1,
      "time_limit": 0,
      "is_daily": false,
      "is_weekly": false,
      "is_repeatable": false,
      "status": 3,
      "item_objectives": [
        {
          "item_id": 12345,
          "required_count": 10,
          "current_count": 7
        }
      ],
      "creature_objectives": [
        {
          "npc_or_go_id": 25318,
          "required_count": 8,
          "current_count": 3
        }
      ],
      "explored": false,
      "timer": 0
    }
  ],
  "completed_quests": [
    {
      "quest_id": 54321,
      "title": "Completed Quest",
      "description": "This quest has been completed.",
      "level": 65,
      "min_level": 60,
      "quest_type": 0,
      "suggested_players": 1,
      "time_limit": 0,
      "is_daily": true,
      "is_weekly": false,
      "is_repeatable": false,
      "status": 6
    }
  ],
  "active_count": 1,
  "completed_count": 1
}
```

**Quest Data Fields:**
- `quest_id`: Unique quest identifier
- `title`: Quest title
- `description`: Quest description/details
- `level`: Quest level
- `min_level`: Minimum level required to take the quest
- `quest_type`: Quest type identifier
- `suggested_players`: Suggested number of players for the quest
- `time_limit`: Time limit for the quest (0 = no time limit)
- `is_daily`: Whether the quest is a daily quest
- `is_weekly`: Whether the quest is a weekly quest
- `is_repeatable`: Whether the quest can be repeated
- `status`: Quest status (3 = incomplete, 1 = complete, 6 = rewarded)

**Quest Objectives (for active quests only):**
- `item_objectives`: Required items for quest completion
- `creature_objectives`: Required creature kills or GameObject interactions
- `explored`: Whether exploration objective is complete
- `timer`: Current quest timer value

**Quest Status Values:**
- `0`: QUEST_STATUS_NONE
- `1`: QUEST_STATUS_COMPLETE (ready to turn in)
- `3`: QUEST_STATUS_INCOMPLETE (in progress)
- `5`: QUEST_STATUS_FAILED
- `6`: QUEST_STATUS_REWARDED (completed and turned in)

---

## Error Responses

All endpoints return appropriate HTTP status codes and error messages:

- `400 Bad Request`: When player name is missing
- `404 Not Found`: When player is not found or not online
- `500 Internal Server Error`: When an unexpected error occurs

**Error Response Format:**
```json
{
  "error": "Player not found or not online",
  "timestamp": 1642694400
}
```

---

## Usage Examples

### Get Player Skills
```bash
curl http://localhost:8080/api/player/PlayerName/skills
```

### Get Player Quests
```bash
curl http://localhost:8080/api/player/PlayerName/quests
```

---

## Integration Notes

- All endpoints require the player to be online
- Player names are case-sensitive
- The API supports CORS for web-based applications
- All responses are in JSON format with UTF-8 encoding
- The endpoints integrate with existing AzerothCore Player, Quest, and Skill systems

---

## Example Responses

### Health Check Response
```json
{
  "status": "ok",
  "timestamp": 1704729600,
  "uptime_seconds": 86400
}
```

### Server Information Response
```json
{
  "uptime_seconds": 86400,
  "current_time": 1704729600,
  "start_time": 1704643200,
  "player_count": 25,
  "max_player_count": 50,
  "active_sessions": 27,
  "queued_sessions": 0,
  "total_sessions": 27,
  "uptime_formatted": "1d 0h 0m 0s"
}
```

### Player Information Response
```json
{
  "name": "PlayerName",
  "guid": 12345,
  "level": 80,
  "race": 1,
  "class": 1,
  "gender": 0,
  "health": {
    "current": 25000,
    "max": 25000
  },
  "power": {
    "type": 0,
    "current": 15000,
    "max": 15000
  },
  "position": {
    "x": -8949.95,
    "y": -132.493,
    "z": 83.5312,
    "orientation": 0.0
  },
  "location": {
    "map_id": 0,
    "zone_id": 1519,
    "area_id": 5148
  },
  "guild": {
    "id": 1,
    "name": "Guild Name",
    "rank": "Member"
  },
  "group": {
    "id": 12345,
    "type": "party",
    "size": 3,
    "leader": "LeaderName"
  },
  "status": {
    "alive": true,
    "in_combat": false,
    "ghost": false,
    "resting": true,
    "away": false,
    "dnd": false,
    "gm": false
  },
  "money": 1000000
}
```

### Player Stats Response
```json
{
  "attributes": {
    "strength": 120,
    "agility": 150,
    "stamina": 200,
    "intellect": 80,
    "spirit": 90
  },
  "combat": {
    "attack_power": 2500,
    "ranged_attack_power": 1800,
    "spell_power": 1200,
    "critical_chance": {
      "melee": 15.5,
      "ranged": 12.3,
      "spell": 18.7
    },
    "hit_chance": {
      "melee": 8.0,
      "spell": 12.0
    }
  },
  "resistances": {
    "armor": 15000,
    "fire": 0,
    "nature": 0,
    "frost": 0,
    "shadow": 0,
    "arcane": 0,
    "holy": 0
  },
  "talents": {
    "used": 71,
    "free": 0,
    "total": 71
  },
  "status": {
    "alive": true,
    "in_combat": false,
    "resting": true,
    "ghost": false,
    "player_vs_player": false,
    "away": false,
    "dnd": false
  }
}
```

### Player Equipment Response
```json
{
  "items": [
    {
      "id": 12345,
      "name": "Epic Sword",
      "type": "weapon",
      "subtype": "sword",
      "equip_slot": "main_hand",
      "quality": "epic",
      "icon": "path/to/icon.png",
      "damage": {
        "min": 100,
        "max": 200
      },
      "speed": 2.5,
      "stats": {
        "strength": 10,
        "agility": 5
      },
      "sockets": [
        {
          "type": "red",
          "gem_id": 5678
        }
      ],
      "spells": [
        {
          "id": 6789,
          "name": "Flame Strike",
          "effect": "damage",
          "power": 50
        }
      ],
      "durability": {
        "current": 150,
        "max": 150
      }
    }
  ]
}
```

### Player Skills and Talents Response
```json
{
  "skills": [
    {
      "skill_id": 171,
      "skill_step": 1,
      "current_value": 285,
      "max_value": 300,
      "pure_value": 285,
      "permanent_bonus": 0,
      "temporary_bonus": 0,
      "name": "Alchemy"
    }
    // ... more skills
  ],
  "talents": {
    "active_spec": 0,
    "specs_count": 1,
    "talent_points": {
      "available": 5,
      "used": 66,
      "total": 71
    }
  }
}
```

### Player Quests Response
```json
{
  "active_quests": [
    {
      "quest_id": 12345,
      "title": "The Test Quest",
      "description": "This is a test quest description.",
      "level": 70,
      "min_level": 68,
      "quest_type": 0,
      "suggested_players": 1,
      "time_limit": 0,
      "is_daily": false,
      "is_weekly": false,
      "is_repeatable": false,
      "status": 3,
      "item_objectives": [
        {
          "item_id": 12345,
          "required_count": 10,
          "current_count": 7
        }
      ],
      "creature_objectives": [
        {
          "npc_or_go_id": 25318,
          "required_count": 8,
          "current_count": 3
        }
      ],
      "explored": false,
      "timer": 0
    }
  ],
  "completed_quests": [
    {
      "quest_id": 54321,
      "title": "Completed Quest",
      "description": "This quest has been completed.",
      "level": 65,
      "min_level": 60,
      "quest_type": 0,
      "suggested_players": 1,
      "time_limit": 0,
      "is_daily": true,
      "is_weekly": false,
      "is_repeatable": false,
      "status": 6
    }
  ],
  "active_count": 1,
  "completed_count": 1
}
```

## Configuration

Add these options to your `worldserver.conf`:

```ini
# Enable the Game State API
GameStateAPI.Enable = 1

# Server binding (default: 127.0.0.1)
GameStateAPI.Host = "0.0.0.0"

# Server port (default: 8080)
GameStateAPI.Port = 8080

# CORS allowed origin (default: *)
GameStateAPI.AllowedOrigin = "*"
```

## Technical Implementation

### Libraries Used
- **httplib.h**: Modern C++ HTTP server library (header-only)
- **nlohmann/json**: Industry-standard JSON library for C++ (header-only)

### Key Improvements
- **Type Safety**: No manual JSON string building
- **CORS Support**: Proper handling of cross-origin requests
- **Error Handling**: Structured error responses with HTTP status codes
- **Logging**: Comprehensive logging for debugging and monitoring
- **Thread Safety**: Proper thread management for HTTP server
- **RESTful Design**: Standard HTTP methods and response codes

### Benefits
- **Self-contained**: All dependencies included in module
- **Portable**: No system package requirements
- **Maintainable**: Clean, modern C++ code
- **Extensible**: Easy to add new endpoints
- **Production Ready**: Proper error handling and logging

## Building

This module is self-contained and requires no external dependencies. Simply enable it in your AzerothCore build configuration and compile normally.

## License

Released under GNU AGPL v3 License, same as AzerothCore.
