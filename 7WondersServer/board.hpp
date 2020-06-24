#ifndef BOARD_HPP
#define BOARD_HPP

#include <QObject>
#include <QVector>
#include <QTimer>

#include <random>

#include "tcpserver.hpp"
#include "card.hpp"
#include "wonder.hpp"
#include "player.hpp"
#include "action.hpp"

class Player;
enum StatusPlayer : int;

/***************** BOARD *****************
 *
 *               playerId 5
 *              playerPosId 2
 *
 *   playerId 2    TABLE    playerId 3
 *  playerPosId 1          playerPosId 3
 *
 *               playerId 1
 *              playerPosId 0
 *
 *
 * left player = playerPosId + 1 % total
 * right player = playerPosId - 1 % total (warning: + total)
 *
 ****************************************/



class Board : public QObject {
    Q_OBJECT

public:
    enum StatusBoard {
        StatusWaitingReady,
        StatusWaitingFaces,
        StatusGameStarted,
        StatusGameOver
    };

    struct BoardState {
        int nbPlayers; // might be more than 7 - nbAIs
        int nbAIs; // might be more than 7 - nbPlayers
        PlayerId maxGenPlayerId;
        QVector<CardsByAgeByPlayer> remainingCards;
        QVector<Player *> players;
        QVector<CardId> discardedCards;
        int currentRound; // from 0 to 5
        Age currentAge; // from 1 to 3
        StatusBoard status;
    };

    explicit Board(QObject * parent = nullptr, bool fake=false);
    virtual ~Board();

    void mainLoop();
    PlayerId addPlayer(Player * player);
    void removePlayer(Player * player);
    size_t getNbPlayers() const;
    Player * getPlayer(PlayerId playerId, size_t offset = 0) const;
    TcpServer & getServer() { return tcpserver; }
    bool isValidAction(PlayerId playerId, const Action & action, QString & optMessage, QVector<Resource> * listMissing = nullptr);
    bool isNewAge() const;
    BoardView toBoardView();
    BoardState saveBoardState() const;
    void restoreFakeBoardState(const BoardState & state);
    void playSingleAction(PlayerId playerId, Action & action);

signals:

public slots:
    void addPlayerName(const QTcpSocket * socket, const char * name);
    void setNumberAIs(int number);
    void setPlayerReady(PlayerId playerId, bool ready);
    void askWonder(PlayerId playerId, WonderId wonderId);
    void movePlayerUp(int index);
    void movePlayerDown(int index);
    void setRandomWonders(bool value);
    void setRandomFaces(bool value);
    void setRandomPlaces(bool value);
    void askStartGame();
    void selectFace(PlayerId playerId, WonderFace face);
    void askAction(PlayerId playerId, const Action & action);

private:
    void initAllWonders();
    void initAllCards();
    void gameLoop();
    void gameProcess();
    void commitActionsParts1and2();
    void commitActionsPart1(PlayerId playerId);
    void commitActionsPart2(PlayerId playerId);
    void commitActionPart1(PlayerId playerId, int actionId);
    void commitActionPart2(PlayerId playerId, int actionId);
    void applyCardEffectsPart1(PlayerId playerId, CardId cardId);
    void applyCardEffectsPart2(PlayerId playerId, CardId cardId);
    void resolveMilitaryConflicts();
    bool isValidActionFromDiscarded(PlayerId playerId, const Action & action, QString & optMessage);
    bool isValidActionFreeCard(PlayerId playerId, const Action & action, QString & optMessage);
    bool isValidActionCopyGuild(PlayerId playerId, const Action & action, QString & optMessage);
    void showChoice();

public:
    size_t getPlayerArrayId(PlayerId playerId) const;
    Player * getLeftPlayer(PlayerId playerId) const;
    Player * getRightPlayer(PlayerId playerId) const;
    QVector<Player *> getApplyPlayers(PlayerId playerId, WhichPlayer which) const;
    QVector<CardId> getRandomCardsByAge(Age age);
    QVector<CardId> getRandomGuilds();
    QVector<CardId> & getCardsByAgeRoundPlayer(Age a, int round, PlayerId p);
    void removeCardForAgeRoundPlayer(CardId cardId, Age a, int round, PlayerId p);
    void discardRemainingCardsForAge(Age a);
    void setAllPlayers(StatusPlayer status);
    bool areAllPlayers(StatusPlayer status);
    bool areAllPlayersNot(StatusPlayer status);
    bool isLastRound() const;
    bool isLastAge() const;
    bool isGameOver() const;

    static const int nbAges = 3;
    static const int nbRounds = 6;
    static const int maxPlayers = 7;

    BoardState state;
    TcpServer tcpserver;
    QTimer * timer;
    bool randomWonders;
    bool randomFaces;
    bool randomPlaces;

    std::default_random_engine random;
};

#endif // BOARD_HPP

