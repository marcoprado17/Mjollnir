#include "GameLogic.h"
#include <iostream>
#include <stdio.h>

namespace mjollnir { namespace vigridr {

GameLogic::GameLogic(int32_t playerId1, int32_t playerId2) {
  player1_ = playerId1;
  player2_ = playerId2;
  winner_ = std::to_string(kNoWinner);
  hasFinished_ = false;
  for (int32_t i = 0; i < kWidth; ++i) {
    for (int32_t j = 0; j < kHeight; ++j) {
      field_[i][j] = kEmpty;
    }
  }
  // two players
  for (size_t i = 0; i < 2; ++i) {
    worldModel_.players.push_back(Player());
    for (size_t j = 0; j < kpInit[i].size(); j++) {
      Coordinate pos;
      pos.x = kpInit[i][j].first;
      pos.y = kpInit[i][j].second;
      field_[pos.x][pos.y] = (i==0 ? playerId1 : playerId2);
      worldModel_.players[i].body.push_back(pos);
    }
  }
}

Coordinate GameLogic::getUpdatedHeadPosition(Coordinate pos, Direction dir) {
  Coordinate nextHeadPosition = pos;
  switch (dir) {
    case Direction::UP:
      nextHeadPosition.y++;
      break;
    case Direction::DOWN:
      nextHeadPosition.y--;
      break;
    case Direction::LEFT:
      nextHeadPosition.x--;
      break;
    case Direction::RIGHT:
      nextHeadPosition.x++;
      break;
  }
  return nextHeadPosition;
}

bool GameLogic::isValidCoordinate(Coordinate pos) {
  return pos.x < kWidth && pos.y < kHeight && pos.x >= 0 && pos.y >=0;
}

bool GameLogic::update(Command command, int32_t playerId) {
  size_t idx = 0;
  if(playerId == player1_){
    idx = 0;
  }
  else if(playerId == player2_){
    idx = 1;
  }
  else {
    return false;
  }

  Coordinate nextHeadPosition = getUpdatedHeadPosition(
    worldModel_.players[idx].body.back(), command.direction);
  worldModel_.players[idx].body.push_back(nextHeadPosition);
  
  if (isValidCoordinate(nextHeadPosition) &&
      field_[nextHeadPosition.x][nextHeadPosition.y] == kEmpty) {
    field_[nextHeadPosition.x][nextHeadPosition.y] = playerId;
  }
  else {
    if (winner_ == std::to_string(kNoWinner)) {
      winner_ = (player1_ == playerId) ? std::to_string(player2_) : std::to_string(player1_);
    }
    else {
      winner_ = std::to_string(kNoWinner);  // tie
    }
    hasFinished_ = true;
  }
  return true;
}

bool GameLogic::shouldPrintWorldModel(int32_t playerId){
  return true;
}

bool GameLogic::shouldIncrementCycle(int32_t playerId){
  return true;
}

WorldModel GameLogic::getWorldModel() {
  return worldModel_;
}

bool GameLogic::isFinished() {
  return hasFinished_;
}
  
std::string GameLogic::getWinner() {
  return winner_;
}

GameDescription GameLogic::getGameDescription(int32_t playerId) const {
  GameDescription gameDescription;
  gameDescription.field.width = kWidth;
  gameDescription.field.height = kHeight;
  gameDescription.myIndex = (playerId == player1_) ? 0 : 1;
  return gameDescription;
}

size_t GameLogic::getNumberOfPlayers() const {
  return numberOfPlayers_;
}

TotalWorldModel GameLogic::getTotalWorldModel() const {
  return twm_;
}

GameResult GameLogic::createGameResult(std::string result, int32_t id) {
  if (result == std::to_string(id)) {
    return GameResult::WON;
  }
  if (result == "-1") {
    return GameResult::TIED;
  }
  return GameResult::LOST;
}

}}  // namespaces
