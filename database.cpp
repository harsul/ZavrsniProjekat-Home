#include "database.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QVariant>
#include <QVector>
#include <QDate>

Database &Database::instance()
{
    static Database db;
    return db;
}

Database::Database()
{
}

Database::~Database()
{
    close();
}

QString Database::dbFilePath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);
    if (!dir.exists())
        dir.mkpath(".");
    return path + "/kucni_budzet.db";
}

bool Database::open()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(dbFilePath());

    if (!m_db.open())
        return false;

    createTables();
    return true;
}

void Database::close()
{
    if (m_db.isOpen())
        m_db.close();
}

void Database::createTables()
{
    QSqlQuery query(m_db);
    query.exec(
        "CREATE TABLE IF NOT EXISTS transactions ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  date TEXT NOT NULL,"
        "  type TEXT NOT NULL,"
        "  category TEXT NOT NULL,"
        "  amount REAL NOT NULL,"
        "  description TEXT"
        ")"
    );
}

bool Database::addTransaction(const Transaction &t)
{
    QSqlQuery query(m_db);
    query.prepare(
        "INSERT INTO transactions (date, type, category, amount, description) "
        "VALUES (:date, :type, :category, :amount, :description)"
    );
    query.bindValue(":date", t.date.toString(Qt::ISODate));
    query.bindValue(":type", (t.type == Transaction::Income) ? "income" : "expense");
    query.bindValue(":category", t.category);
    query.bindValue(":amount", t.amount);
    query.bindValue(":description", t.description);
    return query.exec();
}

bool Database::removeTransaction(int id)
{
    QSqlQuery query(m_db);
    query.prepare("DELETE FROM transactions WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}

QList<Transaction> Database::allTransactions()
{
    QList<Transaction> list;
    QSqlQuery query(m_db);
    query.exec("SELECT id, date, type, category, amount, description FROM transactions ORDER BY date DESC");

    while (query.next()) {
        Transaction t;
        t.id = query.value(0).toInt();
        t.date = QDate::fromString(query.value(1).toString(), Qt::ISODate);
        t.type = (query.value(2).toString() == "income") ? Transaction::Income : Transaction::Expense;
        t.category = query.value(3).toString();
        t.amount = query.value(4).toDouble();
        t.description = query.value(5).toString();
        list.append(t);
    }
    return list;
}

QList<Transaction> Database::transactionsByMonth(int year, int month)
{
    QList<Transaction> list;
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, date, type, category, amount, description "
        "FROM transactions "
        "WHERE strftime('%Y', date) = :year AND strftime('%m', date) = :month "
        "ORDER BY date DESC"
    );
    query.bindValue(":year", QString::number(year));
    query.bindValue(":month", QString("%1").arg(month, 2, 10, QChar('0')));
    query.exec();

    while (query.next()) {
        Transaction t;
        t.id = query.value(0).toInt();
        t.date = QDate::fromString(query.value(1).toString(), Qt::ISODate);
        t.type = (query.value(2).toString() == "income") ? Transaction::Income : Transaction::Expense;
        t.category = query.value(3).toString();
        t.amount = query.value(4).toDouble();
        t.description = query.value(5).toString();
        list.append(t);
    }
    return list;
}

double Database::monthlyTotal(int year, int month, Transaction::Type type)
{
    QSqlQuery query(m_db);
    query.prepare(
        "SELECT COALESCE(SUM(amount), 0) FROM transactions "
        "WHERE strftime('%Y', date) = :year "
        "AND strftime('%m', date) = :month "
        "AND type = :type"
    );
    query.bindValue(":year", QString::number(year));
    query.bindValue(":month", QString("%1").arg(month, 2, 10, QChar('0')));
    query.bindValue(":type", (type == Transaction::Income) ? "income" : "expense");
    query.exec();

    if (query.next())
        return query.value(0).toDouble();
    return 0.0;
}

void Database::yearlyTotals(int year, QVector<double> &incomes, QVector<double> &expenses)
{
    incomes.fill(0, 12);
    expenses.fill(0, 12);

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT strftime('%m', date) AS month, type, SUM(amount) "
        "FROM transactions "
        "WHERE strftime('%Y', date) = :year "
        "GROUP BY month, type"
    );
    query.bindValue(":year", QString::number(year));
    query.exec();

    while (query.next()) {
        int month = query.value(0).toInt() - 1;
        QString type = query.value(1).toString();
        double total = query.value(2).toDouble();

        if (month >= 0 && month < 12) {
            if (type == "income")
                incomes[month] = total;
            else
                expenses[month] = total;
        }
    }
}
