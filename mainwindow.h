#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QTableWidget>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QSpinBox>
#include "transaction.h"
#include "chartwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void addTransaction();
    void deleteTransaction();
    void onTypeChanged(int index);
    void onMonthFilterChanged();
    void onYearFilterChanged();
    void onChartYearChanged(int year);

private:
    Ui::MainWindow *ui;

    QComboBox *m_typeCombo;
    QComboBox *m_categoryCombo;
    QDateEdit *m_dateEdit;
    QDoubleSpinBox *m_amountSpin;
    QLineEdit *m_descriptionEdit;

    QTableWidget *m_transactionTable;
    QComboBox *m_filterMonth;
    QSpinBox *m_filterYear;
    QLabel *m_totalIncomeLabel;
    QLabel *m_totalExpenseLabel;
    QLabel *m_balanceLabel;

    ChartWidget *m_chart;
    QSpinBox *m_chartYearSpin;

    void setupUi();
    void populateCategories();
    void refreshTransactionTable();
    void refreshMonthlySummary();
    void refreshChart();
};

#endif // MAINWINDOW_H
