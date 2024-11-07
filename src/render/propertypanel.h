#ifndef DL_PROPERTYPANEL_H
#define DL_PROPERTYPANEL_H

#include <QWidget>
#include <QPointer>
#include "meshitem.h"

class PropertyPanel : public QWidget
{
Q_OBJECT
protected:
  PropertyPanel(QWidget* parent = nullptr);

public slots:
  virtual void updateAllProperties() = 0;

public:
  MeshItem* currentMesh() const;
  void setCurrentMesh(MeshItem* mesh);

private:
  QPointer<MeshItem> m_currentMesh;
};

class BlankPropertyPanel : public PropertyPanel
{
Q_OBJECT
public:
  BlankPropertyPanel(QWidget* parent = nullptr);

  virtual void updateAllProperties();
};

#endif
