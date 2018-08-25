#pragma once

#include "ui_LayoutOptions_SM.h"
#include "ui_LayoutOptions_FMMM.h"
#include "vizStressMinimization.h"
#include <ogdf/energybased/FMMMLayout.h>
#include "vizComponentSplitterLayout.h"
#include <vector>

using namespace std;
using namespace ogdf;

class VizGraph;


class VizLayout {
  public:
    VizLayout(VizGraph* _vg);

    virtual QString getName() {};
    virtual QString getDescription() {};
    virtual void setupOptions(QWidget *parent) {};
    virtual void deleteOptions() {};
    virtual void reset() {};
    virtual void apply(double ratio) {};
    virtual void applyFromGUI(double ratio) {};
    virtual LayoutModule* getLayoutModule() {};
    virtual QWidget* getWidget() {};
    
  protected:
    QWidget* widget;
    void exec(double ratio);
    VizGraph* vg;
    
  private:
    
};

class VizLayoutSM : public VizLayout {
  public:
    VizLayoutSM(VizGraph* _vg) : VizLayout(_vg) { 
      SM = NULL;
    }
    virtual QWidget* getWidget() {
      if (!widget) {
        widget = new QWidget();
        optionsForm.setupUi(widget);
      }
      return widget;
    };
    virtual QString getName() {
      return "Stress Minimization";
    };
    virtual QString getDescription() {
      return "Energy-based layout using stress minimization. It is slower than FMMM, but provides better visual results.";
    };
    virtual void reset();
    virtual void apply(double ratio);
    virtual void applyFromGUI(double ratio);
    virtual LayoutModule* getLayoutModule() {
      return SM;
    }
  
  protected:
    VizStressMinimization* SM;
    Ui::SMLayoutOptionForm optionsForm;
};

class VizLayoutFMMM : public VizLayout  {
  public:
    VizLayoutFMMM(VizGraph* _vg) : VizLayout(_vg) {
      FMMM = NULL;
    };
    virtual QWidget* getWidget() {
      if (!widget) {
        widget = new QWidget();
        optionsForm.setupUi(widget);
        optionsForm.QvS->addItem("Gorgeous and efficient", QVariant((int)FMMMOptions::QualityVsSpeed::GorgeousAndEfficient));
        optionsForm.QvS->addItem("Beautiful and fast", QVariant((int)FMMMOptions::QualityVsSpeed::BeautifulAndFast));
        optionsForm.QvS->addItem("Nice and incredible speed", QVariant((int)FMMMOptions::QualityVsSpeed::NiceAndIncredibleSpeed));
      }
      return widget;
    };
    virtual QString getName() {
      return "Fast Multipole Multilevel Method";
    };
    virtual QString getDescription() {
      return "FMMM is a force-directed graph drawing method. It provides poorer visual results than Stress Minimization, but is very fast and thus also suited also for very large graphs.";
    };
    virtual void reset();
    virtual void apply(double ratio);
    virtual void applyFromGUI(double ratio);
    virtual LayoutModule* getLayoutModule() {
      return FMMM;
    }
    
    
  protected:
    FMMMLayout* FMMM;
    Ui::FMMMLayoutOptionForm optionsForm;
};
