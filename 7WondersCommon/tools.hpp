#ifndef TOOLS_HPP
#define TOOLS_HPP

#include <initializer_list>

#include <QCoreApplication>
#include <QString>
#include <QStringList>
#include <QDir>

namespace Tools {
    QString createPath(std::initializer_list<QString> list);
    QString imagePath(QString name);
    QString imageCardPath(QString name);
    QString imageWonderPath(QString name);
    QString imageTokenPath(QString name);
    QStringList mySplit(const QString & string, const char * separator);
}


// avoid to include Qt GUI module just to have color names
namespace Colors {
    extern const char * white;
    extern const char * black;
    extern const char * cyan;
    extern const char * darkCyan;
    extern const char * red;
    extern const char * darkRed;
    extern const char * magenta;
    extern const char * darkMagenta;
    extern const char * green;
    extern const char * darkGreen;
    extern const char * yellow;
    extern const char * darkYellow;
    extern const char * blue;
    extern const char * darkBlue;
    extern const char * gray;
    extern const char * darkGray;
    extern const char * lightGray;
}

#endif // TOOLS_HPP

