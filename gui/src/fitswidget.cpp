#include <QPainter>

#include "fitswidget.h"
#include "fitstantrum.h"
#include "stretch.h"

#define FITS_READ_T double

FITSWidget::FITSWidget(QWidget *parent)
    : QWidget(parent),
      _sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding),
      _filename(0),
      _fits(0),
      _cacheImage(0),
      _showStretched(false),
      _zoom(1.0)
{
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);

    setSizePolicy(_sizePolicy);
}

QSize FITSWidget::sizeHint() const
{
    return QSize(800, 600);
}

QSize FITSWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

const ELS::FITSImage *FITSWidget::getImage() const
{
    return _fits;
}

void FITSWidget::setFile(const char *filename)
{
    if ((_filename == 0) || (strcmp(filename, _filename) != 0))
    {
        try
        {
            ELS::FITSImage *tmpFits = ELS::FITSImage::load(filename);

            if (_fits != 0)
            {
                delete _fits;
            }

            _filename = filename;
            _fits = tmpFits;

            emit fileChanged(_filename);
        }
        catch (ELS::FITSException *e)
        {
            fprintf(stderr, "FITSException: %s for file %s\n", e->getErrText(), filename);
            delete e;
        }
    }
}

void FITSWidget::setStretched(bool isStretched)
{
    if (_showStretched != isStretched)
    {
        _showStretched = isStretched;

        if (_cacheImage != 0)
        {
            delete _cacheImage;
            _cacheImage = 0;
            update();
        }
    }
}

void FITSWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numSteps = event->angleDelta() / 120;
    printf("Wheel: steps: vert: %d horiz: %d inverted? %s\n",
           numSteps.y(), numSteps.x(), event->inverted() ? "yes" : "no");
}

void FITSWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    int realWidth = width();
    int realHeight = height();

    // painter.drawRect(0, 0, realWidth, realHeight);

    int border = 1;

    int w = realWidth - (border * 2);
    int h = realHeight - (border * 2);

    if (_cacheImage == 0)
    {
        _cacheImage = convertImage();
    }

    int imgW = _fits->getWidth();
    int imgH = _fits->getHeight();

    QRect target;
    if ((imgW < w) && (imgH < h))
    {
        target.setLeft((w - imgW) / 2);
        target.setTop((h - imgH) / 2);
        target.setWidth(imgW);
        target.setHeight(imgH);
    }
    else
    {
        float imgAspect = (float)imgW / (float)imgH;
        float winAspect = (float)w / (float)h;

        if (imgAspect >= winAspect)
        {
            target.setLeft(border);
            target.setWidth(w);
            int targetHeight = (int)(w / imgAspect);
            target.setTop((h - targetHeight) / 2 + border);
            target.setHeight(targetHeight);
        }
        else
        {
            target.setTop(border);
            target.setHeight(h);
            int targetWidth = (int)(h * imgAspect);
            target.setLeft((w - targetWidth) / 2 + border);
            target.setWidth(targetWidth);
        }
    }

    float zoomNow = (float)target.width() / (float)imgW;
    if (zoomNow != _zoom)
    {
        _zoom = zoomNow;

        emit zoomChanged(_zoom);
    }

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(target, *_cacheImage);
}

QImage *FITSWidget::convertImage() const
{
    int width = _fits->getWidth();
    int height = _fits->getHeight();
    int chanAx = _fits->getChanAx();
    bool isColor = _fits->isColor();

    QImage::Format format = QImage::Format_ARGB32;
    if (!isColor)
    {
        format = QImage::Format_Grayscale8;
    }

    const void *pixels = _fits->getPixels();

    int fitsioDataType = 0;
    switch (_fits->getBitDepth())
    {
    case ELS::FITSImage::BD_INT_8:
        fitsioDataType = TBYTE;
        break;
    case ELS::FITSImage::BD_INT_16:
        fitsioDataType = TUSHORT;
        break;
    case ELS::FITSImage::BD_INT_32:
        fitsioDataType = TUINT;
        break;
    case ELS::FITSImage::BD_FLOAT:
        fitsioDataType = TFLOAT;
        break;
    case ELS::FITSImage::BD_DOUBLE:
        fitsioDataType = TDOUBLE;
        break;
    }

    QImage *qi = new QImage(width,
                            height,
                            format);

    Stretch cunningham(width,
                       height,
                       isColor ? 3 : 1,
                       fitsioDataType);

    if (_showStretched)
    {
        StretchParams sp = cunningham.computeParams((const uint8_t *)pixels);
        cunningham.setParams(sp);
    }

    cunningham.run((uint8_t const *)pixels, qi);

    // switch (_fits->getBitDepth())
    // {
    // case ELS::FITSImage::BD_INT_8:
    //     break;
    // case ELS::FITSImage::BD_INT_16:
    //     if (isColor)
    //     {
    //         convertU16ColorImage(qi,
    //                              width,
    //                              height,
    //                              chanAx,
    //                              (const uint16_t *)pixels);
    //     }
    //     else
    //     {
    //         convertU16MonoImage(qi,
    //                             width,
    //                             height,
    //                             (const uint16_t *)pixels);
    //     }
    //     break;
    // case ELS::FITSImage::BD_INT_32:
    //     break;
    // case ELS::FITSImage::BD_FLOAT:
    //     if (isColor)
    //     {
    //         convertFloatColorImage(qi,
    //                                width,
    //                                height,
    //                                chanAx,
    //                                (const float *)pixels);
    //     }
    //     else
    //     {
    //         convertFloatMonoImage(qi,
    //                               width,
    //                               height,
    //                               (const float *)pixels);
    //     }
    //     break;
    // case ELS::FITSImage::BD_DOUBLE:
    //     if (isColor)
    //     {
    //         convertDoubleColorImage(qi,
    //                                 width,
    //                                 height,
    //                                 chanAx,
    //                                 (const double *)pixels);
    //     }
    //     else
    //     {
    //         convertDoubleMonoImage(qi,
    //                                width,
    //                                height,
    //                                (const double *)pixels);
    //     }
    //     break;
    // }

    return qi;
}

