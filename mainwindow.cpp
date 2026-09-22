#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QFrame>

static QStringList incomeCategories()
{
    return QStringList()
            << "Plata"
            << "Freelance"
            << "Investicije"
            << "Pokloni"
            << "Iznajmljivanje"
            << "Ostali prihodi";
}

static QStringList expenseCategories()
{
    return QStringList()
            << "Hrana i pice"
            << "Stanarina/Najam"
            << "Komunalije"
            << "Transport"
            << "Zabava"
            << "Odjeca"
            << "Zdravlje"
            << "Edukacija"
            << "Ostali rashodi";
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("Kucni Budzet");
    resize(950, 650);

    if (!Database::instance().open()) {
        QMessageBox::critical(this, "Greska", "Nije moguce otvoriti bazu podataka!");
    }

    setupUi();
    refreshTransactionTable();
    refreshMonthlySummary();
    refreshChart();
}

MainWindow::~MainWindow()
{
    Database::instance().close();
    delete ui;
}

void MainWindow::setupUi()
{
    QTabWidget *tabs = new QTabWidget(this);
    setCentralWidget(tabs);

    // ===================== TAB 1: PREGLED =====================
    QWidget *overviewPage = new QWidget();
    QVBoxLayout *overviewLayout = new QVBoxLayout(overviewPage);

    QGroupBox *summaryGroup = new QGroupBox("Mjesecni pregled");
    QHBoxLayout *summaryFilterLayout = new QHBoxLayout();

    QLabel *monthLabel = new QLabel("Mjesec:");
    m_filterMonth = new QComboBox();
    QStringList months;
    months << "Januar" << "Februar" << "Mart" << "April" << "Maj" << "Juni"
           << "Juli" << "August" << "Septembar" << "Oktobar" << "Novembar" << "Decembar";
    m_filterMonth->addItems(months);
    m_filterMonth->setCurrentIndex(QDate::currentDate().month() - 1);

    QLabel *yearLabel = new QLabel("Godina:");
    m_filterYear = new QSpinBox();
    m_filterYear->setRange(2020, 2050);
    m_filterYear->setValue(QDate::currentDate().year());

    summaryFilterLayout->addWidget(monthLabel);
    summaryFilterLayout->addWidget(m_filterMonth);
    summaryFilterLayout->addWidget(yearLabel);
    summaryFilterLayout->addWidget(m_filterYear);
    summaryFilterLayout->addStretch();

    QHBoxLayout *summaryCardsLayout = new QHBoxLayout();

    QFrame *incomeCard = new QFrame();
    incomeCard->setFrameShape(QFrame::StyledPanel);
    incomeCard->setStyleSheet("QFrame { background-color: #E8F5E9; border: 1px solid #A5D6A7; border-radius: 6px; padding: 10px; }");
    QVBoxLayout *incomeCardLayout = new QVBoxLayout(incomeCard);
    QLabel *incomeTitleLabel = new QLabel("Ukupni prihodi");
    incomeTitleLabel->setStyleSheet("font-weight: bold; color: #2E7D32;");
    m_totalIncomeLabel = new QLabel("0.00 KM");
    QFont bigFont = m_totalIncomeLabel->font();
    bigFont.setPointSize(16);
    bigFont.setBold(true);
    m_totalIncomeLabel->setFont(bigFont);
    m_totalIncomeLabel->setStyleSheet("color: #2E7D32;");
    incomeCardLayout->addWidget(incomeTitleLabel);
    incomeCardLayout->addWidget(m_totalIncomeLabel);

    QFrame *expenseCard = new QFrame();
    expenseCard->setFrameShape(QFrame::StyledPanel);
    expenseCard->setStyleSheet("QFrame { background-color: #FFEBEE; border: 1px solid #EF9A9A; border-radius: 6px; padding: 10px; }");
    QVBoxLayout *expenseCardLayout = new QVBoxLayout(expenseCard);
    QLabel *expenseTitleLabel = new QLabel("Ukupni rashodi");
    expenseTitleLabel->setStyleSheet("font-weight: bold; color: #C62828;");
    m_totalExpenseLabel = new QLabel("0.00 KM");
    m_totalExpenseLabel->setFont(bigFont);
    m_totalExpenseLabel->setStyleSheet("color: #C62828;");
    expenseCardLayout->addWidget(expenseTitleLabel);
    expenseCardLayout->addWidget(m_totalExpenseLabel);

    QFrame *balanceCard = new QFrame();
    balanceCard->setFrameShape(QFrame::StyledPanel);
    balanceCard->setStyleSheet("QFrame { background-color: #E3F2FD; border: 1px solid #90CAF9; border-radius: 6px; padding: 10px; }");
    QVBoxLayout *balanceCardLayout = new QVBoxLayout(balanceCard);
    QLabel *balanceTitleLabel = new QLabel("Bilans");
    balanceTitleLabel->setStyleSheet("font-weight: bold; color: #1565C0;");
    m_balanceLabel = new QLabel("0.00 KM");
    m_balanceLabel->setFont(bigFont);
    m_balanceLabel->setStyleSheet("color: #1565C0;");
    balanceCardLayout->addWidget(balanceTitleLabel);
    balanceCardLayout->addWidget(m_balanceLabel);

    summaryCardsLayout->addWidget(incomeCard);
    summaryCardsLayout->addWidget(expenseCard);
    summaryCardsLayout->addWidget(balanceCard);

    QVBoxLayout *summaryGroupLayout = new QVBoxLayout(summaryGroup);
    summaryGroupLayout->addLayout(summaryFilterLayout);
    summaryGroupLayout->addLayout(summaryCardsLayout);

    overviewLayout->addWidget(summaryGroup);

    QGroupBox *chartGroup = new QGroupBox("Godisnji grafik");
    QVBoxLayout *chartGroupLayout = new QVBoxLayout(chartGroup);

    QHBoxLayout *chartFilterLayout = new QHBoxLayout();
    QLabel *chartYearLabel = new QLabel("Godina:");
    m_chartYearSpin = new QSpinBox();
    m_chartYearSpin->setRange(2020, 2050);
    m_chartYearSpin->setValue(QDate::currentDate().year());
    chartFilterLayout->addWidget(chartYearLabel);
    chartFilterLayout->addWidget(m_chartYearSpin);
    chartFilterLayout->addStretch();

    m_chart = new ChartWidget();

    chartGroupLayout->addLayout(chartFilterLayout);
    chartGroupLayout->addWidget(m_chart, 1);

    overviewLayout->addWidget(chartGroup, 1);

    tabs->addTab(overviewPage, "Pregled");

    // ===================== TAB 2: TRANSAKCIJE =====================
    QWidget *transactionsPage = new QWidget();
    QVBoxLayout *transLayout = new QVBoxLayout(transactionsPage);

    QGroupBox *inputGroup = new QGroupBox("Nova transakcija");
    QHBoxLayout *inputMainLayout = new QHBoxLayout(inputGroup);

    QFormLayout *formLayout = new QFormLayout();

    m_typeCombo = new QComboBox();
    m_typeCombo->addItem("Prihod");
    m_typeCombo->addItem("Rashod");
    formLayout->addRow("Tip:", m_typeCombo);

    m_categoryCombo = new QComboBox();
    formLayout->addRow("Kategorija:", m_categoryCombo);

    m_dateEdit = new QDateEdit(QDate::currentDate());
    m_dateEdit->setCalendarPopup(true);
    m_dateEdit->setDisplayFormat("dd.MM.yyyy");
    formLayout->addRow("Datum:", m_dateEdit);

    m_amountSpin = new QDoubleSpinBox();
    m_amountSpin->setRange(0.01, 999999.99);
    m_amountSpin->setDecimals(2);
    m_amountSpin->setSuffix(" KM");
    m_amountSpin->setValue(0.00);
    formLayout->addRow("Iznos:", m_amountSpin);

    m_descriptionEdit = new QLineEdit();
    m_descriptionEdit->setPlaceholderText("Opis transakcije...");
    formLayout->addRow("Opis:", m_descriptionEdit);

    inputMainLayout->addLayout(formLayout);

    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->addStretch();
    QPushButton *addBtn = new QPushButton("Dodaj transakciju");
    addBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; padding: 8px 16px; border: none; border-radius: 4px; font-weight: bold; }"
                          "QPushButton:hover { background-color: #388E3C; }");
    addBtn->setMinimumHeight(35);
    buttonLayout->addWidget(addBtn);
    buttonLayout->addStretch();
    inputMainLayout->addLayout(buttonLayout);

    transLayout->addWidget(inputGroup);

    QGroupBox *listGroup = new QGroupBox("Lista transakcija");
    QVBoxLayout *listLayout = new QVBoxLayout(listGroup);

    QPushButton *deleteBtn = new QPushButton("Obrisi odabranu");
    deleteBtn->setStyleSheet("QPushButton { background-color: #F44336; color: white; padding: 6px 12px; border: none; border-radius: 4px; }"
                             "QPushButton:hover { background-color: #C62828; }");

    QHBoxLayout *listToolbar = new QHBoxLayout();
    listToolbar->addStretch();
    listToolbar->addWidget(deleteBtn);
    listLayout->addLayout(listToolbar);

    m_transactionTable = new QTableWidget();
    m_transactionTable->setColumnCount(5);
    QStringList headers;
    headers << "Datum" << "Tip" << "Kategorija" << "Iznos" << "Opis";
    m_transactionTable->setHorizontalHeaderLabels(headers);
    m_transactionTable->horizontalHeader()->setStretchLastSection(true);
    m_transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_transactionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_transactionTable->setAlternatingRowColors(true);
    m_transactionTable->verticalHeader()->setVisible(false);
    m_transactionTable->setColumnWidth(0, 100);
    m_transactionTable->setColumnWidth(1, 70);
    m_transactionTable->setColumnWidth(2, 140);
    m_transactionTable->setColumnWidth(3, 110);

    listLayout->addWidget(m_transactionTable);

    transLayout->addWidget(listGroup, 1);

    tabs->addTab(transactionsPage, "Transakcije");

    populateCategories();

    connect(m_typeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
    connect(addBtn, SIGNAL(clicked()), this, SLOT(addTransaction()));
    connect(deleteBtn, SIGNAL(clicked()), this, SLOT(deleteTransaction()));
    connect(m_filterMonth, SIGNAL(currentIndexChanged(int)), this, SLOT(onMonthFilterChanged()));
    connect(m_filterYear, SIGNAL(valueChanged(int)), this, SLOT(onYearFilterChanged()));
    connect(m_chartYearSpin, SIGNAL(valueChanged(int)), this, SLOT(onChartYearChanged(int)));
}

