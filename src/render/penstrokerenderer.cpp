#include "penstrokerenderer.h"
#include "meshitem.h"
#include "meshrenderdata.h"
#include "gripitem.h"
#include "propertypanel.h"
#include "mathutil.h"
#include <QPainter>
#include <QPainterPath>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QFrame>
#include <QColorDialog>
#include <QMouseEvent>

#define _USE_MATH_DEFINES
#include <cmath>

PenStrokeRenderer::PenStrokeRenderer()
: AbstractMeshRenderer()
{
  // initializers only
}

void PenStrokeRenderer::render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions*)
{
  int numVertices = mesh->numBoundaryVertices();
  if (numVertices < 2) {
    return;
  }

  painter->setPen(mesh->strokePen());
  QTransform transform = painter->deviceTransform().inverted();
  double dx = transform.m11();
  double dy = transform.m22();
  double pixelSize = std::sqrt(dx * dx + dy * dy);

  if (data->strokeOwner == this && data->lastStrokeScale == pixelSize) {
    painter->drawPath(data->strokePath);
    return;
  }

  data->strokeOwner = this;
  data->lastStrokeScale = pixelSize;

  QPainterPath path;
  GripItem* grip = mesh->boundaryVertex(numVertices - 1);
  QPointF lastPos = grip->scenePos();
  if (grip->isSmooth()) {
    path.moveTo((mesh->boundaryVertex(0)->scenePos() + lastPos) / 2.0);
  } else {
    path.moveTo(lastPos);
  }
  for (int i = 0; i < numVertices; i++) {
    grip = mesh->boundaryVertex(i);
    QPointF pos = grip->scenePos();
    if (grip->isSmooth()) {
      QPointF nextPos = mesh->boundaryVertex((i + 1) % numVertices)->scenePos();
      QPointF midpoint1 = (pos + lastPos) / 2.0;
      QPointF midpoint2 = (pos + nextPos) / 2.0;
      path.lineTo(midpoint1);
      double radiansPerPixel = pixelSize / (M_PI_2 * (QLineF(pos, midpoint1).length() + QLineF(pos, midpoint2).length()));
      QPointF v1 = midpoint1 - pos;
      QPointF v2 = midpoint2 - pos;
      for (double t = radiansPerPixel; t < M_PI_2; t += radiansPerPixel) {
        path.lineTo(ellipsePos(v1, v2, pos, t));
      }
      path.lineTo(midpoint2);
    } else {
      path.lineTo(grip->scenePos());
    }
    lastPos = pos;
  }
  data->strokePath = path;
  painter->drawPath(path);
}

QJsonObject PenStrokeRenderer::serialize(const MeshItem* mesh, const MeshRenderData*) const
{
  QJsonObject json;
  QPen pen = mesh->strokePen();

  json["type"] = "pen";
  json["color"] = pen.color().name();
  json["width"] = pen.widthF();
  json["join"] = (int)pen.joinStyle();
  json["cap"] = (int)pen.capStyle();
  json["dash"] = (int)pen.style();

  return json;
}

void PenStrokeRenderer::deserialize(const QJsonObject& json, MeshItem* mesh, MeshRenderData*) const
{
  QPen pen;
  QColor color;
  color.setNamedColor(json["color"].toString());
  pen.setColor(color);
  pen.setWidthF(json["width"].toDouble());
  pen.setJoinStyle(Qt::PenJoinStyle(json["join"].toInt()));
  pen.setCapStyle(Qt::PenCapStyle(json["cap"].toInt()));
  pen.setStyle(Qt::PenStyle(json["dash"].toInt()));
  mesh->setStrokePen(pen);
}

