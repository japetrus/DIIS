/******************************************************************************
AppWindow class
******************************************************************************/
#include "AppWindow.h"

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   Constructor / Destructor
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ****************************************************************************
 * AppWindow
 * Constructor
 * ***************************************************************************/
AppWindow::AppWindow(AppConfig &_appConfig)
{
    appConfig = &_appConfig;
    connect(appConfig, SIGNAL(valuesChanged()), this, SLOT(updateActiveGeometryElements()));


    setWindowIcon(QIcon(":/icons/icon.icns"));

    log = new LogWidget(NULL);

    /* Setup the film widget */
    gandolfiFilm = new FilmWidget(this, statusBar(), log, *appConfig);

    /* Setup the scrollbar widget and add the film */
    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(gandolfiFilm);
    setCentralWidget(scrollArea);

    /* A few final things... */
    setWindowTitle(tr("DIIS"));
    resize(815,500);
    statusBar()->showMessage("Open a film to begin.");

    /* Determine supported image formats...*/
    QList<QByteArray> formats = QImageReader::supportedImageFormats ();
    QString sformats = "Supported formats: ";
    for (int i = 0; i < formats.size(); i++)
    {
        sformats.append(formats.at(i).constData());
        if (i != formats.size()-1) sformats.append(", ");
    }
    log->addMessage(sformats);

    geoDialog = new QDialog(this);
    UiGeo.setupUi(geoDialog);
    updateActiveGeometryElements();

    imageDialog = new QDialog(this);
    UiImage.setupUi(imageDialog);

    connect(UiImage.hsBrightness, SIGNAL(valueChanged(int)), this, SLOT(updateImageDialog()));

    connect(UiImage.hsGamma, SIGNAL(valueChanged(int)), this, SLOT(updateImageDialog()));
    connect(UiImage.buttonBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()), this, SLOT(updateImage()));

    connect(UiGeo.buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()), this, SLOT(resetGeometry()));
    connect(UiGeo.buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(applyGeometry()));
    if (appConfig->getOptIndex() == SymmetryOpt)
    {

        connect(UiGeo.sb0CenterY, SIGNAL(valueChanged(int)), this, SLOT(updateOthersFromCenters()));
        connect(UiGeo.sb180CenterY, SIGNAL(valueChanged(int)), this, SLOT(updateOthersFromCenters()));
        connect(UiGeo.sb0CenterX, SIGNAL(valueChanged(int)), this, SLOT(updateOthersFromCenters()));
        connect(UiGeo.sb180CenterX, SIGNAL(valueChanged(int)), this, SLOT(updateOthersFromCenters()));
    } else
    {
        connect(UiGeo.sbRadius, SIGNAL(valueChanged(double)), this, SLOT(updateCentersFromOthers()));
        connect(UiGeo.sbAlpha, SIGNAL(valueChanged(double)), this, SLOT(updateCentersFromOthers()));
    }
    connect(gandolfiFilm, SIGNAL(geometryUpdated()), this, SLOT(updateGeometryValues()));
    connect(UiGeo.btnApplyRotation, SIGNAL(clicked()), this, SLOT(applyAlphaRotation()));

    geoDockWidget = new QDockWidget("Geometry", this);
    geoDockWidget->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    geoDockWidget->setWidget(geoDialog);
    geoDockWidget->setFixedSize(225,510);
    this->addDockWidget(Qt::RightDockWidgetArea, geoDockWidget);

    logDockWidget = new QDockWidget("Log", this);
    logDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    logDockWidget->setWidget(log);
    logDockWidget->setMinimumHeight(100);
    this->addDockWidget(Qt::BottomDockWidgetArea, logDockWidget);
    logDockWidget->hide();

    /* Create menus and buttons for main window */
    createActions();
    createMenus();
    createToolBar();
    updateActions();
}

/* ****************************************************************************
 * ~AppWindow
 * Destructor
 * ***************************************************************************/
AppWindow::~AppWindow()
{
    delete gandolfiFilm;
    delete scrollArea;
    delete log;
}