void MainWindow::populateCategories()
{
    m_categoryCombo->clear();
    if (m_typeCombo->currentIndex() == 0)
        m_categoryCombo->addItems(incomeCategories());
    else
        m_categoryCombo->addItems(expenseCategories());
}

void MainWindow::onTypeChanged(int)
{
    populateCategories();
}

void MainWindow::addTransaction()
{
    if (m_amountSpin->value() < 0.01) {
        QMessageBox::warning(this, "Greska", "Molimo unesite iznos veci od 0.");
        return;
    }

    Transaction t;
    t.date = m_dateEdit->date();
    t.type = (m_typeCombo->currentIndex() == 0) ? Transaction::Income : Transaction::Expense;
    t.category = m_categoryCombo->currentText();
    t.amount = m_amountSpin->value();
    t.description = m_descriptionEdit->text().trimmed();

    if (!Database::instance().addTransaction(t)) {
        QMessageBox::warning(this, "Greska", "Nije moguce sacuvati transakciju u bazu.");
        return;
    }

    m_amountSpin->setValue(0.00);
    m_descriptionEdit->clear();

    refreshTransactionTable();
    refreshMonthlySummary();
    refreshChart();
}

void MainWindow::deleteTransaction()
{
    int row = m_transactionTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Info", "Molimo odaberite transakciju za brisanje.");
        return;
    }

    int id = m_transactionTable->item(row, 0)->data(Qt::UserRole).toInt();

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Potvrda",
        "Da li ste sigurni da zelite obrisati ovu transakciju?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        Database::instance().removeTransaction(id);
        refreshTransactionTable();
        refreshMonthlySummary();
        refreshChart();
    }
}

