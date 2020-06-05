#include "card.hpp"

#include <iostream>

const char * CardTypeToStr(CardType type) {
    switch (type) {
    case TypeResourceRaw:   return "ResourceRaw";
    case TypeResourceManufactured:   return "ResourceManufactured";
    case TypeCommerce:      return "Commerce";
    case TypeCivilisation:  return "Civilisation";
    case TypeMilitary:      return "Military";
    case TypeScience:       return "Science";
    case TypeGuild:         return "Guild";
    case TypeWonder:        return "Wonder";
    case TypeMilitaryDefeat:return "MilitaryDefeat";
    case TypeNone:          return "NONE TYPE";
    }
}


const char * CardTypeToShortStr(CardType type) {
    switch (type) {
    case TypeResourceRaw:   return "ResR";
    case TypeResourceManufactured:   return "ResM";
    case TypeCommerce:      return "Comm";
    case TypeCivilisation:  return "Civi";
    case TypeMilitary:      return "Mili";
    case TypeScience:       return "Scien";
    case TypeGuild:         return "Guild";
    case TypeWonder:        return "Wonder";
    case TypeMilitaryDefeat:return "Defeat";
    case TypeNone:          return "NONE";
    }
}


const char * ScienceTypeToStr(ScienceType science) {
    switch (science) {
    case ScienceNo:  return "ScienceNo";
    case Science1:   return "Science1";
    case Science2:   return "Science2";
    case Science3:   return "Science3";
    case ScienceAll: return "ScienceAll";
    default:         return "Science ERROR";
    }
}


const char * ScienceTypeToShortStr(ScienceType science) {
    switch (science) {
    case ScienceNo:  return "SNo";
    case Science1:   return "S1";
    case Science2:   return "S2";
    case Science3:   return "S3";
    case ScienceAll: return "SAll";
    default:         return "SERR";
    }
}


int utilCountSciencePoints(const QVector<ScienceType> & listSciences) {
    QVector<int> nbSciences(3, 0);
    return utilCountSciencePointsRec(listSciences, 0, nbSciences);
}


int utilCountSciencePointsRec(const QVector<ScienceType> & listSciences, int currentId, QVector<int> & nbSciences) {
    if (currentId >= listSciences.size()) {
        int count = 0;
        int min = 99;
        for (int i=0; i<3; ++i) {
            int val = nbSciences[i];
            count += val * val;
            if (min > val) {
                min = val;
            }
        }
        return count + min * 7;
    }

    ScienceType science = listSciences[currentId];
    if (science == ScienceAll) {
        int best = 0;
        int bestId = 0;
        for (int i=0; i<3; ++i) {
            nbSciences[i]++;
            int tmp = utilCountSciencePointsRec(listSciences, currentId + 1, nbSciences);
            if (tmp > best) {
                best = tmp;
                bestId = i;
            }
            nbSciences[i]--;
        }
        nbSciences[bestId]++;
        return best;
    } else {
        nbSciences[science]++;
        return utilCountSciencePointsRec(listSciences, currentId + 1, nbSciences);
    }
}


const char * WhichPlayerToStr(WhichPlayer which) {
    static const char * iToA[] = {"None", "Self", "Left", "Error", "Right", "Error", "Both", "All"};
    return iToA[which];
}


const char * WhichPlayerToShortStr(WhichPlayer which) {
    static const char * iToA[] = {"N", "S", "L", "E", "R", "E", "B", "A"};
    return iToA[which];
}


QString Card::getMainText() const {
    QString retStr;
    QString incomeStr = incomeResources.display();
    switch (type) {
    case TypeNone:
    case TypeMilitaryDefeat:
        retStr = "None";
        break;
    case TypeResourceRaw:
    case TypeResourceManufactured:
        retStr = incomeStr;
        break;
    case TypeCommerce:
    case TypeMilitary:
    case TypeCivilisation:
    case TypeScience:
    case TypeGuild:
    case TypeWonder:
        retStr = "VP: " + QString::number(points) + " ";
        retStr += "MP: " + QString::number(shields) + " ";
        retStr += "Sc: " + QString(ScienceTypeToShortStr(science)) + "\n";
        if (incomeStr.length() > 0) {
            retStr += incomeStr + "\n";
        }
        if (incomeCoins > 0) {
            retStr += QString::number(incomeCoins)  + "\n";
        }
        for ( CardType typeCondition : cardTypesCondition) {
            retStr += QString(CardTypeToShortStr(typeCondition)) + " ";
        }
        retStr += WhichPlayerToStr(target);
        break;
    }

    return retStr;
}


