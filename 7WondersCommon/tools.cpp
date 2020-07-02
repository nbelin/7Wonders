#include "tools.hpp"

namespace Tools {
    QString createPath(std::initializer_list<QString> list) {
        QString path = QCoreApplication::applicationDirPath() + QDir::separator() + "..";
        for (auto elem: list)
        {
            path += QDir::separator();
            path += elem;
        }
        return path;
    }

    QString imagePath(QString name) {
        return QString(":/images/") + name;
        return createPath({"..", "7WondersUI", "images", name});
    }

    QString imageCardPath(QString name) {
        return QString(":/images/cards/") + name;
        return createPath({"..", "7WondersUI", "images", "cards", name});
    }

    QString imageWonderPath(QString name) {
        return QString(":/images/wonders/") + name;
        return createPath({"..", "7WondersUI", "images", "wonders", name});
    }

    QString imageTokenPath(QString name) {
        return QString(":/images/tokens/") + name;
        return createPath({"..", "7WondersUI", "images", "tokens", name});
    }

    QStringList mySplit(const QString & string, const char * separator) {
        if (string.isEmpty()) {
            return QStringList();
        }
        return string.split(separator);
    }
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
