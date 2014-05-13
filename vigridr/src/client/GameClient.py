import sys
import glob
import argparse
import time

sys.path.append('../thrifts/gen-py')
sys.path.insert(0, glob.glob('../../third-parties/python/lib')[0])

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

from ClientLogic import Solution
from GameServer import Game
from Command.ttypes import Command
from GameModel.ttypes import GameStatus
    
def synchronize(t):
    if t>0:
        time.sleep(t/1000.0)

def play_game(client):
    gameInit = client.ready()
    solution = Solution(gameInit)
    gameInfo = gameInit.gameInfo
    start_time = time.time();
    while True:
        processing_time_ms = 1000*(time.time() - start_time);
        synchronize(gameInfo.nextWorldModelTimeEstimateMs - processing_time_ms)
        gameInfo = client.getGameInfo()
        start_time = time.time();
        wm = gameInfo.worldModel
        if gameInfo.gameStatus == GameStatus.FINISHED:
            break
        if gameInfo.isMyTurn:
            command = solution.play_turn(wm)
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
