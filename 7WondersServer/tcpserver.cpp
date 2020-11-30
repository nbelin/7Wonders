#include "tcpserver.hpp"

#include "tcpcommon.hpp"

#include <iostream>

TcpServer::TcpServer(QObject *parent) : QObject(parent) {
    server.listen(QHostAddress::Any, TcpCommon::port);
    QObject::connect(&server, &QTcpServer::newConnection, this, &TcpServer::newConnection);
}


void TcpServer::sendDebug(const QString & message) {
    sendAll(TcpCommon::encodeSingle(TcpCommon::debug, message));
}


void TcpServer::setPlayerId(const QTcpSocket * socket, PlayerId playerId) {
    for (PlayerSocket & ps : playerSockets) {
        if (ps.socket == socket) {
            ps.playerId = playerId;
        }
    }
    sendToPlayer(playerId, TcpCommon::encodeSingle(TcpCommon::playerId, QString::number(playerId)));
}


void TcpServer::showChoice(const Choice & choice) {
    sendAll(TcpCommon::encodeSingle(TcpCommon::showChoice, choice.toString()));
}


void TcpServer::showChoiceFace(PlayerId playerId, WonderId wonderId) {
    sendToPlayer(playerId, TcpCommon::encodeSingle(TcpCommon::showChoiceFace, QString::number(wonderId).toStdString().c_str()));
}


void TcpServer::startGame() {
    sendAll(TcpCommon::encodeEmpty(TcpCommon::gameStarts));
}


void TcpServer::showBoard(const BoardView & boardView) {
    sendAll(TcpCommon::encodeSingle(TcpCommon::showBoard, boardView.toString()));
}


void TcpServer::showBoardToPlayer(PlayerId playerId, const BoardView & boardView) {
    sendToPlayer(playerId, TcpCommon::encodeSingle(TcpCommon::showBoard, boardView.toString()));
}


void TcpServer::showCardsToPlay(PlayerId playerId, int possibleActions, const QVector<CardId> & cards) {
    QStringList list;
    list.append(QString::number(possibleActions));
    for ( CardId cardId : cards ) {
        list.append(QString::number(cardId));
    }
    sendToPlayer(playerId, TcpCommon::encodeMulti(TcpCommon::cardsToPlay, list));
}


void TcpServer::confirmAction(PlayerId playerId, bool valid, const QString & optMessage) {
    QStringList list;
    list.append(QString::number(valid));
    list.append(optMessage);
    sendToPlayer(playerId, TcpCommon::encodeMulti(TcpCommon::confirmAction, list));
}


void TcpServer::gameOver() {
    sendAll(TcpCommon::encodeEmpty(TcpCommon::gameOver));
}


void TcpServer::sendMessage(const QString & color, const QString & message) {
    QStringList list;
    list.append(color);
    list.append(message);
    sendAll(TcpCommon::encodeMulti(TcpCommon::userMessage, list));
}


void TcpServer::sendMessageToPlayer(PlayerId playerId, const QString & color, const QString & message) {
    QStringList list;
    list.append(color);
    list.append(message);
    sendToPlayer(playerId, TcpCommon::encodeMulti(TcpCommon::userMessage, list));
}


void TcpServer::sendMessageNotToPlayer(PlayerId playerId, const QString & color, const QString & message) {
    QStringList list;
    list.append(color);
    list.append(message);
    const QString & str = TcpCommon::encodeMulti(TcpCommon::userMessage, list);
    for (const PlayerSocket & ps : playerSockets) {
        if (ps.playerId != playerId) {
            sendToPlayer(ps.playerId, str);
        }
    }
}


void TcpServer::newConnection()
{
    QTcpSocket * clientSocket = server.nextPendingConnection();
    QObject::connect(clientSocket, &QTcpSocket::readyRead, this, &TcpServer::readyRead);
    QObject::connect(clientSocket, &QTcpSocket::stateChanged, this, &TcpServer::socketStateChanged);

    playerSockets.push_back(PlayerSocket(clientSocket));
    sendDebug(clientSocket->peerAddress().toString() + " connected to server!");
}


void TcpServer::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    if (socketState == QAbstractSocket::UnconnectedState)
    {
        QTcpSocket * sender = static_cast<QTcpSocket*>(QObject::sender());
        PlayerSocket ps = getPlayerSocket(sender);
        playerSockets.removeOne(ps);
        if (playerSockets.empty()) {
            std::cout << "No more active connexion" << std::endl;
            exit(EXIT_FAILURE);
        }
        //sendMessage(Colors::black, QString::number(ps.playerId) + " disconnected");
        playerDisconnected(ps.playerId);
    }
}


