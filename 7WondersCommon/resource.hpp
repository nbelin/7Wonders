#ifndef RESOURCE_H
#define RESOURCE_H

#include <QString>
#include <QVector>

typedef unsigned char Currency;
typedef unsigned char ResUnit;

class Resource {
public:
    enum ArrayId {
        IdClay = 0,
        IdOre,
        IdStone,
        IdWood,
        IdGlass,
        IdLoom,
        IdPapyrus,
        IdMAX  //<< must be last
    };

    static const int ONE_CLAY              = 1;
    static const int ONE_ORE               = 2;
    static const int ONE_STONE             = 4;
    static const int ONE_WOOD              = 8;
    static const int ONE_GLASS             = 16;
    static const int ONE_LOOM              = 32;
    static const int ONE_PAPYRUS           = 64;

    static const int ONE_ALL_RAW           = ONE_CLAY + ONE_ORE + ONE_STONE + ONE_WOOD;
    static const int ONE_ALL_MANUFACTURED  = ONE_GLASS + ONE_LOOM + ONE_PAPYRUS;
    static const int ONE_ALL     = ONE_ALL_RAW + ONE_ALL_MANUFACTURED;

    ResUnit array[IdMAX];

    Resource();
    Resource(const Resource & other);
    Resource & operator = (const Resource & other);
    Resource(ResUnit clay, ResUnit ore, ResUnit stone, ResUnit wood, ResUnit glass, ResUnit loom, ResUnit papyrus);
    Resource(int oneList, int multiplicator=1);

    bool hasOnlyOne() const;
    bool isZero() const;
    size_t getOneIndex() const;
    void addResource(const Resource & other);
    void substractResource(const Resource & other);
    void replaceByNonZero(const Resource & other);
    int multiplyAndSum(const Resource & other) const;
    QVector<size_t> commonIndexes(const Resource & other) const;

    QString display() const;
    QString toString() const;
    void fromString(const QString & str);

private:
    static constexpr const char * sep = "{";
};


#endif // RESOURCE_H
