/* ***************************************************************************
 * FilmWidget.cpp: implements a widget to display and analyze xrd patterns
 * author: Joe Petrus
 * date: June 10th 2010
 * ***************************************************************************/

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Headers, definitions, etc.
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

#include "FilmWidget.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * Methods
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ***************************************************************************
 * method: FilmWidget
 * description: initializes widget and creates plotting window.
 * ***************************************************************************/
FilmWidget::FilmWidget(QWidget *parent, QStatusBar* _sb, LogWidget *_log, AppConfig &_appConfig ) : QLabel(parent)
{
    appConfig = &_appConfig;

    /* Initialize pointers */
    filmData = NULL;
    intData = NULL;
    sb = _sb;
    log = _log;


    /* Turn on mouse tracking to get click events. */
    setMouseTracking(true);
    setScaledContents(true);
    altDown = false;

    /* Setup 2Theta plot window */
    twoThetaWindow = new TwoThetaWindow(this, *appConfig);

    /* Initialize some other stuff to zero, for now. */
    filmDPM = 0.0;
    intArea = QRect(0,0,0,0);
    optRegion0A = QRect(0,0,0,0);
    optRegion0B = QRect(0,0,0,0);
    optRegion180A = QRect(0,0,0,0);
    optRegion180B = QRect(0,0,0,0);
    optRegionA = &optRegion0A;
    optRegionB = &optRegion0B;

    cropPoint1 = QPoint(0,0);
    cropPoint2 = QPoint(0,0);
    deskewPoint1 = QPoint(0,0);
    deskewPoint2 = QPoint(0,0);
}

void FilmWidget::getCurrentConfigValues()
{
    //AppConfig c(NULL);
   // lambda = c.getLambda();

}

void FilmWidget::updateIntArea()
{
    if (currentGeometry.zeroDegreeCenter != QPoint(0,0) && currentGeometry.oneEightyDegreeCenter != QPoint(0,0))
    {
        // Currently doesn't reflect an integration width from the preferences...
        intArea.setX(currentGeometry.zeroDegreeCenter.x() - (0.0254*filmDPM)/2.0);
        intArea.setY(currentGeometry.zeroDegreeCenter.y());
        intArea.setWidth(0.0254*filmDPM);
        intArea.setHeight(M_PI*currentGeometry.radius*filmDPM);

        if (optRegionSh.width() == 0) { optRegionSh = intArea; }
    }
}

/* setFilm
 * Loads a given film (from fileName) and does a rough crop and rotate.
 *
 */
void FilmWidget::setFilm(QString fileName)
{
    /* Load the film in to a temporary QImage. */
    QImage *tempData = new QImage( fileName );

    /* Give an error if the film couldn't be loaded. */
    if (tempData->isNull()) {
        QMessageBox::information(this, tr("Gandolfi"), tr("Could not load film: %1.").arg(fileName));
        log->addMessage(tr("[FilmWidget] Could not load film: %1.").arg(fileName));
        return;
    }

    if (qGray(tempData->pixel(0,0)) == 0)
    {
        tempData->invertPixels();
        log->addMessage("[Main] Inverting film.");
    }

    /* Reort and set the dots per meter (DPM). */
    log->addMessage(QString("[Main] Film has DPM = ") +
                    QString::number(tempData->dotsPerMeterX()) +
                    QString(" (DPI = ") +
                    QString::number(tempData->dotsPerMeterX()*0.0254) +
                    QString(")"));
    filmDPM = tempData->dotsPerMeterX();
    //filmDPM = 450;

    /* Set film data to the roughly rotated and cropped film. */
    filmData = new QImage( tempData->copy() );
    delete tempData;

    /* If for some reason filmData == NULL, give an error. */
    if (filmData->isNull()) {
        QMessageBox::information(this, tr("Gandolfi"), tr("Could not load film: %1.").arg(fileName));
        return;
    }

    /* Invert pixels (because we want the lines to be high intensity (white). */
    filmData->invertPixels();

    /* Finally, set the QImage as the image to use for this widget. */
    this->setPixmap(QPixmap::fromImage(*filmData));

    log->addMessage("[Main] Film has been loaded.");
    twoThetaWindow->setSuggestedName(fileName);
    suggestedName = fileName.split(".").at(0);
    //suggestedName.chop(4);
}

void FilmWidget::setImage(QImage newImage)
{
    this->setPixmap(QPixmap::fromImage(newImage));
}

void FilmWidget::invert()
{
    filmData->invertPixels();
    this->setPixmap(QPixmap::fromImage(*filmData));
}

void FilmWidget::startCrop()
{
    sb->showMessage("Click the upper left crop point.");
    log->addMessage("[FilmWidget] Started crop routine (phase=1).");
    cropPhase = 1;
}

void FilmWidget::startDeskew()
{
    sb->showMessage("Click the upper deskew point.");
    log->addMessage("[FilmWidget] Started deskew rountine (phase=1).");
    deskewPhase = 1;
}

void FilmWidget::rotate()
{
    bool ok;
    double angle = QInputDialog::getDouble(this, tr("Rotate"), tr("Rotation angle [deg]: "), 0, -360.0, 360.0, 3, &ok);

    if (ok)
    {
        rotate(angle);
    }
}

QPoint FilmWidget::rotatePoint(QPoint in, double a)
{
    QPoint out(0,0);

    double cosa = cos(a*M_PI/180.0);
    double sina = sin(a*M_PI/180.0);

    if (a >= 0)
    {
        out.setX( cosa*in.x() - sina*in.y() + sina*filmData->height() );
        out.setY( cosa*in.y() + sina*in.x() );
    } else
    {
        out.setX( cosa*in.x() - sina*in.y() );
        out.setY( cosa*in.y() + sina*in.x() - sina*filmData->width() );
    }

    return out;
}

void FilmWidget::rotate(double a)
{
    log->addMessage(tr("[FilmWidget] Rotating by %1 degrees.").arg(QString::number(a)));
    previousGeometry = currentGeometry;
    currentGeometry.zeroDegreeCenter = rotatePoint(currentGeometry.zeroDegreeCenter, a);
    currentGeometry.oneEightyDegreeCenter = rotatePoint(currentGeometry.oneEightyDegreeCenter, a);
    currentGeometry.radius = (1.0/M_PI)*(currentGeometry.oneEightyDegreeCenter.y() - currentGeometry.zeroDegreeCenter.y())/filmDPM;

    updateGeometry();

    QMatrix rotTrans;
    rotTrans.rotate(a);
    QImage * oldFilmData = filmData;
    filmData = new QImage( filmData->transformed(rotTrans, Qt::SmoothTransformation) );
    delete oldFilmData;

    this->setPixmap(QPixmap::fromImage(*filmData));
    this->repaint();
}

void FilmWidget::darken()
{
    for (int x = 0; x < filmData->width(); x++)
    {
        for (int y = 0; y < filmData->height(); y++)
        {
            QRgb pv = filmData->pixel(x,y);
            filmData->setPixel(x,y,qRgb(qRed(pv)/2, qGreen(pv)/2, qBlue(pv)/2));
        }
    }

    this->setPixmap(QPixmap::fromImage(*filmData));
}

void FilmWidget::lighten()
{
    for (int x = 0; x < filmData->width(); x++)
    {
        for (int y = 0; y < filmData->height(); y++)
        {
            QRgb pv = filmData->pixel(x,y);
            filmData->setPixel(x,y,qRgb(qRed(pv)*2, qGreen(pv)*2, qBlue(pv)*2));

            if (qGray(filmData->pixel(x,y)) > 256)
                filmData->setPixel(x,y, qRgb(255,255,255));
        }
    }

    this->setPixmap(QPixmap::fromImage(*filmData));
}

void FilmWidget::closeFilm()
{
    this->setPixmap(NULL);
    delete filmData;
}

void FilmWidget::updateGeometry()
{
    // Update integration area and redraw
    updateIntArea();

    int x0shift = currentGeometry.zeroDegreeCenter.x() - previousGeometry.zeroDegreeCenter.x();
    int y0shift = currentGeometry.zeroDegreeCenter.y() - previousGeometry.zeroDegreeCenter.y();
    cout << "x0shift = " << x0shift << ", y0shift = " << y0shift << endl;

    optRegion0A.moveCenter(QPoint(optRegion0A.center().x() + x0shift, optRegion0A.center().y() + y0shift));
    optRegion0B.moveCenter(QPoint(optRegion0B.center().x() + x0shift, optRegion0B.center().y() + y0shift));

    optRegionSh.moveCenter(QPoint(optRegionSh.center().x() + x0shift, optRegionSh.center().y() + y0shift));

    int x180shift = currentGeometry.oneEightyDegreeCenter.x() - previousGeometry.oneEightyDegreeCenter.x();
    int y180shift = currentGeometry.oneEightyDegreeCenter.y() - previousGeometry.oneEightyDegreeCenter.y();
    cout << "x180shift = " << x180shift << ", y180shift = " << y180shift << endl;

    optRegion180A.moveCenter(QPoint(optRegion180A.center().x() + x180shift, optRegion180A.center().y() + y180shift));
    optRegion180B.moveCenter(QPoint(optRegion180B.center().x() + x180shift, optRegion180B.center().y() + y180shift));

    this->repaint();
}

