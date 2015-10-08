#include <qstylepainter.h>
#include <QApplication>
#include <QStandardItemModel>

#include "SComboBox.h"

SComboBox::SComboBox(QWidget* parent) : QComboBox(parent) {
  connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          &SComboBox::resize);
}

SComboBox::~SComboBox() {
}

void SComboBox::addItemWithPopupText(const QString& text,
                                     const QString& popupText,
                                     const QVariant& userData) {
  QComboBox::addItem(popupText, userData);

  if (QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model())) {
    QStandardItem* item = m->item(count() - 1);
    item->setData(text, Qt::WhatsThisRole);
  }
}

QString SComboBox::currentText() const {
  if (QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model())) {
    QStandardItem* item = m->item(currentIndex());
    QVariant v = item->data(Qt::WhatsThisRole);
    if (v.isValid()) {
      return item->data(Qt::WhatsThisRole).toString();
    }
  }

  return QComboBox::currentText();
}

// Copied from qcombobox.cpp
void SComboBox::paintEvent(QPaintEvent*) {
  QStylePainter painter(this);
  painter.setPen(palette().color(QPalette::Text));

  // draw the combobox frame, focusrect and selected etc.
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  if (QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model())) {
    QStandardItem* item = m->item(currentIndex());
    QVariant v = item->data(Qt::WhatsThisRole);
    if (v.isValid()) {
      opt.currentText = item->data(Qt::WhatsThisRole).toString();
    }
  }
  painter.drawComplexControl(QStyle::CC_ComboBox, opt);

  // draw the icon and text
  painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void SComboBox::resize(int) {
  const QFontMetrics& font_metrics = fontMetrics();
  //  int width = font_metrics.boundingRect(text).width() + iconSize().width() + 4;
  int width = font_metrics.boundingRect(currentText()).width();
  QSize tmp(width, 0);
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  tmp = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, tmp, this);
  setFixedWidth(tmp.expandedTo(QApplication::globalStrut()).width());
}
