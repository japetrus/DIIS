#ifndef TWOTHETAPLOT_H
#define TWOTHETAPLOT_H

/* QT Related headers */
#include <QMainWindow>
#include <QLabel>
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
#include <QtNetwork/QNetworkAccessManager>
#include <QUrl>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

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

using namespace std;

/* TwoThetaPlot class
* Used to display 2theta versus intensity. */
class TwoThetaPlot : public QwtPlot
{
    Q_OBJECT

    /* Public members */
public:
    TwoThetaPlot(QWidget *parent, AppConfig &_appConfig);

    void setXYData(double *_xd, double *_yd, int _s, double sx= 0, double ex = 180.0);
    void setYMinData(double *_yd);
    void setYMaxData(double *_yd);

    void mmOn() { cDataMin->show(); cDataMax->show(); this->replot(); }
    void mmOff() { cDataMin->hide(); cDataMax->hide(); this->replot(); }

    void bgsOn() { cDataBGS->show(); cData->hide(); this->replot(); }
    void bgsOff() { cDataBGS->hide(); cData->show(); this->replot(); }

    int getSize() { return size; }
    void changeRange(double x1, double x2) { setAxisScale(QwtPlot::xBottom, x1, x2, 15.0); this->replot(); }
    void resetYRange(double x1 = 0.0, double x2 = 180.0);
    double* getXValues() { return xd; }
    double* getYValues() { return yd; }
    int nZeros(double v);

    double maxYinRange(double start, double end);

    QwtPlotCurve* getData() { return cData; }
    QwtPlotCurve* getDataBGS() { return cDataBGS; }
    void doBackgroundSubtraction(double cR);
    double findCircleY(double **d, int N, double cX, double cR);
    double minY(double **d, int N);
    double maxY(double **d, int N);

    void reset()
    {
        for (int i = 0; i < size; i ++)
            yd[i] = ydo[i];
    }

protected slots:
    void updateBackgroundSubtraction() { doBackgroundSubtraction(appConfig->getCircleRadius()); }

    /* Private members */
private:
    AppConfig *appConfig;
    double *xd;
    double *yd;
    double *ydmin;
    double *ydmax;
    double *ydo;
    int size;

    QwtPlotCurve *cData;
    QwtPlotCurve *cDataBGS;
    QwtPlotCurve *cDataMin;
    QwtPlotCurve *cDataMax;

    bool first;

};

#endif // TWOTHETAPLOT_H
