/*
 *  TwoThetaWindow.cpp
 *  GandolfiViewer
 *
 *  Created by Joe Petrus on 08/02/10.
 *
 */

#include "TwoThetaWindow.h"

TwoThetaWindow::TwoThetaWindow(QWidget *parent, AppConfig &_appConfig) : QMainWindow(parent, Qt::Window)
{
    appConfig = &_appConfig;
    start2Theta = 0.0;
    end2Theta = 180.0;
    //QVBoxLayout *layout = new QVBoxLayout(this);
    initializeDB();
    sb = statusBar();
    twoThetaXYPlot = new TwoThetaPlot(this, *appConfig);
    this->setCentralWidget(twoThetaXYPlot);
    twoThetaXYPlot->setCanvasBackground(Qt::white);
    twoThetaXYPlot->resize(650,500);

    twoThetaXYPlot->show();
    twoThetaXYPlot->setAxisScale(QwtPlot::xBottom, start2Theta, end2Theta, 15.0);

    this->setWindowTitle(tr("Diffractogram"));
    this->resize(900,500);

    this->hide();

    zoomer = new QwtPlotZoomer(twoThetaXYPlot->canvas());
    zoomer->setEnabled(false);
    picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                               QwtPicker::PointSelection,
                               QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOff,
                               twoThetaXYPlot->canvas());
    picker->setRubberBandPen(QColor(Qt::black));
    picker->setRubberBand(QwtPicker::CrossRubberBand);
    picker->setTrackerPen(QColor(Qt::black));	
    picker->setEnabled(true);

    connect(picker, SIGNAL(selected(const QwtDoublePoint &)), SLOT(selected(const QwtDoublePoint &)));
    connect(picker, SIGNAL(moved(const QwtDoublePoint &)), SLOT(selected(const QwtDoublePoint &)));
    macroDialog = new QDialog(this);
    UiMacro.setupUi(macroDialog);
    /* Construct toolbar */
    toolbar = new QToolBar(this);
    toolbar->setFloatable(false);
    toolbar->setMovable(false);
    this->addToolBar(toolbar);
    createToolbar();


    this->setContentsMargins(0, 0, 0, 0);
    twoThetaXYPlot->setContentsMargins(10, 10, 10, 0);

    //connect(toolbar, SIGNAL())


    searchDialog = new QWidget(dockWidget);
    UiD.setupUi(searchDialog);


    dockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    dockWidget->setWidget(searchDialog);
    this->addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    connect(UiD.listMatches, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(openWebMineral()));
    //connect(UiD.listMatches, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(showMineralLines(QListWidgetItem*)));
    //connect(UiD.listMatches, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(showMineralLines(QListWidgetItem*)));
    connect(UiD.listMatches, SIGNAL(currentRowChanged(int)), this, SLOT(showMineralLines(int)));
    searchDialog->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
}