QPoint FilmWidget::optimizeCenterXSharpness(int location)
{
    previousGeometry = currentGeometry;
    QRect reg;
    if (location == ZERO_DEGREES)
        reg = this->optRegionSh;
    else
        reg = this->optRegionSh;

    int iter = 0;
    int delta = 1;
    int y = 0;

    double gamma = (sqrt(5.0) - 1.0)/2.0;

    int x1 = -appConfig->getXRangeSharpness();
    int x2 =  appConfig->getXRangeSharpness();
    int x3 = int(floor(x2 - gamma*(x2-x1)));
    int x4 = int(floor(x1 + gamma*(x2-x1)));


    //double r1 = -integrateRegion(0.025,x1,y,reg);
    //double r2 = -integrateRegion(0.025,x2,y,reg);
    double r3 = -integrateRegion(0.025,x3,y,reg);
    double r4 = -integrateRegion(0.025,x4,y,reg);

    do
    {
        cout << "Iter[" << iter << "] x1 = " << x1 << ", x2 = " << x2 << ", x3 = " << x3 << ", x4 = " << x4 << endl;
        if (r3 <= r4)
        {
            x2 = x4;
            x4 = x3;
            r4 = r3;

            x3 = int(floor(x2 - gamma*(x2-x1) + 0.5));
            r3 = -integrateRegion(0.025,x3,y,reg);
        } else
        {
            x1 = x3;
            x3 = x4;
            r3 = r4;

            x4 = int(floor(x1 + gamma*(x2-x1) + 0.5));
            r4 = -integrateRegion(0.025,x4,y,reg);
        }

        iter++;
        delta = abs(x2-x1);

    } while (iter < 500 && delta > 1 );//&& abs(x1) < 20);

    int optx = 0;

    if ( (-integrateRegion(0.025,x1,y,reg)) <= (-integrateRegion(0.025,x2,y,reg)) )
    {
        cout << "Optimized x point is x = " << x1 << endl;
        optx = x1;
    } else
    {
        cout << "Optimized x point is x = " << x2 << endl;
        optx = x2;
    }

    if (location == ZERO_DEGREES)
        currentGeometry.zeroDegreeCenter.setX(currentGeometry.zeroDegreeCenter.x() - optx);
    else
        currentGeometry.oneEightyDegreeCenter.setX(currentGeometry.oneEightyDegreeCenter.x() - optx);

    this->updateGeometry();

    if (location == ZERO_DEGREES)
        return currentGeometry.zeroDegreeCenter;
    else
        return currentGeometry.oneEightyDegreeCenter;
}

double FilmWidget::optimizeAlphaSharpness()
{
    previousGeometry = currentGeometry;
    QRect reg = optRegionSh;

    int iter = 0;
    double deltaa = 1e20;
    double deltash = 1e20;
    int y = 0;
    int x = 0;

    double gamma = (sqrt(5.0) - 1.0)/2.0;

    double a1 = -appConfig->getAlphaRange();
    double a2 =  appConfig->getAlphaRange();
    double a3 = a2 - gamma*(a2-a1);
    double a4 = a1 + gamma*(a2-a1);
/*
    for (double a = -0.5; a <= 0.5; a = a + 0.02)
    {
        currentGeometry.alpha = a;
        double r = -integrateRegion(0.05, x, y, reg);
        cout << a << ", " << r << endl;
    }*/
/*
    for (double p = -0.5; p <= 0.5; p = p + 0.02)
    {
        currentGeometry.phi = p;
        double r = -integrateRegion(0.05, x, y, reg);
        cout << p << ", " << r << endl;
    }*/

    double r1; //-integrateRegion(0.025,x,y,reg);
    double r2; // = -integrateRegion(0.025,x,y,reg);
    currentGeometry.alpha = a3;
    double r3 = -integrateRegion(0.025,x,y,reg);
    currentGeometry.alpha = a4;
    double r4 = -integrateRegion(0.025,x,y,reg);

    do
    {

        if (r3 <= r4)
        {
            a2 = a4;
            r2 = r4;
            a4 = a3;
            r4 = r3;

            a3 = a2 - gamma*(a2-a1);
            currentGeometry.alpha = a3;
            r3 = -integrateRegion(0.025,x,y,reg);
        } else
        {
            a1 = a3;
            r1 = r3;
            a3 = a4;
            r3 = r4;

            a4 = a1 + gamma*(a2-a1);
            currentGeometry.alpha = a4;
            r4 = -integrateRegion(0.025,x,y,reg);
        }

        iter++;
        deltaa = abs(a2-a1);
        deltash = abs(r2-r1);
        //cout << "Iter[" << iter << "] p1 = " << p1 << ", p2 = " << p2 << ", p3 = " << p3 << ", p4 = " << p4 << endl;
    } while (iter < 500 && deltaa > 0.0001 && deltash > 0.00001 );//&& abs(x1) < 20);

    currentGeometry.alpha = (a1 + a2)/2.0;

    log->addMessage(tr("Alpha optimized to %1").arg(currentGeometry.alpha));

    return currentGeometry.alpha;
}

double FilmWidget::optimizePhiSharpness()
{
    previousGeometry = currentGeometry;
    QRect reg = optRegionSh;

    int iter = 0;
    double deltap = 1e20;
    double deltash = 1e20;
    int y = 0;
    int x = 0;

    double gamma = (sqrt(5.0) - 1.0)/2.0;

    double p1 = -appConfig->getPhiRange();
    double p2 =  appConfig->getPhiRange();
    double p3 = p2 - gamma*(p2-p1);
    double p4 = p1 + gamma*(p2-p1);


    /*for (double p = -0.01; p <= 0.01; p = p + 0.001)
    {
        currentGeometry.phi = p;
        double r = -integrateRegion(0.025, x, y, reg);
        cout << p << ", " << r << endl;
    }*/

    double r1; //-integrateRegion(0.025,x,y,reg);
    double r2; // = -integrateRegion(0.025,x,y,reg);
    currentGeometry.phi = p3;
    double r3 = -integrateRegion(0.025,x,y,reg);
    currentGeometry.phi = p4;
    double r4 = -integrateRegion(0.025,x,y,reg);

    do
    {

        if (r3 <= r4)
        {
            p2 = p4;
            r2 = r4;
            p4 = p3;
            r4 = r3;

            p3 = p2 - gamma*(p2-p1);
            currentGeometry.phi = p3;
            r3 = -integrateRegion(0.025,x,y,reg);
        } else
        {
            p1 = p3;
            r1 = r3;
            p3 = p4;
            r3 = r4;

            p4 = p1 + gamma*(p2-p1);
            currentGeometry.phi = p4;
            r4 = -integrateRegion(0.025,x,y,reg);
        }

        iter++;
        deltap = abs(p2-p1);
        deltash = abs(r2-r1);
        cout << "Iter[" << iter << "] p1 = " << p1 << ", p2 = " << p2 << ", p3 = " << p3 << ", p4 = " << p4 << endl;
    } while (iter < 500 && deltap > 0.0001 && deltash > 0.00001 );//&& abs(x1) < 20);

    currentGeometry.phi = (p1 + p2)/2.0;

    log->addMessage(tr("Phi optimized to %1").arg(currentGeometry.phi));

    return currentGeometry.phi;
}


QPoint FilmWidget::optimizeCenterXSymmetry(int location)
{
    previousGeometry = currentGeometry;
    if (location == ZERO_DEGREES)
    {
        optRegionA = &optRegion0A;
        optRegionB = &optRegion0B;
    }
    else
    {
        optRegionA = &optRegion180A;
        optRegionB = &optRegion180B;
    }

    int iter = 0;
    int delta = 1;
    int y = 0;

    double gamma = (sqrt(5.0) - 1.0)/2.0;



    int x1 = -appConfig->getXRangeSymmetry();
    int x2 =  appConfig->getXRangeSymmetry();
    int x3 = int(floor(x2 - gamma*(x2-x1) + 0.5));
    int x4 = int(floor(x1 + gamma*(x2-x1) + 0.5));

    //double r1 = residual(x1, y, filmData);
    //double r2 = residual(x2, y, filmData);
    double r3 = residual(x3, y, filmData);
    double r4 = residual(x4, y, filmData);
/*
    for (int x = -50; x <= 50; x++)
    {
        for (int y = -50; y <= 50; y++)
        {
            double r = residual(x,y,filmData);
            cout << x << ", " << y << ", " << r << endl;

        }
    }
*/

    do
    {
        if (r3 <= r4)
        {
            x2 = x4;
            x4 = x3;
            r4 = r3;

            x3 = int(floor(x2 - gamma*(x2-x1) + 0.5));
            r3 = residual(x3, y, filmData);
        } else
        {
            x1 = x3;
            x3 = x4;
            r3 = r4;

            x4 = int(floor(x1 + gamma*(x2-x1) + 0.5));
            r4 = residual(x4, y, filmData);
        }

        iter++;
        delta = abs(x2-x1);
    } while (iter < 500 && delta > 1 );//&& abs(x1) < 20);

    int optx = 0;

    if (residual(x1, y, filmData) <= residual(x2, y, filmData))
    {
        cout << "Optimized x point is x = " << x1 << ", iters = " << iter <<  endl;
        optx = x1;
    } else
    {
        cout << "Optimized x point is x = " << x2 << ", iters = " << iter << endl;
        optx = x2;
    }

    if (location == ZERO_DEGREES)
        currentGeometry.zeroDegreeCenter.setX(currentGeometry.zeroDegreeCenter.x() + optx);
    else
        currentGeometry.oneEightyDegreeCenter.setX(currentGeometry.oneEightyDegreeCenter.x() + optx);

    this->updateGeometry();

    if (location == ZERO_DEGREES)
        return currentGeometry.zeroDegreeCenter;
    else
        return currentGeometry.oneEightyDegreeCenter;

}

