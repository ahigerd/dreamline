#include "propertypanel.h"
#include "meshitem.h"

PropertyPanel::PropertyPanel(QWidget* parent)
: QWidget(parent), m_currentMesh(nullptr)
{
  // initializers only
}

MeshItem* PropertyPanel::currentMesh() const
{
  return m_currentMesh;
}

void PropertyPanel::setCurrentMesh(MeshItem* mesh)
{
  if (m_currentMesh) {
    QObject::disconnect(m_currentMesh, 0, this, 0);
  }
  m_currentMesh = mesh;
  updateAllProperties();
}

BlankPropertyPanel::BlankPropertyPanel(QWidget* parent)
: PropertyPanel(parent)
{
  // initializers only
}

void BlankPropertyPanel::updateAllProperties()
{
  // no-op
}
