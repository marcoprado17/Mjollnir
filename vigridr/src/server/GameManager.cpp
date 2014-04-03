#include "GameManager.h"

#include <stdexcept>

#include "GameConfig.h"
#include "GameType.h"

namespace mjollnir { namespace vigridr {

constexpr std::chrono::milliseconds kWorldModelUpdateMsStep1(2);
constexpr std::chrono::milliseconds kWorldModelUpdateMsStep2(1);

void PlayerTurnData::init(int32_t id) {
  setId(id);
  resetCommand();
}

void PlayerTurnData::setCommand(Command command, game_time time) {
  isCommandSet_ = true;
  lastUpdateTime_ = time;
  command_ = std::move(command);
}

void PlayerTurnData::resetCommand() {
  isCommandSet_ = false;
}

void PlayerTurnData::setIsTurn(bool isTurn) {
  isTurn_ = isTurn;
}

void GameManager::updateTime(const std::chrono::milliseconds& d) {
  nextUpdateTime_ = game_clock::now() + d;
  nextWorldModelTime_ = nextUpdateTime_ + config::updateTimeUpperBoundMs;
}

void GameManager::init() {
  std::unique_lock<std::mutex> lock0(playerMutex_[0], std::defer_lock);
  std::unique_lock<std::mutex> lock1(playerMutex_[1], std::defer_lock);
  std::unique_lock<std::mutex> lock2(gameInfoMutex_, std::defer_lock);
  std::lock(lock0, lock1, lock2);
  playerTurnData_[0].setIsTurn(true);
  playerTurnData_[1].setIsTurn(true);
  if (config::gameType == GameType::TURN) {
    playerTurnData_[1].setIsTurn(false);
  }
  gameInfo_.cycle = 0;
  updateTime(config::firstCicleWaitS);
}

void GameManager::nextTurn() {
  std::this_thread::sleep_until(nextWorldModelTime_ - kWorldModelUpdateMsStep1);
  std::unique_lock<std::mutex> lock0(playerMutex_[0], std::defer_lock);
  std::unique_lock<std::mutex> lock1(playerMutex_[1], std::defer_lock);
  std::unique_lock<std::mutex> lock2(gameInfoMutex_, std::defer_lock);
  std::lock(lock0, lock1, lock2);
  if (config::gameType == GameType::TURN) {
    for (size_t i = 0; i < kMaxPlayers; ++i) {
        playerTurnData_[i].setIsTurn(!playerTurnData_[i].isTurn());   
    } 
  }
  gameInfo_.cycle++;
  gameInfo_.gameStatus = GameStatus::RUNNING;
  std::this_thread::sleep_until(nextWorldModelTime_ - kWorldModelUpdateMsStep2);
  gameInfo_.worldModel = gameLogic_.getWorldModel();
  std::this_thread::sleep_until(nextWorldModelTime_);
  updateTime(config::cycleWaitMs);
}

void GameManager::execute(const PlayerTurnData& turnData) {
  if (!turnData.isTurn()) return;   
  gameLogic_.update(turnData.getCommand(), turnData.getId()); 
}

void GameManager::updaterTask() {
  while (true) {
    nextTurn();
    std::this_thread::sleep_until(nextUpdateTime_);
    std::cout << "Updater task" << std::endl;
    std::array<PlayerTurnData, kMaxPlayers> turns;
    {
      std::unique_lock<std::mutex> lock0(playerMutex_[0], std::defer_lock);
      std::unique_lock<std::mutex> lock1(playerMutex_[1], std::defer_lock);
      std::lock(lock0, lock1);
      if(playerTurnData_[0].isTurn() &&
         !playerTurnData_[0].isCommandSet()) {
        throw std::runtime_error("No command set for player 0\n");
        // TODO(luizmramos): handle this case
      } 
      if(playerTurnData_[1].isTurn() &&
         !playerTurnData_[1].isCommandSet()) {
        throw std::runtime_error("No command set for player 1\n");
        // TODO(luizmramos): handle this case
      }
      if (playerTurnData_[0].getLastUpdatedTime() < 
          playerTurnData_[1].getLastUpdatedTime()) {
        turns[0] = playerTurnData_[0];  // copy
        turns[1] = playerTurnData_[1];  // copy
      }
      else {
        turns[0] = playerTurnData_[1];  // copy
        turns[1] = playerTurnData_[0];  // copy
      }
      if(playerTurnData_[0].isTurn()) { playerTurnData_[0].resetCommand(); }
      if(playerTurnData_[1].isTurn()) { playerTurnData_[1].resetCommand(); }
    }
    for (auto& turn : turns) {
      execute(turn);
    }
    if (gameLogic_.isFinished()) {
      gameInfo_.gameStatus = GameStatus::FINISHED;
      gameInfo_.worldModel = gameLogic_.getWorldModel();
      break;
    }
  }
}

GameManager::GameManager(int32_t playerId0, int32_t playerId1) 
  : idToIdx_({{playerId0, 0}, {playerId1, 1}}),
    idxToId_({{playerId0, playerId1}}),
    gameLogic_(playerId0, playerId1) {
  playerTurnData_[0].init(playerId0);
  playerTurnData_[1].init(playerId1);
  gameInfo_.gameStatus = GameStatus::WAITING;
  init();
  updaterThread_ = std::thread([this]() { 
    updaterTask(); 
  });
}

CommandStatus GameManager::update(const Command& command, int32_t playerId) {
  auto it = idToIdx_.find(playerId);
  // TODO(luizmramos): CHECK
  if (it == idToIdx_.end()) {
    std::cout << "Unknown player." << std::endl;
    return CommandStatus::ERROR;
  }
  int32_t idx = it->second;
  // TODO(luizmramos): CHECK
  if (idx < 0 || idx > 1){
    std::cout << "Invalid idx." << std::endl;
    return CommandStatus::ERROR;
  }

  {
    std::unique_lock<std::mutex> lock(playerMutex_[idx]);
    playerTurnData_[idx].setCommand(command, game_clock::now());
  }
  return CommandStatus::SUCCESS;
}

void GameManager::getGameInfo(GameInfo& gameInfo, int32_t playerId) {
  std::unique_lock<std::mutex> lock(gameInfoMutex_);
  gameInfo = gameInfo_;
  gameInfo.updateTimeLimitMs = 
    std::chrono::duration_cast<std::chrono::milliseconds>(
      nextUpdateTime_-game_clock::now()).count();
  gameInfo.nextWorldModelTimeEstimateMs = 
    std::chrono::duration_cast<std::chrono::milliseconds>(
      nextWorldModelTime_-game_clock::now()).count();
  {
    auto it = idToIdx_.find(playerId);
    if (it == idToIdx_.end()) {
      throw std::runtime_error("Unknown player");
    }
    size_t idx = it->second;
    std::unique_lock<std::mutex> lock1(playerMutex_[idx], std::defer_lock);
    gameInfo.isMyTurn = playerTurnData_[idx].isTurn();
  }
  printf("%d %d\n", 
         gameInfo.updateTimeLimitMs, gameInfo.nextWorldModelTimeEstimateMs);
}  

}}