void TwoThetaWindow::createToolbar()
{
    toolbar->clear();

    QToolButton *btnSaveUDF = new QToolButton(toolbar);
    btnSaveUDF->setText("Save UDF");
    btnSaveUDF->setIcon( QIcon(":/icons/Resources/saveudf.png") );
    btnSaveUDF->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnSaveUDF);

    QToolButton *btnSaveCSV = new QToolButton(toolbar);
    btnSaveCSV->setText("Save CSV");
    btnSaveCSV->setIcon( QIcon(":/icons/Resources/savecsv.png") );
    btnSaveCSV->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnSaveCSV);

    QToolButton *btnPrint = new QToolButton(toolbar);
    btnPrint->setText("Print");
    btnPrint->setIcon(QIcon(":/icons/Resources/print.png"));
    btnPrint->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnPrint);

    toolbar->addSeparator();

    QToolButton *btnZoom = new QToolButton(toolbar);
    btnZoom->setText("Zoom");
    btnZoom->setIcon( QIcon(":/icons/Resources/viewmag.png") );
    btnZoom->setCheckable(true);
    btnZoom->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnZoom);

    QToolButton *btnIntScale = new QToolButton(toolbar);
    btnIntScale->setText("Scale Intensity");
    btnIntScale->setIcon( QIcon(":/icons/Resources/mult.png") );
    btnIntScale->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnIntScale);

    QToolButton *btnIntOffset = new QToolButton(toolbar);
    btnIntOffset->setText("Offset Intensity");
    btnIntOffset->setIcon( QIcon(":/icons/Resources/offset.png") );
    btnIntOffset->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnIntOffset);

    QToolButton *btnTruncate = new QToolButton(toolbar);
    btnTruncate->setText("Truncate");
    btnTruncate->setIcon( QIcon(":/icons/Resources/truncate.png"));
    btnTruncate->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnTruncate);

    QToolButton *btnBGS = new QToolButton(toolbar);
    btnBGS->setText("Background Subtraction");
    btnBGS->setIcon( QIcon(":/icons/Resources/bgs.png") );
    btnBGS->setCheckable(true);
    btnBGS->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnBGS);

    QToolButton *btnMinMax = new QToolButton(toolbar);
    btnMinMax->setText("Min/Max");
    btnMinMax->setIcon( QIcon(":/icons/Resources/minmax.png") );
    btnMinMax->setCheckable(true);
    btnMinMax->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnMinMax);

    toolbar->addSeparator();
    int kbShortCut = 1;
    QMapIterator<QString, MineralMacro> macroIter(appConfig->getMineralMacros());
    while (macroIter.hasNext()) {
        macroIter.next();
        macroButtonArray.append(new QToolMacro(toolbar));
        macroButtonArray[macroButtonArray.size()-1]->setText(macroIter.value().title);
        macroButtonArray[macroButtonArray.size()-1]->setToolTip("Right click to configure");
        macroButtonArray[macroButtonArray.size()-1]->setToolButtonStyle(Qt::ToolButtonTextOnly);
        macroButtonArray[macroButtonArray.size()-1]->setShortcut(QKeySequence(QString::number(kbShortCut)));
        toolbar->addWidget(macroButtonArray[macroButtonArray.size()-1]);
        kbShortCut++;
    }

    toolbar->addSeparator();

    QLabel *spacerLabel = new QLabel("");
    spacerLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    toolbar->addWidget(spacerLabel);

    dockWidget = new QDockWidget("Mineral Identification", this);
    QToolButton *btnSearch = new QToolButton(toolbar);
    QAction *searchAct = dockWidget->toggleViewAction();
    searchAct->setIcon(QIcon(":/icons/Resources/find.png"));
    searchAct->setText("Mineral Identification");
    btnSearch->setDefaultAction(searchAct);
    btnSearch->setCheckable(true);
    btnSearch->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnSearch);

    QToolButton *btnReset = new QToolButton(toolbar);
    btnReset->setText("Reset Plot");
    btnReset->setIcon(QIcon(":/icons/Resources/reset.png"));
    btnReset->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnReset);

    connect(btnZoom, SIGNAL(toggled(bool)), SLOT(enableZoomMode(bool)));
    connect(btnMinMax, SIGNAL(toggled(bool)), SLOT(toggleMinMax(bool)));
    connect(btnBGS, SIGNAL(toggled(bool)), SLOT(toggleBGS(bool)));
    connect(btnSaveUDF, SIGNAL(clicked()), SLOT(saveUDF()));
    connect(btnSaveCSV, SIGNAL(clicked()), SLOT(saveCSV()));
    connect(btnIntOffset, SIGNAL(clicked()), SLOT(intOffset()));
    connect(btnIntScale, SIGNAL(clicked()), SLOT(intScale()));
    connect(btnTruncate, SIGNAL(clicked()), SLOT(truncate()));
    connect(btnReset, SIGNAL(clicked()), SLOT(reset()));
    connect(btnPrint, SIGNAL(clicked()), SLOT(printData()));

    MineralMacro mm;
    for (int i = 0; i < macroButtonArray.size(); i ++)
    {
        QAction *a = new QAction(macroButtonArray[i]->text(), this);
        a->setData(macroButtonArray[i]->text());
        macroButtonArray[i]->setDefaultAction(a);
        connect(macroButtonArray[i], SIGNAL(triggered(QAction*)), SLOT(doMacro(QAction*)));
    }

}

void TwoThetaWindow::showMineralLines(int i)
{
    if (UiD.listMatches->currentRow() == i)
        return;
    else
        showMineralLines(UiD.listMatches->item(i));

}

void TwoThetaWindow::contextMenuEvent(QContextMenuEvent *event)
{
    cout << "Context menu event" << endl;
    cout << toolbar->frameGeometry().x() << " " << toolbar->frameGeometry().y() << " " << toolbar->frameGeometry().width() << " " << toolbar->frameGeometry().height() << endl;
    cout << event->pos().x() << " " << event->pos().y() << endl;

    if (toolbar->frameGeometry().contains(event->pos()))
    {
        QMenu* contextMenu = new QMenu ( this );
        Q_CHECK_PTR ( contextMenu );


        contextMenu->addAction("Add macro", this, SLOT(addMacro()));

        contextMenu->popup( QCursor::pos() );
        contextMenu->exec ();

        delete contextMenu;
        contextMenu = 0;
    }

}

