#ifndef DL_TOOL_H
#define DL_TOOL_H

#include <QApplication>
class QAction;
class QActionGroup;

class Tool : public QObject {
Q_OBJECT
public:
  enum Type {
    MoveVertex,
    Color,
  };
  Q_ENUM(Type)

  ~Tool() {}

  static QAction* makeAction(QActionGroup* group, Tool::Type type);
  static Tool* get(Tool::Type type);

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

class MoveVertexTool : public BaseTool<MoveVertexTool> {
Q_OBJECT
public:
  MoveVertexTool();
};

class ColorTool : public BaseTool<ColorTool> {
Q_OBJECT
public:
  ColorTool();
};

#endif
