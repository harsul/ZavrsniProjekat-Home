#include "chartwidget.h"
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QDate>
#include <qmath.h>

ChartWidget::ChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_year(QDate::currentDate().year())
{
    m_incomes.fill(0, 12);
    m_expenses.fill(0, 12);
    setMinimumHeight(300);
}

void ChartWidget::setData(const QVector<double> &incomes, const QVector<double> &expenses, int year)
{
    m_incomes = incomes;
    m_expenses = expenses;
    m_year = year;
    update();
}

void ChartWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int w = width();
    int h = height();
    int marginLeft = 70;
    int marginRight = 20;
    int marginTop = 40;
    int marginBottom = 50;

    int chartW = w - marginLeft - marginRight;
    int chartH = h - marginTop - marginBottom;

    if (chartW < 100 || chartH < 50)
        return;

    painter.fillRect(rect(), QColor(250, 250, 250));

    QFont titleFont = painter.font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.setPen(QColor(50, 50, 50));
    painter.drawText(QRect(0, 5, w, marginTop - 5), Qt::AlignCenter,
                     QString("Godisnji pregled - %1").arg(m_year));

    double maxVal = 0;
    for (int i = 0; i < 12; i++) {
        if (m_incomes[i] > maxVal) maxVal = m_incomes[i];
        if (m_expenses[i] > maxVal) maxVal = m_expenses[i];
    }
    if (maxVal < 100) maxVal = 100;
    maxVal = qCeil(maxVal / 100.0) * 100;

    painter.setPen(QPen(QColor(200, 200, 200), 1));
    QFont axisFont = painter.font();
    axisFont.setPointSize(8);
    axisFont.setBold(false);
    painter.setFont(axisFont);

    int gridLines = 5;
    for (int i = 0; i <= gridLines; i++) {
        int y = marginTop + chartH - (i * chartH / gridLines);
        painter.setPen(QPen(QColor(220, 220, 220), 1, Qt::DashLine));
        painter.drawLine(marginLeft, y, marginLeft + chartW, y);

        double val = (maxVal * i) / gridLines;
        painter.setPen(QColor(100, 100, 100));
        painter.drawText(QRect(0, y - 10, marginLeft - 5, 20), Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(val, 'f', 0));
    }

    painter.setPen(QPen(QColor(150, 150, 150), 1));
    painter.drawLine(marginLeft, marginTop, marginLeft, marginTop + chartH);
    painter.drawLine(marginLeft, marginTop + chartH, marginLeft + chartW, marginTop + chartH);

    static const char* months[] = {
        "Jan", "Feb", "Mar", "Apr", "Maj", "Jun",
        "Jul", "Aug", "Sep", "Okt", "Nov", "Dec"
    };

    double groupWidth = (double)chartW / 12.0;
    double barWidth = groupWidth * 0.35;
    double gap = groupWidth * 0.05;

    QColor incomeColor(76, 175, 80);
    QColor expenseColor(244, 67, 54);

    for (int i = 0; i < 12; i++) {
        double x = marginLeft + i * groupWidth;

        double incH = (m_incomes[i] / maxVal) * chartH;
        double expH = (m_expenses[i] / maxVal) * chartH;

        QRectF incRect(x + gap, marginTop + chartH - incH, barWidth, incH);
        painter.fillRect(incRect, incomeColor);

        QRectF expRect(x + gap + barWidth + 2, marginTop + chartH - expH, barWidth, expH);
        painter.fillRect(expRect, expenseColor);

        painter.setPen(QColor(100, 100, 100));
        painter.drawText(QRect(x, marginTop + chartH + 5, groupWidth, 20),
                         Qt::AlignCenter, months[i]);
    }

    int legendY = marginTop + chartH + 28;
    int legendX = marginLeft;

    painter.fillRect(legendX, legendY, 12, 12, incomeColor);
    painter.setPen(QColor(80, 80, 80));
    painter.drawText(legendX + 16, legendY + 11, "Prihodi");

    painter.fillRect(legendX + 90, legendY, 12, 12, expenseColor);
    painter.drawText(legendX + 106, legendY + 11, "Rashodi");
}
