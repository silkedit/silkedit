#pragma once

#include <QString>
#include <QObjectList>

class Filtering {
 public:
  /**
   * @brief Check if s1 contains s2 in a locale aware manner
   * @param s1
   * @param s2
   * @return
   */
  static bool contains(const QString& s1, const QString& s2);

  Filtering() = default;
  virtual ~Filtering() = default;

  virtual bool filter(const QString& filterText);
  void resetFilter();

 protected:
  void addTargetObject(QObject* object) { m_objects.append(object); }
  void addTargetObjects(QObjectList objects) { m_objects.append(objects); }

 private:
  bool filter(const QString& filterText, QObjectList objects);

  QObjectList m_objects;
};