QString Card::getShortText() const {
    QString retStr;
    QString incomeStr = incomeResources.display();
    switch (type) {
    case TypeNone:
        retStr = "None";
        break;
    case TypeResourceRaw:
    case TypeResourceManufactured:
        retStr = incomeStr;
        break;
    case TypeCommerce:
    case TypeGuild:
    case TypeWonder:
        retStr = QString(WhichPlayerToShortStr(target)) + " " + QString::number(incomeCoins);
        break;
    case TypeCivilisation:
        retStr = QString::number(points);
        break;
    case TypeMilitary:
        retStr = QString::number(shields);
        break;
    case TypeScience:
        retStr = ScienceTypeToShortStr(science);
        break;
    }
    return retStr;
}


QVector<Card> AllCards::allCards;


const Card & AllCards::getCard(CardId cardId) {
    return allCards[cardId];
}


CardId AllCards::getCardId(QString cardName) {
    for (const Card & card : allCards) {
        if (cardName == card.name) {
            return card.id;
        }
    }
    std::cout << "Error card not found: " << cardName.toStdString() << std::endl;
    return CardIdInvalid;
}


void AllCards::init() {
    allCards.clear();

    // Age 1
    addResourceRaw("Lumber Yard", 1, {3, 4}, 0, Resource(Resource::ONE_WOOD));
    addResourceRaw("Stone Pit", 1, {3, 5}, 0, Resource(Resource::ONE_STONE));
    addResourceRaw("Clay Pool", 1, {3, 5}, 0, Resource(Resource::ONE_CLAY));
    addResourceRaw("Ore Vein", 1, {3, 4}, 0, Resource(Resource::ONE_ORE));

    addResourceRaw("Tree Farm", 1, {6}, 1, Resource(Resource::ONE_WOOD | Resource::ONE_CLAY));
    addResourceRaw("Excavation", 1, {4}, 1, Resource(Resource::ONE_STONE | Resource::ONE_CLAY));
    addResourceRaw("Clay Pit", 1, {3}, 1, Resource(Resource::ONE_CLAY | Resource::ONE_ORE));
    addResourceRaw("Timber Yard", 1, {3}, 1, Resource(Resource::ONE_STONE | Resource::ONE_WOOD));
    addResourceRaw("Forest Cave", 1, {5}, 1, Resource(Resource::ONE_WOOD | Resource::ONE_ORE));
    addResourceRaw("Mine", 1, {6}, 1, Resource(Resource::ONE_ORE | Resource::ONE_STONE));

    addResourceManufactured("Loom", 1, {3, 6}, 0, Resource(Resource::ONE_LOOM));
    addResourceManufactured("Glassworks", 1, {3, 6}, 0, Resource(Resource::ONE_GLASS));
    addResourceManufactured("Press", 1, {3, 6}, 0, Resource(Resource::ONE_PAPYRUS));

    addCivilisation("Pawnshop", 1, {4, 7}, 0, Resource(), 3);
    CardId idBaths = addCivilisation("Baths", 1, {3, 7}, 0, Resource(Resource::ONE_STONE), 3);
    CardId idAltar = addCivilisation("Altar", 1, {3, 5}, 0, Resource(), 2);
    CardId idTheater = addCivilisation("Theater", 1, {3, 6}, 0, Resource(), 2);

    addCommerce("Taverne", 1, {4, 5, 7}, 0, Resource(), 5, Resource());
    CardId idEastTrading = addCommerce("East Trading Post", 1, {3, 7}, 0, Resource(), 0, Resource(Resource::ONE_ALL_RAW), PlayerRight);
    CardId idWestTrading = addCommerce("West Trading Post", 1, {3, 7}, 0, Resource(), 0, Resource(Resource::ONE_ALL_RAW), PlayerLeft);
    CardId idMarketPlace = addCommerce("Market Place", 1, {3, 6}, 0, Resource(), 0, Resource(Resource::ONE_ALL_MANUFACTURED), PlayerBoth);

    addMilitary("Stockade", 1, {3, 7}, 0, Resource(Resource::ONE_WOOD), 1);
    addMilitary("Barracks", 1, {3, 5}, 0, Resource(Resource::ONE_ORE), 1);
    addMilitary("Guard Tower", 1, {3, 4}, 0, Resource(Resource::ONE_CLAY), 1);

    CardId idApothecary = addScience("Apothecary", 1, {3, 5}, 0, Resource(Resource::ONE_LOOM), Science1);
    CardId idWorkshop = addScience("Workshop", 1, {3, 7}, 0, Resource(Resource::ONE_GLASS), Science2);
    CardId idScriptorium = addScience("Scriptorium", 1, {3, 4}, 0, Resource(Resource::ONE_PAPYRUS), Science3);

    // Age 2
    addResourceRaw("Sawmill", 2, {3, 4}, 1, Resource(Resource::ONE_WOOD, 2));
    addResourceRaw("Quarry", 2, {3, 4}, 1, Resource(Resource::ONE_STONE, 2));
    addResourceRaw("Brickyard", 2, {3, 4}, 1, Resource(Resource::ONE_CLAY, 2));
    addResourceRaw("Foundry", 2, {3, 4}, 1, Resource(Resource::ONE_ORE, 2));

    addResourceManufactured("Loom", 2, {3, 5}, 0, Resource(Resource::ONE_LOOM));
    addResourceManufactured("Glassworks", 2, {3, 5}, 0, Resource(Resource::ONE_GLASS));
    addResourceManufactured("Press", 2, {3, 5}, 0, Resource(Resource::ONE_PAPYRUS));

    addCivilisation("Aqueduct", 2, {3, 7}, 0, Resource(Resource::ONE_STONE, 3), 5, {idBaths});
    CardId idTemple = addCivilisation("Temple", 2, {3, 6}, 0, Resource(Resource::ONE_WOOD | Resource::ONE_CLAY | Resource::ONE_GLASS), 3, {idAltar});
    CardId idStatue = addCivilisation("Statue", 2, {3, 7}, 0, Resource(0, 2, 0, 1, 0, 0, 0), 4, {idTheater});
    addCivilisation("Courthouse", 2, {3, 5}, 0, Resource(2, 0, 0, 0, 0, 1, 0), 5, {idScriptorium});

    CardId idForum = addCommerce("Forum", 2, {3, 6, 7}, 0, Resource(Resource::ONE_CLAY, 2), 0, Resource(Resource::ONE_ALL_MANUFACTURED),
                PlayerNone, {}, 0, {idEastTrading, idWestTrading});
    CardId idCaravansery = addCommerce("Caravansery", 2, {3, 5, 6}, 0, Resource(Resource::ONE_WOOD, 2), 0, Resource(Resource::ONE_ALL_RAW),
                PlayerNone, {}, 0, {idMarketPlace});
    addCommerce("Vineyard", 2, {3, 6}, 0, Resource(), 1, Resource(), PlayerAll, {TypeResourceRaw});
    addCommerce("Bazar", 2, {4, 7}, 0, Resource(), 2, Resource(), PlayerAll, {TypeResourceManufactured});

    CardId idWalls = addMilitary("Walls", 2, {3, 7}, 0, Resource(Resource::ONE_STONE, 3), 2);
    CardId idTrainingGround = addMilitary("Training Ground", 2, {4, 6, 7}, 0, Resource(0, 2, 0, 1, 0, 0, 0), 2);
    addMilitary("Stables", 2, {3, 5}, 0, Resource(1, 1, 0, 1, 0, 0, 0), 2, {idApothecary});
    addMilitary("Archery Range", 2, {3, 6}, 0, Resource(0, 1, 0, 2, 0, 0, 0), 2, {idWorkshop});

    CardId idDispensary = addScience("Dispensary", 2, {3, 4}, 0, Resource(0, 2, 0, 0, 1, 0, 0), Science1, {idApothecary});
    CardId idLaboratory = addScience("Laboratory", 2, {3, 5}, 0, Resource(2, 0, 0, 0, 0, 0, 1), Science2, {idWorkshop});
    CardId idLibrary = addScience("Library", 2, {3, 6}, 0, Resource(0, 0, 2, 0, 0, 1, 0), Science3, {idScriptorium});
    CardId idSchool = addScience("School", 2, {3, 7}, 0, Resource(Resource::ONE_WOOD | Resource::ONE_PAPYRUS), Science3);

    // Age 3
    addCivilisation("Pantheon", 3, {3, 6}, 0, Resource(2, 1, 0, 0, 1, 1, 1), 7, {idTemple});
    addCivilisation("Gardens", 3, {3, 4}, 0, Resource(2, 0, 0, 1, 0, 0, 0), 5, {idStatue});
    addCivilisation("Town Hall", 3, {3, 5, 6}, 0, Resource(0, 1, 2, 0, 1, 0, 0), 6);
    addCivilisation("Palace", 3, {3, 7}, 0, Resource(Resource::ONE_ALL), 8);
    addCivilisation("Senate", 3, {3, 5}, 0, Resource(0, 1, 1, 2, 0, 0, 0), 6, {idLibrary});

    addCommerce("Haven", 3, {3, 4}, 0, Resource(Resource::ONE_ORE | Resource::ONE_WOOD | Resource::ONE_LOOM), 1, Resource(),
                PlayerSelf, {TypeResourceRaw}, 1, {idForum});
    addCommerce("Lighthouse", 3, {3, 6}, 0, Resource(Resource::ONE_STONE | Resource::ONE_GLASS), 1, Resource(),
                PlayerSelf, {TypeCommerce}, 1, {idCaravansery});
    addCommerce("Chamber Of Commerce", 3, {4, 6}, 0, Resource(2, 0, 0, 0, 0 ,0, 1), 2, Resource(),
                PlayerSelf, {TypeResourceManufactured}, 2);
    addCommerce("Arena", 3, {3, 5, 7}, 0, Resource(0, 1, 2, 0, 0, 0, 0), 3, Resource(),
                PlayerSelf, {TypeWonder}, 1);

    addMilitary("Fortifications", 3, {3, 7}, 0, Resource(0, 3, 1, 0, 0, 0, 0), 3, {idWalls});
    addMilitary("Circus", 3, {4, 5, 6}, 0, Resource(0, 1, 3, 0, 0, 0, 0), 3, {idTrainingGround});
    addMilitary("Arsenal", 3, {3, 4, 7}, 0, Resource(0, 1, 0, 2, 0, 1, 0), 3);
    addMilitary("Siege Workshop", 3, {3, 5}, 0, Resource(3, 0, 0, 1, 0, 0, 0), 3, {idLaboratory});

    addScience("Lodge", 3, {3, 6}, 0, Resource(2, 0, 0, 0, 0, 1, 1), Science1, {idDispensary});
    addScience("Observatory", 3, {3, 7}, 0, Resource(0, 2, 0, 0, 1, 1, 0), Science2, {idLaboratory});
    addScience("University", 3, {3, 4}, 0, Resource(0, 0, 0, 2, 1, 0, 1), Science3, {idLibrary});
    addScience("Academy", 3, {3, 7}, 0, Resource(0, 0, 3, 0, 1, 0, 0), Science1, {idSchool});
    addScience("Study", 3, {3, 5}, 0, Resource(0, 0, 0, 1, 0, 1, 1), Science2, {idSchool});

    addGuild("Workers Guild", 0, Resource(1, 2, 1, 1, 0, 0, 0), SpecialNone, PlayerBoth, {TypeResourceRaw}, 0, 1);
    addGuild("Craftmen Guild", 0, Resource(0, 2, 2, 0, 0, 0, 0), SpecialNone, PlayerBoth, {TypeResourceManufactured}, 0, 2);
    addGuild("Traders Guild", 0, Resource(Resource::ONE_ALL_MANUFACTURED), SpecialNone, PlayerBoth, {TypeCommerce}, 0, 1);
    addGuild("Philosophers Guild", 0, Resource(3, 0, 0, 0, 0, 1, 1), SpecialNone, PlayerBoth, {TypeScience}, 0, 1);
    addGuild("Spy Guild", 0, Resource(3, 0, 0, 0, 1, 0, 0), SpecialNone, PlayerBoth, {TypeMilitary}, 0, 1);
    addGuild("Strategy Guild", 0, Resource(0, 2, 1, 0, 0, 1, 0), SpecialNone, PlayerBoth, {TypeMilitaryDefeat}, 0, 1);
    addGuild("Shipowners Guild", 0, Resource(0, 0, 0, 3, 1, 0, 1), SpecialNone, PlayerSelf, {TypeResourceRaw, TypeResourceManufactured, TypeGuild}, 0, 1);
    addGuild("Scientists Guild", 0, Resource(0, 2, 0, 2, 0, 0, 1), SpecialNone, PlayerNone, {}, 0, 0, ScienceAll);
    addGuild("Magistrates Guild", 0, Resource(0, 0, 1, 3, 0, 1, 0), SpecialNone, PlayerBoth, {TypeCivilisation}, 0, 1);
    addGuild("Builders Guild", 0, Resource(2, 0, 2, 0, 1, 0, 0), SpecialNone, PlayerAll, {TypeWonder}, 0, 1);
}