void TwoThetaWindow::addMacro()
{

    int res = macroDialog->exec();

    if (res == QDialog::Accepted)
    {
        MineralMacro mm;
        mm.title = UiMacro.leTitle->text();
        mm.angleStart = UiMacro.sbStart->value();
        mm.angleStop = UiMacro.sbStop->value();
        mm.scale = UiMacro.sbScale->value();
        mm.offset = UiMacro.sbOffset->value();
        mm.autoSave = UiMacro.cbAutosave->isChecked();

        appConfig->saveMineralMacro(mm.title, mm);
    }

    createToolbar();

    appConfig->saveConfig();
}

void TwoThetaWindow::showMineralLines(QListWidgetItem *lwi)
{
    cout << "Current mineral = " << currentMineral.toStdString() << ", text = " << lwi->text().toStdString() << endl;

    if (UiD.listMatches->count() == 0) return;


    minSymbol.setStyle(QwtSymbol::DTriangle);
    minSymbol.setSize(11,11);

    QString sMineral = lwi->text();
    if (sMineral == currentMineral)
    {
        for (int i = 0; i < minMarkers.count(); i++)
            minMarkers[i]->setValue(-100,1);


        currentMineral = "";
        UiD.listMatches->blockSignals(true);
        lwi->setSelected(false);
        twoThetaXYPlot->replot();
        UiD.listMatches->blockSignals(false);
        return;
    }

    Mineral mineral = mindb[sMineral];
    cout << "Trying to show " << mineral.name.toStdString() << " with " << mineral.nLines << " lines" << endl;

    for (int i = 0; i < mineral.lines.count(); i++)
    {
        if (i >= minMarkers.count())
        {
            QwtPlotMarker *pm = new QwtPlotMarker();
            minMarkers.append(pm);
            minMarkers[i]->attach(this->twoThetaXYPlot);
        }
        double d = mineral.lines[i];
        double I = mineral.intensities[i] * this->twoThetaXYPlot->maxYinRange(0.0, 180.0);
        double tt = 2*(180.0/M_PI)*asin(CuLambda/(2*d*1e-10));
        minMarkers[i]->setValue(tt,I);
        minMarkers[i]->setLineStyle(QwtPlotMarker::VLine);
        minMarkers[i]->setSymbol(minSymbol);
    }

    for (int i = mineral.lines.count(); i < minMarkers.count(); i++)
        minMarkers[i]->setValue(-100,1);

    twoThetaXYPlot->replot();
    currentMineral = sMineral;
    this->repaint();
}

void TwoThetaWindow::toggleMinMax(bool b)
{
    if (b)
        twoThetaXYPlot->mmOn();
    else
        twoThetaXYPlot->mmOff();
}

void TwoThetaWindow::toggleBGS(bool b)
{
    if (b)
        twoThetaXYPlot->bgsOn();
    else
        twoThetaXYPlot->bgsOff();
}

void TwoThetaWindow::printData()
{
    /* Make sure QT printing is a go */
#ifndef QT_NO_PRINTER
    QPrintDialog dialog(&printer, this);
    printer.setOrientation(QPrinter::Landscape);
    printer.setPaperSize(QPrinter::Letter);
    if (dialog.exec()) {
        QPainter painter(&printer);
        twoThetaXYPlot->print(&painter, painter.viewport() );
    }
#endif
}

void TwoThetaWindow::doMacro(QAction * qa)
{
    cout << "Do Macro Called" << endl;
    MineralMacro mm = appConfig->getMineralMacro(qa->data().toString());
    QToolMacro *qtm = NULL;

    for (int i = 0; i < macroButtonArray.size(); i++)
    {
        if (macroButtonArray[i]->text() == mm.title)
            qtm = macroButtonArray[i];
    }

    if (qtm == NULL)
        return;

    bool rightButton = qtm->isRightClick();
    cout << "rightButton = " << rightButton << endl;
    // Left button click => execute the macro
    if (! rightButton)
    {
        truncate(false, mm.angleStart, mm.angleStop);
        doScale( mm.scale );
        doOffset( mm.offset );
    }
    // Right button click => configure the macro
    else
    {

        connect(UiMacro.btnDelete, SIGNAL(clicked()), this, SLOT(deleteMacro()));
        UiMacro.leTitle->setText(mm.title);
        UiMacro.sbStart->setValue(mm.angleStart);
        UiMacro.sbStop->setValue(mm.angleStop);
        UiMacro.sbOffset->setValue(mm.offset);
        UiMacro.sbScale->setValue(mm.scale);
        UiMacro.cbAutosave->setChecked(mm.autoSave);

        int res = macroDialog->exec();

        if (res == QDialog::Accepted)
        {
            MineralMacro mm;
            mm.title = UiMacro.leTitle->text();
            mm.angleStart = UiMacro.sbStart->value();
            mm.angleStop = UiMacro.sbStop->value();
            mm.scale = UiMacro.sbScale->value();
            mm.offset = UiMacro.sbOffset->value();
            mm.autoSave = UiMacro.cbAutosave->isChecked();

            appConfig->saveMineralMacro(mm.title, mm);
        }

        createToolbar();

        appConfig->saveConfig();
    }

}

