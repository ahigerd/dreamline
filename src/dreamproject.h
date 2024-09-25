#ifndef DL_DREAMPROJECT_H
#define DL_DREAMPROJECT_H

#include <QGraphicsScene>
class QGraphicsRectItem;

class DreamProject : public QGraphicsScene
{
Q_OBJECT
public:
  DreamProject(const QSizeF& pageSize, QObject* parent = nullptr);

  QSizeF pageSize() const;
  void setPageSize(const QSizeF& size);

protected:
  void drawBackground(QPainter* p, const QRectF& rect);

private:
  QRectF pageRect;
};

#endif
