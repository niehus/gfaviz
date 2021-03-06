#include "vizapp.h"

#include <QFileInfo>
#include <QFileDialog>
#include "vizDialogs.h"
#include <QPixmapCache>

VizApp::VizApp(int & argc, char *argv[]) : QApplication(argc, argv) {
  QPixmapCache::setCacheLimit(51200);
  parseArguments();
  window = NULL;
  if (settings.guiEnabled) {
    window = new QMainWindow();
    ui.setupUi(window);
    window->show();
    connect(ui.actionOpen, &QAction::triggered, this, &VizApp::openDialog);
    connect(ui.actionRender, &QAction::triggered, this, &VizApp::renderDialog);
    connect(ui.actionClose, &QAction::triggered, this, &VizApp::quitDialog);
    connect(ui.actionSave, &QAction::triggered, this, &VizApp::saveDialog);
    connect(ui.actionAbout, &QAction::triggered, this, &VizApp::aboutDialog);
    connect(ui.actionHelp, &QAction::triggered, this, &VizApp::helpDialog);
    connect(ui.actionSelectAll, &QAction::triggered, this, &VizApp::selectAll);
    connect(ui.actionSelectNone, &QAction::triggered, this,
            &VizApp::selectNone);
    connect(ui.actionSelectInvert, &QAction::triggered, this,
            &VizApp::selectInvert);
    connect(ui.tabWidget, &QTabWidget::tabCloseRequested, this,
            &VizApp::closeTab);
  }

  const QStringList& filenames = optionParser.positionalArguments();
  for (int i = 0; i < filenames.size(); i++) {
    open(filenames[i]);
  }

  //ui.vizCanvas->scale(2.0,2.0);
}

void VizApp::open(const QString& filename) {
  VizGraph *g = new VizGraph(window);
  if (settings.guiEnabled) {
    //g->setDisplaySize(890,680);
    ui.tabWidget->addTab(g, filename);
    ui.tabWidget->setCurrentWidget(g);
    QCoreApplication::processEvents();
  }

  try {
    g->init(filename, settings);
    graphs.insert(g);
  }
  catch (const exception& e) {
    delete g;
    if (settings.guiEnabled) {
      showMessage("Error opening file \"" + filename + "\": " + e.what());
    } else {
      cerr << "Fatal error: " << e.what() << endl << endl;
    }
  }
}
void VizApp::parseArguments() {
  optionParser.addHelpOption();
  optionParser.addPositionalArgument("filenames",
                                     "Name of the file(s) to be opened.",
                                     "[filenames...]");

  QCommandLineOption optionDisableGui(QStringList() << "n" << "no-gui",
                                      "Disable GUI");
  optionParser.addOption(optionDisableGui);

  QCommandLineOption optionRender(QStringList() << "r" << "render",
                                  "Render graph(s) into file(s).");
  optionParser.addOption(optionRender);

  QCommandLineOption optionOutputFile(QStringList() << "o" << "output",
                                      "Render graph(s) into <filename>",
                                      "filename","");
  optionParser.addOption(optionOutputFile);

#ifndef NOSVG
  QCommandLineOption optionOutputFormat(QStringList() << "f" << "output-format",
                                        "File format for the output. If no "
                                        "value is specified, format will be "
                                        "inferred from the file suffix "
                                        "specified in the --output option. "
                                        "Possible values: BMP, PNG, JPG, "
                                        "JPEG, PBM, XBM, XPM, SVG. "
                                        "Default: PNG", "format", "PNG");
#else
  QCommandLineOption optionOutputFormat(QStringList() << "f" <<
                                        "output-format", "File format for the "
                                        "output. If no value is specified, "
                                        "format will be inferred from the "
                                        "file suffix specified in the "
                                        "--output option. Possible values: "
                                        "BMP, PNG, JPG, JPEG, PBM, XBM, XPM. "
                                        "Default: PNG","format","PNG");
#endif
  optionParser.addOption(optionOutputFormat);

  QCommandLineOption optionWidth(QStringList() << "W" << "width",
                                 "Width of the output file in pixels.",
                                 "width", "1120");
  optionParser.addOption(optionWidth);

  QCommandLineOption optionHeight(QStringList() << "H" << "height",
                                  "Height of the output file in pixels.",
                                  "height", "768");
  optionParser.addOption(optionHeight);

  QCommandLineOption optionTransparency(QStringList() << "t" << "transparency",
                                        "Transparent background in rendered "
                                        "images (only png).\n");
  optionParser.addOption(optionTransparency);

  QCommandLineOption optionStylesheet(QStringList() << "s" << "usestyle",
                                      "Use the style options represented by "
                                      "the stylesheet <filename>.", "filename");
  optionParser.addOption(optionStylesheet);

  VizGraphSettings::addOptions(&optionParser);
  optionParser.process(*this);

  settings.renderEnabled = (optionParser.isSet(optionOutputFile) ||
                            optionParser.isSet(optionRender));
  settings.guiEnabled = !optionParser.isSet(optionDisableGui);
  settings.outputFile = optionParser.value(optionOutputFile);

  settings.outputFormat = "png";
  int lastdot = settings.outputFile.lastIndexOf(".");
  if (lastdot != -1) {
    QString fmt = settings.outputFile.mid(lastdot+1);
    if (isValidImageFormat(fmt)) {
      settings.outputFormat = fmt;
      settings.outputFile.chop(fmt.length()+1);
    }
  }

  if (optionParser.isSet(optionOutputFormat)) {
    if (!settings.renderEnabled) {
      qWarning("--output-format option has no effect without "
               "--output or --render.");
    }
    settings.outputFormat = optionParser.value(optionOutputFormat);
    if (!isValidImageFormat(settings.outputFormat)) {
      qCritical("\"%s\" is not a valid output format!",
                qUtf8Printable(settings.outputFormat));
    }
  }
  settings.width = optionParser.value(optionWidth).toInt();
  settings.height = optionParser.value(optionHeight).toInt();
  settings.transparency = optionParser.isSet(optionTransparency);

  if (optionParser.isSet(optionStylesheet)) {
    settings.graphSettings.fromJsonFile(optionParser.value(optionStylesheet));
  }

  settings.graphSettings.setFromOptionParser(&optionParser);
}


