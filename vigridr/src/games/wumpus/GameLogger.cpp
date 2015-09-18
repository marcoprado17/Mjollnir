#include "GameLogger.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

namespace mjollnir { namespace vigridr {

std::vector<WorldModel> wmList;
std::vector<TotalWorldModel> twmList;
GameDescription gd1, gd2;
std::string player1Id, player2Id;

ptree createPt(const TotalWorldModel& twm) {
  ptree twmPt;
  std::string elemStr;
  std::string directionStr;

  ptree tablePt;
  for (auto line : twm.map) {
    ptree linePt;
    for (auto elem : line) {
      
      elemStr = "";
      if(elem.player) 
        elemStr = elemStr + "A";
      elemStr = elemStr + "-";
      if(elem.breeze)
        elemStr = elemStr + "B";
      elemStr = elemStr + "-";
      if(elem.gold)
        elemStr = elemStr + "G";
      if(elem.pit) 
        elemStr = elemStr + "P";
      elemStr = elemStr + "-";
      if(elem.stench)
        elemStr = elemStr + "S";
      elemStr = elemStr + "-";
      if(elem.wumpus)
        elemStr = elemStr + "W";
      
      linePt.push_back(std::make_pair("", ptree(elemStr)));
    }
    tablePt.push_back(std::make_pair("", linePt));
  }
  
  switch(twm.playerDirection){
    case UP:
      directionStr = "UP";
      break;
    case RIGHT:
      directionStr = "RIGHT";
      break;
    case DOWN:
      directionStr = "DOWN";
      break;
    case LEFT:
      directionStr = "LEFT";
      break;
  }

  twmPt.push_back(std::make_pair("world map",tablePt));
  twmPt.push_back(std::make_pair("player direction", ptree(directionStr)));

  return twmPt;
}

ptree createGameDescriptionPt() {
  ptree gdPt;
  gdPt.put(player1Id, gd1.playerType == PlayerType::PLAYER ? "PLAYER":"COMPUTER");
  gdPt.put(player2Id, gd2.playerType == PlayerType::PLAYER ? "PLAYER":"COMPUTER");
  return gdPt;
}
void GameLogger::logWorldModel(const WorldModel& wm, const TotalWorldModel& twm) {
  wmList.push_back(wm);
  twmList.push_back(twm);
}

void GameLogger::logGameDescription(const GameDescription& description1,
                                    const std::string& player1,
                                    const GameDescription& description2,
                                    const std::string& player2) {
  gd1 = description1;
  gd2 = description2;
  player1Id = player1;
  player2Id = player2;
}


void GameLogger::flushLog() {
  ptree twmListPt;

  for (auto twm : twmList) {
    twmListPt.push_back(std::make_pair("", createPt(twm)));
  }
  ptree gamePt;
  gamePt.push_back(std::make_pair("twmList", twmListPt));
  gamePt.push_back(std::make_pair("gameDescription",
                                  createGameDescriptionPt()));
  std::ofstream file;
  file.open("logs");
  write_json (file, gamePt, false);
  file.close();
}

}}