#pragma once

#include <QComboBox>

/**
 * @brief Custom QComboBox to be able to show different texts for combo box and popup list.
 *
 * Current implementation is crude. editable combo box, using custom model are not supported.
 * This is implemented by abusing WhatsThisRole.
 *
 */
class ComboBox : public QComboBox {
  Q_OBJECT

 public:
  explicit ComboBox(QWidget* parent = 0);
  ~ComboBox();
  void addItemWithPopupText(const QString& text,
                            const QString& popupText,
                            const QVariant& userData = QVariant());
  QString currentText() const;

 protected:
  void paintEvent(QPaintEvent*);
  void resize(int index);

 private:
};
