#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QStringList>
#include <QColor>

#include "card.hpp"
#include "playerview.hpp"
#include "action.hpp"
#include "choice.hpp"

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = 0);
    virtual ~TcpClient();
    bool connectServer(QString ip);
    void disconnectServer();
    QString lastError() const;

    void setPlayerName(const char * name);
    void setNumberAIs(int value);
    void setPlayerReady(bool state);
    void askWonder(WonderId wonder);
    void movePlayerUp(int index);
    void movePlayerDown(int index);
    void setRandomWonders(bool state);
    void setRandomFaces(bool state);
    void setRandomPlaces(bool state);
    void askStartGame();
    void selectFace(WonderFace face);
    void askAction(const Action & action);

signals:
    void setPlayerId(PlayerId playerId);
    void showChoice(const Choice & choice);
    void showChoiceFace(WonderId wonderId);
    void startGame();
    void showBoard(const QString & boardString);
    void showCardsToPlay(int possibleActions, const QVector<CardId> & cards);
    void confirmAction(bool valid, const QString & optMessage);
    void gameOver();
    void userMessage(const QColor & color, const QString & message);

public slots:
    void socketStateChanged(QAbstractSocket::SocketState socketState);
    void readyRead();

private:
    QTcpSocket socket;
    void send(const QString & message);

    void parsePlayerId(const QStringList & args);
    void parseShowChoice(const QStringList & args);
    void parseShowChoiceFace(const QStringList & args);
    void parseStartGame(const QStringList & args);
    void parseShowBoard(const QStringList & args);
    void parseCardsToPlay(const QStringList & args);
    void parseConfirmAction(const QStringList & args);
    void parseGameOver(const QStringList & args);
    void parseUserMessage(const QStringList & args);
};

#endif // TCPCLIENT_H
