#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QList>
#include "transaction.h"

class Database
{
public:
    static Database &instance();

    bool open();
    void close();

    bool addTransaction(const Transaction &t);
    bool removeTransaction(int id);
    QList<Transaction> allTransactions();
    QList<Transaction> transactionsByMonth(int year, int month);

    double monthlyTotal(int year, int month, Transaction::Type type);
    void yearlyTotals(int year, QVector<double> &incomes, QVector<double> &expenses);

private:
    Database();
    ~Database();
    Database(const Database &);
    Database &operator=(const Database &);

    QSqlDatabase m_db;
    void createTables();
    QString dbFilePath();
};

#endif // DATABASE_H
