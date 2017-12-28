/* ***************************************************************************
 * AppConfig.cpp: implements program configuration class
 * author: Joe Petrus
 * date: June 10th 2010
 * ***************************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Headers, definitions, etc.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "AppConfig.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Constructor / Destructor
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ***************************************************************************
 * method: AppConfig
 * description: constructor.  Loads the configuration and sets up dialog.
 * ***************************************************************************/
AppConfig::AppConfig(QWidget *parent) : QObject(parent)
{    
    cout << "Loading config..." << endl;
    loadConfig();
    prefsDialog = new QDialog(parent);
    UiPrefs.setupUi(prefsDialog);

    /* Initialize values of dialog from loaded config */
    UiPrefs.cbSource->setCurrentIndex(sourceIndex);
    UiPrefs.sbCameraRadius->setValue(cameraRadius*1000.0);

    UiPrefs.cbActiveOpt->setCurrentIndex(optIndex);
    UiPrefs.chkIterateUntilNoChange->setChecked(untilNoChange);
    UiPrefs.sbXRangeSharpness->setValue(xRangeSharpness);
    UiPrefs.sbYRangeSharpness->setValue(yRangeSharpness);
    UiPrefs.sbXRangeSymmetry->setValue(xRangeSymmetry);
    UiPrefs.sbYRangeSymmetry->setValue(yRangeSymmetry);
    UiPrefs.sbAlphaRange->setValue(alphaRange);
    UiPrefs.sbPhiRange->setValue(phiRange);
    UiPrefs.sbCircleRadius->setValue(circleRadius);
    UiPrefs.sbRadiusRange->setValue(radiusRange*1000.0);
    UiPrefs.sbIntWidth->setValue(intWidth*1000.0);
    UiPrefs.sbMachineOffset->setValue(machineOffset);

    connect(UiPrefs.buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(saveConfig()));
    connect(UiPrefs.buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), prefsDialog, SLOT(hide()));
    connect(UiPrefs.chkIterateUntilNoChange, SIGNAL(toggled(bool)), this, SLOT(updateNoChange(bool)));
    connect(UiPrefs.sbBinSize, SIGNAL(valueChanged(double)), this, SLOT(updateIntStep(double)));
    connect(UiPrefs.cbActiveOpt, SIGNAL(currentIndexChanged(int)), this, SLOT(updateOptIndex(int)));
    connect(UiPrefs.cbSource, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSourceIndex(int)));
    connect(UiPrefs.sbCircleRadius, SIGNAL(valueChanged(double)), this, SLOT(updateCircleRadius(double)));
    connect(UiPrefs.sbXRangeSharpness, SIGNAL(valueChanged(int)), this, SLOT(updateXRangeSharpness(int)));
    connect(UiPrefs.sbYRangeSharpness, SIGNAL(valueChanged(int)), this, SLOT(updateYRangeSharpness(int)));
    connect(UiPrefs.sbXRangeSymmetry, SIGNAL(valueChanged(int)), this, SLOT(updateXRangeSymmetry(int)));
    connect(UiPrefs.sbYRangeSymmetry, SIGNAL(valueChanged(int)), this, SLOT(updateYRangeSymmetry(int)));
    connect(UiPrefs.sbPhiRange, SIGNAL(valueChanged(double)), this, SLOT(updatePhiRange(double)));
    connect(UiPrefs.sbAlphaRange, SIGNAL(valueChanged(double)), this, SLOT(updateAlphaRange(double)));
    connect(UiPrefs.sbCameraRadius, SIGNAL(valueChanged(double)), this, SLOT(updateCameraRadius(double)));
    connect(UiPrefs.sbRadiusRange, SIGNAL(valueChanged(double)), this, SLOT(updateRadiusRange(double)));
    connect(UiPrefs.sbIntWidth, SIGNAL(valueChanged(double)), this, SLOT(updateIntWidth(double)));
    connect(UiPrefs.sbMachineOffset, SIGNAL(valueChanged(double)), this, SLOT(updateMachineOffset(double)));
    connect(UiPrefs.tbAdd, SIGNAL(clicked()), this, SLOT(addRegion()));
    connect(UiPrefs.tbDelete, SIGNAL(clicked()), this, SLOT(deleteRegion()));

    populateRegions();

}