void MainWindow::refreshTransactionTable()
{
    QList<Transaction> transactions = Database::instance().allTransactions();

    m_transactionTable->setRowCount(transactions.size());
    for (int i = 0; i < transactions.size(); i++) {
        const Transaction &t = transactions[i];

        QTableWidgetItem *dateItem = new QTableWidgetItem(t.date.toString("dd.MM.yyyy"));
        dateItem->setTextAlignment(Qt::AlignCenter);
        dateItem->setData(Qt::UserRole, t.id);

        QString typeStr = (t.type == Transaction::Income) ? "Prihod" : "Rashod";
        QTableWidgetItem *typeItem = new QTableWidgetItem(typeStr);
        typeItem->setTextAlignment(Qt::AlignCenter);
        if (t.type == Transaction::Income)
            typeItem->setForeground(QColor(46, 125, 50));
        else
            typeItem->setForeground(QColor(198, 40, 40));

        QTableWidgetItem *catItem = new QTableWidgetItem(t.category);

        QString amountStr = QString::number(t.amount, 'f', 2) + " KM";
        QTableWidgetItem *amountItem = new QTableWidgetItem(amountStr);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        if (t.type == Transaction::Income)
            amountItem->setForeground(QColor(46, 125, 50));
        else
            amountItem->setForeground(QColor(198, 40, 40));

        QTableWidgetItem *descItem = new QTableWidgetItem(t.description);

        m_transactionTable->setItem(i, 0, dateItem);
        m_transactionTable->setItem(i, 1, typeItem);
        m_transactionTable->setItem(i, 2, catItem);
        m_transactionTable->setItem(i, 3, amountItem);
        m_transactionTable->setItem(i, 4, descItem);
    }
}

