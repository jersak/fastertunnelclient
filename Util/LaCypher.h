#ifndef LACYPHER_H
#define LACYPHER_H

#include <QObject>

class LaCypher : public QObject
{
public:
    LaCypher(QObject *parent);

    static QString cypher(QString word);
};

#endif // LACYPHER_H
