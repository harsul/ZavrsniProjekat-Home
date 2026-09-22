#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QString>
#include <QDate>
#include <QJsonObject>

struct Transaction
{
    enum Type { Income, Expense };

    QDate date;
    Type type;
    QString category;
    double amount;
    QString description;

    QJsonObject toJson() const
    {
        QJsonObject obj;
        obj["date"] = date.toString(Qt::ISODate);
        obj["type"] = (type == Income) ? "income" : "expense";
        obj["category"] = category;
        obj["amount"] = amount;
        obj["description"] = description;
        return obj;
    }

    static Transaction fromJson(const QJsonObject &obj)
    {
        Transaction t;
        t.date = QDate::fromString(obj["date"].toString(), Qt::ISODate);
        t.type = (obj["type"].toString() == "income") ? Income : Expense;
        t.category = obj["category"].toString();
        t.amount = obj["amount"].toDouble();
        t.description = obj["description"].toString();
        return t;
    }
};

#endif // TRANSACTION_H