void MainWindow::refreshMonthlySummary()
{
    int month = m_filterMonth->currentIndex() + 1;
    int year = m_filterYear->value();

    double totalIncome = Database::instance().monthlyTotal(year, month, Transaction::Income);
    double totalExpense = Database::instance().monthlyTotal(year, month, Transaction::Expense);

    m_totalIncomeLabel->setText(QString::number(totalIncome, 'f', 2) + " KM");
    m_totalExpenseLabel->setText(QString::number(totalExpense, 'f', 2) + " KM");

    double balance = totalIncome - totalExpense;
    m_balanceLabel->setText(QString::number(balance, 'f', 2) + " KM");

    if (balance >= 0)
        m_balanceLabel->setStyleSheet("color: #2E7D32; font-size: 16pt; font-weight: bold;");
    else
        m_balanceLabel->setStyleSheet("color: #C62828; font-size: 16pt; font-weight: bold;");
}

void MainWindow::refreshChart()
{
    int year = m_chartYearSpin->value();
    QVector<double> incomes, expenses;
    Database::instance().yearlyTotals(year, incomes, expenses);
    m_chart->setData(incomes, expenses, year);
}

void MainWindow::onMonthFilterChanged()
{
    refreshMonthlySummary();
}

void MainWindow::onYearFilterChanged()
{
    refreshMonthlySummary();
}

void MainWindow::onChartYearChanged(int)
{
    refreshChart();
}
