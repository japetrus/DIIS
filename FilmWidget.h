/* ***************************************************************************
 * FilmWidget.h: defines a widget to display and analyze xrd patterns
 * author: Joe Petrus
 * date: June 10th 2010
 * ***************************************************************************/
#ifndef FilmWidget_H
#define FilmWidget_H

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Headers, definitions, etc.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* Qt related headers */
#include <QMainWindow>
#include <QLabel>
#include <QPrinter>
#include <QMouseEvent>
#include <QPen>
#include <QtGui>
#include <QRect>
#include <QDesktopWidget>
#include <QMatrix>
#include <QRgb>
#include <QVarLengthArray>

/* Standard c++ headers */
#include <iostream>
#include <cmath>
#include <fstream>
#include <ctime>

/* Qwt related headers */
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
#endif

/* Program headers */
#include "LogWidget.h"
#include "AppConfig.h"
#include "TwoThetaWindow.h"

using namespace std;

/* Definitions */
#define pow2(x) pow(x,2.0)
#define UPPER_REGION 0
#define LOWER_REGION 1
#define ZERO_DEGREES 0
#define ONEEIGHTY_DEGREES 1

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Structures
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

struct Geometry
{
    QPoint zeroDegreeCenter;
    QPoint zeroDegreeGuess;
    QPoint oneEightyDegreeCenter;
    QPoint oneEightyDegreeGuess;
    double phi;
    double alpha;
    double radius;

    double sharpness;
    bool integrated;
    QPoint *activeCenter;    

    Geometry() : zeroDegreeCenter(QPoint(0,0)), zeroDegreeGuess(QPoint(0,0)),
                 oneEightyDegreeCenter(QPoint(0,0)), oneEightyDegreeGuess(QPoint(0,0)),                 
                 phi(0.0), alpha(0.0), radius(0.0), sharpness(0.0), integrated(false),
                 activeCenter(&zeroDegreeCenter) {}
};

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Main class definition
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ***************************************************************************
 * class: FilmWidget
 * description: QWidget based on QLabel to display and manipulate xrd patterns.
 * ***************************************************************************/
class FilmWidget : public QLabel
{
    Q_OBJECT

public:
    FilmWidget(QWidget *parent, QStatusBar *_sb, LogWidget *_log, AppConfig &_appConfig );
	
    /* Film access methods */
    void setFilm(QString);
    QImage getFilm() { return *filmData; }
    double getScaleFactor() { return scaleFactor; }
    void setScaleFactor(double sf) { scaleFactor = sf; }
    int getDPM() { return filmDPM; }
    int getDPI() { return filmDPM*0.0254; }
    double getRadius() { return currentGeometry.radius; }

    void setRadius(double _R) { currentGeometry.radius = _R; }
    void setDPM(int dpm) { filmDPM = dpm; }
    void setData(QImage d) { filmData = new QImage(d); }

    QMainWindow *getTwoThetaPlot() { return twoThetaWindow; }
    void updateIntArea();
    void updateGeometry();
    QRect getIntArea() { return intArea; }
    bool isIntegrated() { return currentGeometry.integrated; }
    Geometry* getGeometry() { return &currentGeometry; }
    Geometry* getPreviousGeometry() { return &previousGeometry; }
    QRect getQRectFromAngleRange(double start, double stop);
    double* getAngleRangeFromQRect(QRect r);

    /* Film analysis methods */
    double integrate(double resolution = 0.025, int xo = 0, int yo = 0);
    double integrateRegion(double res, int xo, int yo, QRect reg);
    double integrateRegionDifference(double res, int xo, int yo, QRect regA, QRect regB);

    QPoint optimizeCenterXSharpness(int location = ZERO_DEGREES);
    QPoint optimizeCenterXSymmetry(int location = ZERO_DEGREES);
    QPoint optimizeCenterYSharpness(int location = ZERO_DEGREES);
    QPoint optimizeCenterYSymmetry(int location = ZERO_DEGREES);
    double optimizeRadiusSharpness();
    double optimizePhiSharpness();
    double optimizeAlphaSharpness();
    double optimizeRadiusSymmetry();
    double optimizeRotationSymmetry();
    double residual(int cgx, int cgy, QImage *data);
    double regionIntensity(QRect r, QImage *d);
    /* UI methods */
    void showTwoThetaWindow() { twoThetaWindow->show(); }
    void setAltDown(bool ad) { altDown = ad; }

    void setMoveActive(bool ma) { moveActive = ma; }
    void setResizeActive(bool ra) { resizeActive = ra; }
    void setExcludeActive(bool ea) { excludeActive = ea; }

    void setImage(QImage newImage);
    QPoint rotatePoint(QPoint in, double a);

public slots:
        void startCrop();
        void startDeskew();
        void invert();
	void darken();
	void lighten();
        void rotate();
        void rotate(double a);
        void closeFilm();
	
	/* IO methods */
        void saveTIFF();
	void saveUDF();
	void saveCSV();	

        void getCurrentConfigValues();

        void moveOptimizationRegion(QAction*);
        void saveOptimizationRegion();
	
signals:
        void geometryUpdated();

/* Protected members */	
protected:
	/* Widget events */
	void mousePressEvent( QMouseEvent* );
	void mouseReleaseEvent(	QMouseEvent* );
	void mouseMoveEvent( QMouseEvent* );
	void paintEvent( QPaintEvent* );
	//void keyPressEvent(QKeyEvent* );
	//void keyReleaseEvent(QKeyEvent* );
	
/* Private members */
private:
        AppConfig *appConfig;
	/* Film variables */
	QImage *filmData;
	double filmDPM;
        double lambda;

        QString suggestedName;
	
	/* UI elements */
	double scaleFactor;
	QStatusBar *sb;	
        LogWidget *log;
        TwoThetaWindow *twoThetaWindow;
        //TwoThetaPlot *twoThetaWindow;
	bool altDown;

        bool moveActive;
        bool resizeActive;
        bool excludeActive;

	int mouseStartX;
	int mouseStartY;
        int cropPhase;
        QPoint cropPoint1;
        QPoint cropPoint2;
        int deskewPhase;
        QPoint deskewPoint1;
        QPoint deskewPoint2;
	
	/* Integration variables */
	QRect intArea;
	double **intData;
	double intResolution;

	/* Points and areas */
        QPoint guessCenter0;
        QPoint guessCenter180;
        QPoint *guessCenter;
        QRect optRegion0A;
        QRect optRegion0B;
        QRect optRegion180A;
        QRect optRegion180B;
        QRect optRegionSh;
        QRect *optRegionA;
        QRect *optRegionB;

        QVector<QRect> excludeRegions;

        Geometry currentGeometry;
        Geometry previousGeometry;
	
};

#endif
