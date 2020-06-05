#ifndef ACTION_H
#define ACTION_H

#include <QString>

#include "card.hpp"

enum ActionType {
    noAction = 0,
    playCard = 1,
    buildWonder = 2,
    discardCard = 4,
    playFreeCard = 8,
    playDiscarded = 16,
    copyGuild = 32
};

QString ActionTypeToString(ActionType actionType);
ActionType ActionTypeFromString(const QString & str);

enum ActionStatus {
    awaitingValidation,
    validated,
    step1commited,
    step2commited
};

class Action
{
public:
    Action();
    void reset();
    void simplifyForView();
    void display();
    QString toString() const;
    void fromString(const QString & str);

    ActionType type;
    CardId card;
    Resource boughtFromLeft;
    Resource boughtFromRight;
    ActionStatus status;

private:
    static constexpr const char * sep = "}";
};

#endif // ACTION_H
