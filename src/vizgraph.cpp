#include "vizgraph.h"
#include <ogdf/energybased/SpringEmbedderKK.h>
#include <ogdf/energybased/SpringEmbedderFRExact.h>
#include <ogdf/planarity/PlanarizationLayout.h>
#include <ogdf/energybased/DavidsonHarelLayout.h>
#include <ogdf/energybased/DTreeMultilevelEmbedder.h>
#include <ogdf/energybased/FastMultipoleEmbedder.h>
#include <ogdf/energybased/GEMLayout.h>
#include <ogdf/energybased/PivotMDS.h>
#include <ogdf/energybased/StressMinimization.h>
#include <ogdf/packing/ComponentSplitterLayout.h>
#include <vizStressMinimization.h>
#include <vizComponentSplitterLayout.h>

#include <ogdf/misclayout/BertaultLayout.h>
#include <vizBertaultLayout.h>

#include <QSvgGenerator>
//#include <QSvg>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QJsonDocument>
#include <QFileDialog>

//#include <ogdf/fileformats/GraphIO.h>

//todo: http://amber-v7.cs.tu-dortmund.de/doku.php/gsoc2013-ideas Node Overlap Removal (Noverlap in gephi)

VizGraph::VizGraph(const QString& filename, const VizAppSettings& appSettings, QWidget *parent) : QWidget(parent) {
  //GeneralMap map = edges;
  
  cout << "Opening " << filename.toStdString() << "..." << endl;
  //elements.resize(VIZ_ELEMENTUNKNOWN);
  //selectedElems.resize(VIZ_ELEMENTUNKNOWN);
  settings = appSettings.graphSettings;
  settings.filename = filename;
  viewWidth = appSettings.width;
  viewHeight = appSettings.height;
  setObjectName(QStringLiteral("tab"));
  form.setupUi(this);
  connect(form.ButtonZoomIn, &QPushButton::clicked, this, &VizGraph::zoomIn);
  connect(form.ButtonZoomOut, &QPushButton::clicked, this, &VizGraph::zoomOut);
  connect(form.ButtonZoomDefault, &QPushButton::clicked, this, &VizGraph::zoomDefault);
  connect(form.SearchButton, &QPushButton::clicked, this, &VizGraph::search);
  connect(form.SearchName, SIGNAL(returnPressed()),form.SearchButton,SIGNAL(clicked()));
  connect(form.StyleLoadButton, &QPushButton::clicked, this, &VizGraph::loadStyleDialog);
  connect(form.StyleSaveButton, &QPushButton::clicked, this, &VizGraph::saveStyleDialog);

  #define addStyleFormElement(a,b,c,d) connect(a,b,this,&VizGraph::styleChanged); addStyleSetting(a,c,d);
  addStyleFormElement(form.styleSegShow, &QCheckBox::stateChanged, VIZ_SEGMENT, VIZ_DISABLESEGMENTS);
  addStyleFormElement(form.styleSegWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_SEGMENT, VIZ_SEGMENTWIDTH);
  addStyleFormElement(form.styleSegColor, &ColorButton::valueChanged, VIZ_SEGMENT, VIZ_SEGMENTMAINCOLOR);
  addStyleFormElement(form.styleSegOutlineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_SEGMENT, VIZ_SEGMENTOUTLINEWIDTH);
  addStyleFormElement(form.styleSegOutlineColor, &ColorButton::valueChanged, VIZ_SEGMENT, VIZ_SEGMENTOUTLINECOLOR);
  addStyleFormElement(form.styleSegLabelShow, &QCheckBox::stateChanged, VIZ_SEGMENT, VIZ_SHOWSEGMENTLABELS);
  addStyleFormElement(form.styleSegLabelFont, &QFontComboBox::currentFontChanged, VIZ_SEGMENT, VIZ_SEGLABELFONT);
  addStyleFormElement(form.styleSegLabelSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_SEGMENT, VIZ_SEGLABELFONTSIZE);
  addStyleFormElement(form.styleSegLabelColor, &ColorButton::valueChanged, VIZ_SEGMENT, VIZ_SEGLABELCOLOR);
  addStyleFormElement(form.styleSegLabelOutlineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_SEGMENT, VIZ_SEGLABELOUTLINEWIDTH);
  addStyleFormElement(form.styleSegLabelOutlineColor, &ColorButton::valueChanged, VIZ_SEGMENT, VIZ_SEGLABELOUTLINECOLOR);
  
  addStyleFormElement(form.styleEdgeShow, &QCheckBox::stateChanged, VIZ_EDGE, VIZ_DISABLEEDGES);
  addStyleFormElement(form.styleEdgeWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_EDGE, VIZ_EDGEWIDTH);
  addStyleFormElement(form.styleEdgeColor, &ColorButton::valueChanged, VIZ_EDGE, VIZ_EDGECOLOR);
  addStyleFormElement(form.styleEdgeLabelShow, &QCheckBox::stateChanged, VIZ_EDGE, VIZ_SHOWEDGELABELS);
  addStyleFormElement(form.styleEdgeLabelFont, &QFontComboBox::currentFontChanged, VIZ_EDGE, VIZ_EDGELABELFONT);
  addStyleFormElement(form.styleEdgeLabelSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_EDGE, VIZ_EDGELABELFONTSIZE);
  addStyleFormElement(form.styleEdgeLabelColor, &ColorButton::valueChanged, VIZ_EDGE, VIZ_EDGELABELCOLOR);
  addStyleFormElement(form.styleEdgeLabelOutlineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_EDGE, VIZ_EDGELABELOUTLINEWIDTH);
  addStyleFormElement(form.styleEdgeLabelOutlineColor, &ColorButton::valueChanged, VIZ_EDGE, VIZ_EDGELABELOUTLINECOLOR);
  
  addStyleFormElement(form.styleLabelShow, &QCheckBox::stateChanged, VIZ_ELEMENTUNKNOWN, VIZ_SHOWALLLABELS);
  addStyleFormElement(form.styleLabelFont, &QFontComboBox::currentFontChanged, VIZ_ELEMENTUNKNOWN, VIZ_LABELFONT);
  addStyleFormElement(form.styleLabelSize, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_ELEMENTUNKNOWN, VIZ_LABELFONTSIZE);
  addStyleFormElement(form.styleLabelColor, &ColorButton::valueChanged, VIZ_ELEMENTUNKNOWN, VIZ_LABELCOLOR);
  addStyleFormElement(form.styleLabelOutlineWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), VIZ_ELEMENTUNKNOWN, VIZ_LABELOUTLINEWIDTH);
  addStyleFormElement(form.styleLabelOutlineColor, &ColorButton::valueChanged, VIZ_ELEMENTUNKNOWN, VIZ_LABELOUTLINECOLOR);
  
  view = form.vizCanvas;
  //view->setObjectName(QStringLiteral("vizCanvas"));
  view->setGeometry(QRect(0, 0, viewWidth+2, viewHeight+2));
  view->setDragMode(QGraphicsView::RubberBandDrag); //QGraphicsView::ScrollHandDrag);
  //view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  //view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  
  
  QElapsedTimer timer;
  timer.start();
  
  gfa = new GfaGraph();
  gfa->open(qPrintable(filename));
  //gfa->print();
  determineParams();
  if (gfa->hasTag(VIZ_OPTIONSTAG, GFA_TAG_JSON)) {
    char* styledata = gfa->getTag(VIZ_OPTIONSTAG, GFA_TAG_JSON)->getStringValue();
    QJsonDocument jsondata = QJsonDocument::fromJson(styledata);
    if (!jsondata.isNull()) {
      settings.fromJson(jsondata.object());
    }
  }
  
  GA = GraphAttributes(G, GraphAttributes::nodeGraphics | GraphAttributes::edgeGraphics | GraphAttributes::edgeDoubleWeight );
  edgeLengths = EdgeArray<double>(G);
  
  const GfaSegmentMap& gfa_segments = gfa->getSegments();
  for (auto it : gfa_segments) {
    addNode(it.second);
  }
  
  const GfaEdgeMap& gfa_edges = gfa->getEdges();
  for (auto it : gfa_edges) {
    addEdge(it.second);
  }
  
  const GfaGapMap& gfa_gaps = gfa->getGaps();
  for (auto it : gfa_gaps) {
    addGap(it.second);
  }
  
  const GfaGroupMap& gfa_groups = gfa->getGroups();
  for (auto it : gfa_groups) {
    addGroup(it.second);
  }
  
  const GfaFragmentVector& gfa_fragments = gfa->getFragments();
  for (GfaFragment* frag : gfa_fragments) {
    addFragment(frag);
  }
  
  calcLayout();
  draw();
  
  fillTreeView();
  //GraphIO::writeGML(GA, QString(filename + ".gml").toStdString());

  
  if (appSettings.renderEnabled) {
    QString outputFile = "";
    if (appSettings.outputFile == "") {
      QFileInfo fi(filename);
      outputFile = fi.fileName();
    } else {
      outputFile = appSettings.outputFile;
    }
    QString basename = outputFile;
    int suffix = 1;
    while (QFileInfo(outputFile + "." + appSettings.outputFormat).exists()) {
      outputFile = basename + "." + QString::number(suffix);
      suffix++;
    }
    renderToFile(outputFile, appSettings.outputFormat);
  }
  
  cout << "Finished after " << timer.elapsed() << " milliseconds." << endl;
}
void VizGraph::setDisplaySize(unsigned int width, unsigned int height) {
  double p_xmul = (double)width / (double)viewWidth;
  double p_ymul = (double)height / (double)viewHeight;
  double scale = min(p_xmul, p_ymul);
  view->scale(scale,scale);
  view->setGeometry(QRect(0, 0, width+2, height+2));
}
void VizGraph::calcLayout() { 
  cout << "Calculating Layout" << endl;
  /*PlanarizationLayout PL;
  PL.call(GA);
  cout << "Planarized graph" << endl;
  
  BertaultLayout bl(100); //(20,20);
  bl.call(GA);
  cout << "Layouting done" << endl;*/
  FMMMLayout* fmmm = new FMMMLayout();
  fmmm->randSeed(rand());
  //fmmm.minDistCC(150);
  //fmmm.fixedIterations(1);
  //fmmm.fineTuningIterations(0);
  fmmm->useHighLevelOptions(true);
  //fmmm.unitEdgeLength(1.0*2);
  fmmm->newInitialPlacement(true);
  //fmmm.qualityVersusSpeed(FMMMLayout::qvsGorgeousAndEfficient);
  //fmmm->repForcesStrength(3);
  //fmmm->postStrengthOfRepForces(0.2);
  //fmmm->call(GA, edgeLengths);

  //SpringEmbedderKK sekk;
  //sekk.call(GA, edgeLengths);
  
  //SpringEmbedderFRExact sefr;
  //sefr.call(GA);

  //GA.scale(15);
  //ownLayout();
  
  
  //VizBertaultLayout bl(20,20);
  //bl.call(GA);
  
  //sefr.minDistCC(5);
  //sefr.iterations(10);
  
  //DavidsonHarelLayout dhl;
  //dhl.call(GA);
  
  //DTreeMultilevelEmbedder2D *dme = new DTreeMultilevelEmbedder2D(); //Auch noch recht gut
  //dme.call(GA);
  
  //FastMultipoleEmbedder fme;
  //fme.call(GA);
  
  //GEMLayout gl;
  //gl.call(GA);
  
  //PivotMDS pmds; //sehr schnell, viele Vertizes übereinander
  //pmds.call(GA);
  
  
  StressMinimization *sm = new StressMinimization(); //Favorit!
  sm->useEdgeCostsAttribute(true);
  sm->layoutComponentsSeparately(true);
  sm->setIterations(50);
  //sm.call(GA);
  //TODO: TargetRatio des Packers anpassen
  VizComponentSplitterLayout compLayouter;//(m_hasEdgeCostsAttribute);
  compLayouter.setRatio(viewWidth/viewHeight); // * 1.3);
  compLayouter.setLayoutModule(sm);
  //compLayouter.setLayoutModule(dme);
  if (settings.get(VIZ_USEFMMM).toBool())
    compLayouter.setLayoutModule(fmmm);
  compLayouter.call(GA);

  
  //cout << GA.boundingBox().p1() << endl;
  //cout << GA.boundingBox().p2() << endl;

  //double p_xmul = view->viewport()->width() / (GA.boundingBox().p2().m_x - GA.boundingBox().p1().m_x);
  //double p_ymul = view->viewport()->height() / (GA.boundingBox().p2().m_y - GA.boundingBox().p1().m_x);
  GA.scale(2.0);
  double p_xmul = viewWidth / (GA.boundingBox().p2().m_x - GA.boundingBox().p1().m_x);
  double p_ymul = viewHeight / (GA.boundingBox().p2().m_y - GA.boundingBox().p1().m_y);
  double scale = min(p_xmul, p_ymul);
  //GA.scale(scale*1.0);
  //scale *= 0.5; 
  view->scale(scale,scale);
  //cout << view->viewport()->width() << " x " << view->viewport()->height() << endl;
}