void AppWindow::updateActiveGeometryElements()
{
    if (appConfig->getOptIndex() == SymmetryOpt)
    {
        UiGeo.cbOpt0Center->setEnabled(true);
        UiGeo.cbOpt180Center->setEnabled(true);
        UiGeo.cbOptAlpha->setEnabled(true);
        UiGeo.cbOptRadius->setEnabled(true);
        UiGeo.cbOptPhi->setEnabled(false);
    } else
    {
        UiGeo.cbOpt0Center->setEnabled(true);
        UiGeo.cbOpt180Center->setEnabled(false);
        UiGeo.cbOptAlpha->setEnabled(true);
        UiGeo.cbOptRadius->setEnabled(true);
        UiGeo.cbOptPhi->setEnabled(true);
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   Action Handlers
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ****************************************************************************
 * openFilm
 * Gets a film filename from the user and calls the GandolfiFilm::setFilm
 * procedure.  Also does a few other initialization things.
 * ***************************************************************************/
void AppWindow::openFilm()
{
    QDir p = QDir(QString::fromStdString(appConfig->getLastPath()));

    /* Get the film image filename from the user */
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Film"), QString::fromStdString(appConfig->getLastPath()));
    log->addMessage("Trying to load:\n " + fileName);

    /* If a file is selected -- open it */
    if (!fileName.isEmpty())
    {
        /* Save the file's path as the last used path */
        appConfig->setLastPath(fileName.section('/', 0, -2).toStdString());
        appConfig->saveConfig();

        this->setWindowTitle("DIIS - " + fileName);

        /* Tell the film widget to load the film */
        gandolfiFilm->setFilm(fileName);
        gandolfiFilm->setScaleFactor(1.0);


        /* Change menus/actions to reflect a film being loaded */        
        fitToWindowAct->setEnabled(true);
        updateActions();
        scaleImage(1.0);        
    }
}

/* ****************************************************************************
 * showPrefs
 * Shows the preferences window
 * ***************************************************************************/
void AppWindow::showPrefs() { appConfig->showPrefs(); }

/* ****************************************************************************
 * zoomIn
 * Scales the image to 125% size
 * ***************************************************************************/
void AppWindow::zoomIn()
{
    scaleImage(1.25);
    log->addMessage("Zoomed in (125%).");
}

/* ****************************************************************************
 * zoomOut
 * Scales the image to 75% size
 * ***************************************************************************/
void AppWindow::zoomOut()
{
    scaleImage(0.75);
    log->addMessage("Zoomed out (75%).");
}

/* ****************************************************************************
 * normalSize
 * Scales the image to 100% of the original size
 * ***************************************************************************/
void AppWindow::normalSize()
{
    gandolfiFilm->adjustSize();
    gandolfiFilm->setScaleFactor(1.0);
    log->addMessage("Zoomed to full size (100%).");
}

/* ****************************************************************************
 * fitToWindow
 * Scales the image such that it fits the current window size
 * ***************************************************************************/
void AppWindow::fitToWindow()
{
    //    bool fitToWindow = fitToWindowAct->isChecked();
    //    scrollArea->setWidgetResizable(fitToWindow);
    //    if (!fitToWindow) {
    //        normalSize();
    //    }
    //    updateActions();

    Q_ASSERT(gandolfiFilm->pixmap());

    gandolfiFilm->setScaleFactor(this->width()/gandolfiFilm->width());
    gandolfiFilm->resize(gandolfiFilm->getScaleFactor() * gandolfiFilm->pixmap()->size());

    //adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    // adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(gandolfiFilm->getScaleFactor() < 10.0);
    zoomOutAct->setEnabled(gandolfiFilm->getScaleFactor() > 0.05);

    QDesktopWidget *qdw = QApplication::desktop();
    QRect sdim = qdw->availableGeometry();

    if (gandolfiFilm->width() < sdim.width() )
        resize(gandolfiFilm->width()+20, sdim.height()-20);
    else
        resize(sdim.width()+20, sdim.height()-20);
}

/* ****************************************************************************
 * integrate
 * Integrates the film using the preferred step size
 * ***************************************************************************/
void AppWindow::integrate()
{
    if (gandolfiFilm->getTwoThetaPlot()->isHidden()) showTwoThetaWindowAct->toggle();

    gandolfiFilm->integrate(appConfig->getIntStepSize());

    updateActions();
}

/* ****************************************************************************
 * changeDPI
 * Changes the DPI (or DPM) of the image by scaling the film image
 * ***************************************************************************/
void AppWindow::changeDPI()
{
    QImage film = gandolfiFilm->pixmap()->toImage();
    int currentWidth = film.width();
    int currentDPI = gandolfiFilm->getDPI();

    bool ok;
    int newDPI = QInputDialog::getInt(this, tr("Change DPI"), tr("New DPI:"), currentDPI, 100, 5000, 25, &ok);

    if (ok) {
        double ratio = (double)newDPI/(double)currentDPI;
        int newWidth = int(ratio*currentWidth);

        gandolfiFilm->setPixmap(QPixmap::fromImage(film.scaledToWidth(newWidth, Qt::SmoothTransformation)));
        gandolfiFilm->setData(film.scaledToWidth(newWidth, Qt::SmoothTransformation));
        gandolfiFilm->setDPM(newDPI/0.0254);

        log->addMessage(tr("[GandolfiFilm] Old DPI %1, new DPI %2.").arg(QString::number(currentDPI), QString::number(newDPI)));
    }
}

/* ****************************************************************************
 * crop
 * Initializes the cropping procedure
 * ***************************************************************************/
void AppWindow::crop() { gandolfiFilm->startCrop(); }

/* ****************************************************************************
 * deskew
 * Initializes the deskewing procedure
 * ***************************************************************************/
void AppWindow::deskew() { gandolfiFilm->startDeskew(); }

/* ****************************************************************************
 * optimize
 * Uses the preferences to guide the optimization process
 * ***************************************************************************/
void AppWindow::optimize()
{
    int optSteps = (UiGeo.cbOpt0Center->isChecked()*2 + UiGeo.cbOpt180Center->isChecked()*2 + UiGeo.cbOptAlpha->isChecked() + UiGeo.cbOptPhi->isChecked() + UiGeo.cbOptRadius->isChecked());
    QProgressDialog progress("Performing optimization", "Abort", 0, optSteps, this);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.setMinimumDuration(0.0);
    progress.setValue(1);

    bool sharpOpt;

    if (appConfig->getOptIndex() == SharpnessOpt)
        sharpOpt = true;
    else
        sharpOpt = false;

    if (UiGeo.cbOpt0Center->isChecked())
    {
        log->addMessage("Optimzing 0 degree center...");
        progress.setWindowTitle(tr("Iteration: %1").arg(1));


        if (sharpOpt)
        {
            QPoint lastPoint = gandolfiFilm->getGeometry()->zeroDegreeCenter;
            QPoint newPoint;
            int newX = gandolfiFilm->optimizeCenterXSharpness(ZERO_DEGREES).x();
            progress.setValue(progress.value() + 1);
            cout << "Calling y opt routine..." << endl;
            int newY = gandolfiFilm->optimizeCenterYSharpness(ZERO_DEGREES).y();
            progress.setValue(progress.value() + 1);
            newPoint = QPoint(newX, newY);

            while (newPoint != lastPoint && appConfig->getUntilNoChange())
            {
                progress.setValue(progress.value() - 2);
                newX = gandolfiFilm->optimizeCenterXSharpness(ZERO_DEGREES).x();
                progress.setValue(progress.value() + 1);
                newY = gandolfiFilm->optimizeCenterYSharpness(ZERO_DEGREES).y();
                progress.setValue(progress.value() + 1);
                lastPoint = newPoint;
                newPoint = QPoint(newX, newY);
            }
        } else
        {
            QPoint lastPoint = gandolfiFilm->getGeometry()->zeroDegreeCenter;
            QPoint newPoint;
            int newX = gandolfiFilm->optimizeCenterXSymmetry(ZERO_DEGREES).x();
            int newY = gandolfiFilm->optimizeCenterYSymmetry(ZERO_DEGREES).y();
            newPoint = QPoint(newX, newY);

            while (newPoint != lastPoint && appConfig->getUntilNoChange())
            {
                newX = gandolfiFilm->optimizeCenterXSymmetry(ZERO_DEGREES).x();
                newY = gandolfiFilm->optimizeCenterYSymmetry(ZERO_DEGREES).y();
                lastPoint = newPoint;
                newPoint = QPoint(newX, newY);
            }

        }


    }

    if (UiGeo.cbOpt180Center->isChecked())
    {
        log->addMessage("Optimizing 180 degree center...");
        if (sharpOpt)
        {
            QPoint lastPoint = gandolfiFilm->getGeometry()->oneEightyDegreeCenter;
            QPoint newPoint;
            int newX = gandolfiFilm->optimizeCenterXSharpness(ONEEIGHTY_DEGREES).x();
            int newY = gandolfiFilm->optimizeCenterYSharpness(ONEEIGHTY_DEGREES).y();
            newPoint = QPoint(newX, newY);

            while (newPoint != lastPoint && appConfig->getUntilNoChange())
            {
                newX = gandolfiFilm->optimizeCenterXSharpness(ONEEIGHTY_DEGREES).x();
                newY = gandolfiFilm->optimizeCenterYSharpness(ONEEIGHTY_DEGREES).y();
                lastPoint = newPoint;
                newPoint = QPoint(newX, newY);
            }
        } else
        {
            QPoint lastPoint = gandolfiFilm->getGeometry()->oneEightyDegreeCenter;
            QPoint newPoint;
            int newX = gandolfiFilm->optimizeCenterXSymmetry(ONEEIGHTY_DEGREES).x();
            int newY = gandolfiFilm->optimizeCenterYSymmetry(ONEEIGHTY_DEGREES).y();
            newPoint = QPoint(newX, newY);

            while (newPoint != lastPoint && appConfig->getUntilNoChange())
            {
                newX = gandolfiFilm->optimizeCenterXSymmetry(ONEEIGHTY_DEGREES).x();
                newY = gandolfiFilm->optimizeCenterYSymmetry(ONEEIGHTY_DEGREES).y();
                lastPoint = newPoint;
                newPoint = QPoint(newX, newY);
            }
        }
        progress.setValue(progress.value() + 1);

    }

    if (UiGeo.cbOptAlpha->isChecked())
    {
        log->addMessage("Optimizing alpha rotation...");
        if (sharpOpt)
            gandolfiFilm->optimizeAlphaSharpness();
        else
            gandolfiFilm->optimizeRotationSymmetry();
        progress.setValue(progress.value() + 1);

    }

    if (UiGeo.cbOptRadius->isChecked())
    {
        log->addMessage("Optimizing camera radius...");
        if (sharpOpt)
            gandolfiFilm->optimizeRadiusSharpness();
        else
            gandolfiFilm->optimizeRadiusSymmetry();

        progress.setValue(progress.value() + 1);

    }



    if (UiGeo.cbOptPhi->isChecked())
    {
        log->addMessage("Optimizing phi rotation...");
        gandolfiFilm->optimizePhiSharpness();
        progress.setValue(progress.value() + 1);

    }

    progress.setValue(optSteps);
    this->updateGeometryValues();
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   Utility Methods
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ****************************************************************************
 * createActions
 * Create GUI actions
 * ***************************************************************************/
void AppWindow::createActions()
{
    /* File menu actions */
    openAct = new QAction(tr("&Open"), this);
    openAct->setShortcut(tr("Ctrl+O"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFilm()));

    saveTIFFAct = new QAction(tr("Save &TIFF as..."), this);
    connect(saveTIFFAct, SIGNAL(triggered()), gandolfiFilm, SLOT(saveTIFF()));

    saveUDFAct = new QAction(tr("Save &UDF as..."), this);
    saveUDFAct->setShortcut(tr("Ctrl+S"));
    connect(saveUDFAct, SIGNAL(triggered()), gandolfiFilm, SLOT(saveUDF()));

    saveCSVAct = new QAction(tr("Save &CSV as..."), this);
    connect(saveCSVAct, SIGNAL(triggered()), gandolfiFilm, SLOT(saveCSV()));

    closeAct = new QAction(tr("Close"), this);
    closeAct->setShortcut(tr("Ctrl+W"));
    connect(closeAct, SIGNAL(triggered()), gandolfiFilm, SLOT(closeFilm()));

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    prefsAct = new QAction(tr("Preferences"), this);
    connect(prefsAct, SIGNAL(triggered()), this, SLOT(showPrefs()));

    // View menu actions...

    zoomInAct = new QAction(tr("Zoom &In (25%)"), this);
    zoomInAct->setShortcut(tr("Ctrl+z"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(tr("Zoom &Out (25%)"), this);
    zoomOutAct->setShortcut(tr("Shift+z"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    normalSizeAct = new QAction(tr("&Normal Size"), this);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);
    connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));
    connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

    // Tools menu actions...

    cropAct = new QAction(tr("Crop"), this);
    connect(cropAct, SIGNAL(triggered()), this, SLOT(crop()));

    deskewAct = new QAction(tr("Deskew"), this);
    connect(deskewAct, SIGNAL(triggered()), this, SLOT(deskew()));

    invertAct = new QAction(tr("In&vert"), this);
    connect(invertAct, SIGNAL(triggered()), gandolfiFilm, SLOT(invert()));

    imageAct = new QAction(tr("Brightness"), this);
    connect(imageAct, SIGNAL(triggered()), this, SLOT(showImageDialog()));

    rotateAct = new QAction(tr("&Rotate"), this);
    connect(rotateAct, SIGNAL(triggered()), gandolfiFilm, SLOT(rotate()));

    dpiAct = new QAction(tr("&Change DPI"), this);
    connect(dpiAct, SIGNAL(triggered()), this, SLOT(changeDPI()));

    optimizeAct = new QAction(tr("&Optimize"), this);
    connect(optimizeAct, SIGNAL(triggered()), this, SLOT(optimize()));

    integrateAct = new QAction(tr("&Integrate"), this);
    connect(integrateAct, SIGNAL(triggered()), this, SLOT(integrate()));

    // Window menu actions...
    showAppWindowAct = new QAction(tr("Main Window"), this);
    connect(showAppWindowAct, SIGNAL(triggered()), this, SLOT(show()));

    showTwoThetaWindowAct = new QAction(tr("Two Theta Plot"), this);
    //showTwoThetaWindowAct->setCheckable(true);
    connect(showTwoThetaWindowAct, SIGNAL(triggered()), this, SLOT(showTwoThetaWindow()));
    //connect(gandolfiFilm->getTwoThetaPlot(), SIGNAL(), showTwoThetaWindowAct, SLOT(toggle()));

    showLogDockAct = logDockWidget->toggleViewAction();
    //connect(showLogWindowAct, SIGNAL(toggled(bool)), this, SLOT(toggleLogWindow(bool)));
    //connect(log, SIGNAL(rejected()), showLogWindowAct, SLOT(toggle()));

    showAboutWindowAct = new QAction(tr("About"), this);
    connect(showAboutWindowAct, SIGNAL(triggered()), this, SLOT(showAboutWindow()));

}

/* ****************************************************************************
 * createMenus
 * Create GUI menus
 * ***************************************************************************/
void AppWindow::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveUDFAct);
    fileMenu->addAction(saveCSVAct);
    fileMenu->addAction(saveTIFFAct);
    fileMenu->addAction(prefsAct);
    fileMenu->addSeparator();
    fileMenu->addAction(closeAct);
    fileMenu->addAction(exitAct);

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);

    QMenu *imageMenu = new QMenu(tr("Image"), this);
    imageMenu->addAction(imageAct);
    imageMenu->addAction(invertAct);
    imageMenu->addSeparator();
    imageMenu->addAction(cropAct);
    imageMenu->addAction(deskewAct);
    imageMenu->addAction(rotateAct);
    imageMenu->addSeparator();
    imageMenu->addAction(dpiAct);

    toolsMenu = new QMenu(tr("&Tools"), this);
    toolsMenu->addAction(optimizeAct);
    toolsMenu->addAction(integrateAct);

    windowMenu = new QMenu(tr("&Window"), this);
    //windowMenu->addAction(showAppWindowAct);
    windowMenu->addAction(geoDockWidget->toggleViewAction());
    windowMenu->addAction(showTwoThetaWindowAct);
    windowMenu->addAction(showLogDockAct);
    windowMenu->addAction(showAboutWindowAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(imageMenu);
    menuBar()->addMenu(toolsMenu);
    menuBar()->addMenu(windowMenu);
}

/* ****************************************************************************
 * createToolBar
 * Create GUI toolbar
 * ***************************************************************************/
void AppWindow::createToolBar()
{
    toolbar = new QToolBar(this);
    toolbar->setFloatable(false);
    toolbar->setMovable(false);

    QToolButton *btnOpenFilm = new QToolButton(toolbar);
    btnOpenFilm->setText("Open");
    btnOpenFilm->setShortcut(tr("Ctrl+o"));
    btnOpenFilm->setToolTip("Open film [Ctrl+O]");
    btnOpenFilm->setIcon(QIcon(":/icons/Resources/open.png"));
    btnOpenFilm->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnOpenFilm);
    connect(btnOpenFilm, SIGNAL(clicked()), this, SLOT(openFilm()));

    QToolButton *btnSaveTIFF = new QToolButton(toolbar);
    btnSaveTIFF->setText("Save TIFF");
    btnSaveTIFF->setShortcut(tr("S"));
    btnSaveTIFF->setToolTip(tr("Save film as TIFF [S]"));
    btnSaveTIFF->setToolTip("Save film image");
    btnSaveTIFF->setIcon( QIcon(":/icons/Resources/savetiff.png") );
    btnSaveTIFF->setToolButtonStyle(Qt::ToolButtonIconOnly);	
    toolbar->addWidget(btnSaveTIFF);
    connect(btnSaveTIFF, SIGNAL(clicked()), gandolfiFilm, SLOT(saveTIFF()));

    toolbar->addSeparator();

    QToolButton *btnShowPrefs = new QToolButton(toolbar);
    btnShowPrefs->setText("Preferences");
    btnShowPrefs->setShortcut(tr("P"));
    btnShowPrefs->setToolTip(tr("Show preferences window [P]"));
    btnShowPrefs->setIcon( QIcon(":/icons/Resources/prefs.png") );
    btnShowPrefs->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addWidget(btnShowPrefs);
    connect(btnShowPrefs, SIGNAL(clicked()), this, SLOT(showPrefs()));

    toolbar->addSeparator();

    btnResize = new QToolButton(toolbar);
    btnResize->setText("Resize");
    btnResize->setShortcut(tr("R"));
    btnResize->setToolTip(tr("Resize optimization regions [R]"));
    btnResize->setCheckable(true);
    btnResize->setIcon(QIcon(":/icons/Resources/resize.png"));
    btnResize->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnResize);
    connect(btnResize, SIGNAL(toggled(bool)), this, SLOT(toggleResize(bool)));
    resizeActive = false;
    btnResize->setChecked(false);

    btnMove = new QToolButton(toolbar);
    btnMove->setText("Move");
    btnMove->setShortcut(tr("M"));
    btnMove->setToolTip(tr("Move optimization regions [M]"));
    btnMove->setIcon(QIcon(":/icons/Resources/move.png"));
    btnMove->setCheckable(true);
    btnMove->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnMove);
    connect(btnMove, SIGNAL(toggled(bool)), this, SLOT(toggleMove(bool)));
    moveActive = false;
    btnMove->setChecked(false);

    btnExclude = new QToolButton(toolbar);
    btnExclude->setText("Exclude");
    btnExclude->setShortcut(tr("E"));
    btnExclude->setToolTip(tr("Exclude integration regions [E]"));
    btnExclude->setIcon(QIcon(":/icons/Resources/exclude.png"));
    btnExclude->setCheckable(true);
    btnExclude->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnExclude);
    connect(btnExclude, SIGNAL(toggled(bool)), this, SLOT(toggleExclude(bool)));
    excludeActive = false;
    btnExclude->setChecked(false);

    toolbar->addSeparator();

    QToolButton *btnOpt = new QToolButton(toolbar);
    btnOpt->setText("Optimize");
    btnOpt->setIcon( QIcon(":/icons/Resources/opt.png") );
    btnOpt->setShortcut(tr("O"));
    btnOpt->setToolTip(tr("Optimize the 2theta = 0 position [O]"));
    btnOpt->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addWidget(btnOpt);
    connect(btnOpt, SIGNAL(clicked()), this, SLOT(optimize()));

    QToolButton *btnInt = new QToolButton(toolbar);
    btnInt->setText("Integrate");
    btnInt->setIcon( QIcon(":/icons/Resources/int.png") );
    btnInt->setShortcut(tr("I"));
    btnInt->setToolTip(tr("Integrate the film [I]"));
    btnInt->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addWidget(btnInt);
    connect(btnInt, SIGNAL(clicked()), this, SLOT(integrate()));

    toolbar->addSeparator();
