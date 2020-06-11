#include "resource.hpp"

#include <QStringList>

Resource::Resource() {
    for ( size_t i=0; i<IdMAX; ++i ) {
        array[i] = 0;
    }
}


Resource::Resource(const Resource & other) {
    for ( size_t i=0; i<IdMAX; ++i ) {
        array[i] = other.array[i];
    }
}


Resource::Resource(ResUnit clay, ResUnit ore, ResUnit stone, ResUnit wood, ResUnit glass, ResUnit loom, ResUnit papyrus) {
    array[IdClay] = clay;
    array[IdOre] = ore;
    array[IdStone] = stone;
    array[IdWood] = wood;
    array[IdGlass] = glass;
    array[IdLoom] = loom;
    array[IdPapyrus] = papyrus;
}


Resource::Resource(int oneList, int multiplicator): Resource() {
    if ( ( oneList & ONE_CLAY ) != 0 ) {
        array[IdClay] = multiplicator;
    }
    if ( ( oneList & ONE_ORE ) != 0 ) {
        array[IdOre] = multiplicator;
    }
    if ( ( oneList & ONE_STONE ) != 0 ) {
        array[IdStone] = multiplicator;
    }
    if ( ( oneList & ONE_WOOD ) != 0 ) {
        array[IdWood] = multiplicator;
    }
    if ( ( oneList & ONE_GLASS ) != 0 ) {
        array[IdGlass] = multiplicator;
    }
    if ( ( oneList & ONE_LOOM ) != 0 ) {
        array[IdLoom] = multiplicator;
    }
    if ( ( oneList & ONE_PAPYRUS ) != 0 ) {
        array[IdPapyrus] = multiplicator;
    }
}


Resource & Resource::operator = (const Resource & other) {
    for ( size_t i=0; i<IdMAX; ++i ) {
        array[i] = other.array[i];
    }
    return *this;
}


bool Resource::operator == (const Resource & other) const {
    for ( size_t i=0; i<IdMAX; ++i ) {
        if (array[i] != other.array[i]) {
            return false;
        }
    }
    return true;
}


bool Resource::hasOnlyOne() const {
    size_t nbNonZeros = 0;
    for ( unsigned char resource : array ) {
        if (resource > 0) {
            nbNonZeros++;
        }
        if (nbNonZeros > 1) {
            return false;
        }
    }
    return nbNonZeros == 1;
}


bool Resource::isZero() const {
    for ( unsigned char resource : array ) {
        if (resource > 0) {
            return false;
        }
    }
    return true;
}


size_t Resource::getOneIndex() const {
    for (size_t i=0; i<IdMAX; ++i) {
        if (array[i] > 0) {
            return i;
        }
    }
    return 0;
}


void Resource::addResource(const Resource & other) {
    for (size_t i=0; i<IdMAX; ++i) {
        array[i] += other.array[i];
    }
}


void Resource::substractResource(const Resource & other) {
    for (size_t i=0; i<IdMAX; ++i) {
        if (array[i] > other.array[i]) {
            array[i] -= other.array[i];
        } else {
            array[i] = 0;
        }
    }
}


void Resource::replaceByNonZero(const Resource & other) {
    for (size_t i=0; i<IdMAX; ++i) {
        if (other.array[i] > 0) {
            array[i] = other.array[i];
        }
    }
}


int Resource::multiplyAndSum(const Resource & other) const {
    int sum = 0;
    for (size_t i=0; i<IdMAX; ++i) {
        sum += array[i] * other.array[i];
    }
    return sum;
}


QVector<size_t> Resource::commonIndexes(const Resource & other) const {
    QVector<size_t> res;
    for (size_t i=0; i<IdMAX; ++i) {
        if ((array[i] * other.array[i]) > 0) {
            res.append(i);
        }
    }
    return res;
}


QString Resource::display() const {
    static const char * toStr[] = {"C ", "O ", "S ", "W ", "G ", "L ", "P "};
    QString retStr;

    for ( size_t i=0; i<IdMAX; ++i ) {
        if ( array[i] != 0 ) {
            if (array[i] > 1) {
                retStr += QString::number(array[i]);
            }
            retStr += toStr[i];
        }
    }

    return retStr;
}


QString Resource::toString() const {
    QString res;
    QStringList list;
    for (size_t i=0; i<IdMAX; ++i) {
        list.append(QString::number(array[i]));
    }
    res = list.join(sep);
    return res;
}


void Resource::fromString(const QString & str) {
    QStringList list = str.split(sep);
    for (size_t i=0; i<IdMAX; ++i) {
        array[i] = list[i].toInt();
    }
}
