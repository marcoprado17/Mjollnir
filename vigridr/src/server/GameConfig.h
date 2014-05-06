#ifndef VIGRIDR_SERVER_GAME_CONFIG
#define VIGRIDR_SERVER_GAME_CONFIG

#include <chrono>

#include "GameType.h"

namespace mjollnir { namespace vigridr { namespace config {
constexpr std::chrono::milliseconds cycleDurationMs(3);  // >= 3ms
constexpr std::chrono::seconds firstCicleDurationS(10);
constexpr std::chrono::milliseconds updateTimeUpperBoundMs(2);
constexpr GameType gameType = GameType::TURN;
}}}

#endif