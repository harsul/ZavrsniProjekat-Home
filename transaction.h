#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QString>
#include <QDate>

struct Transaction
{
    enum Type { Income, Expense };

    int id;
    QDate date;
    Type type;
    QString category;
    double amount;
    QString description;

    Transaction() : id(-1), type(Income), amount(0.0) {}
};

#endif // TRANSACTION_H