void AppConfig::populateRegions()
{
    UiPrefs.twRegions->clear();
    UiPrefs.twRegions->setRowCount(optRegions.count());
    UiPrefs.twRegions->setColumnCount(3);

    QMapIterator<QString, OptimizationRegion> regionIter(optRegions);
    int regionCount = 0;
    while (regionIter.hasNext()) {
        regionIter.next();

        QTableWidgetItem *qtwi = new QTableWidgetItem(regionIter.value().name);
        UiPrefs.twRegions->setItem(regionCount, 0, qtwi);
        QTableWidgetItem *qtwi2 = new QTableWidgetItem(QString::number(regionIter.value().start));
        UiPrefs.twRegions->setItem(regionCount, 1, qtwi2);
        QTableWidgetItem *qtwi3 = new QTableWidgetItem(QString::number(regionIter.value().stop));
        UiPrefs.twRegions->setItem(regionCount, 2, qtwi3);

        regionCount++;
    }
}

void AppConfig::addRegion()
{
    UiPrefs.twRegions->setRowCount(UiPrefs.twRegions->rowCount() + 1);
}

void AppConfig::deleteRegion()
{
    UiPrefs.twRegions->removeRow(UiPrefs.twRegions->currentRow());
}

void AppConfig::saveRegionsFromTable()
{
    optRegions.clear();

    for (int i = 0; i < UiPrefs.twRegions->rowCount(); i++)
    {
        OptimizationRegion oreg;
        oreg.name = UiPrefs.twRegions->item(i,0)->text();
        oreg.start = UiPrefs.twRegions->item(i, 1)->text().toDouble();
        oreg.stop = UiPrefs.twRegions->item(i, 2)->text().toDouble();

        optRegions[oreg.name] = oreg;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   Configuration loading and saving methods
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ****************************************************************************
 * method: loadConfig
 * description: uses ConfigFile to load configuration from current directory
 *   in windows/linux or from package sub-directory in osx.  Default values
 *   that will be used if a configuration file is not found are located here.
 * ***************************************************************************/
void AppConfig::loadConfig()
{
    QString cdir = QDir::currentPath();
#ifdef __APPLE__
    if (QDir::currentPath().contains("DIIS.app/Contents/MacOS"))
        cdir = QDir::currentPath();
    else
        cdir = QDir::currentPath() + QString("/DIIS.app/Contents/MacOS");
#endif
    ConfigFile config( cdir.toStdString() + "/DIIS.cfg" );

    config.readInto(lastPath, "path", QDir::currentPath().toStdString());
    config.readInto(cameraRadius, "camera_radius", 0.1146/2.0);
    config.readInto(intStepSize, "integration_resolution", 0.025);
    config.readInto(untilNoChange, "iterate_until_no_change", false);
    config.readInto(optIndex, "optimization_scheme_index", 1);
    config.readInto(sourceIndex, "source_index", 1);
    config.readInto(circleRadius, "circle_radius", 2.0);
    config.readInto(xRangeSharpness, "optimization_xrange_sharpness", 10);
    config.readInto(yRangeSharpness, "optimization_yrange_sharpness", 10);
    config.readInto(xRangeSymmetry, "optimization_xrange_symmetry", 10);
    config.readInto(yRangeSymmetry, "optimization_yrange_symmetry", 10);
    config.readInto(phiRange, "optimization_phirange_sharpness", 0.01);
    config.readInto(alphaRange, "optimization_alpharange_sharpness", 0.01);
    config.readInto(radiusRange, "optimization_radiusrange_sharpness", 1.0);
    config.readInto(machineOffset, "machine_offset", 0.0);
    config.readInto(intWidth, "integration_width", 0.0254);
    config.readInto(normalizeOpt, "optimization_normalize", false);
    config.readInto(useSearchGrid, "optimization_searchgrid", false);

    int regionCount = 0;
    config.readInto(regionCount, "region_count", 0);
    for (int i = 0; i < regionCount; i++)
    {
        OptimizationRegion oreg;
        string tempName;
        int start, stop;
        config.readInto(tempName, "region_" + QString::number(i).toStdString() + "_name", tr("Region").toStdString());
        oreg.name = QString::fromStdString(tempName);

        config.readInto(start, "region_" + QString::number(i).toStdString() + "_start", 0);
        config.readInto(stop, "region_" + QString::number(i).toStdString() + "_stop", 0);

        oreg.start = start;
        oreg.stop = stop;

        optRegions[oreg.name] = oreg;
    }


    int macroCount = 0;
    config.readInto(macroCount, "macro_count", 0);

    for (int i = 0; i < macroCount; i++)
    {
        MineralMacro mm;
        string tempTitle;
        config.readInto(tempTitle, "macro_" + QString::number(i).toStdString() + "_title", tr("Macro").toStdString());
        mm.title = QString::fromStdString(tempTitle);
        config.readInto(mm.angleStart, "macro_" + QString::number(i).toStdString() + "_start", 0.0);
        config.readInto(mm.angleStop, "macro_" + QString::number(i).toStdString() + "_stop", 180.0);
        config.readInto(mm.scale, "macro_" + QString::number(i).toStdString() + "_scale", 1.0);
        config.readInto(mm.offset, "macro_" + QString::number(i).toStdString() + "_offset", 0.0);
        config.readInto(mm.autoSave, "macro_" + QString::number(i).toStdString() + "_autosave", false);
        mineralMacros[mm.title] = mm;
    }
}

/* ****************************************************************************
 * method: saveConfig
 * description: uses ConfigFile to save configuration in current directory
 *   in windows/linux or to package sub-directory in osx.  Emits a signal
 *   so that other classes can update if necessary.
 * ***************************************************************************/
void AppConfig::saveConfig()
{
    QString cdir = QDir::currentPath();
#ifdef __APPLE__
    if (QDir::currentPath().contains("DIIS.app/Contents/MacOS"))
        cdir = QDir::currentPath();
    else
        cdir = QDir::currentPath() + QString("/DIIS.app/Contents/MacOS");
#endif
    ConfigFile config( cdir.toStdString() + "/DIIS.cfg" );

    config.add("path", lastPath);
    config.add("camera_radius", cameraRadius);
    config.add("integration_resolution", intStepSize);
    config.add("iterate_until_no_change", untilNoChange);
    config.add("optimization_scheme_index", optIndex);
    config.add("source_index", sourceIndex);
    config.add("circle_radius", circleRadius);
    config.add("optimization_xrange_sharpness", xRangeSharpness);
    config.add("optimization_yrange_sharpness", yRangeSharpness);
    config.add("optimization_xrange_symmetry", xRangeSymmetry);
    config.add("optimization_yrange_symmetry", yRangeSymmetry);
    config.add("optimization_phirange_sharpness", phiRange);
    config.add("optimization_alpharange_sharpness", alphaRange);
    config.add("optimization_radiusrange_sharpness", radiusRange);
    config.add("integration_width", intWidth);
    config.add("machine_offset", machineOffset);
    config.add("optimization_searchgrid", useSearchGrid);
    config.add("optimization_normalize", normalizeOpt);

    saveRegionsFromTable();
    QMapIterator<QString, OptimizationRegion> regionIter(optRegions);
    int regionCount = 0;
    while (regionIter.hasNext()) {
        regionIter.next();

        string cfgkey = "region_" + QString::number(regionCount).toStdString() + "_";
        config.add(cfgkey + "name", regionIter.value().name.toStdString());
        config.add(cfgkey + "start", regionIter.value().start);
        config.add(cfgkey + "stop", regionIter.value().stop);
        regionCount++;
    }

    config.add("region_count", regionCount);

    QMapIterator<QString, MineralMacro> macroIter(mineralMacros);
    int macroCount = 0;
    while (macroIter.hasNext()) {
        macroIter.next();

        string cfgkey = "macro_"  + QString::number(macroCount).toStdString() + "_";
        config.add(cfgkey + "title", macroIter.value().title.toStdString());
        config.add(cfgkey + "start", macroIter.value().angleStart);
        config.add(cfgkey + "stop", macroIter.value().angleStop);
        config.add(cfgkey + "scale", macroIter.value().scale);
        config.add(cfgkey + "offset", macroIter.value().offset);
        config.add(cfgkey + "autosave", macroIter.value().autoSave);
        macroCount++;
    }

    config.add("macro_count", macroCount);

    QString outFile = cdir + QString("/DIIS.cfg");
    ofstream cfgFile(outFile.toStdString().c_str());
    if (cfgFile.good())
        cfgFile << config;

    emit valuesChanged();
    prefsDialog->hide();
}
