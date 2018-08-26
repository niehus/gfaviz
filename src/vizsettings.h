#pragma once
#include <QString>
#include <QMap>
#include <QVariant>
#include <QCommandLineParser>
#include <QJsonObject>
#include <vector>
#include "headers.h"

enum VizGraphParam {
  //VIZ_BASESPERNODE,
  VIZ_NONE = -1,
  VIZ_BACKGROUNDCOLOR,
  VIZ_SHOWALLLABELS,
  VIZ_SHOWSEGMENTLABELS,
  VIZ_SHOWEDGELABELS,
  VIZ_SHOWGAPLABELS,
  VIZ_SHOWGROUPLABELS,
  VIZ_DISABLESEGMENTS,
  VIZ_DISABLEEDGES,
  VIZ_DISABLEGAPS,
  VIZ_DISABLEFRAGMENTS,
  VIZ_DISABLEGROUPS,
  VIZ_SEGMENTWIDTH,
  VIZ_SEGMENTOUTLINEWIDTH,
  VIZ_SEGMENTMAINCOLOR,
  VIZ_SEGMENTOUTLINECOLOR,
  VIZ_EDGEWIDTH,
  VIZ_EDGECOLOR,
  VIZ_DOVETAILWIDTH,
  VIZ_DOVETAILCOLOR,
  VIZ_INTERNALWIDTH,
  VIZ_INTERNALCOLOR,
  VIZ_GROUPWIDTH,
  VIZ_GROUPCOLORS,
  VIZ_GROUPCOLOR,
  VIZ_GAPWIDTH,
  VIZ_GAPCOLOR,
  VIZ_LABELFONT,
  VIZ_LABELFONTSIZE,
  VIZ_LABELCOLOR,
  VIZ_LABELOUTLINEWIDTH,
  VIZ_LABELOUTLINECOLOR,
  VIZ_SEGLABELFONT,
  VIZ_SEGLABELFONTSIZE,
  VIZ_SEGLABELCOLOR,
  VIZ_SEGLABELOUTLINEWIDTH,
  VIZ_SEGLABELOUTLINECOLOR,
  VIZ_EDGELABELFONT,
  VIZ_EDGELABELFONTSIZE,
  VIZ_EDGELABELCOLOR,
  VIZ_EDGELABELOUTLINEWIDTH,
  VIZ_EDGELABELOUTLINECOLOR,
  VIZ_GROUPLABELFONT,
  VIZ_GROUPLABELFONTSIZE,
  VIZ_GROUPLABELCOLOR,
  VIZ_GROUPLABELOUTLINEWIDTH,
  VIZ_GROUPLABELOUTLINECOLOR,
  VIZ_GAPLABELFONT,
  VIZ_GAPLABELFONTSIZE,
  VIZ_GAPLABELCOLOR,
  VIZ_GAPLABELOUTLINEWIDTH,
  VIZ_GAPLABELOUTLINECOLOR,
  VIZ_FRAGMENTLABELFONT,
  VIZ_FRAGMENTLABELFONTSIZE,
  VIZ_FRAGMENTLABELCOLOR,
  VIZ_FRAGMENTLABELOUTLINEWIDTH,
  VIZ_FRAGMENTLABELOUTLINECOLOR,
  VIZ_SEGLABELSHOWLENGTH,
  //VIZ_EDGELABELSHOWLENGTH,
  VIZ_MINWEIGHT,
  VIZ_USEFMMM,
  VIZ_LASTPARAM //Needs to be the last parameter
};
class VizGraphParamAttrib{
  public:
    VizGraphParamAttrib() {}
    VizGraphParamAttrib(const QString& _name, const QString& _description, QMetaType::Type _type, VizGraphParam _fallback=VIZ_NONE, QVariant _defaultvalue = QVariant(), bool _userdefined = true, bool _saveable = true);
    QString name;
    QString description;
    QMetaType::Type type;
    QVariant defaultvalue;
    VizGraphParam fallback;
    bool userdefined;
    bool saveable;
    QCommandLineOption* cmdOption;
};

class VizGraphSettings {
  public:
    VizGraphSettings();
    static std::vector<VizGraphParamAttrib> params;
    static QMap<QString,VizGraphParam> nameToParamMap; //QHash better?
    static QMap<VizGraphParam,VizGraphParam> dependentParams; //QHash better?
    static void initParams();
    static void addOptions(QCommandLineParser* parser);
    //static VizGraphParamAttrib getAttrib(VizGraphParam p);
    
    void setFromOptionParser(QCommandLineParser* parser);
    const QVariant get(VizGraphParam p, VizGraphSettings* fallback = NULL) const;
    const QVariant getDefault(VizGraphParam p) const;
    void set(VizGraphParam p, QVariant val, bool overwrite = true);
    void unset(VizGraphParam p);
    void unsetAll();
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    void fromJsonFile(QString stylesheet);
    int size();
    
    QString filename;
    double basesPerNode = 1;
    
    
    /*bool showSegmentLabels = false;
    bool showEdgeLabels = false;
    bool showGapLabels = false;
    double minWeight = 12;*/
  private:
    QMap<VizGraphParam,QVariant> values;
}; 

typedef struct {
  bool guiEnabled;
  bool renderEnabled;
  QString outputFile;
  QString outputFormat;
  unsigned int width;
  unsigned int height;
  VizGraphSettings graphSettings;
} VizAppSettings;