void TwoThetaWindow::deleteMacro()
{
    cout << "Removing " << UiMacro.leTitle->text().toStdString() << endl;
    appConfig->deleteMacro(UiMacro.leTitle->text());
    appConfig->saveConfig();
    macroDialog->hide();

    createToolbar();
}

void TwoThetaWindow::reset()
{
    twoThetaXYPlot->reset();
    setXYData(twoThetaXYPlot->getXValues(), twoThetaXYPlot->getYValues(), twoThetaXYPlot->getSize(), 0.0, 180.0);
    start2Theta = 0.0;
    end2Theta = 180.0;
}

void TwoThetaWindow::setXYData(double *xd, double *yd, int s, int sx, int ex) {
    twoThetaXYPlot->setXYData(xd,yd, s, sx, ex);
    QwtPlotZoomer *oldZoomer = zoomer;
    bool en = oldZoomer->isEnabled();
    zoomer = new QwtPlotZoomer(twoThetaXYPlot->canvas());
    delete oldZoomer;
    zoomer->setEnabled(en);
    findPrimarySpacings();
}

void TwoThetaWindow::initializeDB()
{

    QString qline;
    QStringList dbentry;
    QString currentMin = "";
    Mineral min;

    ifstream dbfile ("xraydb.csv");
    ifstream chemfile ("chemistry.csv");

    if (dbfile.is_open())
    {
      while (dbfile)
      {

        double L, I;
        QString name;
        string line;
        dbfile >> line;
        QString qline = QString::fromStdString(line);
        QStringList entry = qline.split(",");
        if (entry.count() < 3) break;
        name = entry.at(0);

        L = entry.at(1).toDouble();
        I = entry.at(2).toDouble();

        if (currentMin == name)
        {
            min.intensities.append(I);
            min.lines.append(L);
            min.nLines = min.nLines + 1;
        } else
        {
            mindb.insert(min.name, min);
            min.name = name;
            currentMin = name;
            min.intensities.clear();
            min.lines.clear();
            min.nLines = 1;
            min.lines.append(L);
            min.intensities.append(I);
        }
      }

      mindb.insert(min.name, min);
      dbfile.close();

    } else cout << "Unable to open mineral database" << endl;

    if (chemfile.is_open())
    {
        while(chemfile)
        {
            string line;
            chemfile >> line;
            QString qline = QString::fromStdString(line);
            QStringList entry = qline.split(",");
            QList<QString> list = getElementsFromString(qline.section(",", 1));
            mindb[qline.section(",", 0, 0)].formula = qline.section(",", 1).remove("\"");
            mindb[qline.section(",", 0, 0)].elements = list;

        }
    }
    else cout << "Unable to open chemistry file." << endl;

}

QList<QString> TwoThetaWindow::getElementsFromString(QString s)
{
    QList<QString> elements;
    QList<QString> allElements;
    allElements << "H" << "He" << "Li" << "Be" << "B" << "C" << "N" << "O" << "F" << "Ne";
    allElements << "Na" << "Mg" << "Al" << "Si" << "P" << "S" << "Cl" << "Ar";
    allElements << "K" << "Ca" << "Sc" << "Ti" << "V" << "Cr" << "Mn" << "Fe" << "Co" << "Ni" << "Cu" << "Zn" << "Ga" << "Ge" << "As" << "Se" << "Br" << "Kr";
    allElements << "Rb" << "Sr" << "Y" << "Zr" << "Nb" << "Mo" << "Tc" << "Ru" << "Rh" << "Pd" << "Ag" << "Cd" << "In" << "Sn" << "Sb" << "Te" << "I" << "Xe";
    allElements << "Cs" << "Ba" << "La" << "Hf" << "Ta" << "W" << "Re" << "Os" << "Is" << "Pt" << "Au" << "Hg" << "Tl" << "Pb" << "Bi" << "Po" << "At" << "Rn";
    allElements << "Fr" << "Ra" << "Ac" << "Rf" << "Db" << "Sg" << "Bh" << "Hs" << "Mt" << "Ds" << "Rg";
    allElements << "Ce" << "Pr" << "Nd" << "Pm" << "Sm" << "Eu" << "Gd" << "Tb" << "Dy" << "Ho" << "Er" << "Tm" << "Yb" << "Lu";
    allElements << "Th" << "Pa" << "U" << "Np" << "Pu" << "Am" << "Cm" << "Bk" << "Cf" << "Es" << "Fm" << "Md" << "No" << "Lr";

    s.remove("\"");

    for (int i = 0; i < allElements.count(); i ++)
    {
        if (s.contains(allElements.at(i)))
        {
            int j = 0;

            while (j < s.length())
            {
                int si = s.indexOf(allElements.at(i), j);
                if (si == -1)
                    break;
                else
                    si = si + allElements.at(i).length();
                if (si == s.length())
                {
                    elements.append(allElements.at(i));
                    break;
                } else if (! (si < s.length()))
                    break;
                QChar c = s.at(si);

                if (c.isUpper() || c == '+' || c == ')' || c == '(' || c == '[' || c == ']' || c.isDigit() || c == ',')
                {
                    elements.append(allElements.at(i));
                    break;
                }

                j = s.indexOf(allElements.at(i), j) + 1;
                if (j == 0) break;
            }


        }
    }

    return elements;
}

