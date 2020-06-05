#include "action.hpp"

#include <iostream>

Action::Action() : type(noAction), card(CardIdInvalid), status(awaitingValidation) {

}


void Action::reset() {
    type = noAction;
    card = CardIdInvalid;
    boughtFromLeft = Resource();
    boughtFromRight = Resource();
    status = awaitingValidation;
}


void Action::simplifyForView() {
    if (type == buildWonder || type == discardCard) {
        card = CardIdInvalid;
    }
}


void Action::display() {
    std::cout << ActionTypeToString(type).toStdString()
              << " "
              << QString::number(card).toStdString()
              << " "
              << boughtFromLeft.display().toStdString()
              << " "
              << boughtFromRight.display().toStdString()
              << " "
              << QString::number(status).toStdString()
              << std::endl;
}


QString ActionTypeToString(ActionType actionType) {
    QString res;
    switch (actionType) {
    case noAction:
        res = "N";
        break;
    case playCard:
        res = "P";
        break;
    case buildWonder:
        res = "W";
        break;
    case discardCard:
        res = "D";
        break;
    case playFreeCard:
        res = "F";
        break;
    case playDiscarded:
        res = "I";
        break;
    case copyGuild:
        res = "G";
        break;
    }
    return res;
}


ActionType ActionTypeFromString(const QString & str) {
    ActionType actionType;
    if (str == "N") {
        actionType = noAction;
    } else if (str == "P") {
        actionType = playCard;
    } else if (str == "W") {
        actionType = buildWonder;
    } else if (str == "D") {
        actionType = discardCard;
    } else if (str == "F") {
        actionType = playFreeCard;
    } else if (str == "I") {
        actionType = playDiscarded;
    } else if (str == "G") {
        actionType = copyGuild;
    } else {
        std::cout << "ERROR PARSING WONDER FACE: " << str.toStdString() << std::endl;
    }
    return actionType;
}


QString Action::toString() const {
    QString res;
    QStringList list;
//    list.append(ActionTypeToString(type));
    list.append(QString::number(type));
    list.append(QString::number(card));
    list.append(boughtFromLeft.toString());
    list.append(boughtFromRight.toString());
    res = list.join(sep);
    return res;
}


void Action::fromString(const QString & str) {
    QStringList list = str.split(sep);
//    type = ActionTypeFromString(list[0]);
    type = (ActionType) list[0].toInt();
    card = list[1].toInt();
    boughtFromLeft.fromString(list[2]);
    boughtFromRight.fromString(list[3]);
}
