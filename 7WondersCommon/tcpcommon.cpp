#include "tcpcommon.hpp"

#include <iostream>


QStringList TcpCommon::decodeMessages(const QString & data) {
    QStringList result;
    if (! data.startsWith(start)) {
        std::cout << "ERROR MESSAGE DOES NOT START WITH " << start << std::endl;
        std::cout << data.toStdString() << std::endl;
        return result;
    }
    QStringList messages = data.split(start);
    for (int i=1; i<messages.size(); ++i) { // first split is empty
        const QString & str = messages[i];
        if (! str.endsWith(end)) {
            std::cout << "ERROR MESSAGE DOES NOT END WITH " << end << std::endl;
            std::cout << str.toStdString() << "<EOF>" << std::endl;
            return result;
        }
        result.append(str.mid(0, str.size() - 1));
    }
    return result;
}


QString TcpCommon::encodeEmpty(const char * prefix) {
    QString message = start;
    message += prefix;
    message += end;
    return message;
}


QString TcpCommon::encodeSingle(const char * prefix, const QString & value) {
    QString message = start;
    message += prefix;
    message += separator;
    message += value;
    message += end;
    return message;
}


QString TcpCommon::encodeMulti(const char * prefix, const QStringList & strList) {
    QString message = start;
    message += prefix;
    for (QString str : strList) {
        message += separator;
        message += str;
    }
    message += end;
    return message;
}