void TwoThetaWindow::findPrimarySpacings()
{
    QwtPlotCurve *data = twoThetaXYPlot->getDataBGS();
    double lambda = appConfig->getLambda();

    double d1 = 0.0;
    double I1 = 0.0;
    double d2 = 0.0;
    double I2 = 0.0;
    double d3 = 0.0;
    double I3 = 0.0;

    for (int i = 1; i < data->dataSize()/2; i++)
    {

        if (data->y(i-1) < data->y(i) && data->y(i+1) < data->y(i))
        {
            if (data->y(i) > I1)
            {
                d3 = d2;
                I3 = I2;
                d2 = d1;
                I2 = I1;
                d1 = data->x(i);
                I1 = data->y(i);
            }
            else if (data->y(i) > I2)
            {
                d3 = d2;
                I3 = I2;
                d2 = data->x(i);
                I2 = data->y(i);
            }
            else if (data->y(i) > I3)
            {
                d3 = data->x(i);
                I3 = data->y(i);
            }
        }
    }

    // Convert 2theta into d spacings
    d1 = 1e10*lambda/(2.0*sin((M_PI/180.0)*0.5*d1));
    d2 = 1e10*lambda/(2.0*sin((M_PI/180.0)*0.5*d2));
    d3 = 1e10*lambda/(2.0*sin((M_PI/180.0)*0.5*d3));

    cout << "d1 = " << d1 << ", I1 = " << I1 << endl;
    cout << "d2 = " << d2 << ", I2 = " << I2 << endl;
    cout << "d3 = " << d3 << ", I3 = " << I3 << endl;

    UiD.leD1->setText(QString::number(d1));
    UiD.leD2->setText(QString::number(d2));
    UiD.leD3->setText(QString::number(d3));
    UiD.sbTolerance->setValue(0.02);

    connect(UiD.btnSearch, SIGNAL(clicked()), this, SLOT(searchClicked()));

    searchDialog->show();
}

