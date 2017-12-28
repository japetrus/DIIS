/* ***************************************************************************
 * AppConfig.h: defines program configuration class
 * author: Joe Petrus
 * date: June 10th 2010
 * ***************************************************************************/
#ifndef AppConfig_H
#define AppConfig_H

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Headers, definitions, etc.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include <QDialog>
#include <QPushButton>
#include <QDir>
#include <QVarLengthArray>

#include <iostream>

#include "ConfigFile/ConfigFile.h"
#include "ui_PreferencesDialog.h"

using namespace std;

#define SymmetryOpt 0
#define SharpnessOpt 1
#define CuIndex 0
#define CoIndex 1
#define CuLambda 1.5418e-10
#define CoLambda 1.7902e-10

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Structures
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

struct OptimizationRegion
{
    QString name;
    double start;
    double stop;
};

struct MineralMacro
{
    QString title;
    double angleStart;
    double angleStop;
    double scale;
    double offset;
    bool autoSave;
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Main class definition
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ***************************************************************************
 * class: AppConfig
 * description: loads and saves and provides access to program configuration
 *   for other classes.  Also provides a user interface to change some
 *   preferences.
 * ***************************************************************************/
class AppConfig : public QObject
{
    Q_OBJECT

public:
    AppConfig(QWidget *parent);

    /* Access methods */
    string getLastPath() { return lastPath; }

    bool getUntilNoChange() { return untilNoChange; }
    bool getNormalizeOpt() { return normalizeOpt; }
    bool getUseSearchGrid() { return useSearchGrid; }

    int getOptIndex() { return optIndex; }
    int getXRangeSharpness() { return xRangeSharpness; }
    int getYRangeSharpness() { return yRangeSharpness; }
    int getXRangeSymmetry() { return xRangeSymmetry; }
    int getYRangeSymmetry() { return yRangeSymmetry; }

    double getAlphaRange() { return alphaRange; }
    double getPhiRange() { return phiRange; }
    double getRadiusRange() { return radiusRange; }    
    double getCircleRadius() { return circleRadius; }
    double getCameraRadius() { return cameraRadius; }
    double getIntStepSize() { return intStepSize; }
    double getIntWidth() { return intWidth; }
    double getMachineOffset() { return machineOffset; }

    double getLambda() { return (sourceIndex == CuIndex) ? CuLambda : CoLambda; }

    QMap<QString, OptimizationRegion> getOptRegions() { return optRegions; }
    OptimizationRegion getOptRegion(QString name) { return optRegions[name]; }
    QMap<QString, MineralMacro> getMineralMacros() { return mineralMacros; }
    MineralMacro getMineralMacro(QString title) { return mineralMacros[title]; }

    /* Storing methods */
    void saveOptRegion(QString name, OptimizationRegion oreg) { optRegions[name] = oreg; populateRegions(); }
    void saveMineralMacro(QString title, MineralMacro mm) {  mineralMacros[title] = mm;  }
    void setLastPath(string p) { lastPath = p; }

    void deleteMacro(QString title) { mineralMacros.remove(title); }

    void populateRegions();
    void saveRegionsFromTable();


signals:
    void valuesChanged();

public slots:
    void loadConfig();
    void saveConfig();
    void showPrefs() { prefsDialog->show(); populateRegions(); }
    void addRegion();
    void deleteRegion();

private slots:
    void updateNoChange(bool b) { untilNoChange = b; }
    void updateIntStep(double d) { intStepSize = d; }
    void updateOptIndex(int i) { optIndex = i; }
    void updateSourceIndex(int i) { sourceIndex = i; }
    void updateCircleRadius(double cR) { circleRadius = cR; }
    void updateXRangeSharpness(int x) { xRangeSharpness = x; }
    void updateYRangeSharpness(int y) { yRangeSharpness = y; }
    void updateXRangeSymmetry(int x) { xRangeSymmetry = x; }
    void updateYRangeSymmetry(int y) { yRangeSymmetry = y; }
    void updatePhiRange(double r) { phiRange = r; }
    void updateAlphaRange(double r) { alphaRange = r; }
    void updateRadiusRange(double r) { radiusRange = r/1000.0; }
    void updateCameraRadius(double r) { cameraRadius = r/1000.0; }
    void updateIntWidth(double w) { intWidth = w/1000.0; }
    void updateMachineOffset(double m) { machineOffset = m; }
    void updateUseSearchGrid(bool b) { useSearchGrid = b; }
    void updateNormalizeOpt(bool b) { normalizeOpt = b; }

private:
    Ui::PreferencesDialog UiPrefs;
    QDialog *prefsDialog;
    ConfigFile *cfgFile;

    /* General */
    int sourceIndex;
    double cameraRadius;

    /* Optimization */
    int optIndex;    
    int xRangeSymmetry;
    int yRangeSymmetry;
    int xRangeSharpness;
    int yRangeSharpness;
    bool untilNoChange;
    bool normalizeOpt;
    bool useSearchGrid;
    double phiRange;
    double alphaRange;
    double radiusRange;

    /* Integration */
    double intStepSize;
    double circleRadius;
    double intWidth;
    double machineOffset;

    /* Other */
    string lastPath;
    QMap<QString, OptimizationRegion> optRegions;
    QMap<QString, MineralMacro> mineralMacros;
};

#endif
