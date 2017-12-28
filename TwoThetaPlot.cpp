#include "TwoThetaPlot.h"

TwoThetaPlot::TwoThetaPlot(QWidget *parent, AppConfig &_appConfig) : QwtPlot(parent)
{
    appConfig = &_appConfig;

    // Set axis titles
    setAxisTitle(xBottom, "2Theta");
    setAxisTitle(yLeft, "Intensity");

    // Insert all curves, only data visible
    cData = new QwtPlotCurve("2Theta Data");
    cData->setRenderHint(QwtPlotItem::RenderAntialiased);
    cData->setPen(QPen(Qt::red));
    cData->attach(this);

    cDataBGS = new QwtPlotCurve("2Theta Data Background Subracted");
    cDataBGS->setRenderHint(QwtPlotItem::RenderAntialiased);
    cDataBGS->setPen(QPen(Qt::red));
    cDataBGS->attach(this);
    cDataBGS->hide();

    cDataMin = new QwtPlotCurve("2Theta Data");
    cDataMin->setRenderHint(QwtPlotItem::RenderAntialiased);
    cDataMin->setPen(QPen(Qt::blue));
    cDataMin->attach(this);
    cDataMin->hide();

    cDataMax = new QwtPlotCurve("2Theta Data");
    cDataMax->setRenderHint(QwtPlotItem::RenderAntialiased);
    cDataMax->setPen(QPen(Qt::blue));
    cDataMax->attach(this);
    cDataMax->hide();

    first = true;
}

void TwoThetaPlot::setXYData(double *_xd, double *_yd, int _s, double sx, double ex)
{
    if (first)
    {
        ydo = new double[_s];
        for (int i = 0; i < _s; i++)
        {
             ydo[i] = _yd[i];
             //cout << ydo[i] << endl;
         }

        first = false;
    }

    xd = _xd;
    yd = _yd;
    size = _s;

    cData->setData(xd, yd, size);


    setAxisScale(QwtPlot::xBottom, sx, ex, 15.0);
    double m = maxYinRange(sx, ex);
    int my = ceil(m/pow(10.0,nZeros(m)) + 0.5)*pow(10.0,nZeros(m));
    int ss = ceil(m/pow(10.0,nZeros(m)) + 0.5)*pow(10.0,nZeros(m)-1);
    setAxisScale(QwtPlot::yLeft, 0, my, ss);

    cout << " Circle Radius = " << appConfig->getCircleRadius() << endl;

    this->doBackgroundSubtraction(appConfig->getCircleRadius());
    connect(appConfig, SIGNAL(circleRadiusChanged()), this, SLOT(updateBackgroundSubtraction()));
    this->replot();
}

void TwoThetaPlot::setYMinData(double *_yd)
{
    ydmin = _yd;
    cDataMin->setData(xd, ydmin, size);

    this->replot();
}

void TwoThetaPlot::setYMaxData(double *_yd)
{
    ydmax = _yd;
    cDataMax->setData(xd, ydmax, size);

    this->replot();
}

void TwoThetaPlot::resetYRange(double x1, double x2)
{

    double m = maxYinRange(x1, x2);

    int my = ceil(m/pow(10.0,nZeros(m)) + 0.5)*pow(10.0,nZeros(m));
    int ss = ceil(m/pow(10.0,nZeros(m)) + 0.5)*pow(10.0,nZeros(m)-1);
    setAxisScale(QwtPlot::yLeft, 0, my, ss);
    this->replot();
}

int TwoThetaPlot::nZeros(double v)
{
    int n = 0;
    while (v / pow(10.0,n) >= 10 && n < 10)
        n++;

    return n;
}

double TwoThetaPlot::maxYinRange(double start, double end)
{
    double step = xd[2]-xd[1];

    int starti = int(start/step);
    int endi = int(end/step);

    double m = 0.0;

    for (int i = starti; i < endi; i++)
    {
        if (yd[i] > m)
            m = yd[i];
    }

    return m;
}