/*
    btnRegion = new QToolButton(toolbar);
    btnRegion->setIcon( QIcon(":/icons/Resources/region.png"));
    btnRegion->setText("Upper");
    btnRegion->setShortcut(tr("Space"));
    btnRegion->setToolTip(tr("Switch between upper and lower region [Space]"));
    btnRegion->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    toolbar->addWidget(btnRegion);
    connect(btnRegion, SIGNAL(clicked()), this, SLOT(toggleRegion()));
*/
    QLabel *spacerLabel = new QLabel("");
    spacerLabel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    toolbar->addWidget(spacerLabel);

    QToolButton *btnLog = new QToolButton(toolbar);
    QAction *logAct = logDockWidget->toggleViewAction();
    logAct->setIcon(QIcon(":/icons/Resources/log.png"));
    logAct->setText("Log");
    btnLog->setDefaultAction(logAct);
    btnLog->setCheckable(true);
    btnLog->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnLog);

    QToolButton *btnGeometry = new QToolButton(toolbar);
    QAction *geoAct = geoDockWidget->toggleViewAction();
    geoAct->setIcon(QIcon(":/icons/Resources/geometry.png"));
    geoAct->setText("Geometry");
    btnGeometry->setDefaultAction(geoAct);
    btnGeometry->setCheckable(true);
    btnGeometry->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->addWidget(btnGeometry);


    addToolBar(Qt::TopToolBarArea, toolbar);
}

