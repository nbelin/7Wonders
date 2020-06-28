#ifndef CHOICE_HPP
#define CHOICE_HPP

#include <QString>
#include <QVector>

#include "wonder.hpp"
#include "tools.hpp"
#include "playerview.hpp"

class Choice
{
public:
    struct PlayerChoice {
        PlayerId id;
        QString name;
        WonderId wonderId;
        bool ready;
    };

    Choice();
    QString toString() const;
    void fromString(const QString & str);

    QVector<PlayerChoice> players;
    int numberAIs;
    bool randomWonders;
    bool randomFaces;
    bool randomPlaces;

private:
    static constexpr const char * sep = "~";
    static constexpr const char * subSep = "/";
    static constexpr const char * subSubSep = "}";
};

#endif // CHOICE_HPP
