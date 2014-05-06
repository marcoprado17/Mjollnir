import sys
import glob
import argparse
import time
from random import randint

sys.path.append('../server/gen-py')
sys.path.insert(0, glob.glob('../../third-parties/python/lib')[0])

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from GameServer import Game
from WorldModel.ttypes import Marker
from Command.ttypes import Command
from Command.ttypes import Coordinate
from GameModel.ttypes import GameStatus

def print_world_model(wm):
  for i in range(3):
    for j in range(3):
      toPrint = ""
      if wm.table[i][j] == Marker.X:
        toPrint = "X"
      elif wm.table[i][j] == Marker.O:
        toPrint = "O"
      else:
        toPrint = "-"

      print toPrint + " ",
    print
    print
  print

def play_turn(wm):
  command = Command(Coordinate())
  while True:
    x = randint(0,2)
    y = randint(0,2)
    if wm.table[x][y] == Marker.UNMARKED:
      command.coordinate.x = x
      command.coordinate.y = y
      break
  return command


def init():
  pass

def synchronize(t):
  if t>0:
    time.sleep(t/1000.0)

def play_game(client):
  init()
  gameInfo = client.ready()
  while True:
    synchronize(gameInfo.nextWorldModelTimeEstimateMs)
    gameInfo = client.getGameInfo()
    wm = gameInfo.worldModel
    print_world_model(wm)
    if gameInfo.gameStatus == GameStatus.FINISHED:
      break
    if gameInfo.isMyTurn:
      command = play_turn(wm)
      client.sendCommand(command)
    

def run_client():
  parser = argparse.ArgumentParser()
  parser.add_argument("--port", \
                      help="Port used to connect with the server.", \
                      type=int)
  args = parser.parse_args()
  if not args.port:
    args.port = 9090

  try:
    transport = TSocket.TSocket('localhost', args.port)
    transport = TTransport.TBufferedTransport(transport)
    protocol = TBinaryProtocol.TBinaryProtocol(transport)
    client = Game.Client(protocol)
    transport.open()
    play_game(client)
    transport.close()
  except Thrift.TException, tx:
    print 'ERROR: %s' % (tx.message)


if __name__ == "__main__":
  run_client()