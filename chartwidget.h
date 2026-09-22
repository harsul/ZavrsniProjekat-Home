#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <QWidget>
#include <QVector>

class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = 0);

    void setData(const QVector<double> &incomes, const QVector<double> &expenses, int year);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QVector<double> m_incomes;
    QVector<double> m_expenses;
    int m_year;
};

#endif // CHARTWIDGET_H
