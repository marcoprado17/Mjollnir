#ifndef VIGRIDR_SERVER_GAME_CONFIG
#define VIGRIDR_SERVER_GAME_CONFIG

#include <chrono>

#include "GameType.h"

namespace mjollnir { namespace vigridr { namespace config {
constexpr std::chrono::milliseconds cycleDurationMs(100);  // >= 3ms
constexpr std::chrono::seconds firstCicleDurationS(4);
constexpr std::chrono::milliseconds updateTimeUpperBoundMs(10);
constexpr GameType gameType = GameType::CONTINUOUS;
}}}

#endif