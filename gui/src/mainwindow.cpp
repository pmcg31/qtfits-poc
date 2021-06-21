#include <QApplication>

#include "mainwindow.h"
#include "fitsimage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      mainPane(),
      layout(&mainPane),
      label(tr("Whale shit!")),
      fitsWidget(),
      onIcon(":/icon/stretch-icon.png"),
      offIcon(":/icon/stretch-icon-off.png"),
      stretchBtn(offIcon, ""),
      showingStretched(false)
{
    stretchBtn.setStyleSheet("QPushButton{border: none;border-radius: 7px;background-color:rgba(255, 255, 255,32);}");
    stretchBtn.setIconSize(QSize(20, 20));
    stretchBtn.setMinimumSize(30, 30);
    stretchBtn.setMaximumSize(30, 30);
    stretchBtn.setCheckable(true);

    layout.addWidget(&label);
    layout.addWidget(&fitsWidget);
    layout.addWidget(&stretchBtn);

    setCentralWidget(&mainPane);

    QObject::connect(&fitsWidget, &FITSWidget::fileChanged,
                     this, &MainWindow::fitsFileChanged);
    QObject::connect(&fitsWidget, &FITSWidget::fileFailed,
                     this, &MainWindow::fitsFileFailed);
    QObject::connect(&stretchBtn, &QPushButton::toggled,
                     this, &MainWindow::stretchToggled);
    QObject::connect(this, &MainWindow::toggleStretched,
                     &fitsWidget, &FITSWidget::setStretched);

    QStringList args = QApplication::arguments();
    if (args.length() < 2)
    {
        printf("No file specified\n");
        fflush(stdout);
    }
    else
    {
        if (args.length() == 2)
        {
            const char *filename = args.at(1).toLocal8Bit().constData();
            printf("Setting file %s\n", filename);
            fflush(stdout);
            fitsWidget.setFile(filename);
        }
        else
        {
            fprintf(stderr, "Whoa, buddy, file or no file and that's it. What's all this extra shit?\n");
            fflush(stderr);
        }
    }
}

MainWindow::~MainWindow()
{
}

void MainWindow::fitsFileChanged(const char *filename)
{
    printf("File loaded: %s\n", filename);

    const ELS::FITSImage *image = fitsWidget.getImage();
    printf("%s\n", image->getImageType());
    printf("%s\n", image->getSizeAndColor());
    fflush(stdout);
}

void MainWindow::fitsFileFailed(const char *filename,
                                const char *errText)
{
    printf("File failed [%s]: %s\n", errText, filename);
}

void MainWindow::stretchToggled(bool isChecked)
{
    if (showingStretched != isChecked)
    {
        showingStretched = isChecked;

        stretchBtn.setChecked(showingStretched);
        if (showingStretched)
        {
            stretchBtn.setIcon(onIcon);
        }
        else
        {
            stretchBtn.setIcon(offIcon);
        }

        emit toggleStretched(showingStretched);
    }
}
