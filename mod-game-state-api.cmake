#
# Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
# Released under GNU AGPL v3 License: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
#

# Module-specific configuration for mod-game-state-api
# This file ensures the module has access to utilities without requiring project-wide changes

# Add include directories for this module
target_include_directories(modules
  PUBLIC
    ${CMAKE_SOURCE_DIR}/src/common/Utilities
    ${CMAKE_CURRENT_LIST_DIR}/include)

message("  -> Game State API Module: Added utilities, nlohmann/json, and httplib include paths")