QPoint FilmWidget::optimizeCenterYSharpness(int location)
{

    previousGeometry = currentGeometry;
    QRect regA;
    QRect regB;

    if (location == ZERO_DEGREES)
    {
        regA = optRegion0B;
        regB = optRegion0A;
    } else
    {
        regA = optRegion180B;
        regB = optRegion180A;
    }

    regA = optRegionSh;

    int iter = 0;
    int delta = 1;
    int x = 0;

    double gamma = (sqrt(5.0) - 1.0)/2.0;

    /*for (int y = -50; y <= 50; y++)
    {
        for (int x = -50; x <= 50; x++)
        {
            double r = integrateRegion(0.05, x, y, optRegionSh);
            cout << x << ", " <<  y << ", " << r << endl;
        }
    }*/


    int y1 = -appConfig->getYRangeSharpness();
    int y2 =  appConfig->getYRangeSharpness();
    int y3 = int(floor(y2 - gamma*(y2-y1) + 0.5));
    int y4 = int(floor(y1 + gamma*(y2-y1) + 0.5));



    //double r1 = integrateRegionDifference(0.025,x,y1,regA, regB);
    //double r2 = integrateRegionDifference(0.025,x,y2,regA, regB);
    double r3 = -integrateRegion(0.05, x, y3, regA); //integrateRegionDifference(0.025,x,y3,regA, regB);
    double r4 = -integrateRegion(0.05, x, y4, regA); //integrateRegionDifference(0.025,x,y4,regA, regB);

    do
    {

        if (r3 <= r4)
        {
            y2 = y4;
            y4 = y3;
            r4 = r3;

            y3 = int(floor(y2 - gamma*(y2-y1) + 0.5));
            r3 = -integrateRegion(0.05, x, y3, regA); //integrateRegionDifference(0.025,x,y3,regA,regB);
        } else
        {
            y1 = y3;
            y3 = y4;
            r3 = r4;

            y4 = int(floor(y1 + gamma*(y2-y1) + 0.5));
            r4 = -integrateRegion(0.05, x, y4, regA); //integrateRegionDifference(0.025,x,y4,regA,regB);
        }

        iter++;
        delta = abs(y2-y1);
        cout << "Iter[" << iter << "] y1 = " << y1 << ", y2 = " << y2 << ", y3 = " << y3 << ", y4 = " << y4 << endl;
    } while (iter < 500 && delta > 1 );//&& abs(x1) < 20);

    int opty = 0;

    //if (integrateRegionDifference(0.025,x,y1,regA,regB) <= integrateRegionDifference(0.025,x,y2,regA,regB))
    if ( (-integrateRegion(0.05,x,y1,regA)) <= (-integrateRegion(0.05,x,y2,regA)) )
    {
        cout << "Optimized y point is y = " << y1 << endl;
        opty = y1;
    } else
    {
        cout << "Optimized y point is y = " << y2 << endl;
        opty = y2;
    }


    if (location == ZERO_DEGREES)
        currentGeometry.zeroDegreeCenter.setY(currentGeometry.zeroDegreeCenter.y() - opty);
    else
        currentGeometry.oneEightyDegreeCenter.setY(currentGeometry.oneEightyDegreeCenter.y() - opty);

    this->updateGeometry();

    if (location == ZERO_DEGREES)
        return currentGeometry.zeroDegreeCenter;
    else
        return currentGeometry.oneEightyDegreeCenter;
}

QPoint FilmWidget::optimizeCenterYSymmetry(int location)
{
    previousGeometry = currentGeometry;
    if (location == ZERO_DEGREES)
    {
        optRegionA = &optRegion0A;
        optRegionB = &optRegion0B;
    }
    else
    {
        optRegionA = &optRegion180A;
        optRegionB = &optRegion180B;
    }

    int iter = 0;
    int delta = 1;
    int x = 0;

    double gamma = (sqrt(5.0) - 1.0)/2.0;

    int y1 = -appConfig->getYRangeSymmetry();
    int y2 =  appConfig->getYRangeSymmetry();
    int y3 = y2 - gamma*(y2-y1);
    int y4 = y1 + gamma*(y2-y1);

    //double r1 = residual(x, y1, filmData);
    //double r2 = residual(x, y2, filmData);
    double r3 = residual(x, y3, filmData);
    double r4 = residual(x, y4, filmData);

    do
    {
        if (r3 <= r4)
        {
            y2 = y4;
            y4 = y3;
            r4 = r3;

            y3 = int(floor(y2 - gamma*(y2-y1) + 0.5));
            r3 = residual(x, y3, filmData);
        } else
        {
            y1 = y3;
            y3 = y4;
            r3 = r4;

            y4 = int(floor(y1 + gamma*(y2-y1) + 0.5));
            r4 = residual(x, y4, filmData);
        }

        iter++;
        delta = abs(y2-y1);
    } while (iter < 500 && delta > 1 );//&& abs(x1) < 20);

    int opty = 0;

    if (residual(x, y1, filmData) <= residual(x, y2, filmData))
    {
        cout << "Optimized y point is y = " << y1 << endl;
        opty = y1;
    } else
    {
        cout << "Optimized y point is y = " << y2 << endl;
        opty = y2;
    }


    if (location == ZERO_DEGREES)
        currentGeometry.zeroDegreeCenter.setY(currentGeometry.zeroDegreeCenter.y() + opty);
    else
        currentGeometry.oneEightyDegreeCenter.setY(currentGeometry.oneEightyDegreeCenter.y() + opty);

    this->updateGeometry();

    if (location == ZERO_DEGREES)
        return currentGeometry.zeroDegreeCenter;
    else
        return currentGeometry.oneEightyDegreeCenter;


}

double FilmWidget::optimizeRotationSymmetry()
{
    int x1 = currentGeometry.zeroDegreeCenter.x();
    int x2 = currentGeometry.oneEightyDegreeCenter.x();
    int y1 = currentGeometry.zeroDegreeCenter.y();
    int y2 = currentGeometry.oneEightyDegreeCenter.y();

    double a = -(180.0/M_PI)*atan( double(x2-x1)/double(y2-y1) );

    cout << "Optimizing rotation to " << a << " degrees." << endl;

    currentGeometry.alpha = a;
    updateGeometry();
    //this->rotate(a);

    return a;
}

double FilmWidget::optimizeRadiusSymmetry()
{
    //int x1 = currentGeometry.zeroDegreeCenter.x();
    //int x2 = currentGeometry.oneEightyDegreeCenter.x();
    int y1 = currentGeometry.zeroDegreeCenter.y();
    int y2 = currentGeometry.oneEightyDegreeCenter.y();

    double R = (1.0/M_PI)*(y2-y1)/filmDPM;

    currentGeometry.radius = R;
    updateGeometry();

    return R;
}

double FilmWidget::optimizeRadiusSharpness()
{
    previousGeometry = currentGeometry;
    QRect reg = optRegionSh;

    int iter = 0;
    double deltarad = 1e20;
    double deltash = 1e20;
    int y = 0;
    int x = 0;

    double gamma = (sqrt(5.0) - 1.0)/2.0;

    double rad1 = appConfig->getCameraRadius()-appConfig->getRadiusRange();
    double rad2 = appConfig->getCameraRadius()+appConfig->getRadiusRange();
    double rad3 = rad2 - gamma*(rad2-rad1);
    double rad4 = rad1 + gamma*(rad2-rad1);


   /*for (double rad = 0.050; rad <= 0.065; rad = rad + 0.0001)
    {
        currentGeometry.radius = rad;
        double r = -integrateRegion(0.05, x, y, reg);
        cout << rad << ", " << r << endl;
    }*/

    //reg.setHeight(int(floor(M_PI*rad1*filmDPM) + 0.5));
    currentGeometry.radius = rad1;
    double r1 =-integrateRegion(0.025,x,y,reg);
    //reg.setHeight(int(floor(M_PI*rad2*filmDPM) + 0.5));
    currentGeometry.radius = rad2;
    double r2 = -integrateRegion(0.025,x,y,reg);
    currentGeometry.radius = rad3;
    //reg.setHeight(int(floor(M_PI*rad3*filmDPM) + 0.5));
    double r3 = -integrateRegion(0.025,x,y,reg);
    currentGeometry.radius = rad4;
    //reg.setHeight(int(floor(M_PI*rad4*filmDPM) + 0.5));
    double r4 = -integrateRegion(0.025,x,y,reg);

    do
    {

        if (r3 <= r4)
        {
            rad2 = rad4;
            r2 = r4;
            rad4 = rad3;
            r4 = r3;

            rad3 = rad2 - gamma*(rad2-rad1);
            currentGeometry.radius = rad3;
            //reg.setHeight(int(floor(M_PI*rad3*filmDPM) + 0.5));
            r3 = -integrateRegion(0.025,x,y,reg);
        } else
        {
            rad1 = rad3;
            r1 = r3;
            rad3 = rad4;
            r3 = r4;

            rad4 = rad1 + gamma*(rad2-rad1);
            currentGeometry.radius = rad4;
            //reg.setHeight(int(floor(M_PI*rad4*filmDPM) + 0.5));
            r4 = -integrateRegion(0.025,x,y,reg);
        }

        iter++;
        deltarad = abs(rad2-rad1);
        deltash = abs(r2-r1);
        cout << "Iter[" << iter << "] rad1 = " << rad1 << ", rad2 = " << rad2 << ", rad3 = " << rad3 << ", rad4 = " << rad4 << endl;
        cout << "DR = " << deltarad << ", Dsh = " << deltash << endl;
    } while (iter < 500 && deltarad > 0.000001); //&& deltash > 0.0000001 );//&& abs(x1) < 20);

    currentGeometry.radius = (rad1 + rad2)/2.0;

    log->addMessage(tr("Radius optimized to %1").arg(currentGeometry.radius));
    updateGeometry();

    return currentGeometry.radius;
}

