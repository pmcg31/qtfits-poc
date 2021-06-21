#include <QApplication>

#include "mainwindow.h"
#include "fitsimage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      mainPane(),
      layout(&mainPane),
      fitsWidget(),
      onIcon(":/icon/stretch-icon.png"),
      offIcon(":/icon/stretch-icon-off.png"),
      bottomLayout(),
      stretchBtn(offIcon, ""),
      showingStretched(false),
      currentZoom("--"),
      zoomFitBtn("fit"),
      zoom100Btn("1:1")
{
    const QSize iconSize(20, 20);
    const QSize btnSize(30, 30);
    const char *btnStyle = "QPushButton{border: none;border-radius: 7px;background-color: #444;}";
    stretchBtn.setStyleSheet(btnStyle);
    stretchBtn.setIconSize(iconSize);
    stretchBtn.setMinimumSize(btnSize);
    stretchBtn.setMaximumSize(btnSize);
    stretchBtn.setCheckable(true);

    zoomFitBtn.setEnabled(true);
    zoomFitBtn.setStyleSheet(btnStyle);
    zoomFitBtn.setMinimumSize(btnSize);
    zoomFitBtn.setMaximumSize(btnSize);
    zoom100Btn.setEnabled(true);
    zoom100Btn.setStyleSheet(btnStyle);
    zoom100Btn.setMinimumSize(btnSize);
    zoom100Btn.setMaximumSize(btnSize);

    currentZoom.setStyleSheet("QLabel{border: 1px solid #666;border-radius: 7px;color: #999;}");
    currentZoom.setAlignment(Qt::AlignCenter);
    currentZoom.setMinimumWidth(65);

    bottomLayout.addWidget(&stretchBtn);
    bottomLayout.addStretch(1);
    bottomLayout.addWidget(&zoomFitBtn);
    bottomLayout.addWidget(&zoom100Btn);
    bottomLayout.addWidget(&currentZoom);

    layout.addWidget(&fitsWidget);
    layout.addLayout(&bottomLayout);

    setCentralWidget(&mainPane);

    QObject::connect(&fitsWidget, &FITSWidget::fileChanged,
                     this, &MainWindow::fitsFileChanged);
    QObject::connect(&fitsWidget, &FITSWidget::fileFailed,
                     this, &MainWindow::fitsFileFailed);
    QObject::connect(&fitsWidget, &FITSWidget::actualZoomChanged,
                     this, &MainWindow::fitsZoomChanged);
    QObject::connect(&stretchBtn, &QPushButton::toggled,
                     this, &MainWindow::stretchToggled);
    QObject::connect(this, &MainWindow::toggleStretched,
                     &fitsWidget, &FITSWidget::setStretched);
    QObject::connect(&zoomFitBtn, &QPushButton::clicked,
                     this, &MainWindow::zoomFitClicked);
    QObject::connect(&zoom100Btn, &QPushButton::clicked,
                     this, &MainWindow::zoom100Clicked);

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
            fprintf(stderr, "You confused me with all those arguments\n");
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

void MainWindow::fitsZoomChanged(float zoom)
{
    char tmp[10];

    sprintf(tmp, "%.1f%%", zoom * 100);
    currentZoom.setText(tmp);
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

void MainWindow::zoomFitClicked(bool /* isChecked */)
{
    fitsWidget.setZoom(-1.0);
}

void MainWindow::zoom100Clicked(bool /* isChecked */)
{
    fitsWidget.setZoom(1.0);
}