void VizApp::openDialog() {
  QFileDialog dialog(window);
  dialog.setFileMode(QFileDialog::ExistingFiles);
  dialog.setNameFilter("All GFA files (*.gfa *.gfa1 *.gfa2 *)");

  if (dialog.exec()) {
    const QStringList& filenames = dialog.selectedFiles();
    for (int i = 0; i < filenames.size(); i++) {
      open(filenames[i]);
    }
  }
}

void VizApp::renderDialog() {
  VizRenderDialog* dialog = new VizRenderDialog(window);
  if (dialog->exec()) {
    VizGraph* g = (VizGraph*)ui.tabWidget->currentWidget();
    if (!g)
      return;
    g->renderToFile(dialog->filename, dialog->filetype, dialog->w, dialog->h,
                    dialog->transparency);
  }
}

void VizApp::saveDialog() {
  VizSaveDialog* dialog = new VizSaveDialog(window);
  if (dialog->exec()) {
    VizGraph* g = (VizGraph*)ui.tabWidget->currentWidget();
    if (!g)
      return;
    g->saveGFA(dialog->filename, dialog->filetype, dialog->saveStyle,
               dialog->saveLayout);
  }
}

void VizApp::quitDialog() {
  quit();
}

void VizApp::aboutDialog() {
  VizAboutDialog* dialog = new VizAboutDialog(window);
  dialog->exec();
}

void VizApp::helpDialog() {
  VizHelpDialog* dialog = new VizHelpDialog(window);
  dialog->exec();
}

void VizApp::closeTab(int index) {
  ((VizGraph*)ui.tabWidget->widget(index))->cancelLayout();
  delete ui.tabWidget->widget(index);
}

void VizApp::selectAll() {
  ((VizGraph*)ui.tabWidget->currentWidget())->selectAll();
}

void VizApp::selectNone() {
  ((VizGraph*)ui.tabWidget->currentWidget())->selectNone();
}

void VizApp::selectInvert() {
  ((VizGraph*)ui.tabWidget->currentWidget())->selectInvert();
}
