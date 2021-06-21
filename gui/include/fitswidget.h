#ifndef FITSWIDGET_H
#define FITSWIDGET_H

#include <QWidget>
#include <QWheelEvent>
#include <QString>
#include <fitsio.h>

#include "fitsimage.h"

class FITSWidget : public QWidget
{
    Q_OBJECT

public:
    class Info
    {
    public:
        int bitDepthEnum;
        char imageType[100];
        int numAxis;
        long axLengths[3];
        int chanAx;
        int width;
        int height;
        long numPixels;
        char sizeAndColor[200];
        long fpixel[3];
        double *imageArray;
        double maxPixelVal;
        double minPixelVal;
    };

public:
    explicit FITSWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    const ELS::FITSImage *getImage() const;

    const char *getFilename() const;
    bool getStretched() const;
    float getZoom() const;

public slots:
    void setFile(const char *filename);
    void setStretched(bool isStretched);
    void setZoom(float zoom);

signals:
    void fileChanged(const char *filename);
    void fileFailed(const char *filename,
                    const char *errText);
    void zoomChanged(float zoom);
    void actualZoomChanged(float zoom);

protected:
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;

    QImage *convertImage() const;

protected:
    enum ZoomAdjustStrategy
    {
        ZAS_CLOSEST,
        ZAS_HIGHER,
        ZAS_LOWER
    };

    void _internalSetZoom(float zoom);

    static float adjustZoom(float desiredZoom,
                            ZoomAdjustStrategy strategy = ZAS_CLOSEST);

private:
    QSizePolicy _sizePolicy;
    const char *_filename;
    ELS::FITSImage *_fits;
    QImage *_cacheImage;
    bool _showStretched;
    float _zoom;
    float _actualZoom;

private:
    static const float g_validZooms[];
};

#endif // FITSWIDGET_H