void TwoThetaWindow::searchClicked()
{
    disconnect(UiD.listMatches, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(showMineralLines(QListWidgetItem*)));
    disconnect(UiD.listMatches, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(showMineralLines(QListWidgetItem*)));
    disconnect(UiD.listMatches, SIGNAL(currentRowChanged(int)), this, SLOT(showMineralLines(int)));
    UiD.listMatches->clear();
    connect(UiD.listMatches, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(showMineralLines(QListWidgetItem*)));
    connect(UiD.listMatches, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(showMineralLines(QListWidgetItem*)));
    connect(UiD.listMatches, SIGNAL(currentRowChanged(int)), this, SLOT(showMineralLines(int)));
    mineralUrls.clear();

    double l1 = UiD.leD1->text().toDouble();
    double l2 = UiD.leD2->text().toDouble();
    double l3 = UiD.leD3->text().toDouble();
    double tol = UiD.sbTolerance->value();

    QStringList reqElements;
    if (!UiD.leElements->text().isEmpty())
    {
        reqElements = UiD.leElements->text().split(",");
        cout << "Required elements: ";
        for (int i = 0; i < reqElements.count(); i ++)
            cout << reqElements.at(i).toStdString() << ", ";
        cout << endl;
    }

    bool l1sat, l2sat, l3sat;
    l1sat = false;
    l2sat = false;
    l3sat = false;

    QList<QString> matches;

    QMapIterator<QString, Mineral> mindbIter(mindb);

    while (mindbIter.hasNext()) {
        mindbIter.next();

        l1sat = false;
        l2sat = false;
        l3sat = false;

        for (int j = 0; j < mindbIter.value().nLines; j++) {
            if (j >= mindbIter.value().lines.count())
                break;

            if ( ( (l1 - l1*tol) < mindbIter.value().lines[j] ) && ( (l1 + l1*tol) > mindbIter.value().lines[j] ) )
                l1sat = true;

            if ( ( (l2 - l2*tol) < mindbIter.value().lines[j] ) && ( (l2 + l2*tol) > mindbIter.value().lines[j] ) )
                l2sat = true;

            if ( ( (l3 - l3*tol) < mindbIter.value().lines[j] ) && ( (l3 + l3*tol) > mindbIter.value().lines[j] ) )
                l3sat = true;
        }

        if (l1sat && l2sat && l3sat)
        {
            bool hasElements = true;
            cout << "Considering " << mindbIter.value().name.toStdString() << endl;
            for (int w = 0; w < reqElements.count(); w++)
            {
                if (!mindbIter.value().elements.contains(reqElements.at(w)))
                    hasElements = false;
            }

            if (hasElements)
                matches << mindbIter.value().name;// + " " + mindbIter.value().formula;
        }

    }

    for (int i = 0; i < matches.count(); i++)
    {
        cout << matches[i].toStdString() << endl;
        QListWidgetItem *item = new QListWidgetItem(matches[i]);
        item->setToolTip(mindb[matches[i]].formula);
        UiD.listMatches->addItem(item);

    }

    if (matches.count() == 0)
    {
        cout << "No Matches found." << endl;
        UiD.listMatches->addItem("No matches found.");
    }
}

void TwoThetaWindow::openWebMineral()
{
    int i = UiD.listMatches->currentRow();
    QDesktopServices::openUrl(QUrl(mineralUrls[i]));
}
/*
void TwoThetaWindow::finishedSlot(QNetworkReply* reply)
{
    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    QVariant redirectionTargetUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    // no error received?
    if (reply->error() == QNetworkReply::NoError)
    {
        QByteArray bytes = reply->readAll();
        QString string(bytes);

        QStringList minSL = string.split("<td align=\"left\"><a href=\"../");
        QStringList formulaSL = string.split("<td align=\"left\"><small>");

        // First one won't be a mineral (thus i = 1, not 0)
        for (int i = 1; i < minSL.size(); i++)
        {
            QString mineralWeb = tr("http://www.webmineral.com/%1").arg(minSL.at(i).split("\">").at(0));
            QString mineral = mineralWeb;
            mineral.chop(6);
            mineral = mineral.split("/").at(mineral.split("/").size()-1);
            QString formula = formulaSL.at(i).split("<").at(0);
            cout << "Mineral " << i << " = " << mineral.toStdString() << " - " << mineralWeb.toStdString() << endl;
            UiD.listMatches->addItem(tr("%1 - [%2]").arg(mineral, formula));
            mineralUrls.append(mineralWeb);
        }

        if (minSL.size() == 1)
            UiD.listMatches->addItem("No matches found.\nTry increasing the tolerance.");
    }
    else
    {
        cout << "Error in WebMineral search..." << endl;
    }

    delete reply;
}*/

void TwoThetaWindow::saveCSV()
{
    //AppConfig c(this);
    QDir p = QDir(QString::fromStdString(appConfig->getLastPath()));
    QString suggest = QString::fromStdString(appConfig->getLastPath()).append(suggestedName).append(".csv");
    cout << "suggest = " << suggest.toStdString() << endl;
    QFileDialog qfd(this, tr("Save CSV file as..."), QString::fromStdString(appConfig->getLastPath()), tr("*.csv"));

    qfd.selectFile(tr("").append(suggestedName).append(".csv"));
    qfd.setAcceptMode(QFileDialog::AcceptSave);
    QString csvFilename;
    int res = qfd.exec();

    if (res == QFileDialog::Accepted)
    {
        csvFilename = qfd.selectedFiles().at(0);

        QwtPlotCurve *data = twoThetaXYPlot->getData();
        double intResolution = data->x(2)-data->x(1);

        int starti = floor(start2Theta/intResolution + 0.5);
        int endi = floor(end2Theta/intResolution + 0.5);

        ofstream dataFile (csvFilename.toAscii());
        if (dataFile.is_open())
        {
            appConfig->setLastPath(csvFilename.section('/', 0, -2).toStdString());
            appConfig->saveConfig();

            for (int row = starti; row <= endi; row++) {
                dataFile << data->x(row) << ", " << data->y(row) << "\n";
            }
            dataFile.close();
        }
        else {
            cout << "Unable to save file..." << endl;
        }
    }
}