void VizGraph::draw() {
  scene = new QGraphicsScene();
  //scene = new VizScene();
  //view->setOptimizationFlag(QGraphicsView::IndirectPainting); //TODO: bessere alternative?
  connect(scene, &QGraphicsScene::selectionChanged, this, &VizGraph::selectionChanged);
  view->setScene(scene);
  view->setRenderHints(QPainter::Antialiasing);
  scene->setBackgroundBrush(QBrush(settings.get(VIZ_BACKGROUNDCOLOR).value<QColor>()));
  
  cout << "drawing gaps... ";
  for (auto it : getGaps()) {
    it.second->draw();
  }
  cout << "done!" << endl;
  
  cout << "drawing edges... ";
  for (auto it : getEdges()) {
    it.second->draw();
  }
  cout << "done!" << endl;
  
  cout << "drawing fragments... ";
  for (auto it : getFragments()) {
    it.second->draw();
  }
  cout << "done!" << endl;
  
  cout << "drawing nodes... ";
  for (auto it : getNodes()) {
    it.second->draw();
  }
  cout << "done!" << endl;
  
  cout << "drawing groups... ";
  double epsilon = 0.001;
  for (auto it : getGroups()) {
    it.second->draw();
    it.second->setZValue(-1.0 - epsilon);
    if (it.second->labelItem)
      it.second->labelItem->setZValue(-1.0 - epsilon);
    epsilon += 0.001;
  }
  cout << "done!" << endl;
  selectionChanged(); //To disable non-needed style tabs
}

