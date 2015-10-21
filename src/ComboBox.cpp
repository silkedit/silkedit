#include <qstylepainter.h>
#include <QApplication>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QStyledItemDelegate>

#include "ComboBox.h"

// Delegate class to style a popup of a combo box
class ComboDelegate : public QStyledItemDelegate {
 public:
  void paint(QPainter* painter,
             const QStyleOptionViewItem& option,
             const QModelIndex& index) const {
    QStyleOptionViewItem newOption(option);
    QString type = index.data(Qt::AccessibleDescriptionRole).toString();
    if (type == QLatin1String("separator")) {
      QStyledItemDelegate::paint(painter, newOption, index);
      int y = (newOption.rect.top() + newOption.rect.bottom()) / 2;
      painter->setPen(newOption.palette.color(QPalette::Active, QPalette::Dark));
      painter->drawLine(newOption.rect.left(), y, newOption.rect.right(), y);
    } else {
      newOption.rect = newOption.rect.adjusted(-2, 0, 20, 0);
      QStyledItemDelegate::paint(painter, newOption, index);
    }
  }
};

ComboBox::ComboBox(QWidget* parent) : QComboBox(parent) {
  connect(this, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
          &ComboBox::resize);

  setItemDelegate(new ComboDelegate);
}

ComboBox::~ComboBox() {
}

void ComboBox::addItemWithPopupText(const QString& text,
                                    const QString& popupText,
                                    const QVariant& userData) {
  QComboBox::addItem(popupText, userData);

  if (QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model())) {
    int index = count() - 1;
    QStandardItem* item = m->item(index);
    item->setData(text, Qt::WhatsThisRole);
  }
}

QString ComboBox::currentText() const {
  if (QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model())) {
    QStandardItem* item = m->item(currentIndex());
    QVariant v = item->data(Qt::WhatsThisRole);
    if (v.isValid()) {
      return v.toString();
    }
  }

  return QComboBox::currentText();
}

void ComboBox::paintEvent(QPaintEvent*) {
  QStyleOptionComboBox option;
  initStyleOption(&option);
  if (QStandardItemModel* m = qobject_cast<QStandardItemModel*>(model())) {
    QStandardItem* item = m->item(currentIndex());
    QVariant v = item->data(Qt::WhatsThisRole);
    if (v.isValid()) {
      option.currentText = item->data(Qt::WhatsThisRole).toString();
    }
  }
  QStylePainter painter(this);
  painter.setPen(palette().color(QPalette::Text));
  painter.drawComplexControl(QStyle::CC_ComboBox, option);
  painter.drawControl(QStyle::CE_ComboBoxLabel, option);
}

void ComboBox::resize(int) {
  const QFontMetrics& font_metrics = fontMetrics();
  //  int width = font_metrics.boundingRect(text).width() + iconSize().width() + 4;
  int width = font_metrics.boundingRect(currentText()).width();
  QSize tmp(width, 0);
  QStyleOptionComboBox opt;
  initStyleOption(&opt);
  tmp = style()->sizeFromContents(QStyle::CT_ComboBox, &opt, tmp, this);
  setFixedWidth(tmp.expandedTo(QApplication::globalStrut()).width() + 13);
}