void FITSWidget::convertU16MonoImage(QImage *qi,
                                     int width,
                                     int height,
                                     const uint16_t *pixels) const
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t val = (uint8_t)((double)pixels[y * width + x] / 257);
            qi->setPixelColor(x, y, QColor::fromRgb(val, val, val));
        }
    }
}

void FITSWidget::convertFloatMonoImage(QImage *qi,
                                       int width,
                                       int height,
                                       const float *pixels) const
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t val = (uint8_t)(pixels[y * width + x] * 255);
            qi->setPixelColor(x, y, QColor::fromRgb(val, val, val));
        }
    }
}

void FITSWidget::convertDoubleMonoImage(QImage *qi,
                                        int width,
                                        int height,
                                        const double *pixels) const
{
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            uint8_t val = (uint8_t)(pixels[y * width + x] * 255);
            qi->setPixelColor(x, y, QColor::fromRgb(val, val, val));
        }
    }
}

void FITSWidget::convertU16ColorImage(QImage *qi,
                                      int width,
                                      int height,
                                      int chanAx,
                                      const uint16_t *pixels) const
{
    uint8_t valR;
    uint8_t valG;
    uint8_t valB;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            switch (chanAx)
            {
            case 1:
                valR = (uint8_t)((double)pixels[3 * (x + width * y) + 2] / 257);
                valG = (uint8_t)((double)pixels[3 * (x + width * y) + 1] / 257);
                valB = (uint8_t)((double)pixels[3 * (x + width * y) + 0] / 257);
                break;
            case 3:
                valR = (uint8_t)((double)pixels[x + width * y + 2 * width * height] / 257);
                valG = (uint8_t)((double)pixels[x + width * y + 1 * width * height] / 257);
                valB = (uint8_t)((double)pixels[x + width * y + 0 * width * height] / 257);
                break;
            }

            qi->setPixelColor(x, y, QColor::fromRgb(valR, valG, valB));
        }
    }
}

void FITSWidget::convertFloatColorImage(QImage *qi,
                                        int width,
                                        int height,
                                        int chanAx,
                                        const float *pixels) const
{
    uint8_t valR;
    uint8_t valG;
    uint8_t valB;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            switch (chanAx)
            {
            case 1:
                valR = 255 * pixels[3 * (x + width * y) + 2];
                valG = 255 * pixels[3 * (x + width * y) + 1];
                valB = 255 * pixels[3 * (x + width * y) + 0];
                break;
            case 3:
                valR = 255 * pixels[x + width * y + 2 * width * height];
                valG = 255 * pixels[x + width * y + 1 * width * height];
                valB = 255 * pixels[x + width * y + 0 * width * height];
                break;
            }

            qi->setPixelColor(x, y, QColor::fromRgb(valR, valG, valB));
        }
    }
}

void FITSWidget::convertDoubleColorImage(QImage *qi,
                                         int width,
                                         int height,
                                         int chanAx,
                                         const double *pixels) const
{
    uint8_t valR;
    uint8_t valG;
    uint8_t valB;

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            switch (chanAx)
            {
            case 1:
                valR = 255 * pixels[3 * (x + width * y) + 2];
                valG = 255 * pixels[3 * (x + width * y) + 1];
                valB = 255 * pixels[3 * (x + width * y) + 0];
                break;
            case 3:
                valR = 255 * pixels[x + width * y + 2 * width * height];
                valG = 255 * pixels[x + width * y + 1 * width * height];
                valB = 255 * pixels[x + width * y + 0 * width * height];
                break;
            }

            qi->setPixelColor(x, y, QColor::fromRgb(valR, valG, valB));
        }
    }
}