/*VizGraph::~VizGraph() {
  
}*/

void VizGraph::addNode(GfaSegment* seg) {
  elements[VIZ_SEGMENT][seg] = new VizNode(seg, this);
}

void VizGraph::addEdge(GfaEdge* edge) {
  elements[VIZ_EDGE][edge] = new VizEdge(edge, this);
}

void VizGraph::addGap(GfaGap* gap) {
  elements[VIZ_GAP][gap] = new VizGap(gap, this);
}

void VizGraph::addGroup(GfaGroup* group) {
  QStringList colors = settings.get(VIZ_GROUPCOLORS).toString().split(',');
  int coloridx = getGroups().size() % colors.size();
  elements[VIZ_GROUP][group] = new VizGroup(group, this);
  elements[VIZ_GROUP][group]->setOption(VIZ_GROUPCOLOR,colors[coloridx],false);
}

void VizGraph::addFragment(GfaFragment* fragment) {
  elements[VIZ_FRAGMENT][fragment] = new VizFragment(fragment, this);
}

void VizGraph::determineParams() {
  const GfaSegmentMap& gfa_segments = gfa->getSegments();
  double p_maxnodes = 10;
  unsigned long p_longestNode = 1;
  for (auto it : gfa_segments) {
    p_longestNode = max(p_longestNode, it.second->getLength());
  }
  settings.basesPerNode = p_longestNode / p_maxnodes;
}