double FilmWidget::residual(int xoffset, int yoffset, QImage *data)
{	
    //QRect *optRegionA = &optRegion0A;
    //QRect *optRegionB = &optRegion0B;
    QRect oaMod = *optRegionA;
    QRect obMod = *optRegionB;

    oaMod.moveCenter(QPoint(oaMod.center().x() + xoffset, oaMod.center().y() + yoffset));
    obMod.moveCenter(QPoint(obMod.center().x() + xoffset, obMod.center().y() + yoffset));

    double res = 0;
    int xmin = oaMod.x();
    int xmax = oaMod.x() + oaMod.width();
    int ymin = oaMod.y();
    int ymax = obMod.y() + obMod.height();


    double rIA = regionIntensity(oaMod, data);
    double rIB = regionIntensity(obMod, data);

    for (int x = xmin; x < xmax; x++) {
        for (int y = ymin; y < ymin + oaMod.height(); y++) {
            res += 0.5*(qGray(data->pixel(xmax-(x-xmin), ymax - (y-ymin))) + qGray(data->pixel(x,y)) )*pow2((rIB/rIA)*qGray(data->pixel(x,y)) - qGray(data->pixel(xmax-(x-xmin), ymax - (y-ymin))));
            //res += pow2((rIB/rIA)*qGray(data->pixel(x,y)) - qGray(data->pixel(xmax-(x-xmin), ymax - (y-ymin))));
        }
    }

    this->repaint();
    return res;
}

double FilmWidget::regionIntensity(QRect r, QImage *d)
{
    double normIntensity = 0;

    for(int x = r.x(); x < r.x() + r.width(); x++)
    {
        for(int y = r.y(); y < r.y() + r.height(); y++)
        {
            normIntensity += qGray(d->pixel(x,y));
        }
    }

    return normIntensity/(r.width()*r.height());
}

// Do integration as in Matsuzaki paper
double FilmWidget::integrate(double resolution, int xo, int yo)
{
    sb->showMessage("Performing integration...");

    this->intResolution = resolution;
    double c;
    double L;
    double twoTheta;
    double bg = 0.0;
    int twoThetaIndex;
    double cp = cos(currentGeometry.phi*M_PI/180.0);
    double sp = sin(currentGeometry.phi*M_PI/180.0);
    double ca = cos(currentGeometry.alpha*M_PI/180.0);
    double sa = sin(currentGeometry.alpha*M_PI/180.0);
    double R = currentGeometry.radius;


    // Allocate array
    intData = new double*[int(180.0/resolution)];
    for (int i = 0; i < int(180.0/resolution); i++)
        intData[i] = new double[3];

    double intDataMin[int(180.0/resolution)];
    double intDataMax[int(180.0/resolution)];

    QRect intArea = this->intArea;
    intArea.moveCenter(QPoint(intArea.center().x() + xo, intArea.center().y() + yo));

    // Initialize 2theta values and set intensities to zero
    for (int i = 0; i < int(180.0/resolution); i++) {
        intData[i][0] = i*resolution;
        intData[i][1] = 0.0;
        intData[i][2] = 0.0;
        intDataMin[i] = 1e10;
        intDataMax[i] = 0.0;
    }

    // Loop through image data, determine 2theta and store intensity
    for (int x = intArea.x(); x < intArea.x()+intArea.width(); x++) {
        c = double(x-intArea.x()-intArea.width()/2.0)/filmDPM;
        for (int y = intArea.y(); y < intArea.y()+intArea.height(); y++) {

            bool inexclude = false;

            for (int i = 0; i < excludeRegions.size(); i++) {
                if (excludeRegions[i].contains(x, y))
                    inexclude = true;
            }

            if (inexclude) continue;

            L = double(y-intArea.y())/filmDPM;

            int nx, ny;

            if (currentGeometry.alpha >= 0)
            {
                nx = x*ca - y*sa + sa*filmData->height();
                ny = x*sa + y*ca;
            } else
            {
                nx = x*ca - y*sa;
                ny = x*sa + y*ca - sa*filmData->height();
            }


            //twoTheta = (180.0/M_PI)*atan(sqrt( pow2( cp*((c)*cb+R*sb*sin((L)/R)) - R*sp*cos((L)/R)) + pow2(R*cb*sin((L)/R) - (c)*sb)  )/(R*cos((L)/R)*cp + sp*((c)*cb + R*sb*sin((L)/R))) );
            twoTheta = (180.0/M_PI)*atan( sqrt( ( pow2( c*cp + R*sp*cos(L/R) ) + pow2(R*sin(L/R)) ) / pow2( -c*sp + R*cp*cos(L/R) ) ) );

            if (twoTheta > 180.0) twoTheta = twoTheta - 180.0;
            if (twoTheta < 0) twoTheta = -twoTheta;

            if (L > intArea.height()/(2.0*filmDPM)) { twoTheta = 180 - twoTheta; }

            twoThetaIndex = floor(twoTheta/resolution + 0.5);

            if (twoThetaIndex < int(180.0/resolution))
            {
                if (x < filmData->width() && y < filmData->height())
                {
                    intData[twoThetaIndex][1] += qGray(filmData->pixel(nx, ny));
                    intData[twoThetaIndex][2] = intData[twoThetaIndex][2] + 1;
                }

                if (qGray(filmData->pixel(x,y)) < intDataMin[twoThetaIndex])
                    intDataMin[twoThetaIndex] = qGray(filmData->pixel(nx,ny));

                if (qGray(filmData->pixel(x,y)) > intDataMax[twoThetaIndex])
                    intDataMax[twoThetaIndex] = qGray(filmData->pixel(nx,ny));
            }
        }
    }

    double Sum_WiIi = 0.0;
    double Sum_Wi = 0.0;
    double Sum_WiIisqrt = 0.0;

    for (int i = 0; i < int(180.0/resolution); i++) {
        Sum_WiIi += intData[i][1];


        if ( intData[i][2] != 0 )
            intData[i][1] = intData[i][1]/intData[i][2];
        else if ( intData[i][2] == 0 && i > 0 )
            intData[i][1] = intData[i-1][1];

        Sum_WiIisqrt += intData[i][2]*sqrt(intData[i][1]);
        Sum_WiIi += intData[i][2]*intData[i][1];
        Sum_Wi += intData[i][2];
    }

    double I = Sum_WiIi/Sum_Wi;
    double Isqrt = Sum_WiIisqrt/Sum_Wi;

    double sh = sqrt( I - pow(Isqrt,2.0) )/ Isqrt;
    log->addMessage(tr("[Plot] Sharpness = %1").arg(sh));

    this->sb->showMessage("Integration complete.");

    double *xd = new double[int(180.0/resolution)];
    for (int i = 0; i < int(180.0/resolution); i++) {
        xd[i] = intData[i][0];
    }

    double *yd = new double[int(180.0/resolution)];
    for (int i = 0; i < int(180.0/resolution); i++) {
        //if (i < (int(180.0/resolution)-5) && sqrt(pow((intData[i][1] - intData[i+3][1]),2.0)) < 2)
        //    bg = intData[i][1];

        yd[i] = intData[i][1] - bg;
    }

    twoThetaWindow->setXYData(xd, yd, int(180.0/resolution));
    twoThetaWindow->setYMinData(intDataMin);
    twoThetaWindow->setYMaxData(intDataMax);
    twoThetaWindow->show();

    currentGeometry.sharpness = sh;
    emit geometryUpdated();

    return sh;

}


