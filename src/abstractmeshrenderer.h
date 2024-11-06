#ifndef DL_ABSTRACTMESHRENDERER_H
#define DL_ABSTRACTMESHRENDERER_H

#include <QMap>
#include <QPointer>
#include "propertypanel.h"
class MeshItem;
class MeshRenderData;
class QPainter;
class GLFunctions;
class EditorView;

class IMeshRenderer
{
public:
  virtual ~IMeshRenderer();

  virtual void render(MeshItem* mesh, MeshRenderData* data, QPainter* painter, GLFunctions* gl) = 0;

  virtual PropertyPanel* propertyPanel(EditorView* editor) = 0;

protected:
  IMeshRenderer();
};

template <typename PanelType>
class AbstractMeshRenderer : public IMeshRenderer
{
public:
  virtual PropertyPanel* propertyPanel(EditorView* _editor) final
  {
    // In a templated function, each template parameter gets its own local static variables.
    static QMap<QObject*, QPointer<PropertyPanel>> existingPanels;
    // The only reason for this cast is to avoid needing to #include "editorview.h" here.
    QObject* editor = reinterpret_cast<QObject*>(_editor);
    PropertyPanel* panel = existingPanels.value(editor, nullptr);
    if (!panel) {
      panel = new PanelType();
      if (panel) {
        if (!existingPanels.contains(editor)) {
          QObject::connect(editor, &QObject::destroyed, [](QObject* obj){ existingPanels.remove(obj); });
          QObject::connect(editor, SIGNAL(selectionChanged()), panel, SLOT(updateAllProperties()));
        }
        existingPanels[editor] = panel;
      }
    }
    return panel;
  }

protected:
  AbstractMeshRenderer() : IMeshRenderer() {}
};

#endif
