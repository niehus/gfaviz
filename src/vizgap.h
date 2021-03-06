#pragma once
#include "headers.h"
#include "vizelement.h"
#include "gfa/graph.h"
#include "gfa/gap.h"
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>
#include <QGraphicsLineItem>

using namespace ogdf;
class VizGraph;
class VizNode;

class VizGap : public VizElement {
  public:
    VizGap(GfaGap* _gfa_gap, VizGraph* _vg);
    ~VizGap();

    virtual void draw();
    virtual void initOgdf();
    virtual GfaLine* getGfaElement();
    virtual void addTreeViewInfo(VizTreeItem* parentItem);

  protected:
    virtual QPointF getCenterCoord();
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *);

  private:
    QColor getColor();
    GfaGap* gfa_gap;
    edge ogdf_edge;
    VizNode* viz_nodes[2];
    node connected_subnodes[2];
};