double FilmWidget::integrateRegionDifference(double res, int xo, int yo, QRect regA, QRect regB)
{
    this->intResolution = res;
    double c;
    double L;
    double twoTheta;
    double bg = 0.0;
    int twoThetaIndex;
    double cp = cos(currentGeometry.phi);
    double sp = sin(currentGeometry.phi);
    double ca = cos(currentGeometry.alpha);
    double sa = sin(currentGeometry.alpha);
    double R = currentGeometry.radius;

    regA.setLeft(intArea.left());
    regA.setWidth(intArea.width());
    regB.setLeft(intArea.left());
    regB.setWidth(intArea.width());

    double cStart = double(regA.left()-intArea.x()-intArea.width()/2.0)/filmDPM;
    double LStart = double(regA.top()-intArea.y())/filmDPM;
    //double angleStart = (180.0/M_PI)*atan(sqrt( pow2( cp*((cStart)*cb+R*sb*sin((LStart)/R)) - R*sp*cos((LStart)/R)) + pow2(R*cb*sin((LStart)/R) - (cStart)*sb)  )/(R*cos((LStart)/R)*cp + sp*((cStart)*cb + R*sb*sin((LStart)/R))) );
    double angleStart = (180.0/M_PI)*atan( sqrt( ( pow2( cStart*cp + R*sp*cos(LStart/R) ) + pow2(R*sin(LStart/R)) ) / pow2( -cStart*sp + R*cp*cos(LStart/R) ) ) );

    double cEnd = double(regA.center().x()-intArea.x()-intArea.width()/2.0)/filmDPM;
    double LEnd = double(regA.bottom()-intArea.y())/filmDPM;
    //double angleStop = (180.0/M_PI)*atan(sqrt( pow2( cp*((cEnd)*cb+R*sb*sin((LEnd)/R)) - R*sp*cos((LEnd)/R)) + pow2(R*cb*sin((LEnd)/R) - (cEnd)*sb)  )/(R*cos((LEnd)/R)*cp + sp*((cEnd)*cb + R*sb*sin((LEnd)/R))) );
    double angleStop = (180.0/M_PI)*atan( sqrt( ( pow2( cEnd*cp + R*sp*cos(LEnd/R) ) + pow2(R*sin(LEnd/R)) ) / pow2( -cEnd*sp + R*cp*cos(LEnd/R) ) ) );
    //cout << "A start = " << angleStart << ", A stop = " << angleStop << endl;

    double **intDataA = new double*[int(180.0/res)];
    double **intDataB = new double*[int(180.0/res)];
    for (int i = 0; i < int(180.0/res); i++)
    {
        intDataA[i] = new double[3];
        intDataB[i] = new double[3];
    }

    for (int i = 0; i < int(180.0/res); i++)
    {
        intDataA[i][0] = i*res;
        intDataA[i][1] = 0.0;
        intDataA[i][2] = 0.0;
        intDataB[i][0] = i*res;
        intDataB[i][1] = 0.0;
        intDataB[i][2] = 0.0;
    }

    // Loop through image data, determine 2theta and store intensity
    for (int x = regA.x(); x < regA.x()+regA.width(); x++) {
        c = double(x+xo-regA.x()-regA.width()/2.0)/filmDPM;

        for (int y = regA.y(); y < regA.y()+regA.height(); y++) {

            bool inexclude = false;

            for (int i = 0; i < excludeRegions.size(); i++) {
                if (excludeRegions[i].contains(x, y))
                    inexclude = true;
            }

            if (inexclude) continue;

            L = double(y+ yo-currentGeometry.zeroDegreeCenter.y())/filmDPM;

            //twoTheta = (180.0/M_PI)*atan(sqrt( pow2( cp*((c)*cb+R*sb*sin((L)/R)) - R*sp*cos((L)/R)) + pow2(R*cb*sin((L)/R) - (c)*sb)  )/(R*cos((L)/R)*cp + sp*((c)*cb + R*sb*sin((L)/R))) );
            twoTheta = (180.0/M_PI)*atan( sqrt( ( pow2( c*cp + R*sp*cos(L/R) ) + pow2(R*sin(L/R)) ) / pow2( -c*sp + R*cp*cos(L/R) ) ) );


            if (twoTheta > 180.0) twoTheta = twoTheta - 180.0;
            if (twoTheta < 0) twoTheta = -twoTheta;

            if (L > intArea.height()/(2.0*filmDPM)) { twoTheta = 180 - twoTheta; }

            if (twoTheta > angleStop || twoTheta < angleStart) continue;

            //cout << "ttA = " << twoTheta << endl;

            twoThetaIndex = floor(twoTheta/res + 0.5);


            if (x < filmData->width() && y < filmData->height() && x > 0 && y > 0)
            {
                intDataA[twoThetaIndex][1] += qGray(filmData->pixel(x, y));
                intDataA[twoThetaIndex][2] = intDataA[twoThetaIndex][2] + 1;
            }

        }
    }

    cStart = double(regB.left()-intArea.x()-intArea.width()/2.0)/filmDPM;
    LStart = double(regB.bottom()-intArea.y())/filmDPM;
    //angleStart = (180.0/M_PI)*atan(sqrt( pow2( cp*((cStart)*cb+R*sb*sin((LStart)/R)) - R*sp*cos((LStart)/R)) + pow2(R*cb*sin((LStart)/R) - (cStart)*sb)  )/(R*cos((LStart)/R)*cp + sp*((cStart)*cb + R*sb*sin((LStart)/R))) );

    cEnd = double(regB.center().x()-intArea.x()-intArea.width()/2.0)/filmDPM;
    LEnd = double(regB.top()-intArea.y())/filmDPM;
    //angleStop = (180.0/M_PI)*atan(sqrt( pow2( cp*((cEnd)*cb+R*sb*sin((LEnd)/R)) - R*sp*cos((LEnd)/R)) + pow2(R*cb*sin((LEnd)/R) - (cEnd)*sb)  )/(R*cos((LEnd)/R)*cp + sp*((cEnd)*cb + R*sb*sin((LEnd)/R))) );

    //cout << "B start = " << angleStart << ", B stop = " << angleStop << endl;

    // Loop through image data, determine 2theta and store intensity
    for (int x = regB.x(); x < regB.x()+regB.width(); x++) {
        c = double(x+xo-regB.x()-regB.width()/2.0)/filmDPM;

        for (int y = regB.y(); y < regB.y()+regB.height(); y++) {
            if (y <= 0) break;

            bool inexclude = false;

            for (int i = 0; i < excludeRegions.size(); i++) {
                if (excludeRegions[i].contains(x, y))
                    inexclude = true;
            }

            if (inexclude) continue;

            L = double(y+ yo-currentGeometry.zeroDegreeCenter.y())/filmDPM;

            //twoTheta = (180.0/M_PI)*atan(sqrt( pow2( cp*((c)*cb+R*sb*sin((L)/R)) - R*sp*cos((L)/R)) + pow2(R*cb*sin((L)/R) - (c)*sb)  )/(R*cos((L)/R)*cp + sp*((c)*cb + R*sb*sin((L)/R))) );
            twoTheta = (180.0/M_PI)*atan( sqrt( ( pow2( c*cp + R*sp*cos(L/R) ) + pow2(R*sin(L/R)) ) / pow2( -c*sp + R*cp*cos(L/R) ) ) );


            if (twoTheta > 180.0) twoTheta = twoTheta - 180.0;
            if (twoTheta < 0) twoTheta = - twoTheta;

            if (L > intArea.height()/(2.0*filmDPM)) { twoTheta = 180 - twoTheta; }
            //cout << "ttB = " << twoTheta << endl;
            if (twoTheta > angleStop || twoTheta < angleStart) continue;


            twoThetaIndex = floor(twoTheta/res + 0.5);

            if (x < filmData->width() && y < filmData->height())
            {
                intDataB[twoThetaIndex][1] += qGray(filmData->pixel(x, y));
                intDataB[twoThetaIndex][2] = intDataB[twoThetaIndex][2] + 1;
            }
        }
    }

    double ret = 0.0;

    for (int i = 0; i < int(180.0/res); i++)
    {
        if (intDataA[i][2] != 0)
            intDataA[i][1] = intDataA[i][1]/intDataA[i][2];

        if (intDataB[i][2] != 0)
            intDataB[i][1] = intDataB[i][1]/intDataB[i][2];

       // cout << "i = " << i*res << " A = " << intDataA[i][1] << " B = " << intDataB[i][1] << endl;
        ret = ret + pow(intDataA[i][1] - intDataB[i][1],2.0);
    }

    return ret;
}

double FilmWidget::integrateRegion(double res, int xo, int yo, QRect reg)
{

    this->intResolution = res;
    double c;
    double L;
    double twoTheta;
    double bg = 0.0;
    int twoThetaIndex;
    double cp = cos(currentGeometry.phi*M_PI/180.0);
    double sp = sin(currentGeometry.phi*M_PI/180.0);
    double ca = cos(currentGeometry.alpha*M_PI/180.0);
    double sa = sin(currentGeometry.alpha*M_PI/180.0);
    double R = currentGeometry.radius;

    //reg.setLeft(intArea.left());
    //reg.setWidth(intArea.width());

    double cStart = double(reg.left()-intArea.x()-intArea.width()/2.0)/filmDPM;
    double LStart = double(reg.top()-intArea.y())/filmDPM;
    //double angleStart = (180.0/M_PI)*atan(sqrt( pow2( cp*((cStart)*cb+R*sb*sin((LStart)/R)) - R*sp*cos((LStart)/R)) + pow2(R*cb*sin((LStart)/R) - (cStart)*sb)  )/(R*cos(LStart/R)*cp + sp*(cStart*cb + R*sb*sin(LStart/R))) );
    double angleStart = (180.0/M_PI)*atan( sqrt( ( pow2( cStart*cp + R*sp*cos(LStart/R) ) + pow2(R*sin(LStart/R)) ) / pow2( -cStart*sp + R*cp*cos(LStart/R) ) ) );

    double cEnd = double(reg.center().x()-intArea.x()-intArea.width()/2.0)/filmDPM;
    double LEnd = double(reg.bottom()-intArea.y())/filmDPM;
    //double angleStop = (180.0/M_PI)*atan(sqrt( pow2( cp*((cEnd)*cb+R*sb*sin((LEnd)/R)) - R*sp*cos((LEnd)/R)) + pow2(R*cb*sin((LEnd)/R) - (cEnd)*sb)  )/(R*cos((LEnd)/R)*cp + sp*((cEnd)*cb + R*sb*sin((LEnd)/R))) );
    double angleStop = (180.0/M_PI)*atan( sqrt( ( pow2( cEnd*cp + R*sp*cos(LEnd/R) ) + pow2(R*sin(LEnd/R)) ) / pow2( -cEnd*sp + R*cp*cos(LEnd/R) ) ) );

    intData = new double*[int(180.0/res)];
    for (int i = 0; i < int(180.0/res); i++)
        intData[i] = new double[3];

    for (int i = 0; i < int(180.0/res); i++)
    {
        intData[i][0] = i*res;
        intData[i][1] = 0.0;
        intData[i][2] = 0.0;
    }


    // Loop through image data, determine 2theta and store intensity
    for (int x = reg.x(); x < reg.x()+reg.width(); x++) {
        c = double(x+xo-reg.x()-reg.width()/2.0)/filmDPM;
        for (int y = reg.y(); y < reg.y()+reg.height(); y++) {

            bool inexclude = false;

            for (int i = 0; i < excludeRegions.size(); i++) {
                if (excludeRegions[i].contains(x, y))
                    inexclude = true;
            }

            if (inexclude) continue;

            L = double(y+ yo-intArea.y())/filmDPM;

            int nx, ny;

            if (currentGeometry.alpha >= 0)
            {
                nx = x*ca - y*sa + sa*filmData->height();
                ny = x*sa + y*ca;
            } else
            {
                nx = x*ca - y*sa;
                ny = x*sa + y*ca - sa*filmData->height();
            }
            //c = nc;
            //L = nL;


            //twoTheta = (180.0/M_PI)*atan(sqrt( pow2( cp*((c)*cb+R*sb*sin((L)/R)) - R*sp*cos((L)/R)) + pow2(R*cb*sin((L)/R) - (c)*sb)  )/(R*cos((L)/R)*cp + sp*((c)*cb + R*sb*sin((L)/R))) );
            twoTheta = (180.0/M_PI)*atan( sqrt( ( pow2( c*cp + R*sp*cos(L/R) ) + pow2(R*sin(L/R)) ) / pow2( -c*sp + R*cp*cos(L/R) ) ) );

            if (twoTheta > 180.0) twoTheta = twoTheta - 180.0;
            if (twoTheta < 0) twoTheta = -twoTheta;

            if (L > intArea.height()/(2.0*filmDPM)) { twoTheta = 180 - twoTheta; }

            twoThetaIndex = floor(twoTheta/res + 0.5);


            if (x < filmData->width() && y < filmData->height() && twoThetaIndex < 180.0/res)
            {
                intData[twoThetaIndex][1] += qGray(filmData->pixel(nx, ny));
                intData[twoThetaIndex][2] = intData[twoThetaIndex][2] + 1;
            }

        }
    }

    double Sum_WiIi = 0.0;
    double Sum_Wi = 0.0;
    double Sum_WiIisqrt = 0.0;

    for (int i = 0; i < int(180.0/res); i++) {
        Sum_WiIi += intData[i][1];


        if ( intData[i][2] != 0 )
            intData[i][1] = intData[i][1]/intData[i][2];
        else if ( intData[i][2] == 0 && i > 0 )
            intData[i][1] = intData[i-1][1];

        Sum_WiIisqrt += intData[i][2]*sqrt(intData[i][1]);
        Sum_WiIi += intData[i][2]*intData[i][1];
        Sum_Wi += intData[i][2];
    }

    double I, Isqrt, sh;
    if (Sum_Wi != 0.0)
    {
        I = Sum_WiIi/Sum_Wi;
        Isqrt = Sum_WiIisqrt/Sum_Wi;
    } else
    {
        I = 0;
        Isqrt = 0;
    }


    if (Isqrt == 0.0)
    {
       sh = 0.0;
    } else
    {
        sh = sqrt( I - pow(Isqrt,2.0) )/ Isqrt;
    }

    return sh;
}