CardId AllCards::addWonder(Card card) {
    card.id = allCards.size();
    card.type = CardType::TypeWonder;
    card.age = 0;
    allCards.push_back(card);
    return card.id;
}


CardId AllCards::addResourceRaw(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource incomeResources) {
    Card card(name, CardType::TypeResourceRaw, age, costCoins, Resource(), nbPlayers);
    card.id = allCards.size();
    card.incomeResources = incomeResources;
    allCards.push_back(card);
    return card.id;
}

CardId AllCards::addResourceManufactured(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource incomeResources) {
    Card card(name, CardType::TypeResourceManufactured, age, costCoins, Resource(), nbPlayers);
    card.id = allCards.size();
    card.incomeResources = incomeResources;
    allCards.push_back(card);
    return card.id;
}

CardId AllCards::addCommerce(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, Currency incomeCoins, Resource incomeResources, WhichPlayer target, QVector<CardType> cardTypesCondition, VictoryPoint points, QVector<CardId> freeChain) {
    Card card(name, CardType::TypeCommerce, age, costCoins, costResources, nbPlayers);
    card.id = allCards.size();
    card.incomeCoins = incomeCoins;
    card.incomeResources = incomeResources;
    card.target = target;
    card.cardTypesCondition = cardTypesCondition;
    card.points = points;
    card.freePreviousIds = freeChain;
    allCards.push_back(card);
    return card.id;
}

