#include "titlewidget.h"
#include <QPainter>
#include <QColor>
#include <QPen>

TitleWidget::TitleWidget(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground,true);
}

void TitleWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(this->rect(),QBrush(QColor(255,0,0,00)));
//    painter.setPen(QPen(QColor(255,255,255),3));
//    painter.drawRect(this->rect());
}
