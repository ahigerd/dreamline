#ifndef DL_TOOL_H
#define DL_TOOL_H

#include <QApplication>
class QAction;
class QActionGroup;
class EditorView;
class QMouseEvent;

class Tool : public QObject {
Q_OBJECT
public:
  enum Type {
    MoveVertex,
    Color,
    SplitEdge,
  };
  Q_ENUM(Type)

  ~Tool() {}

  static QAction* makeAction(QActionGroup* group, Tool::Type type);
  static Tool* get(Tool::Type type);

  virtual Qt::CursorShape cursorShape() const;

  virtual void activated(EditorView* editor);
  virtual void deactivated(EditorView* editor);

  // return true to cancel the original event handler
  virtual bool mousePressEvent(EditorView* editor, QMouseEvent* event) = 0;
  virtual bool mouseMoveEvent(EditorView* editor, QMouseEvent* event) = 0;
  virtual bool mouseReleaseEvent(EditorView* editor, QMouseEvent* event) = 0;

protected:
  Tool();

private:
  static void initializeToolData();
};

template <typename T>
class BaseTool : public Tool {
public:
  static Tool* create() { return new T(); }
  ~BaseTool() {}

protected:
  BaseTool() : Tool() {}
};

#endif
