/****************************************************************************
 AppWindow class header
****************************************************************************/

#ifndef AppWindow_H
#define AppWindow_H

#include <QMainWindow>
#include <QLabel>
#include <QPrinter>
#include <QtGui>
#include <QRect>
#include <QDesktopWidget>
#include <QMatrix>
#include <QToolBar>
#include <math.h>
#include "FilmWidget.h"
#include "AppConfig.h"
#include "LogWidget.h"
#include "ui_GeometryDialog.h"
#include "ui_ImageDialog.h"

#define VERSION "2010-06-21 beta"

class AppWindow : public QMainWindow
{
    Q_OBJECT

public:
    AppWindow(AppConfig &_appConfig);
    ~AppWindow();

protected:
    void keyPressEvent(QKeyEvent* );
    void keyReleaseEvent(QKeyEvent* );


private slots:
    void openFilm();
    void showPrefs();
    void showGeometry();
    void updateGeometryValues();
    void resetGeometry();
    void applyGeometry();
    void updateOthersFromCenters();
    void updateCentersFromOthers();
    void updateActiveGeometryElements();

    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();

    void changeDPI();
    void integrate();
    void optimize();
    void crop();
    void deskew();

    void showTwoThetaWindow();
    void showLogWindow();
    void showAboutWindow();
    void showImageDialog() { imageDialog->show(); }
    void toggleMove(bool b);
    void toggleResize(bool b);
    void toggleExclude(bool b);

    void updateImageDialog();
    void updateImage();
    void applyAlphaRotation() { gandolfiFilm->rotate(-UiGeo.sbAlpha->value()); gandolfiFilm->getGeometry()->alpha = 0.0; this->updateGeometryValues(); }

private:
    void createActions();
    void createMenus();
    void createToolBar();
    void updateActions();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    FilmWidget *gandolfiFilm;
    QScrollArea *scrollArea;
    LogWidget *log;
    QPrinter printer;

    //QToolButton *btnRegion;
    QToolButton *btnMove;
    QToolButton *btnResize;
    QToolButton *btnExclude;

    //QPoint lastUpperCenter;
    //QPoint lastLowerCenter;

    // File menu actions
    QAction *openAct;
    QAction *saveTIFFAct;
    QAction *saveCSVAct;
    QAction *saveUDFAct;

    QAction *closeAct;
    QAction *exitAct;
    QAction *prefsAct;

    // View menu actions
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;

    // Image menu actions
    QAction *cropAct;
    QAction *deskewAct;
    QAction *invertAct;
    QAction *rotateAct;
    QAction *imageAct;
    QAction *dpiAct;

    // Tools menu actions
    QAction *integrateAct;
    QAction *optimizeAct;

    // Window menu actions
    QAction *showAppWindowAct;
    QAction *showTwoThetaWindowAct;
    QAction *showLogDockAct;
    QAction *showAboutWindowAct;

    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *toolsMenu;
    QMenu *windowMenu;

    QToolBar *toolbar;
    Ui::GeometryDialog UiGeo;
    Ui::ImageDialog UiImage;
    QDockWidget *geoDockWidget;
    QDockWidget *logDockWidget;
    QDialog *geoDialog;
    QDialog *imageDialog;

    bool moveActive;
    bool resizeActive;
    bool excludeActive;

    AppConfig *appConfig;
};

#endif