VizNode* VizGraph::getNode(GfaSegment *seg) const {
  NodeMap::const_iterator it = getNodes().find(seg);
  if(it != getNodes().end())
    return it->second;
  else
    return NULL;
}

VizEdge* VizGraph::getEdge(GfaEdge *edge) const {
  EdgeMap::const_iterator it = getEdges().find(edge);
  if(it != getEdges().end())
    return it->second;
  else
    return NULL;
}

VizFragment* VizGraph::getFragment(GfaFragment* fragment) const {
  FragmentMap::const_iterator it = getFragments().find(fragment);
  if(it != getFragments().end())
    return it->second;
  else
    return NULL;
}

VizGap* VizGraph::getGap(GfaGap* gap) const {
  GapMap::const_iterator it = getGaps().find(gap);
  if(it != getGaps().end())
    return it->second;
  else
    return NULL;
}

VizGroup* VizGraph::getGroup(GfaGroup* group) const{
  GroupMap::const_iterator it = getGroups().find(group);
  if(it != getGroups().end())
    return it->second;
  else
    return NULL;
}

VizElement* VizGraph::getElement(GfaLine* line) const {
  switch(line->getType()) {
    case GFA_SEGMENT:
      return getNode((GfaSegment*)line);
    case GFA_EDGE:
      return getEdge((GfaEdge*)line);
    case GFA_FRAGMENT:
      return getFragment((GfaFragment*)line);
    case GFA_GAP:
      return getGap((GfaGap*)line);
    case GFA_GROUP:
      return getGroup((GfaGroup*)line);
    default:
      return NULL;
  }
  return NULL;
}
const NodeMap& VizGraph::getNodes() const {
  return (const NodeMap&)elements[VIZ_SEGMENT];
}
const EdgeMap& VizGraph::getEdges() const {
  return (const EdgeMap&)elements[VIZ_EDGE];
}
const GapMap& VizGraph::getGaps() const {
  return (const GapMap&)elements[VIZ_GAP];
}
const GroupMap& VizGraph::getGroups() const {
  return (const GroupMap&)elements[VIZ_GROUP];
}
const FragmentMap& VizGraph::getFragments() const {
  return (const FragmentMap&)elements[VIZ_FRAGMENT];
}