void AppWindow::toggleExclude(bool b)
{
    if (b)
    {
        excludeActive = true;
        resizeActive = false;
        moveActive = false;
        btnMove->setChecked(false);
        btnResize->setChecked(false);
    }
    else
    {
        excludeActive = false;
        btnExclude->setChecked(false);
    }

    gandolfiFilm->setMoveActive(moveActive);
    gandolfiFilm->setResizeActive(resizeActive);
    gandolfiFilm->setExcludeActive(excludeActive);
}

void AppWindow::toggleMove(bool b)
{
    if (b)
    {
        moveActive = true;
        resizeActive = false;
        excludeActive = false;
        btnResize->setChecked(false);
        btnExclude->setChecked(false);
    }
    else
    {
        moveActive = false;
        btnMove->setChecked(false);
    }

    gandolfiFilm->setMoveActive(moveActive);
    gandolfiFilm->setResizeActive(resizeActive);
    gandolfiFilm->setExcludeActive(excludeActive);
}

void AppWindow::toggleResize(bool b)
{
    if (b)
    {
        moveActive = false;
        excludeActive = false;
        resizeActive = true;
        btnMove->setChecked(false);
        btnExclude->setChecked(false);
    }
    else
    {
        resizeActive = false;
        btnResize->setChecked(false);
    }

    gandolfiFilm->setMoveActive(moveActive);
    gandolfiFilm->setResizeActive(resizeActive);
    gandolfiFilm->setExcludeActive(excludeActive);

}

