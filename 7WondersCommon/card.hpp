#ifndef CARD_HPP
#define CARD_HPP

#include <QVector>
#include <QString>

#include "resource.hpp"

typedef int CardId;
const CardId CardIdInvalid = 0;
typedef unsigned char Age;
typedef unsigned char Shield;
typedef unsigned char VictoryPoint;

enum CardType {
    TypeNone,
    TypeResourceRaw,
    TypeResourceManufactured,
    TypeCommerce,
    TypeCivilisation,
    TypeMilitary,
    TypeScience,
    TypeGuild,
    TypeWonder,
    TypeMilitaryDefeat // only for cardTypeCondition
};

const char * CardTypeToStr(CardType type);
const char * CardTypeToShortStr(CardType type);


enum SpecialPower {
    SpecialNone,
    BothCardsAtEndOfAge,
    CopyNeirbyGuild,
    OneFreeCardByAge,
    OneFreeCardFromDiscarded
};

enum ScienceType {
    ScienceNo = -1,
    Science1 = 0,
    Science2,
    Science3,
    ScienceAll
};

const char * ScienceTypeToStr(ScienceType science);
const char * ScienceTypeToShortStr(ScienceType science);
int utilCountSciencePoints(const QVector<ScienceType> & listSciences);
int utilCountSciencePointsRec(const QVector<ScienceType> & listSciences, int currentId, QVector<int> & nbSciences);

enum WhichPlayer {
    PlayerNone  = 0,
    PlayerSelf  = 1,
    PlayerLeft  = 2,
    PlayerRight = 4,
    PlayerBoth  = 6,
    PlayerAll   = 7
};

const char * WhichPlayerToStr(WhichPlayer which);
const char * WhichPlayerToShortStr(WhichPlayer which);

struct Card {

    // common to all card types
    CardId              id;
    const char *        name;
    CardType            type;
    QVector<CardId>     freePreviousIds;
    Age                 age;
    Currency            costCoins;
    Resource            costResources;
    QVector<int>        nbMinPlayers;

    // specific depending card type
    Shield              shields;
    Currency            incomeCoins;
    Resource            incomeResources;
    ScienceType         science;
    VictoryPoint        points;
    WhichPlayer         target;
    QVector<CardType>   cardTypesCondition;
    SpecialPower        special;


    Card():
        id(CardIdInvalid),
        name(nullptr),
        type(TypeNone),
        age(0),
        costCoins(0),
        shields(0),
        incomeCoins(0),
        science(ScienceNo),
        points(0),
        target(PlayerNone),
        special(SpecialNone){}

    Card(const char * name, CardType type, Age age, Currency costCoins, Resource costResources, QVector<int> nbMinPlayers):
        id(CardIdInvalid),
        name(name),
        type(type),
        age(age),
        costCoins(costCoins),
        costResources(costResources),
        nbMinPlayers(nbMinPlayers),
        shields(0),
        incomeCoins(0),
        science(ScienceNo),
        points(0),
        target(PlayerNone),
        special(SpecialNone){}

    void display() const;
    QString getMainText() const;
    QString getShortText() const;
};


typedef QVector<CardId> CardsByPlayer;
typedef QVector<CardsByPlayer> CardsByAgeByPlayer;


struct AllCards {
    static const Card & getCard(CardId cardId);
    static CardId getCardId(QString cardName);

    static QVector<Card> allCards;
    static void init();

    static CardId addWonder(Card card);

private:
    static CardId addResourceRaw(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource incomeResources);
    static CardId addResourceManufactured(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource incomeResources);
    static CardId addCommerce(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, Currency incomeCoins, Resource incomeResources, WhichPlayer target = PlayerNone, QVector<CardType> cardTypesCondition = {}, VictoryPoint points = 0, QVector<CardId> freeChain = {});
    static CardId addScience (const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, ScienceType scienceType, QVector<CardId> freeChain = {});
    static CardId addMilitary(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, Shield shields, QVector<CardId> freeChain = {});
    static CardId addCivilisation(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, VictoryPoint points, QVector<CardId> freeChain = {});
    static CardId addGuild(const char * name, Currency costCoins, Resource costResources, SpecialPower special, WhichPlayer target = PlayerNone, QVector<CardType> cardTypesCondition = {}, Currency incomeCoins = 0, VictoryPoint points = 0, ScienceType science = ScienceNo);
};

#endif // CARD_HPP

