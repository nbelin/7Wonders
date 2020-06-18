#ifndef TCPCOMMUN_HPP
#define TCPCOMMUN_HPP

#include <QString>
#include <QStringList>

class TcpCommon {
public:
    static const unsigned short port = 44714;
    static constexpr const char * start = "$";
    static constexpr const char * end = "&";
    static constexpr const char * separator = ":";
    static constexpr const char * debug = "DEBUG";
    static constexpr const char * playerName = "PLAYER_NAME";
    static constexpr const char * playerId = "PLAYER_ID";
    static constexpr const char * numberAIs = "NUMBER_AIS";
    static constexpr const char * playerReady = "PLAYER_READY";
    static constexpr const char * askWonder = "ASK_WONDER";
    static constexpr const char * movePlayerUp = "MOVE_PLAYER_UP";
    static constexpr const char * movePlayerDown = "MOVE_PLAYER_DOWN";
    static constexpr const char * randomWonders = "RANDOM_WONDERS";
    static constexpr const char * randomFaces = "RANDOM_FACES";
    static constexpr const char * randomPlaces = "RANDOM_PLACES";
    static constexpr const char * showChoice = "SHOW_CHOICE";
    static constexpr const char * askGameStarts = "ASK_GAME_STARTS";
    static constexpr const char * gameStarts = "GAME_STARTS";
    static constexpr const char * showBoard = "SHOW_BOARD";
    static constexpr const char * cardsToPlay = "CARDS_TO_PLAY";
    static constexpr const char * askAction = "ASK_ACTION";
    static constexpr const char * confirmAction = "CONFIRM_ACTION";
    static constexpr const char * gameOver = "GAME_OVER";
    static constexpr const char * userMessage = "USER_MESSAGE";

    static QStringList decodeMessages(const QString & data);
    static QString encodeEmpty(const char * prefix);
    static QString encodeSingle(const char * prefix, const QString & value);
    static QString encodeMulti(const char * prefix, const QStringList & strList);
};

#endif // TCPCOMMUN

