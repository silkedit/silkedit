#ifndef SCOMBOBOX_H
#define SCOMBOBOX_H

#include <QComboBox>

/**
 * @brief Custom QComboBox to be able to show different texts for combo box and popup list.
 *
 * Current implementation is crude. editable combo box, using custom model are not supported.
 * This is implemented by abusing WhatsThisRole.
 *
 */
class SComboBox : public QComboBox {
  Q_OBJECT

 public:
  explicit SComboBox(QWidget* parent = 0);
  ~SComboBox();
  void addItemWithPopupText(const QString& text,
                            const QString& popupText,
                            const QVariant& userData = QVariant());
  QString currentText() const;

 protected:
  void paintEvent(QPaintEvent*);

 private:
  void resize(int index);
};

#endif  // SCOMBOBOX_H
