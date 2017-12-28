/*
 *  TwoThetaWindow.h
 *  GandolfiViewer
 *
 *  Created by Joe Petrus on 08/02/10.
 *
 */

#ifndef TwoThetaWindow_H
#define TwoThetaWindow_H

/* QT Related headers */
#include <QMainWindow>
#include <QLabel>
#include <QMenu>
#include <QPrinter>
#include <QMouseEvent>
#include <QPen>
#include <QFileDialog>
#include <QDir>
#include <QInputDialog>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QStyle>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDockWidget>
#include <QDateTime>
#include <QPainter>
#include <QPrintDialog>
#include <QDesktopServices>
#include <QUrl>


#include "ui_MacroDialog.h"

/* QWT Related headers */
#ifdef __APPLE__
#include "/usr/local/qwt-5.2.2/include/qwt.h"
#include "/usr/local/qwt-5.2.2/include/qwt_plot.h"
#include "/usr/local/qwt-5.2.2/include/qwt_plot_zoomer.h"
#include "/usr/local/qwt-5.2.2/include/qwt_plot_picker.h"
#include "/usr/local/qwt-5.2.2/include/qwt_painter.h"
#include "/usr/local/qwt-5.2.2/include/qwt_plot_canvas.h"
#include "/usr/local/qwt-5.2.2/include/qwt_plot_marker.h"
#include "/usr/local/qwt-5.2.2/include/qwt_plot_curve.h"
#include "/usr/local/qwt-5.2.2/include/qwt_scale_widget.h"
#include "/usr/local/qwt-5.2.2/include/qwt_legend.h"
#include "/usr/local/qwt-5.2.2/include/qwt_scale_draw.h"
#include "/usr/local/qwt-5.2.2/include/qwt_math.h"
#include "/usr/local/qwt-5.2.2/include/qwt_symbol.h"
#else
#include "c:\qwt-5.2.0/include/qwt.h"
#include "c:\qwt-5.2.0/include/qwt_plot.h"
#include "c:\qwt-5.2.0/include/qwt_plot_zoomer.h"
#include "c:\qwt-5.2.0/include/qwt_plot_picker.h"
#include "c:\qwt-5.2.0/include/qwt_painter.h"
#include "c:\qwt-5.2.0/include/qwt_plot_canvas.h"
#include "c:\qwt-5.2.0/include/qwt_plot_marker.h"
#include "c:\qwt-5.2.0/include/qwt_plot_curve.h"
#include "c:\qwt-5.2.0/include/qwt_scale_widget.h"
#include "c:\qwt-5.2.0/include/qwt_legend.h"
#include "c:\qwt-5.2.0/include/qwt_scale_draw.h"
#include "c:\qwt-5.2.0/include/qwt_math.h"
#include "c:\qwt-5.2.0/include/qwt_symbol.h"
#endif

#include <cmath>
#include <iostream>
#include <fstream>
#include <ctime>

#include "AppConfig.h"
#include "TwoThetaPlot.h"
#include "ui_MineralSearchDialog.h"

using namespace std;

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QwtPlot;
class XYPlot;
class TwoThetaPlot;
QT_END_NAMESPACE

/* Weighted Average Cu and Co wavelengths */
#define CuLambda 1.5418e-10
#define CoLambda 1.7902e-10

struct Mineral
{
    QString name;
    QList<QString> elements;
    QString formula;
    QVarLengthArray<double> lines;
    QVarLengthArray<double> intensities;
    int nLines;
};

class QToolMacro : public QToolButton
{
    Q_OBJECT

public:
    QToolMacro(QWidget *parent) : QToolButton(parent) { rightDown = false; }

    void mouseReleaseEvent(QMouseEvent *e)
    {
        rightDown = false;
        QToolButton::mouseReleaseEvent(e);
    }

    void mousePressEvent(QMouseEvent *e) {
        if (e->button() == Qt::RightButton )
            rightDown = true;
        else
            rightDown = false;

        QToolButton::mousePressEvent(e);
        QToolButton::click();
    }

    bool isRightClick() { return rightDown; }

private:
    bool rightDown;
};

class TwoThetaWindow : public QMainWindow
{
    Q_OBJECT

public:
    TwoThetaWindow(QWidget *parent, AppConfig &_appConfig);

    void setXYData(double *xd, double *yd, int s, int sx = 0, int ex = 180);
    void setYMinData(double *yd) { twoThetaXYPlot->setYMinData(yd); }
    void setYMaxData(double *yd) { twoThetaXYPlot->setYMaxData(yd); }
    void setSuggestedName(QString s) { suggestedName = s.split(".").at(0); }
    void doScale(double s);
    void doOffset(double o);
    void sharpness();
    void findPrimarySpacings();
    void contextMenuEvent(QContextMenuEvent *event);
    void createToolbar();

    void initializeDB();

    QList<QString> getElementsFromString(QString s);

    void keyPressEvent(QKeyEvent *);

public slots:
    void saveUDF();
    void saveCSV();
    void reset();
   // void finishedSlot(QNetworkReply* reply);
    void searchClicked();
    void openWebMineral();
    void showMineralLines(QListWidgetItem *);
    void showMineralLines(int);
    void addMacro();

private slots:		
    void selected(const QwtDoublePoint &pos);
    void enableZoomMode(bool z);
    void toggleMinMax(bool b);
    void toggleBGS(bool b);
    void intOffset(bool prompt = true, double offset = 0.0);
    void intScale(bool prompt = true, double scale = 1.0);
    void truncate(bool prompt = true, double start = 0.0, double stop = 180.0);
    void doMacro(QAction *);
    void deleteMacro();

    void printData();

private:
    AppConfig *appConfig;
    QWidget *searchDialog;
    QDialog *macroDialog;
    QDockWidget *dockWidget;
    Ui::MineralSearchDialog UiD;
    TwoThetaPlot *twoThetaXYPlot;
    QwtPlotZoomer *zoomer;
    QwtPlotPicker *picker;
    //QComboBox *cbxSource;
    QToolBar *toolbar;
    QStatusBar *sb;
    QPrinter printer;
    QMap<QString, Mineral> mindb;
    QVarLengthArray<QwtPlotMarker *> minMarkers;
    QwtSymbol minSymbol;
    QString currentMineral;

    QVarLengthArray<QString> mineralUrls;
    int activeD;

    QVarLengthArray<QToolMacro *> macroButtonArray;
   // QToolMacro *btnMacro1;
   // QToolMacro *btnMacro2;
   // QToolMacro *btnMacro3;

    QString suggestedName;


    Ui::MacroDialog UiMacro;

    double start2Theta;
    double end2Theta;
};


inline bool operator<(const Mineral &m1, const Mineral &m2)
{
    return m1.name < m2.name;
}


#endif