void TwoThetaWindow::sharpness()
{
    QwtPlotCurve *data = twoThetaXYPlot->getData();

    double intResolution = data->x(2) - data->x(1);

    int starti = start2Theta/intResolution;
    int endi = end2Theta/intResolution;



    for (int i = starti; i < endi; i ++)
    {



      //  sh = sqrt(I - pow(Isqrt,2))/Isqrt;
    }

}

void TwoThetaWindow::saveUDF()
{
    QDir p = QDir(QString::fromStdString(appConfig->getLastPath()));
    QString suggest = QString::fromStdString(appConfig->getLastPath()).append(suggestedName).append(".udf");
    cout << "suggest = " << suggest.toStdString() << endl;
    QFileDialog qfd(this, tr("Save UDF file as..."), QString::fromStdString(appConfig->getLastPath()), tr("*.udf"));
    //qfd.setNameFilter(suggest);
    qfd.selectFile(tr("").append(suggestedName).append(".udf"));
    qfd.setAcceptMode(QFileDialog::AcceptSave);
    QString udfFilename;
    int res = qfd.exec();

    if (res == QFileDialog::Accepted)
    {
        udfFilename = qfd.selectedFiles().at(0);

        QwtPlotCurve *data = twoThetaXYPlot->getData();

        double intResolution = data->x(2)-data->x(1);
        cout << "Saving UDF file with resolution: " << intResolution << endl;

        QDateTime now(QDate::currentDate(), QTime::currentTime());
        QString fullDate = now.toString("dd-MMM-yyyy hh:mm:ss AP");

        double actualStart = int(start2Theta/intResolution)*intResolution;
        double actualEnd = int(end2Theta/intResolution)*intResolution;

        ofstream udfFile(udfFilename.toAscii());
        if (udfFile.is_open())
        {
            appConfig->setLastPath(udfFilename.section('/', 0, -2).toStdString());
            appConfig->saveConfig();
            udfFile << "SampleIdent, Gandolfi Sample ,/" << endl;
            udfFile << "Title1, Gandolfi 1.0 ,/" << endl;
            udfFile << "Title2, ,/" << endl;
            udfFile << "DiffrType, PW3710 ,/" << endl;
            udfFile << "DiffrNumber, 1 /" << endl;
            udfFile << "Anode, Cu ,/" << endl;
            udfFile << "LabdaAlpha1, 1.54060 ,/" << endl;
            udfFile << "LabdaAlpha2, 1.54443 ,/" << endl;
            udfFile << "RatioAlpha21, 0.50000 ,/" << endl;
            udfFile << "DivergenceSlit, Fixed , 1.00 ,/" << endl;
            udfFile << "ReceivingSlit, 0.10 ,/" << endl;
            udfFile << "MonochromatorUsed, NO ,/" << endl;
            udfFile << "GeneratorVoltage, 40.0 ,/" << endl;
            udfFile << "TubeCurrent, 20.0 ,/" << endl;
            udfFile << "FileDateTime, " << fullDate.toStdString() << " ,/" << endl;
            udfFile << "DataAngleRange, " << actualStart << " , " << actualEnd << " ,/" << endl;
            udfFile << "ScanStepSize, " << intResolution << " ,/" << endl;
            udfFile << "ScanType, Pre-set time ,/" << endl;
            udfFile << "ScanStepTime, 5.00 ,/" << endl;
            udfFile << "RawScan" << endl;

            int starti = start2Theta/intResolution;
            int endi = end2Theta/intResolution;

            for (int i = starti; i <= endi; i++) {
                if ((i - (starti%8)) % 8 == 0 && (i != starti)) { udfFile << endl; }

                udfFile << "\t" << (int)data->y(i);

                if ( ((i - (starti%8)) + 1) % 8 != 0 ) udfFile << ",";
            }

            udfFile << "/" << endl;

        }
    }
}

void TwoThetaWindow::intOffset(bool prompt, double _offset)
{
    bool ok;
    int offset;

    if (prompt)
        offset = QInputDialog::getInt(this, tr("Add offset to integration"), tr("Offset: "), 0, -10000000, 10000000, 1, &ok);
    else
        offset = _offset;

    doOffset(offset);
}

void TwoThetaWindow::doOffset(double offset)
{
    double *yd = twoThetaXYPlot->getYValues();
    double *xd = twoThetaXYPlot->getXValues();
    int s = twoThetaXYPlot->getSize();

    for (int i = 0; i < s; i++ )
    {
        yd[i] = yd[i] + offset;
    }


   this->setXYData(xd, yd, s, start2Theta, end2Theta);
}