void FilmWidget::saveCSV()
{
    this->twoThetaWindow->saveCSV();
}

void FilmWidget::saveUDF()
{
    this->twoThetaWindow->saveUDF();
}

void FilmWidget::saveTIFF()
{
    AppConfig c(this);
    QDir p = QDir(QString::fromStdString(c.getLastPath()));
    QString suggest = QString::fromStdString(c.getLastPath()).append(suggestedName).append(".tif");

    QFileDialog qfd(this, tr("Save TIF file as..."), suggest, "*.tif");
    qfd.setNameFilter(suggest);
    qfd.setAcceptMode(QFileDialog::AcceptSave);
    QString tifFilename;
    int res = qfd.exec();

    if (res == QFileDialog::Accepted)
    {
        tifFilename = qfd.selectedFiles().at(0);
        filmData->setDotsPerMeterX(filmDPM);
        filmData->setDotsPerMeterY(filmDPM);
        filmData->save(tifFilename, "tiff", 100);
    }

}

QRect FilmWidget::getQRectFromAngleRange(double start, double stop)
{
    QRect ret;
    ret = intArea;
    bool foundBottom = false;
    bool foundTop = false;

    for (int x = intArea.x(); x < intArea.x() + intArea.width(); x++ )
    {
        for (int y = intArea.y(); y < intArea.y() + intArea.height(); y++ )
        {
            double c = double(x - intArea.x() - intArea.width()/2.0)/filmDPM;
            double L = double(y - intArea.y())/filmDPM;

            double cp = cos(currentGeometry.phi*M_PI/180.0);
            double sp = sin(currentGeometry.phi*M_PI/180.0);

            double R = currentGeometry.radius;


            double a = (180.0/M_PI)*atan( sqrt( ( pow2( c*cp + R*sp*cos(L/R) ) + pow2(R*sin(L/R)) ) / pow2( -c*sp + R*cp*cos(L/R) ) ) );

            if (a > 180.0) a = a - 180.0;
            if (a < 0) a = -a;

            if (L > intArea.height()/(2.0*filmDPM)) { a = 180 - a; }

            if (a > start && foundTop == false)
            {             
                ret.setY(y-1);
                foundTop = true;
            }

            if (a > stop && foundBottom == false)
            {                                
                ret.setBottom(y+1);
                foundBottom = true;
            }
        }
    }
    return ret;
}

double* FilmWidget::getAngleRangeFromQRect(QRect r)
{
    double* angles = new double[2];
    angles[0] = 0.0;
    angles[1] = 180.0;

    int y = r.y();
    int x = r.width()/2.0;

    double c = 0; //double(x - r.x() -r.width()/2.0)/filmDPM;
    double L = double(y-intArea.y())/filmDPM;

    double cp = cos(currentGeometry.phi*M_PI/180.0);
    double sp = sin(currentGeometry.phi*M_PI/180.0);

    double R = currentGeometry.radius;

    double a = (180.0/M_PI)*atan( sqrt( ( pow2( c*cp + R*sp*cos(L/R) ) + pow2(R*sin(L/R)) ) / pow2( -c*sp + R*cp*cos(L/R) ) ) );

    if (a > 180.0) a = a - 180.0;
    if (a < 0) a = -a;

    if (L > intArea.height()/(2.0*filmDPM)) { a = 180 - a; }

    angles[0] = a;

    y = r.y() + r.height();
    L = double(y-intArea.y())/filmDPM;

    a = (180.0/M_PI)*atan( sqrt( ( pow2( c*cp + R*sp*cos(L/R) ) + pow2(R*sin(L/R)) ) / pow2( -c*sp + R*cp*cos(L/R) ) ) );

    if (a > 180.0) a = a - 180.0;
    if (a < 0) a = -a;

    if (L > intArea.height()/(2.0*filmDPM)) { a = 180 - a; }

    angles[1] = a;

    cout << "start = " << angles[0] << ", stop = " << angles[1] << endl;

    return angles;

}

void FilmWidget::moveOptimizationRegion(QAction* a)
{
    if (a->data().isNull()) {

        bool ok;
        double a1 = QInputDialog::getDouble(this, tr("Specify angluar region"),
                                           tr("Start:"), 0, 0.0, 180.0, 2, &ok);
        double a2 = QInputDialog::getDouble(this, tr("Specify angular region"),
                                            tr("End:"), 180.0, 0.0, 180.0, 2, &ok);

        if (ok)
        {
            cout << "Move to specific region between " << a1 << " and " << a2 << endl;
            QRect r = getQRectFromAngleRange(a1, a2);
            optRegionSh = r;
            this->repaint();
        }
    } else {
        OptimizationRegion r = appConfig->getOptRegion(a->data().toString());
        cout << "Move to x = " << r.start << ", y = " << r.stop << endl;
        optRegionSh = getQRectFromAngleRange(r.start, r.stop);
        this->repaint();
    }
}