QPointF VizGraph::getNodePos(node n) {
  return QPointF(GA.x(n), GA.y(n));
}

void VizGraph::renderToFile(QString filename, QString format) {
  if (format.toUpper() == "SVG") {
    setCacheMode(QGraphicsItem::NoCache);
    QSvgGenerator svgGen;
    svgGen.setFileName(filename + "." + format);
    svgGen.setSize(QSize(viewWidth, viewHeight));
    svgGen.setViewBox(QRect(0, 0, viewWidth, viewHeight));
    svgGen.setTitle(tr("SVG Generator Example Drawing"));
    svgGen.setDescription(tr("An SVG drawing created by the SVG Generator "
                                "Example provided with Qt."));
    QPainter painter( &svgGen );
    scene->render( &painter );
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
  } else {
    QPixmap pixMap = view->grab();
    pixMap.save(filename + "." + format);
  }
}

void VizGraph::zoomIn() {
  view->scale(1.5,1.5);
}

void VizGraph::zoomOut() {
  view->scale(0.666,0.666);
}

void VizGraph::zoomDefault() {
  view->fitInView(GA.boundingBox().p1().m_x,
                  GA.boundingBox().p1().m_y,
                  GA.boundingBox().p2().m_x - GA.boundingBox().p1().m_x,
                  GA.boundingBox().p2().m_y - GA.boundingBox().p1().m_y,
                  Qt::KeepAspectRatio);
}

void VizGraph::search() {
  QString name = form.SearchName->text();
  GfaLine* line = gfa->getLine(name.toStdString());
  if (line == NULL) {
    showMessage("Element \"" + name + "\" could not be found!");
    return;
  }
  VizElement* elem = getElement(line);
  if (elem->isSelected()) {
    showMessage("Element \"" + name + "\" is already selected!");
    return;
  }
  elem->setSelected(true);
  
}

void VizGraph::setCacheMode(QGraphicsItem::CacheMode mode) {
  for (int idx = 0; idx < (int)VIZ_ELEMENTUNKNOWN; idx++) {
    for (auto it : elements[idx]) {
      it.second->setCacheMode(mode);
      if (it.second->labelItem)
        it.second->labelItem->setCacheMode(mode);
    }
  }
  /*for (auto it : gaps) {
    it.second->setCacheMode(mode);
    if (it.second->labelItem)
      it.second->labelItem->setCacheMode(mode);
  }  
  for (auto it : edges) {
    it.second->setCacheMode(mode);
    if (it.second->labelItem)
      it.second->labelItem->setCacheMode(mode);
  }
  for (auto it : getFragments()) {
    it.second->setCacheMode(mode);
    if (it.second->labelItem)
      it.second->labelItem->setCacheMode(mode);
  }
  for (auto it : nodes) {
    it.second->setCacheMode(mode);
    if (it.second->labelItem)
      it.second->labelItem->setCacheMode(mode);
  }*/
}

