#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <QObject>
#include <QVector>

typedef int PlayerId;

#include "board.hpp"
#include "card.hpp"
#include "wonder.hpp"
#include "action.hpp"
#include "resource.hpp"
#include "resourceproduction.hpp"
#include "playerview.hpp"

class Board;

enum StatusPlayer : int {
    StatusNotReady,
    StatusReady,
    StatusPlaying,
    StatusPlayed
};

class Player : public QObject {
    Q_OBJECT

public:
    Player(Board * board, const char * name);
    virtual ~Player();

    void setWonder(WonderId wonderId);
    void newRound();
    void newAge();
    // leave "possibleActions" empty if normal actions (play, wonder, discard[, free])
    void play(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards);
    virtual void playImplem(const QVector<ActionType> & possibleActions, const QVector<CardId> & cards) = 0;
    virtual void showBoard() = 0;
    void addPlayedAction(const Action & action);
    double evaluateScore();

    Board * board;
    PlayerView view;
    Resource priceToBuyLeft;
    Resource priceToBuyRight;
    ResourceProduction production;
    ResourceProduction buyableResources;
    Shield shields;
    StatusPlayer status;
    QVector<Action> actionsToPlay;

    bool canPlayBothCardsAtEndOfAge;
    bool canCopyNeirbyGuild;
    bool canCopyNeirbyGuildAlreadyUsed;
    bool canPlayCardForFree;
    bool canPlayCardForFreeAlreadyUsed;
    bool canPlayCardFromDiscarded;

signals:
    void showCards(const QVector<CardId> & cards);

};

#endif // PLAYER_HPP