void FilmWidget::saveOptimizationRegion()
{
    bool ok;
    QString name = QInputDialog::getText(this, tr("Save optimization region"),
                                         tr("Region name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !name.isEmpty())
    {
        OptimizationRegion oreg;
        oreg.name = name;
        double* ar = getAngleRangeFromQRect(optRegionSh);

        oreg.start = ar[0];
        oreg.stop = ar[1];
        appConfig->saveOptRegion(name, oreg);
        appConfig->saveConfig();
    }
}

/* mousePressEvent
 * Used to get pixel info for a film click (currently print to console and sets a status message).
 * Only useful if filmData != NULL
 */
void FilmWidget::mousePressEvent( QMouseEvent *event)
{
    if (filmData != NULL)
    {
        if ( (1.0/scaleFactor)*event->y() < this->height()/2.0)
        {
            guessCenter = &guessCenter0;
            optRegionA = &optRegion0A;
            optRegionB = &optRegion0B;
            currentGeometry.activeCenter = &currentGeometry.zeroDegreeCenter;
        } else
        {
            guessCenter = &guessCenter180;
            optRegionA = &optRegion180A;
            optRegionB = &optRegion180B;
            currentGeometry.activeCenter = &currentGeometry.oneEightyDegreeCenter;
        }

        if (altDown == true)
        {
            previousGeometry = currentGeometry;
            QPoint oldCenter;
            oldCenter = *currentGeometry.activeCenter;//*guessCenter;

            *guessCenter = QPoint( (1.0/scaleFactor)*event->x(), (1.0/scaleFactor)*event->y());
            *currentGeometry.activeCenter = *guessCenter;

            if (optRegionA == &optRegion0A && currentGeometry.oneEightyDegreeCenter == QPoint(0,0))
            {
                currentGeometry.radius = appConfig->getCameraRadius();// (1.0/M_PI)*(currentGeometry.oneEightyDegreeCenter.y()-currentGeometry.zeroDegreeCenter.y())/getDPM();
                currentGeometry.oneEightyDegreeCenter.setY(currentGeometry.zeroDegreeCenter.y() + M_PI*currentGeometry.radius*filmDPM);
                currentGeometry.oneEightyDegreeCenter.setX(currentGeometry.zeroDegreeCenter.x());
            }

            emit geometryUpdated();

            if (optRegionA->width() == 0) {
                int offset = 1.5*0.0254*filmDPM;
                if (offset % 2 != 0) offset = offset + 1;

                optRegion0A.setWidth(0.0254*filmDPM);
                if (optRegion0A.width() % 2 == 0) optRegion0A.setWidth(optRegion0A.width()+1);

                optRegion0A.setHeight(0.0254*filmDPM);
                if (optRegion0A.height() % 2 == 0) optRegion0A.setHeight(optRegion0A.height()+1);

                optRegion0A.moveCenter(QPoint(currentGeometry.zeroDegreeCenter.x(), currentGeometry.zeroDegreeCenter.y() - offset));

                optRegion0B.setWidth(0.0254*filmDPM);
                if (optRegion0B.width() % 2 == 0) optRegion0B.setWidth(optRegion0B.width()+1);

                optRegion0B.setHeight(0.0254*filmDPM);
                if (optRegion0B.height() % 2 == 0) optRegion0B.setHeight(optRegion0B.height()+1);

                optRegion0B.moveCenter(QPoint(currentGeometry.zeroDegreeCenter.x(), currentGeometry.zeroDegreeCenter.y() + offset));

                optRegion180A.setWidth(0.0254*filmDPM);
                if (optRegion180A.width() % 2 == 0) optRegion180A.setWidth(optRegion180A.width() + 1);

                optRegion180A.setHeight(0.0254*filmDPM);
                if (optRegion180A.height() % 2 == 0) optRegion180A.setHeight(optRegion180A.height() + 1);

                optRegion180A.moveCenter(QPoint(currentGeometry.oneEightyDegreeCenter.x(), currentGeometry.oneEightyDegreeCenter.y() - offset));

                optRegion180B.setWidth(0.0254*filmDPM);
                if (optRegion180B.width() % 2 == 0) optRegion180B.setWidth(optRegion180B.width() + 1);

                optRegion180B.setHeight(0.0254*filmDPM);
                if (optRegion180B.height() % 2 == 0) optRegion180B.setHeight(optRegion180B.height() + 1);

                optRegion180B.moveCenter(QPoint(currentGeometry.oneEightyDegreeCenter.x(), currentGeometry.oneEightyDegreeCenter.y() + offset));

                cout << "ya = " << optRegion0A.center().y() << ", ha = " << optRegion0A.height() << endl;
                cout << "yb = " << optRegion0B.center().y() << ", hb = " << optRegion0B.height() << endl;

            } else {               
                int yA = optRegionA->center().y() - (oldCenter.y()-guessCenter->y());
                int yB = optRegionB->center().y() - (oldCenter.y()-guessCenter->y());
                int ySh = optRegionSh.center().y() - (oldCenter.y()-guessCenter->y());
                optRegionA->moveCenter(QPoint(guessCenter->x(), yA ));
                optRegionB->moveCenter(QPoint(guessCenter->x(), yB ));
                optRegionSh.moveCenter(QPoint(guessCenter->x(), ySh));
                cout << "Moved optimization regions." << endl;
            }
            this->repaint();

            //cout << "OptA = " << optRegionA->x() << ", " << optRegionA->y() << endl;
            //cout << "OptB = " << optRegionB->x() << ", " << optRegionB->y() << endl;
            printf("Guess center set to: %i, %i\n", guessCenter->x(), guessCenter->y());
        }



        if ((resizeActive || moveActive || excludeActive) && event->button() == Qt::LeftButton)
        {
            mouseStartX = (1.0/scaleFactor)*event->x();
            mouseStartY = (1.0/scaleFactor)*event->y();

            if (excludeActive)
            {
                QRect new_region = QRect(mouseStartX, mouseStartY, 0, 0);
                excludeRegions.append(new_region);
            }
        }

        if (excludeActive && event->button() == Qt::RightButton)
        {
            mouseStartX = (1.0/scaleFactor)*event->x();
            mouseStartY = (1.0/scaleFactor)*event->y();

            for (int i = 0; i < excludeRegions.size(); i ++)
            {
                if (excludeRegions[i].contains(mouseStartX, mouseStartY))
                {
                    excludeRegions.remove(i);
                }
            }
        }

        QPoint mPoint(mouseStartX, mouseStartY);
        if ((optRegionSh.contains(mPoint)) && event->button() == Qt::RightButton && appConfig->getOptIndex() == SharpnessOpt)
        {
            QMenu* contextMenu = new QMenu ( this );
            QMenu *regionsMenu = new QMenu ( this );
            Q_CHECK_PTR ( contextMenu );
            Q_CHECK_PTR ( regionsMenu );

            regionsMenu->setTitle("Move to...");
            QAction *sAction = new QAction ("Specify", this);

            //sAction->setData(NULL);
            regionsMenu->addAction( sAction );
            regionsMenu->addSeparator();

            QMap<QString, OptimizationRegion> optRegions = appConfig->getOptRegions();

            QMapIterator<QString, OptimizationRegion> regionIter(optRegions);
            while (regionIter.hasNext()) {
                regionIter.next();
                QString name = regionIter.key();
                QAction *a = new QAction(name, this);
                a->setData(regionIter.value().name);

                regionsMenu->addAction(a);
            }

            for (int i = 0; i < optRegions.count(); i++)
            {
                connect(regionsMenu, SIGNAL(triggered(QAction*)), this, SLOT(moveOptimizationRegion(QAction*)));
            }

            contextMenu->addAction ( "Save region" , this , SLOT (saveOptimizationRegion()) );
            contextMenu->addMenu(regionsMenu);

            contextMenu->popup( QCursor::pos() );
            contextMenu->exec ();
            delete regionsMenu;
            delete contextMenu;
            contextMenu = 0;
            regionsMenu = 0;
        }

        if (cropPhase == 1)
        {
            cropPoint1 = QPoint( (1.0/scaleFactor)*event->x(), (1.0/scaleFactor)*event->y());
            cropPhase = 2;
            this->repaint();
            sb->showMessage("Click the lower right crop point.");
        }
        else if (cropPhase == 2)
        {
            cropPoint2 = QPoint( (1.0/scaleFactor)*event->x(), (1.0/scaleFactor)*event->y());
            cropPhase = 0;
            this->repaint();

            QMessageBox msgBox;
            QString msg;
            msg.sprintf("Accept crop coordinates of (%i, %i) to (%i, %i)?", cropPoint1.x(), cropPoint1.y(), cropPoint2.x(), cropPoint2.y());
            msgBox.setText(msg);
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setDefaultButton(QMessageBox::Ok);
            int ret = msgBox.exec();

            if (ret == QMessageBox::Ok)
            {

                QImage *oldFilm = filmData;
                filmData = new QImage( filmData->copy(QRect(cropPoint1, cropPoint2)) );
                delete oldFilm;

                this->setPixmap(QPixmap::fromImage(*filmData));
                this->repaint();
            }

            cropPoint1.setX(0);
            cropPoint1.setY(0);
            cropPoint2.setX(0);
            cropPoint2.setY(0);
        }

        if (deskewPhase == 1)
        {
            deskewPoint1 = QPoint( (1.0/scaleFactor)*event->x(), (1.0/scaleFactor)*event->y());
            deskewPhase = 2;
            sb->showMessage("Click the lower deskew point.");
        }
        else if (deskewPhase == 2)
        {
            deskewPoint2 = QPoint( (1.0/scaleFactor)*event->x(), (1.0/scaleFactor)*event->y());
            deskewPhase = 0;
            this->repaint();

            double rotAngle = atan( (double)(deskewPoint2.x()-deskewPoint1.x())/(double)(deskewPoint2.y() - deskewPoint1.y()));
            cout << "[FilmWidget] Deskewing by " << rotAngle*180.0/M_PI << " degress." << endl;

            QMessageBox msgBox;
            QString msg;
            msg.sprintf("Rotate film by %f degrees?", rotAngle*180.0/M_PI);
            msgBox.setText(msg);
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setIcon(QMessageBox::Question);
            msgBox.setDefaultButton(QMessageBox::Ok);
            int ret = msgBox.exec();

            if (ret == QMessageBox::Ok)
            {

                QImage *oldFilm = filmData;

                QMatrix rotTrans;
                rotTrans.rotate(rotAngle*180.0/M_PI);
                filmData = new QImage( filmData->transformed(rotTrans, Qt::SmoothTransformation) );
                delete oldFilm;

                this->setPixmap(QPixmap::fromImage(*filmData));
            }
            this->repaint();

            deskewPoint1.setX(0);
            deskewPoint1.setY(0);
            deskewPoint2.setX(0);
            deskewPoint2.setY(0);

        }

        printf("Mouse clicked in film at (%i, %i).  Pixel data = %i\n", event->x(), event->y(), qGray(filmData->pixel( (1.0/scaleFactor)*event->x(), (1.0/scaleFactor)*event->y())));
        QString msg;
        msg.sprintf("Mouse clicked in film at (%i, %i). Pixel data = %i\n", event->x(), event->y(), qGray(filmData->pixel( (1.0/scaleFactor)*event->x(), (1.0/scaleFactor)*event->y())));
        sb->showMessage(msg);
    }
}

void FilmWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int deltaX;
    int deltaY;

    deltaX = (1.0/scaleFactor)*event->x() - mouseStartX;    
    deltaY = (1.0/scaleFactor)*event->y() - mouseStartY;

    if (resizeActive && event->buttons() == Qt::LeftButton)
    {
        if (appConfig->getOptIndex() == SymmetryOpt)
        {
            if (optRegionA->width() + deltaX < filmData->width())
            {
                QPoint ca(optRegionA->center());
                QPoint cb(optRegionB->center());
                QPoint op(*currentGeometry.activeCenter);

                optRegionA->setWidth(optRegionA->width() + deltaX);
                if (optRegionA->width() % 2 == 0) optRegionA->setWidth(optRegionA->width()+1);
                optRegionA->moveCenter(QPoint(op.x(),ca.y()));

                optRegionB->setWidth(optRegionB->width() + deltaX);
                if (optRegionB->width() % 2 == 0) optRegionB->setWidth(optRegionB->width()+1);
                optRegionB->moveCenter(QPoint(op.x(),cb.y()));

            }

            QPoint ca(optRegionA->center());
            QPoint cb(optRegionB->center());

            optRegionA->setHeight(optRegionA->height() + deltaY);
            if (optRegionA->height() % 2 == 0) optRegionA->setHeight(optRegionA->height() + 1);
            optRegionA->moveCenter(ca);

            optRegionB->setHeight(optRegionB->height() + deltaY);
            if (optRegionB->height() % 2 == 0) optRegionB->setHeight(optRegionB->height() + 1);
            optRegionB->moveCenter(cb);
        } else
        {
            QPoint ca(optRegionSh.center());
            QPoint op(currentGeometry.zeroDegreeCenter);

            optRegionSh.setWidth(optRegionSh.width() + deltaX);
            optRegionSh.moveCenter(QPoint(op.x(), ca.y()));

            optRegionSh.setHeight(optRegionSh.height() + deltaY);
            optRegionSh.moveCenter(QPoint(op.x(), ca.y()));
        }
    }

    if ( moveActive && event->buttons() == Qt::LeftButton )
    {
        if (appConfig->getOptIndex() == SymmetryOpt)
        {
            optRegionA->setHeight(optRegionA->height() + deltaY);
            optRegionA->setY(optRegionA->y() + deltaY);

            optRegionB->setHeight(optRegionB->height() - deltaY);
            optRegionB->setY(optRegionB->y() - deltaY);
        } else
        {
            optRegionSh.setHeight(optRegionSh.height() + deltaY);
            optRegionSh.setY(optRegionSh.y() + deltaY);
        }
    }

    if ( excludeActive && event->buttons() == Qt::LeftButton )
    {
        deltaX = (1.0/scaleFactor)*event->x() - mouseStartX;


        deltaY = (1.0/scaleFactor)*event->y() - mouseStartY;

        excludeRegions[excludeRegions.size()-1].setWidth(excludeRegions[excludeRegions.size()-1].width() + deltaX);
        excludeRegions[excludeRegions.size()-1].setHeight(excludeRegions[excludeRegions.size()-1].height() + deltaY);
    }

    mouseStartX = (1.0/scaleFactor)*event->x();
    mouseStartY = (1.0/scaleFactor)*event->y();

    this->repaint();

}

void FilmWidget::mouseMoveEvent(QMouseEvent *event)
{

    if ( (moveActive || resizeActive || excludeActive) && event->buttons() == Qt::LeftButton )
    {
        this->mouseReleaseEvent(event);
    }

    int x = (1.0/scaleFactor)*event->x();
    int y = (1.0/scaleFactor)*event->y();

    double c = double(x-intArea.x()-intArea.width()/2.0)/getDPM();
    double L = double(y-intArea.y())/getDPM();
    double R = getRadius();

    double twoTheta = (180.0/M_PI)*atan( sqrt( ( ( pow2(R) + pow2(c) )/pow2(R) )*(1 + pow2(tan(L/R))) - 1 ) );
    if (L > M_PI*R/2.0) twoTheta = 180.0-twoTheta;

    double d = 1e10*appConfig->getLambda()/(2.0*sin(M_PI*twoTheta/360.0));

    sb->showMessage(tr("Two Theta = %1 - D spacing = %2").arg(twoTheta).arg(d));

}

/* ***************************************************************************
 * method: paintEvent
 * description: Used to do custom drawing of widget (ie. to add lines and
 * boxes). If the film is zoomed, things are drawn according to the scalefactor.
 * ***************************************************************************/
void FilmWidget::paintEvent( QPaintEvent *event )
{
    /* Trigger parent paint event (this draws the image) */
    QLabel::paintEvent(event);

    /* Only draw if a film is loaded. */
    if (filmData != NULL)
    {
        QPainter painter(this);
        QPen pen;

        /* Set common pen properties. */
        pen.setStyle(Qt::SolidLine);
        pen.setWidth(1);

        /* 1. Draw the integration area. */
        pen.setColor(Qt::red);
        painter.setPen(pen);
        painter.setRenderHint(QPainter::Antialiasing, true);
        if (intArea.height() > 0)
        {
            QTransform transformer;
            QPointF pi = QPointF(intArea.center().x(), intArea.y());

            transformer.rotate(currentGeometry.alpha);
            QPointF pf = transformer.map(pi);
            transformer.translate( -(pf.x() - pi.x()), -(pf.y() - pi.y()));

            painter.setTransform(transformer);
            painter.drawRect(scaleFactor*intArea.x(), scaleFactor*intArea.y(), scaleFactor*intArea.width(), scaleFactor*intArea.height());
            painter.resetTransform();
        }
        painter.setRenderHint(QPainter::Antialiasing, false);

        for (int i = 0; i < excludeRegions.size(); i ++)
        {
            QBrush qb(Qt::white, Qt::DiagCrossPattern);
            painter.fillRect(scaleFactor*excludeRegions[i].x(), scaleFactor*excludeRegions[i].y(), scaleFactor*excludeRegions[i].width(), scaleFactor*excludeRegions[i].height(), qb);
        }

        /* 2. Guessed center points. */
        pen.setColor(Qt::white);
        painter.setPen(pen);
        painter.drawEllipse(scaleFactor*currentGeometry.zeroDegreeGuess.x()-6, scaleFactor*currentGeometry.zeroDegreeGuess.y()-6, 12, 12);
        painter.drawEllipse(scaleFactor*currentGeometry.oneEightyDegreeGuess.x()-6, scaleFactor*currentGeometry.oneEightyDegreeGuess.y()-6, 12, 12);

        /* 3. Optimized center point. */
        pen.setColor(Qt::blue);
        painter.setPen(pen);
        painter.drawEllipse(scaleFactor*currentGeometry.zeroDegreeCenter.x()-6, scaleFactor*currentGeometry.zeroDegreeCenter.y()-6, 12, 12);
        if (currentGeometry.zeroDegreeCenter.x() != 0 && currentGeometry.zeroDegreeCenter.y() != 0)
        {
            painter.drawLine(scaleFactor*currentGeometry.zeroDegreeCenter.x(), 0, scaleFactor*currentGeometry.zeroDegreeCenter.x(), scaleFactor*filmData->height());
            painter.drawLine(0, scaleFactor*currentGeometry.zeroDegreeCenter.y(), scaleFactor*filmData->width(), scaleFactor*currentGeometry.zeroDegreeCenter.y());
        }

        /* 4. Optimized center point. */
        pen.setColor(Qt::blue);
        painter.setPen(pen);
        painter.drawEllipse(scaleFactor*currentGeometry.oneEightyDegreeCenter.x()-6, scaleFactor*currentGeometry.oneEightyDegreeCenter.y()-6, 12, 12);
        if (currentGeometry.oneEightyDegreeCenter.x() != 0 && currentGeometry.oneEightyDegreeCenter.y() != 0)
        {
            painter.drawLine(scaleFactor*currentGeometry.oneEightyDegreeCenter.x(), 0, scaleFactor*currentGeometry.oneEightyDegreeCenter.x(), scaleFactor*filmData->height());
            painter.drawLine(0, scaleFactor*currentGeometry.oneEightyDegreeCenter.y(), scaleFactor*filmData->width(), scaleFactor*currentGeometry.oneEightyDegreeCenter.y());
        }

        /* 5. Optimization regions. */
        pen.setColor(Qt::green);
        painter.setPen(pen);

        if (appConfig->getOptIndex() == SymmetryOpt)
        {
            painter.drawRect(scaleFactor*optRegion0A.x(), scaleFactor*optRegion0A.y(), scaleFactor*optRegion0A.width(), scaleFactor*optRegion0A.height());
            painter.drawRect(scaleFactor*optRegion0B.x(), scaleFactor*optRegion0B.y(), scaleFactor*optRegion0B.width(), scaleFactor*optRegion0B.height());
            painter.drawRect(scaleFactor*optRegion180A.x(), scaleFactor*optRegion180A.y(), scaleFactor*optRegion180A.width(), scaleFactor*optRegion180A.height());
            painter.drawRect(scaleFactor*optRegion180B.x(), scaleFactor*optRegion180B.y(), scaleFactor*optRegion180B.width(), scaleFactor*optRegion180B.height());
        } else
        {
            pen.setColor(Qt::green);
            painter.setPen(pen);
            painter.setRenderHint(QPainter::Antialiasing, true);

            QTransform transformer;
            QPointF pi = QPointF(optRegionSh.center().x(), optRegionSh.y());

            transformer.rotate(currentGeometry.alpha);
            QPointF pf = transformer.map(pi);
            transformer.translate( -(pf.x() - pi.x()), -(pf.y() - pi.y()));

            painter.setTransform(transformer);
            painter.drawRect(scaleFactor*optRegionSh.x(), scaleFactor*optRegionSh.y(), scaleFactor*optRegionSh.width(), scaleFactor*optRegionSh.height());
            painter.resetTransform();
        }


        /* 6. Crop points. */
        pen.setColor(Qt::yellow);
        painter.setPen(pen);
        if (cropPoint1.x() != 0 && cropPoint1.y() != 0) {
            //painter.drawEllipse(scaleFactor*cropPoint1.x()-6, scaleFactor*cropPoint1.y()-6, 12, 12);
            painter.drawLine(cropPoint1, QPoint(this->width(), cropPoint1.y()));
            painter.drawLine(cropPoint1, QPoint(cropPoint1.x(), this->height()));
        }

        if (cropPoint2.x() != 0 && cropPoint2.y() != 0) {
            //painter.drawEllipse(scaleFactor*cropPoint2.x()-6, scaleFactor*cropPoint2.y()-6, 12, 12);
            painter.drawLine(cropPoint2, QPoint(0, cropPoint2.y()));
            painter.drawLine(cropPoint2, QPoint(cropPoint2.x(), 0));
        }

        /* Deskewing points. */
        if (deskewPoint1.x() != 0 && deskewPoint1.y() != 0 && (deskewPoint2.x()==0 && deskewPoint2.y()==0))
            painter.drawEllipse(scaleFactor*deskewPoint1.x()-6, scaleFactor*deskewPoint1.y()-6, 12, 12);

        if (deskewPoint2.x() != 0 && deskewPoint2.y() != 0) {
            painter.drawEllipse(scaleFactor*deskewPoint2.x()-6, scaleFactor*deskewPoint2.y()-6, 12, 12);
            painter.drawLine(deskewPoint1, deskewPoint2);
        }

        painter.end();
    }
}
