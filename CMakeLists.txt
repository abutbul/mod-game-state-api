#
# Copyright (C) 2016+ AzerothCore <www.azerothcore.org>
# Released under GNU AGPL v3 License: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
#

# Add our module sources
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/GameStateAPI.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/HttpGameStateServer.cpp")
AC_ADD_SCRIPT("${CMAKE_CURRENT_LIST_DIR}/src/gs_loader.cpp")

message("  -> Prepared: Game State API Module")