void AppWindow::showLogWindow()
{
    log->show();
}

void AppWindow::showTwoThetaWindow()
{
    gandolfiFilm->getTwoThetaPlot()->show();
}

void AppWindow::showAboutWindow()
{
    QDialog *msgBox = new QDialog(this, Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint | Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    msgBox->setWindowTitle("About");
    QString msg = "";
    msg.append(tr("<h3>Gandolfi</h3>"));
    msg.append(tr("<b>Author:</b> J.A. Petrus<br>"));
    msg.append(tr("<b>Suggestions:</b> K.C. Ross and A.M. McDonald<br>"));
    msg.append(tr("<b>Version:</b> %1<br>").arg(VERSION));
    msg.append(tr("<b>Libraries used:</b> <a href=\"http://qt.nokia.com\">Qt4</a>, <a href=\"http://qwt.sourceforge.net\">QwtPlot</a>, <a href=\"http://www-personal.umich.edu/~wagnerr/ConfigFile.html\">ConfigFile</a>"));

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *lbl = new QLabel;
    lbl->setTextFormat(Qt::RichText);
    lbl->setText(msg);
    lbl->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    layout->addWidget(lbl);
    msgBox->setLayout(layout);

    msgBox->exec();
}

void AppWindow::showGeometry()
{
    updateGeometryValues();
    geoDialog->show();
}

void AppWindow::resetGeometry()
{
    log->addMessage("[Main] Resetting geometry");

    Geometry *pgeo = gandolfiFilm->getPreviousGeometry();
    Geometry *geo = gandolfiFilm->getGeometry();

    *geo = *pgeo;

    updateGeometryValues();
    gandolfiFilm->updateGeometry();
}

void AppWindow::applyGeometry()
{
    log->addMessage("[Main] Applying geometry");


    Geometry *previousGeometry = gandolfiFilm->getPreviousGeometry();
    Geometry *newGeometry = gandolfiFilm->getGeometry();

    // Copy previous geometry so that it can be recalled
    *previousGeometry = *newGeometry;

    // 0 deg center
    newGeometry->zeroDegreeCenter = QPoint(UiGeo.sb0CenterX->value(), UiGeo.sb0CenterY->value());

    // 180 deg center
    newGeometry->oneEightyDegreeCenter = QPoint(UiGeo.sb180CenterX->value(), UiGeo.sb180CenterY->value());

    // Phi
    newGeometry->phi = UiGeo.sbPhi->value();

    // Alpha
    newGeometry->alpha = UiGeo.sbAlpha->value();

    // Radius
    newGeometry->radius = UiGeo.sbRadius->value()/1000.0;

    gandolfiFilm->updateGeometry();
}

void AppWindow::updateImageDialog()
{
    UiImage.lblBrightness->setText(QString::number(pow(2.0,UiImage.hsBrightness->value())));
    UiImage.lblGamma->setText(QString::number(UiImage.hsGamma->value()/10.0));
}

void AppWindow::updateImage()
{
    cout << "Modifying image brightness and stuff." << endl;
    QImage tempFilm = gandolfiFilm->getFilm();

    double b = pow(2.0,UiImage.hsBrightness->value());
    double g = double(UiImage.hsGamma->value())/10.0;

    for (int x = 0; x < tempFilm.width(); x++)
    {
        for (int y = 0; y < tempFilm.height(); y++)
        {
            double gv = qGray(tempFilm.pixel(x,y));
            QRgb pv = tempFilm.pixel(x,y);


            //if (gv*b > 255)
            //    tempFilm.setPixel(x,y, qRgb(255, 255, 255));
            //else
                tempFilm.setPixel(x,y, qRgb(qRed(pv)*b, qGreen(pv)*b, qBlue(pv)*b ));
                 gv = qGray(tempFilm.pixel(x,y));
                 pv = tempFilm.pixel(x,y);
            //if (gv + o > 255)
            //    tempFilm.setPixel(x,y, qRgb(255, 255, 255));
            //else
            //    tempFilm.setPixel(x,y, qRgb(qRed(pv) + o, qGreen(pv) + o, qBlue(pv) + o ));

            tempFilm.setPixel(x,y, qRgb(255.0*pow(gv/255.0,g), 255.0*pow(gv/255.0, g), 255.0*pow(gv/255.0,g)));

        }
    }

    gandolfiFilm->setImage(tempFilm);
    gandolfiFilm->repaint();
}

void AppWindow::updateGeometryValues()
{
    log->addMessage("Updating values...");
    Geometry *geo = gandolfiFilm->getGeometry();
    Geometry *pgeo = gandolfiFilm->getPreviousGeometry();

    UiGeo.sb0CenterX->setValue(geo->zeroDegreeCenter.x());
    UiGeo.sb0CenterY->setValue(geo->zeroDegreeCenter.y());
    UiGeo.sb180CenterX->setValue(geo->oneEightyDegreeCenter.x());
    UiGeo.sb180CenterY->setValue(geo->oneEightyDegreeCenter.y());
    UiGeo.sbPhi->setValue(geo->phi);
    UiGeo.sbAlpha->setValue(geo->alpha);
    UiGeo.sbRadius->setValue(geo->radius*1000.0);
    UiGeo.labelCurrentSharpness->setText(QString::number(geo->sharpness));
    UiGeo.labelPreviousSharpness->setText(QString::number(pgeo->sharpness));

    if (appConfig->getOptIndex() == SymmetryOpt)
    {
        updateOthersFromCenters();
    }

    if (geo->sharpness > pgeo->sharpness)
    {
        UiGeo.labelCurrentSharpness->setText(QString("<b><font color=green>") + QString::number(geo->sharpness, 'f', 5) + QString("</font></b>"));
        UiGeo.labelPreviousSharpness->setText(QString("<b><font color=red>") + QString::number(pgeo->sharpness, 'f', 5) + QString("</font></b>"));
    } else if (geo->sharpness < pgeo->sharpness)
    {
        UiGeo.labelCurrentSharpness->setText(QString("<b><font color=red>") + QString::number(geo->sharpness, 'f', 5) + QString("</font></b>"));
        UiGeo.labelPreviousSharpness->setText(QString("<b><font color=green>") + QString::number(pgeo->sharpness, 'f', 5) + QString("</font></b>"));
    } else
    {
        UiGeo.labelCurrentSharpness->setText(QString("<b><font color=black>") + QString::number(geo->sharpness, 'f', 5) + QString("</font></b>"));
        UiGeo.labelPreviousSharpness->setText(QString("<b><font color=black>") + QString::number(pgeo->sharpness, 'f', 5) + QString("</font></b>"));
    }

    gandolfiFilm->updateIntArea();
}


void AppWindow::updateCentersFromOthers()
{
    double R = UiGeo.sbRadius->value()/1000.0;
    double a = UiGeo.sbAlpha->value();

    int x1 = UiGeo.sb0CenterX->value();
    int y1 = UiGeo.sb0CenterY->value();

    int x2 = x1-sin(a*M_PI/180.0)*M_PI*R*gandolfiFilm->getDPM();
    UiGeo.sb180CenterX->setValue(x2);

    int y2 = y1 + cos(a*M_PI/180.0)*M_PI*R*gandolfiFilm->getDPM();
    UiGeo.sb180CenterY->setValue(y2);

}

void AppWindow::updateOthersFromCenters()
{
    int x1 = UiGeo.sb0CenterX->value();
    int x2 = UiGeo.sb180CenterX->value();
    int y1 = UiGeo.sb0CenterY->value();
    int y2 = UiGeo.sb180CenterY->value();

    double R = (1.0/M_PI)*sqrt( abs(x1-x2)*abs(x1-x2) + abs(y1-y2)*abs(y1-y2) )/gandolfiFilm->getDPM();
    UiGeo.sbRadius->setValue(R*1000.0);

    double a = asin( double(x1-x2)/(M_PI*R*gandolfiFilm->getDPM()))*180.0/M_PI;
    UiGeo.sbAlpha->setValue(a);
}

/* ****************************************************************************
 * updateActions
 * Change the availability of certain actions depending on program state
 * ***************************************************************************/
void AppWindow::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());

    if (gandolfiFilm->isIntegrated())
    {
        saveUDFAct->setEnabled(true);
        saveCSVAct->setEnabled(true);
    } else
    {
        saveUDFAct->setEnabled(false);
        saveCSVAct->setEnabled(false);
    }
}