void TwoThetaPlot::doBackgroundSubtraction(double cR)
{
    cout << "cR being forced to 2" << endl; cR = 2.0;
    cout << "Doing bgs with cR = " << cR << endl;
    double **indata; /* 2D array of input data */
    double **outdata; /* 2D array of output data */

    double cX; /* Circle x-coordinate */
    double cY; /* Circle y-coordinate */

    double sP = 1.0;
    double oP = 0.0;
    int N = this->size;

    /* Allocate input and output data array */
    indata = (double **) malloc(N*sizeof(double));
    outdata = (double **) malloc(N*sizeof(double));
    for (int i = 0; i < N; i++)
    {
        indata[i] = (double *) malloc(2*sizeof(double));
        outdata[i] = (double *) malloc(2*sizeof(double));
    }

    for (int i = 0; i < N; i++)
    {
        outdata[i][0] = 0.0;
        outdata[i][1] = 1e20;
    }

    /* Copy cData to indata */
    for (int i = 0; i < N; i++)
    {
        indata[i][0] = cData->data().x(i);
        indata[i][1] = cData->data().y(i);
    }

    //for (int i = 0; i < N; i++)
    //    cout << indata[i][0] << " " << indata[i][1] << endl;


    /* Normalize data so that h/w = 1 for minimal useful signal */
    sP = (maxY(indata, N) - minY(indata,N))/maxY(indata, N);
    oP = minY(indata,N);

    for (int i = 0; i < N; i++)
            indata[i][1] = sP*(indata[i][1] - oP);

    /* Loop through each data point */
    for (int i = 0; i < N; i++)
    {
            /* Calculate analyzing circle location */
            cX = indata[i][0];
            cY = findCircleY(indata, N, cX, cR);

            int starti = floor( (cX - cR - indata[0][0])/(indata[1][0] - indata[0][0]) + 0.5 );
            if (starti < 0) starti = 0;
            if (starti > N-1) starti = N-1;

            int endi = floor( (cX + cR - indata[0][0])/(indata[1][0] - indata[0][0]) + 0.5 );
            if (endi > N-1) endi = N-1;
            if (endi < 0) endi = 0;

            /* Calculate differences between data and circles upper arc */
            for (int xi = starti; xi <= endi; xi++)
            {
                    double diff = indata[xi][1] - (sqrt(pow(cR,2.0) - pow(indata[xi][0]-cX,2.0)) + cY);

                    if (diff < outdata[xi][1])
                            outdata[xi][1] = diff;

            }
    }

    for (int i = 0; i < N; i++)
    {
            outdata[i][0] = indata[i][0];
            outdata[i][1] = (outdata[i][1]/sP);
    }

    double *xdbgs = new double[N];
    double *ydbgs = new double[N];

    /* Save outdata as cDataBGS */
    for (int i = 0; i < N; i++)
    {
        xdbgs[i] = outdata[i][0];
        ydbgs[i] = outdata[i][1];
    }

    cDataBGS->setData(xdbgs, ydbgs, N);
    this->replot();
}

double TwoThetaPlot::findCircleY(double **d, int N, double cX, double cR)
{

    double cY = -cR;

    //cout << "Circle starting at (" << cX << ", " << cY << ")" << endl;

    int starti = floor( (cX - cR - d[0][0])/(d[1][0] - d[0][0]) + 0.5 );
    if (starti < 0) starti = 0;
    if (starti > N-1) starti = N-1;

    int endi = floor( (cX + cR-d[0][0])/(d[1][0] - d[0][0]) + 0.5 );
    if (endi > N-1) endi = N-1;
    if (endi < 0) endi = 0;

    //cout << "Start i = " << starti << ", End i = " << endi << endl;

    double *diffs = new double[endi-starti + 1];

    for (int i = starti; i <= endi; i++)
    {
            double y = sqrt( pow(cR, 2.0) - pow(d[i][0]- cX, 2.0) ) + cY;

            diffs[i-starti] = d[i][1] - y;
    }

    double mindiff = 1e20;

    for (int i = 0; i < endi-starti+1; i++)
    {
            if (diffs[i] < mindiff)
            {
                    mindiff = diffs[i];
            }
    }

    cY = cY + mindiff;

    return cY;
}


double TwoThetaPlot::minY(double **d, int N)
{
        double min = 1e20;
        for (int i = 0; i < N; i++)
        {
                if (d[i][1] < min)
                        min = d[i][1];
        }

        return min;
}

double TwoThetaPlot::maxY(double **d, int N)
{
        double max = 0.0;
        for (int i = 0; i < N; i++)
        {
                if (d[i][1] > max)
                        max = d[i][1];
        }

        return max;
}