PenStrokePropertyPanel::PenStrokePropertyPanel()
: PropertyPanel(nullptr)
{
  QFormLayout* layout = new QFormLayout(this);

  color = new QFrame(this);
  color->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  color->setAutoFillBackground(true);
  color->installEventFilter(this);
  layout->addRow(tr("Color:"), color);

  penWidth = new QDoubleSpinBox(this);
  penWidth->setRange(0.1, 100.0);
  penWidth->setSingleStep(1.0);
  penWidth->setDecimals(1);
  QObject::connect(penWidth, SIGNAL(valueChanged(double)), this, SLOT(setPenWidth(double)));
  layout->addRow(tr("Width:"), penWidth);

  joinStyle = new QComboBox(this);
  joinStyle->addItem(tr("Bevel"), (int)Qt::BevelJoin);
  joinStyle->addItem(tr("Miter"), (int)Qt::MiterJoin);
  joinStyle->addItem(tr("Round"), (int)Qt::RoundJoin);
  QObject::connect(joinStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setJoinStyle()));
  layout->addRow(tr("Join Style:"), joinStyle);

  capStyle = new QComboBox(this);
  capStyle->addItem(tr("Square"), (int)Qt::SquareCap);
  capStyle->addItem(tr("Flat"), (int)Qt::FlatCap);
  capStyle->addItem(tr("Round"), (int)Qt::RoundCap);
  QObject::connect(capStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setCapStyle()));
  layout->addRow(tr("Cap Style:"), capStyle);

  dashStyle = new QComboBox(this);
  dashStyle->addItem(tr("Solid"), (int)Qt::SolidLine);
  dashStyle->addItem(tr("Dash"), (int)Qt::DashLine);
  dashStyle->addItem(tr("Dotted"), (int)Qt::DotLine);
  dashStyle->addItem(tr("Dash Dot"), (int)Qt::DashDotLine);
  dashStyle->addItem(tr("Dash Dot Dot"), (int)Qt::DashDotDotLine);
  QObject::connect(dashStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(setDashStyle()));
  layout->addRow(tr("Dash Style:"), dashStyle);
}

void PenStrokePropertyPanel::updateAllProperties()
{
  if (!currentMesh()) {
    return;
  }
  QPen pen = currentMesh()->strokePen();
  setButtonColor(pen.color());
  penWidth->setValue(pen.widthF());
}

bool PenStrokePropertyPanel::eventFilter(QObject* obj, QEvent* event)
{
  if (currentMesh() && obj == color && event->type() == QEvent::MouseButtonPress) {
    QPen pen = currentMesh()->strokePen();
    QColor newColor = QColorDialog::getColor(pen.color(), this, "Select Color");
    if (newColor.isValid()) {
      pen.setColor(newColor);
      currentMesh()->setStrokePen(pen);
      setButtonColor(newColor);
    }
    return true;
  }
  return PropertyPanel::eventFilter(obj, event);
}

void PenStrokePropertyPanel::setButtonColor(const QColor& c)
{
  QPalette p = color->palette();
  p.setBrush(QPalette::Window, c);
  color->setPalette(p);
}

void PenStrokePropertyPanel::setPenWidth(double width)
{
  if (!currentMesh()) {
    return;
  }
  QPen pen = currentMesh()->strokePen();
  pen.setWidthF(width);
  currentMesh()->setStrokePen(pen);
  currentMesh()->update();
}

void PenStrokePropertyPanel::setJoinStyle()
{
  if (!currentMesh()) {
    return;
  }
  QPen pen = currentMesh()->strokePen();
  pen.setJoinStyle(joinStyle->currentData().value<Qt::PenJoinStyle>());
  currentMesh()->setStrokePen(pen);
  currentMesh()->update();
}

void PenStrokePropertyPanel::setCapStyle()
{
  if (!currentMesh()) {
    return;
  }
  QPen pen = currentMesh()->strokePen();
  pen.setCapStyle(capStyle->currentData().value<Qt::PenCapStyle>());
  currentMesh()->setStrokePen(pen);
  currentMesh()->update();
}

void PenStrokePropertyPanel::setDashStyle()
{
  if (!currentMesh()) {
    return;
  }
  QPen pen = currentMesh()->strokePen();
  pen.setStyle(dashStyle->currentData().value<Qt::PenStyle>());
  currentMesh()->setStrokePen(pen);
  currentMesh()->update();
}