void VizGraph::setStyleTabEnabled(VizElementType t, bool value) {
  if (t == VIZ_SEGMENT) {
    form.StyleTabSegments->setEnabled(value);
    form.StyleTabWidget->setTabEnabled(0,value);
  } else if (t == VIZ_EDGE) {
    form.StyleTabEdges->setEnabled(value);
    form.StyleTabWidget->setTabEnabled(1,value);
  } else if (t == VIZ_GROUP) {
    form.StyleTabGroups->setEnabled(value);
    form.StyleTabWidget->setTabEnabled(2,value);
  } else if (t == VIZ_GAP) {
    form.StyleTabGaps->setEnabled(value);
    form.StyleTabWidget->setTabEnabled(3,value);
  } else if (t == VIZ_FRAGMENT) {
    form.StyleTabFragments->setEnabled(value);
    form.StyleTabWidget->setTabEnabled(4,value);
  }
}
void VizGraph::selectionChanged() {  
  QList<QGraphicsItem*> items = scene->selectedItems();
  
  if (items.size() == 0) {
    form.StyleLoadButton->setEnabled(true);
    form.StyleSaveButton->setEnabled(true);
    setStyleTabEnabled(VIZ_SEGMENT, (getNodes().size() > 0));
    setStyleTabEnabled(VIZ_EDGE, (getEdges().size() > 0));
    setStyleTabEnabled(VIZ_GROUP, (getGroups().size() > 0));
    setStyleTabEnabled(VIZ_GAP, (getGaps().size() > 0));
    setStyleTabEnabled(VIZ_FRAGMENT, (getFragments().size() > 0));
    form.selectionDisplay->setHtml("<b>Current selection:</b><br>No items selected.");
    return;
  }
  form.StyleLoadButton->setEnabled(false);
  form.StyleSaveButton->setEnabled(false);
  
  QString text = "<b>Current selection:</b><br>";
  
  for (int idx = 0; idx < (int)VIZ_ELEMENTUNKNOWN; idx++) {
    selectedElems[idx].clear();
  }
  for (QGraphicsItem* item : items) {
    VizElement* elem = (VizElement*)item;
    selectedElems[elem->getType()].insert(elem);
    if (items.size() > 1) {
      for (VizGroup* group : elem->getGroups()) {
        selectedElems[VIZ_GROUP].insert(group);
        group->setSelected(true);
      }
    }
    //cout << elem->getGfaElement()->getName() << endl;
  }
  for (int idx = 0; idx < (int)VIZ_ELEMENTUNKNOWN; idx++) {
    if (selectedElems[idx].size() == 0) {
      setStyleTabEnabled((VizElementType)idx, false);
      continue;
    }
    setStyleTabEnabled((VizElementType)idx, true);
    int unnamed = 0;
    int named = 0;
    text += "<b>" + VizElement::getTypeName((VizElementType)idx) + "s</b>: ";
    for (VizElement* elem : selectedElems[idx]) {
      if (elem->getGfaElement()->hasName()) {
        if (named > 0)
          text += ", ";
        text += QString::fromStdString(elem->getGfaElement()->getName());
        named++;
      } else {
        unnamed++;
      }
    }
    if (unnamed > 0) {
      if (named > 0)
        text += ", ";
      text += QString::number(unnamed) + " unnamed element";
      if (unnamed > 1)
        text += "s";
    }
    text += "<br>";
  }  
  form.selectionDisplay->setHtml(text);
}

void VizGraph::loadStyleDialog() {
  QFileDialog dialog(parentWidget());
  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setNameFilter("GfaViz stylesheet (*.style)");
  
  if (dialog.exec()) {
    const QStringList& filenames = dialog.selectedFiles();
    for (int i = 0; i < min(filenames.size(),1); i++) {
      //open(filenames[i]);
      cout << filenames[i].toStdString() << endl;
    }
  }
}
void VizGraph::saveStyleDialog() {
  
}

void VizGraph::addStyleSetting(QObject* w, VizElementType t, VizGraphParam p) {
  styleSettings[w] = VizStyleSetting(w,t,p);
}

void VizGraph::styleChanged() {
  QVariant value;
  StyleMap::const_iterator it = styleSettings.find(sender());
  if(it == styleSettings.end()) {
    qWarning("Unknown GUI element");
    return;
  }
  VizStyleSetting style = it->second;
  
  VizGraphParamAttrib param = VizGraphSettings::params[style.targetParam];
  if (param.type == QMetaType::Bool) {
    value = ((QCheckBox*)sender())->isChecked();
  } else if (param.type == QMetaType::QColor) {
    value = ((ColorButton*)sender())->getColor();
  } else if (param.type == QMetaType::Double) {
    value = ((QDoubleSpinBox*)sender())->value();
  } else if (param.type == QMetaType::QFont) {
    value = ((QFontComboBox*)sender())->currentFont();
  } else {
    qCritical("Not implemented");
    return;
  }
  
  for (int idx = (style.targetType != VIZ_ELEMENTUNKNOWN ? style.targetType : 0);
       idx < (style.targetType != VIZ_ELEMENTUNKNOWN ? style.targetType+1 : VIZ_ELEMENTUNKNOWN);
       idx++) {
    if (scene->selectedItems().size() > 0) {
      set<VizElement*> selected = selectedElems[idx]; //Workaround because selectedElems is changed when an item is set to invisible
      for (auto it : selected) {
        it->setOption(style.targetParam, value, true);
      }
    } else {
      settings.set(style.targetParam, value, true);
      for (auto it : elements[idx]) {
        it.second->draw();
      }
    }
  }
  //cout << value.toString().toStdString() << endl;
}
