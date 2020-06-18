#include "tcpclient.hpp"

#include <iostream>

#include "tcpcommon.hpp"


TcpClient::TcpClient(QObject *parent) : QObject(parent) {

}


TcpClient::~TcpClient() { }


bool TcpClient::connectServer(QString ip) {
    std::cout << "connecting to: " << ip.toStdString() << std::endl;
    socket.connectToHost(ip, TcpCommon::port);
    if (! socket.waitForConnected(5000)) {
        std::cout << "Error connecting to: " << ip.toStdString() << std::endl;
        return false;
    }
    QObject::connect(&socket, &QTcpSocket::readyRead, this, &TcpClient::readyRead);
    QObject::connect(&socket, &QTcpSocket::stateChanged, this, &TcpClient::socketStateChanged);
    return true;
}


void TcpClient::disconnectServer() {
    socket.disconnectFromHost();
}


void TcpClient::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    std::cout << "state changed: " << socketState << std::endl;
    if (socketState == QAbstractSocket::UnconnectedState) {
        userMessage("Server disconnected");
    }
}


QString TcpClient::lastError() const {
    return socket.errorString();
}


void TcpClient::readyRead() {
    QByteArray datas = socket.readAll();
    std::cout << "received: " << datas.toStdString() << std::endl;
    //socket.write(QByteArray("ok !\n"));

    QStringList messages = TcpCommon::decodeMessages(datas);

    for (const QString & message : messages) {
        QStringList strList = message.split(TcpCommon::separator);
        const QString & keyword = strList[0];
        QStringList args(strList);
        args.removeFirst();

        if (keyword == TcpCommon::playerId) {
            parsePlayerId(args);
        } else if (keyword == TcpCommon::showChoice) {
            parseShowChoice(args);
        } else if (keyword == TcpCommon::gameStarts) {
            parseStartGame(args);
        } else if (keyword == TcpCommon::showBoard) {
            parseShowBoard(args);
        } else if (keyword == TcpCommon::cardsToPlay) {
            parseCardsToPlay(args);
        } else if (keyword == TcpCommon::confirmAction) {
            parseConfirmAction(args);
        } else if (keyword == TcpCommon::gameOver) {
            parseGameOver(args);
        } else if (keyword == TcpCommon::userMessage) {
            parseUserMessage(args);
        } else {
            std::cout << "Unknown keyword: " << keyword.toStdString() << std::endl;
        }
    }
}


void TcpClient::send(const QString & message) {
    std::cout << "SEND: " << message.toStdString() << "<EOF>" << std::endl;
    socket.write(message.toStdString().c_str());
}


void TcpClient::setPlayerName(const char * name) {
    send(TcpCommon::encodeSingle(TcpCommon::playerName, name));
}


void TcpClient::setNumberAIs(int value) {
    send(TcpCommon::encodeSingle(TcpCommon::numberAIs, std::to_string(value).c_str()));
}


void TcpClient::setPlayerReady(bool state) {
    send(TcpCommon::encodeSingle(TcpCommon::playerReady, std::to_string(state).c_str()));
}


void TcpClient::askWonder(WonderId wonder) {
    send(TcpCommon::encodeSingle(TcpCommon::askWonder, std::to_string(wonder).c_str()));
}


void TcpClient::movePlayerUp(int index) {
    send(TcpCommon::encodeSingle(TcpCommon::movePlayerUp, std::to_string(index).c_str()));
}


void TcpClient::movePlayerDown(int index) {
    send(TcpCommon::encodeSingle(TcpCommon::movePlayerDown, std::to_string(index).c_str()));
}


void TcpClient::setRandomWonders(bool state) {
    send(TcpCommon::encodeSingle(TcpCommon::randomWonders, std::to_string(state).c_str()));
}


void TcpClient::setRandomFaces(bool state) {
    send(TcpCommon::encodeSingle(TcpCommon::randomFaces, std::to_string(state).c_str()));
}


void TcpClient::setRandomPlaces(bool state) {
    send(TcpCommon::encodeSingle(TcpCommon::randomPlaces, std::to_string(state).c_str()));
}


void TcpClient::askStartGame() {
    send(TcpCommon::encodeEmpty(TcpCommon::askGameStarts));
}


void TcpClient::askAction(const Action & action) {
    send(TcpCommon::encodeSingle(TcpCommon::askAction, action.toString()));
}


void TcpClient::parsePlayerId(const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    setPlayerId(args[0].toInt());
}


void TcpClient::parseShowChoice(const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    Choice choice;
    choice.fromString(args[0]);
    showChoice(choice);
}


void TcpClient::parseStartGame(const QStringList & args) {
    if (args.size() != 0) {
        return;
    }
    std::cout << "game starts" << std::endl;
    startGame();
}


void TcpClient::parseShowBoard(const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    showBoard(args[0]);
}


void TcpClient::parseCardsToPlay(const QStringList & args) {
    QVector<CardId> cards;
    for ( const QString & strCardId : args ) {
        cards.append(strCardId.toInt());
    }
    int possibleActions = cards.first();
    cards.removeFirst();
    showCardsToPlay(possibleActions, cards);
}


void TcpClient::parseConfirmAction(const QStringList & args) {
    if (args.size() != 2) {
        return;
    }
    confirmAction(args[0].toInt(), args[1]);
}


void TcpClient::parseGameOver(const QStringList & args) {
    if (args.size() != 0) {
        return;
    }
    gameOver();
}


void TcpClient::parseUserMessage(const QStringList & args) {
    if (args.size() != 1) {
        return;
    }
    userMessage(args[0]);
}
