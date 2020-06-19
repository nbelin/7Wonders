#ifndef TCPSERVER_HPP
#define TCPSERVER_HPP

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QStringList>

//#include "player.hpp"
#include "card.hpp"
#include "boardview.hpp"
#include "action.hpp"
#include "choice.hpp"

typedef int PlayerId;

class TcpServer : public QObject
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);
    void sendDebug(const QString & message);
    void setPlayerId(const QTcpSocket * socket, PlayerId playerId);
    void showChoice(const Choice & choice);
    void showChoiceFace(PlayerId playerId, WonderId wonderId);
    void startGame();
    void showBoard(const BoardView & boardView);
    void showBoardToPlayer(PlayerId playerId, const BoardView & boardView);
    void showCardsToPlay(PlayerId playerId, int possibleActions, const QVector<CardId> & cards);
    void confirmAction(PlayerId playerId, bool valid, const QString & optMessage = QString());
    void gameOver();
    void sendMessage(const QString & message);
    void sendMessageToPlayer(PlayerId playerId, const QString & message);
    void sendMessageNotToPlayer(PlayerId playerId, const QString & message);

signals:
    void addPlayerName(const QTcpSocket * socket, const char * name);
    void setNumberAIs(int number);
    void setPlayerReady(PlayerId playerId, int ready);
    void askWonder(PlayerId playerId, WonderId wonderId);
    void movePlayerUp(int index);
    void movePlayerDown(int index);
    void setRandomWonders(bool value);
    void setRandomFaces(bool value);
    void setRandomPlaces(bool value);
    void askStartGame();
    void selectFace(PlayerId playerId, WonderFace face);
    void askAction(PlayerId playerId, const Action & action);

public slots:
    void newConnection();
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();

private:
    struct PlayerSocket {
        PlayerSocket(QTcpSocket * _socket) { socket = _socket; playerId = -1; }

        QTcpSocket * socket;
        PlayerId playerId;
    };

    QTcpServer server;
    QList<PlayerSocket> playerSockets;

    PlayerSocket getPlayerSocket(const QTcpSocket * socket);
    PlayerId getConnectedPlayer(const QTcpSocket * socket);
    QTcpSocket * getSocket(PlayerId playerId);
    void sendAll(const QString & message);
    void sendToPlayer(PlayerId playerId, const QString & message);

    void parsePlayerName(const QTcpSocket * socket, const QStringList & args);
    void parseNumberAIs(const QTcpSocket * socket, const QStringList & args);
    void parsePlayerReady(const QTcpSocket * socket, const QStringList & args);
    void parseAskWonder(const QTcpSocket * socket, const QStringList & args);
    void parseMovePlayerUp(const QTcpSocket * socket, const QStringList & args);
    void parseMovePlayerDown(const QTcpSocket * socket, const QStringList & args);
    void parseRandomWonders(const QTcpSocket * socket, const QStringList & args);
    void parseRandomFaces(const QTcpSocket * socket, const QStringList & args);
    void parseRandomPlaces(const QTcpSocket * socket, const QStringList & args);
    void parseAskGameStarts(const QTcpSocket * socket, const QStringList & args);
    void parseSelectFace(const QTcpSocket * socket, const QStringList & args);
    void parseAskAction(const QTcpSocket * socket, const QStringList & args);
};

#endif // TCPSERVER_H
