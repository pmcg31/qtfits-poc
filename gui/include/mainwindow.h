#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

#include "fitswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void toggleStretched(bool showStretched);

private:
    void fitsFileChanged(const char *filename);
    void fitsFileFailed(const char *filename,
                        const char *errText);
    void fitsZoomChanged(float zoom);

    void stretchToggled(bool isChecked);

private:
    QWidget mainPane;
    QVBoxLayout layout;
    FITSWidget fitsWidget;
    QIcon onIcon;
    QIcon offIcon;
    QHBoxLayout bottomLayout;
    QPushButton stretchBtn;
    bool showingStretched;
    QLabel currentZoom;
};
#endif // MAINWINDOW_H
