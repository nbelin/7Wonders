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
    const char * white = "white";
    const char * black = "black";
    const char * cyan = "cyan";
    const char * darkCyan = "darkCyan";
    const char * red = "red";
    const char * darkRed = "darkRed";
    const char * magenta = "magenta";
    const char * darkMagenta = "darkMagenta";
    const char * green = "green";
    const char * darkGreen = "darkGreen";
    const char * yellow = "yellow";
    const char * darkYellow = "darkYellow";
    const char * blue = "blue";
    const char * darkBlue = "darkBlue";
    const char * gray = "gray";
    const char * darkGray = "darkGray";
    const char * lightGray = "lightGray";
}

#endif // TOOLS_HPP

