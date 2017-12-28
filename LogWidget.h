#ifndef LogWidget_H
#define LogWidget_H

#include <QMainWindow>
#include <QLabel>
#include <QPrinter>
#include <QtGui>
#include <QRect>
#include <QDesktopWidget>
#include <QMatrix>
#include <QToolBar>
#include <math.h>
#include <iostream>
#include <fstream>
using namespace std;

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE

class LogWidgetWidget : public QListWidget
{
    Q_OBJECT
public:
    LogWidgetWidget(QWidget *parent) : QListWidget(parent)
    {
        this->setSelectionMode(QAbstractItemView::NoSelection);
        this->setWordWrap(true);
        cout << "[LogWidgetWidget] Created." << endl;
    }

public slots:
    void saveLog()
    {
        QString logFilename = QFileDialog::getSaveFileName(this, tr("Save log file as..."), QDir::currentPath());

        ofstream logFile (logFilename.toAscii());
        if (logFile.is_open())
        {
            for (int row = 0; row < this->count(); row++) {
                logFile << this->item(row)->text().toStdString() << endl;
            }
            logFile.close();
        }
        else {
            cout << "Unable to save file..." << endl;
        }
    }

    void clearLog()
    {
        this->clear();
    }

protected:
    void mousePressEvent(QMouseEvent *e)
    {
        if (e->button()==Qt::RightButton)
        {
            QMenu* contextMenu = new QMenu ( this );
            Q_CHECK_PTR ( contextMenu );
            contextMenu->addAction ( "Save" , this , SLOT (saveLog()) );
            contextMenu->addAction ( "Clear" , this , SLOT (clearLog()) );
            contextMenu->popup( QCursor::pos() );
            contextMenu->exec ();
            delete contextMenu;
            contextMenu = 0;
        }
    }
};

class LogWidget : public QDialog
{
    Q_OBJECT

public:
    LogWidget(QWidget *parent) : QDialog(parent)
    {
        listWidget = new LogWidgetWidget(this);
        listWidget->autoScrollMargin();
        this->resize(300,500);
        this->setWindowTitle("Log");

        this->setMouseTracking(true);
    }

    void addMessage(QString msg)
    {
        new QListWidgetItem(msg, listWidget);
        cout << msg.toStdString() << endl;
        listWidget->setCurrentRow(listWidget->count()-1);
    }

protected:
    void resizeEvent ( QResizeEvent * e)
    {
        listWidget->resize(e->size());
    }

private:
    LogWidgetWidget *listWidget;
};
#endif // LogWidget_H