void TwoThetaWindow::intScale(bool prompt, double _scale)
{

    double scale = 1.0;

    if (prompt)
    {
        bool ok;
        scale = QInputDialog::getDouble(this, tr("Scale integration"), tr("Scale by: "), 1, -10000000, 10000000, 6, &ok);
    } else
    {
        scale = _scale;
    }

    doScale(scale);
}

void TwoThetaWindow::doScale(double scale)
{
    double *yd = twoThetaXYPlot->getYValues();
    double *xd = twoThetaXYPlot->getXValues();
    double s = twoThetaXYPlot->getSize();

    for (int i = 0; i < s; i++ )
    {
        yd[i] = yd[i]*scale;
    }

    this->setXYData(xd, yd, s, start2Theta, end2Theta);
}

void TwoThetaWindow::truncate(bool prompt, double _start, double _stop)
{
    if (prompt)
    {
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle("Truncate");
        dialog->resize(420,50);
        QHBoxLayout *layout = new QHBoxLayout(dialog);

        QLabel *l1 = new QLabel("Use 2 theta from");
        layout->addWidget(l1);

        QLineEdit *leS = new QLineEdit(QString::number(start2Theta));
        layout->addWidget(leS);

        QLabel *l2 = new QLabel("to");
        layout->addWidget(l2);

        QLineEdit *leE = new QLineEdit(QString::number(end2Theta));
        layout->addWidget(leE);

        QPushButton *btnOk = new QPushButton("Ok");
        connect(btnOk, SIGNAL(clicked()), dialog, SLOT(accept()));

        QPushButton *btnCancel = new QPushButton("Cancel");
        connect(btnCancel, SIGNAL(clicked()), dialog, SLOT(reject()));

        layout->addWidget(btnOk);
        layout->addWidget(btnCancel);

        int res = dialog->exec();

        if (res == QDialog::Accepted)
        {
            start2Theta = leS->text().toDouble();
            end2Theta = leE->text().toDouble();
            twoThetaXYPlot->changeRange(start2Theta, end2Theta);
            twoThetaXYPlot->resetYRange(start2Theta, end2Theta);
            QwtPlotZoomer *oldZoomer = zoomer;
            bool en = oldZoomer->isEnabled();
            zoomer = new QwtPlotZoomer(twoThetaXYPlot->canvas());
            delete oldZoomer;
            zoomer->setEnabled(en);
        }
    }
    else
    {
        start2Theta = _start;
        end2Theta = _stop;
        twoThetaXYPlot->changeRange(start2Theta, end2Theta);
        twoThetaXYPlot->resetYRange(start2Theta, end2Theta);
        QwtPlotZoomer *oldZoomer = zoomer;
        bool en = oldZoomer->isEnabled();
        zoomer = new QwtPlotZoomer(twoThetaXYPlot->canvas());
        delete oldZoomer;
        zoomer->setEnabled(en);

    }
}

void TwoThetaWindow::enableZoomMode(bool z)
{
    zoomer->setEnabled(z);
    picker->setEnabled(!z);
}

void TwoThetaWindow::selected(const QwtDoublePoint &pos)
{
    QString info;
    double l = appConfig->getLambda();

    info.sprintf("2Theta = %g [deg] -- Intensity = %g [arb] -- d = %g [ang]",
                 pos.x(),
                 pos.y(),
                 1e10*l/(2.0*sin((M_PI/180.0)*0.5*pos.x()))
                 );
    sb->showMessage(info);

    switch (activeD)
    {
    case 1:
        UiD.leD1->setText(QString::number(1e10*l/(2.0*sin((M_PI/180.0)*0.5*pos.x()))));
        break;
    case 2:
        UiD.leD2->setText(QString::number(1e10*l/(2.0*sin((M_PI/180.0)*0.5*pos.x()))));
        break;
    case 3:
        UiD.leD3->setText(QString::number(1e10*l/(2.0*sin((M_PI/180.0)*0.5*pos.x()))));
        break;
    default:
        break;
    }
}	

void TwoThetaWindow::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape) activeD = 0;
    if (e->key() == Qt::Key_F1) activeD = 1;
    if (e->key() == Qt::Key_F2) activeD = 2;
    if (e->key() == Qt::Key_F3) activeD = 3;
    //if (e->key() == Qt::Key_1) doMacro();
    //if (e->key() == Qt::Key_2) doMacro2();
    //if (e->key() == Qt::Key_3) doMacro3();
}
