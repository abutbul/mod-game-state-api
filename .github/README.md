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