CardId AllCards::addScience (const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, ScienceType scienceType, QVector<CardId> freeChain) {
    Card card(name, CardType::TypeScience, age, costCoins, costResources, nbPlayers);
    card.id = allCards.size();
    card.science = scienceType;
    card.freePreviousIds = freeChain;
    allCards.push_back(card);
    return card.id;
}

CardId AllCards::addMilitary(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, Shield shields, QVector<CardId> freeChain) {
    Card card(name, CardType::TypeMilitary, age, costCoins, costResources, nbPlayers);
    card.id = allCards.size();
    card.shields = shields;
    card.freePreviousIds = freeChain;
    allCards.push_back(card);
    return card.id;
}

CardId AllCards::addCivilisation(const char * name, Age age, QVector<int> nbPlayers, Currency costCoins, Resource costResources, VictoryPoint points, QVector<CardId> freeChain) {
    Card card(name, CardType::TypeCivilisation, age, costCoins, costResources, nbPlayers);
    card.id = allCards.size();
    card.points = points;
    card.freePreviousIds = freeChain;
    allCards.push_back(card);
    return card.id;
}

CardId AllCards::addGuild(const char * name, Currency costCoins, Resource costResources, SpecialPower special, WhichPlayer target, QVector<CardType> cardTypesCondition, Currency incomeCoins, VictoryPoint points, ScienceType science) {
    Card card(name, CardType::TypeGuild, 3, costCoins, costResources, {});
    card.id = allCards.size();
    card.special = special;
    card.target = target;
    card.cardTypesCondition = cardTypesCondition;
    card.incomeCoins = incomeCoins;
    card.points = points;
    card.science = science;
    allCards.push_back(card);
    return card.id;
}