/* ****************************************************************************
 * scaleImage
 * Scale the image by <factor>
 * ***************************************************************************/
void AppWindow::scaleImage(double factor)
{
    int w,h;
    h = this->height();

    Q_ASSERT(gandolfiFilm->pixmap());
    gandolfiFilm->setScaleFactor(gandolfiFilm->getScaleFactor()*factor);
    gandolfiFilm->resize(gandolfiFilm->getScaleFactor() * gandolfiFilm->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(gandolfiFilm->getScaleFactor() < 10.0);
    zoomOutAct->setEnabled(gandolfiFilm->getScaleFactor() > 0.05);

    QDesktopWidget *qdw = QApplication::desktop();
    QRect sdim = qdw->availableGeometry();

    if (gandolfiFilm->width() < sdim.width())
        w = gandolfiFilm->width()+ 20;
    else
        w = sdim.width() + 20;

    if (!geoDialog->isHidden())
        w = w + 225;

    resize(w,h);
    //this->setMaximumWidth(w);
    this->setMinimumWidth(750);
}

/* ****************************************************************************
 * adjustScrollBar
 * Not sure why this is used... ???
 * ***************************************************************************/
void AppWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   Event Handlers
   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

/* ****************************************************************************
 * keyPressEvent
 * Tells the film when Alt, Shift, or Ctrl is pressed
 * ***************************************************************************/
void AppWindow::keyPressEvent(QKeyEvent* event)
{
    QMainWindow::keyPressEvent(event);
    if (event->key() == Qt::Key_Alt)
    {
        gandolfiFilm->setAltDown(true);
        statusBar()->showMessage("Place a guess by clicking in the film.");
    }
}

/* ****************************************************************************
 * keyReleaseEvent
 * Tells the film when Alt, Shift, or Ctrl is released
 * ***************************************************************************/
void AppWindow::keyReleaseEvent(QKeyEvent* event)
{
    QMainWindow::keyReleaseEvent(event);
    if (event->key() == Qt::Key_Alt) { gandolfiFilm->setAltDown(false); }
}