void TcpServer::readyRead()
{
    QTcpSocket * sender = static_cast<QTcpSocket*>(QObject::sender());
    QByteArray datas = sender->readAll();
    sendAll(datas);

    QStringList messages = TcpCommon::decodeMessages(datas);
    sendDebug(std::to_string(messages.size()).c_str());

    for (const QString & message : messages) {
        QStringList strList = message.split(TcpCommon::separator);
        const QString & keyword = strList[0];
        QStringList args(strList);
        args.removeFirst();

        if (keyword == TcpCommon::playerName) {
            parsePlayerName(sender, args);
        } else if (keyword == TcpCommon::numberAIs) {
            parseNumberAIs(sender, args);
        } else if (keyword == TcpCommon::playerReady) {
            parsePlayerReady(sender, args);
        } else if (keyword == TcpCommon::askWonder) {
            parseAskWonder(sender, args);
        } else if (keyword == TcpCommon::movePlayerUp) {
            parseMovePlayerUp(sender, args);
        } else if (keyword == TcpCommon::movePlayerDown) {
            parseMovePlayerDown(sender, args);
        } else if (keyword == TcpCommon::randomWonders) {
            parseRandomWonders(sender, args);
        } else if (keyword == TcpCommon::randomFaces) {
            parseRandomFaces(sender, args);
        } else if (keyword == TcpCommon::randomPlaces) {
            parseRandomPlaces(sender, args);
        } else if (keyword == TcpCommon::askGameStarts) {
            parseAskGameStarts(sender, args);
        } else if (keyword == TcpCommon::selectFace) {
            parseSelectFace(sender, args);
        } else if (keyword == TcpCommon::askAction) {
            parseAskAction(sender, args);
        } else {
            sendDebug(QString("Unknown keyword: ") + keyword);
        }
    }
}


TcpServer::PlayerSocket TcpServer::getPlayerSocket(const QTcpSocket * socket) {
    for (PlayerSocket & ps : playerSockets) {
        if (ps.socket == socket) {
            return ps;
        }
    }
    std::cout << "ERROR: player socket not found" << std::endl;
    return TcpServer::PlayerSocket(nullptr);
}


PlayerId TcpServer::getConnectedPlayer(const QTcpSocket * socket) {
    return getPlayerSocket(socket).playerId;
}


QTcpSocket * TcpServer::getSocket(PlayerId playerId) {
    for (PlayerSocket & ps : playerSockets) {
        if (ps.playerId == playerId) {
            return ps.socket;
        }
    }
    std::cout << "ERROR: player socket not found: "<< playerId << std::endl;
    return nullptr;
}


void TcpServer::sendAll(const QString & message) {
    std::cout << "sendAll: " << message.toStdString() << std::endl;
    for (PlayerSocket & ps : playerSockets) {
        ps.socket->write(QByteArray::fromStdString(message.toStdString()));
        ps.socket->flush();
        ps.socket->waitForBytesWritten();
    }
}


void TcpServer::sendToPlayer(PlayerId playerId, const QString & message) {
    std::cout << "sendToPlayer: " << playerId << " " << message.toStdString() << std::endl;
    QTcpSocket * socket = getSocket(playerId);
    if (socket) {
        socket->write(QByteArray::fromStdString(message.toStdString()));
        socket->flush();
        socket->waitForBytesWritten();
    }
}


void TcpServer::parsePlayerName(const QTcpSocket * socket, const QStringList & args) {
    if (args.size() != 2) {
        return;
    }
    //sendDebug("add player name");
    addPlayerName(socket, args[0].toStdString().c_str(), args[1].toInt());
}


void TcpServer::parseNumberAIs(const QTcpSocket * socket, const QStringList & args) {
    (void) socket;
    if (args.size() != 1) {
        return;
    }
    //sendDebug("set number AIs");
    setNumberAIs(args[0].toInt());
}


void TcpServer::parsePlayerReady(const QTcpSocket * socket, const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    //sendDebug("set player ready");
    setPlayerReady(getConnectedPlayer(socket), args[0].toInt());
}


void TcpServer::parseAskWonder(const QTcpSocket * socket, const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    askWonder(getConnectedPlayer(socket), args[0].toInt());
}


void TcpServer::parseMovePlayerUp(const QTcpSocket * socket, const QStringList & args) {
    (void) socket;
    if (args.size() != 1) {
        return;
    }
    movePlayerUp(args[0].toInt());
}


void TcpServer::parseMovePlayerDown(const QTcpSocket * socket, const QStringList & args) {
    (void) socket;
    if (args.size() != 1) {
        return;
    }
    movePlayerDown(args[0].toInt());
}


void TcpServer::parseRandomWonders(const QTcpSocket * socket, const QStringList & args) {
    (void) socket;
    if (args.size() != 1) {
        return;
    }
    setRandomWonders(args[0].toInt());
}


void TcpServer::parseRandomFaces(const QTcpSocket * socket, const QStringList & args) {
    (void) socket;
    if (args.size() != 1) {
        return;
    }
    setRandomFaces(args[0].toInt());
}


void TcpServer::parseRandomPlaces(const QTcpSocket * socket, const QStringList & args) {
    (void) socket;
    if (args.size() != 1) {
        return;
    }
    setRandomPlaces(args[0].toInt());
}


void TcpServer::parseAskGameStarts(const QTcpSocket * socket, const QStringList & args) {
    (void) socket;
    if (args.size() != 0) {
        return;
    }
    //sendDebug("ask game starts");
    askStartGame();
}


void TcpServer::parseSelectFace(const QTcpSocket * socket, const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    PlayerId playerId = getConnectedPlayer(socket);
    WonderFace face = (WonderFace)args[0].toInt();
    selectFace(playerId, face);
}


void TcpServer::parseAskAction(const QTcpSocket * socket, const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    PlayerId playerId = getConnectedPlayer(socket);
    Action action;
    action.fromString(args[0]);
    askAction(playerId, action);
}
