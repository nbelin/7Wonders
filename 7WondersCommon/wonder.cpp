#include "wonder.hpp"
#include "tools.hpp"

#include <iostream>

Wonder::Wonder(): id(0), inGame(false) {

}


const QVector<CardId> & Wonder::getStages(WonderFace wonderFace) const {
    switch(wonderFace) {
    case WonderFaceA:
        return stagesA;
    case WonderFaceB:
        return stagesB;
    }
    return stagesA;
}


const QString & Wonder::getImageFace(WonderFace wonderFace) const {
    switch(wonderFace) {
    case WonderFaceA:
        return imageA;
    case WonderFaceB:
        return imageB;
    }
    return image;
}


QVector<Wonder> AllWonders::allWonders;


const Wonder & AllWonders::getWonder(WonderId wonderId) {
    return allWonders[wonderId];
}


void AllWonders::init() {
    QVector<CardId> stagesA;
    QVector<CardId> stagesB;
    Card workingCard;
    Resource baseIncome;


    // Invalid Wonder
    addWonder("Invalid", baseIncome, stagesA, stagesB, "", "", "");


    // Rhodos
    baseIncome = Resource(Resource::ONE_ORE);
    stagesA.clear();
    stagesB.clear();

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 2);
    workingCard.points = 3;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY, 3);
    workingCard.shields = 2;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ORE, 4);
    workingCard.points = 7;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 3);
    workingCard.incomeCoins = 3;
    workingCard.shields = 1;
    workingCard.points = 3;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ORE, 4);
    workingCard.incomeCoins = 4;
    workingCard.shields = 1;
    workingCard.points = 4;
    stagesB.push_back(AllCards::addWonder(workingCard));

    addWonder("Rhodos", baseIncome, stagesA, stagesB, "rhodos.jpg", "rhodosA.png", "rhodosB.png");


    // Alexandria
    baseIncome = Resource(Resource::ONE_GLASS);
    stagesA.clear();
    stagesB.clear();

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 2);
    workingCard.points = 3;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ORE, 2);
    workingCard.incomeResources = Resource(Resource::ONE_ALL_RAW);
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_GLASS, 2);
    workingCard.points = 7;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY, 2);
    workingCard.incomeResources = Resource(Resource::ONE_ALL_RAW);
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 2);
    workingCard.incomeResources = Resource(Resource::ONE_ALL_MANUFACTURED);
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 3);
    workingCard.points = 7;
    stagesB.push_back(AllCards::addWonder(workingCard));

    addWonder("Alexandria", baseIncome, stagesA, stagesB, "alexandria.jpg", "alexandriaA.png", "alexandriaB.png");


    // Ephesos
    baseIncome = Resource(Resource::ONE_PAPYRUS);
    stagesA.clear();
    stagesB.clear();

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 2);
    workingCard.points = 3;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 2);
    workingCard.incomeCoins = 9;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_PAPYRUS, 2);
    workingCard.points = 7;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 2);
    workingCard.incomeCoins = 4;
    workingCard.points = 2;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 2);
    workingCard.incomeCoins = 4;
    workingCard.points = 3;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ALL_MANUFACTURED);
    workingCard.incomeCoins = 4;
    workingCard.points = 5;
    stagesB.push_back(AllCards::addWonder(workingCard));

    addWonder("Ephesos", baseIncome, stagesA, stagesB, "ephesos.jpg", "ephesosA.png", "ephesosB.png");


    // Babylon
    baseIncome = Resource(Resource::ONE_CLAY);
    stagesA.clear();
    stagesB.clear();

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY, 2);
    workingCard.points = 3;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 3);
    workingCard.science = ScienceAll;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY, 4);
    workingCard.points = 7;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY | Resource::ONE_LOOM);
    workingCard.points = 3;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(0, 0, 0, 2, 1, 0, 0);
    workingCard.special = BothCardsAtEndOfAge;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(3, 0, 0, 0, 0, 0, 1);
    workingCard.science = ScienceAll;
    stagesB.push_back(AllCards::addWonder(workingCard));

    addWonder("Babylon", baseIncome, stagesA, stagesB, "babylon.jpg", "babylonA.png", "babylonB.png");


    // Olympia
    baseIncome = Resource(Resource::ONE_WOOD);
    stagesA.clear();
    stagesB.clear();

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 2);
    workingCard.points = 3;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 2);
    workingCard.special = OneFreeCardByAge;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ORE, 2);
    workingCard.points = 7;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 2);
    workingCard.incomeResources = Resource(Resource::ONE_ALL_RAW);
    workingCard.target = PlayerBoth;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 2);
    workingCard.points = 5;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(0, 2, 0, 0, 0, 1, 0);
    workingCard.special = CopyNeirbyGuild;
    stagesB.push_back(AllCards::addWonder(workingCard));

    addWonder("Olympia", baseIncome, stagesA, stagesB, "olympia.jpg", "olympiaA.png", "olympiaB.png");


    // Halikarnassus
    baseIncome = Resource(Resource::ONE_LOOM);
    stagesA.clear();
    stagesB.clear();

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY, 2);
    workingCard.points = 3;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ORE, 3);
    workingCard.special = OneFreeCardFromDiscarded;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_LOOM, 2);
    workingCard.points = 7;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ORE, 2);
    workingCard.points = 2;
    workingCard.special = OneFreeCardFromDiscarded;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY, 3);
    workingCard.points = 1;
    workingCard.special = OneFreeCardFromDiscarded;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_ALL_MANUFACTURED);
    workingCard.special = OneFreeCardFromDiscarded;
    stagesB.push_back(AllCards::addWonder(workingCard));

    addWonder("Halikarnassus", baseIncome, stagesA, stagesB, "halikarnassus.jpg", "halikarnassusA.png", "halikarnassusB.png");


    // Gizah
    baseIncome = Resource(Resource::ONE_STONE);
    stagesA.clear();
    stagesB.clear();

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 2);
    workingCard.points = 3;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 3);
    workingCard.points = 5;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 4);
    workingCard.points = 7;
    stagesA.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_WOOD, 2);
    workingCard.points = 3;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_STONE, 3);
    workingCard.points = 5;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(Resource::ONE_CLAY, 3);
    workingCard.points = 5;
    stagesB.push_back(AllCards::addWonder(workingCard));

    workingCard = Card();
    workingCard.costResources = Resource(0, 0, 4, 0, 0, 0, 1);
    workingCard.points = 7;
    stagesB.push_back(AllCards::addWonder(workingCard));

    addWonder("Gizah", baseIncome, stagesA, stagesB, "gizah.jpg", "gizahA.png", "gizahB.png");

}


void AllWonders::addWonder(const char * name, Resource baseIncome, QVector<CardId> stagesA, QVector<CardId> stagesB, const char * image, const char * imageA, const char * imageB) {
    Wonder wonder;
    wonder.id = allWonders.size();
    wonder.name = name;
    wonder.baseIncome = baseIncome;
    wonder.stagesA = stagesA;
    wonder.stagesB = stagesB;
    wonder.image = Tools::imageWonderPath(image);
    wonder.imageA = Tools::imageWonderPath(imageA);
    wonder.imageB = Tools::imageWonderPath(imageB);
    allWonders.push_back(wonder);
}

