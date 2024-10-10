#ifndef DL_DREAMPROJECT_H
#define DL_DREAMPROJECT_H

#include <QGraphicsScene>
#include <stdexcept>
class QGraphicsRectItem;

class OpenException : public std::runtime_error
{
public:
  OpenException(const QString& what);
};

class SaveException : public std::runtime_error
{
public:
  SaveException(const QString& what);
};

class DreamProject : public QGraphicsScene
{
Q_OBJECT
public:
  DreamProject(const QSizeF& pageSize, QObject* parent = nullptr);

  QSizeF pageSize() const;
  void setPageSize(const QSizeF& size);

  void open(const QString& path);
  void save(const QString& path);

  QImage render(int dpi = 100);
  bool exportToFile(const QString& path, const QByteArray& format = QByteArray(), int dpi = 100);

  template <typename ItemType>
  static QList<ItemType*> filterItemsByType(const QList<QGraphicsItem*>& items)
  {
    QList<ItemType*> result;
    for (QGraphicsItem* genericItem : items) {
      ItemType* item = dynamic_cast<ItemType*>(genericItem);
      if (item) {
        result << item;
      }
    }
    return result;
  }

  template <typename ItemType>
  QList<ItemType*> itemsOfType() const
  {
    return filterItemsByType<ItemType>(items());
  }

  template <typename ItemType>
  QList<ItemType*> selectedItems() const
  {
    return filterItemsByType<ItemType>(selectedItems());
  }

  // Don't let the templated version shadow the normal version
  using QGraphicsScene::selectedItems;

protected:
  void drawBackground(QPainter* p, const QRectF& rect);

private:
  QRectF pageRect;
};

#endif
