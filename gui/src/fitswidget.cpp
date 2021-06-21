#include <QPainter>

#include "fitswidget.h"
#include "fitstantrum.h"
#include "stretch.h"

/* static */
const float FITSWidget::g_validZooms[] = {
    0.125,
    0.250,
    0.333,
    0.500,
    0.667,
    0.750,
    1.000,
    1.250,
    1.333,
    1.500,
    1.667,
    1.750,
    2.000,
    -1.0};

FITSWidget::FITSWidget(QWidget *parent)
    : QWidget(parent),
      _sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding),
      _filename(0),
      _fits(0),
      _cacheImage(0),
      _showStretched(false),
      _zoom(-1.0),
      _actualZoom(-1.0)
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

const char *FITSWidget::getFilename() const
{
    return _filename;
}

bool FITSWidget::getStretched() const
{
    return _showStretched;
}

float FITSWidget::getZoom() const
{
    return _zoom;
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

void FITSWidget::setZoom(float zoom)
{
    // Adjust zoom to the closest valid value
    if (zoom != -1.0)
    {
        zoom = adjustZoom(zoom);
    }

    if (_zoom != zoom)
    {
        _internalSetZoom(zoom);
    }
}

void FITSWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numSteps = event->angleDelta() / 120;

    float zoom = _actualZoom;
    if (numSteps.y() >= 1)
    {
        zoom = adjustZoom(zoom, ZAS_HIGHER);

        _internalSetZoom(zoom);
    }
    else if (numSteps.y() <= -1)
    {
        zoom = adjustZoom(zoom, ZAS_LOWER);

        _internalSetZoom(zoom);
    }
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
    int imgZoomW = imgW;
    int imgZoomH = imgH;

    if (_zoom != -1.0)
    {
        imgZoomW *= _zoom;
        imgZoomH *= _zoom;
    }

    QRect target;
    QRect source(0, 0, imgW, imgH);
    float zoomNow = _zoom;
    if ((imgZoomW < w) && (imgZoomH < h))
    {
        target.setLeft((w - imgZoomW) / 2);
        target.setTop((h - imgZoomH) / 2);
        target.setWidth(imgZoomW);
        target.setHeight(imgZoomH);
    }
    else
    {
        if (_zoom == -1.0)
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

            zoomNow = (float)target.width() / (float)imgW;
        }
        else
        {
            int widthXtra = imgZoomW - w;
            int heightXtra = imgZoomH - h;

            QRect sourceZoom(0, 0, 0, 0);
            if (widthXtra < 0)
            {
                target.setLeft((w - imgZoomW) / 2 + border);
                target.setWidth(imgZoomW);
                sourceZoom.setLeft(0);
                sourceZoom.setWidth(imgZoomW);
            }
            else
            {
                target.setLeft(border);
                target.setWidth(w);
                sourceZoom.setLeft(widthXtra / 2);
                sourceZoom.setWidth(w);
            }

            if (heightXtra < 0)
            {
                target.setTop((h - imgZoomH) / 2 + border);
                target.setHeight(imgZoomH);
                sourceZoom.setTop(0);
                sourceZoom.setHeight(imgZoomH);
            }
            else
            {
                target.setTop(border);
                target.setHeight(h);
                sourceZoom.setTop(heightXtra / 2);
                sourceZoom.setHeight(h);
            }

            source.setLeft(sourceZoom.left() / _zoom);
            source.setWidth(sourceZoom.width() / _zoom);
            source.setTop(sourceZoom.top() / _zoom);
            source.setHeight(sourceZoom.height() / _zoom);
        }
    }

    if (zoomNow != _actualZoom)
    {
        _actualZoom = zoomNow;

        emit actualZoomChanged(_actualZoom);
    }

    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawImage(target, *_cacheImage, source);
}

QImage *FITSWidget::convertImage() const
{
    int width = _fits->getWidth();
    int height = _fits->getHeight();
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

    return qi;
}

void FITSWidget::_internalSetZoom(float zoom)
{
    _zoom = zoom;

    update();

    emit zoomChanged(_zoom);
}

/* static */
float FITSWidget::adjustZoom(float desiredZoom,
                             ZoomAdjustStrategy strategy /* = ZAS_CLOSEST */)
{
    if (desiredZoom < g_validZooms[0])
    {
        return g_validZooms[0];
    }
    else
    {
        if (desiredZoom == g_validZooms[0])
        {
            if (strategy == ZAS_HIGHER)
            {
                return g_validZooms[1];
            }
            else
            {
                return g_validZooms[0];
            }
        }
        else
        {
            for (int i = 1; true; i++)
            {
                if (g_validZooms[i] == -1.0)
                {
                    return g_validZooms[i - 1];
                }

                if (desiredZoom == g_validZooms[i])
                {
                    switch (strategy)
                    {
                    case ZAS_CLOSEST:
                        return g_validZooms[i];
                    case ZAS_HIGHER:
                        break;
                    case ZAS_LOWER:
                        return g_validZooms[i - 1];
                    }

                    continue;
                }

                if (desiredZoom < g_validZooms[i])
                {
                    switch (strategy)
                    {
                    case ZAS_CLOSEST:
                    {
                        float tmp1 = g_validZooms[i] - desiredZoom;
                        float tmp2 = desiredZoom - g_validZooms[i - 1];

                        if (tmp1 < tmp2)
                        {
                            return g_validZooms[i];
                        }
                        else
                        {
                            return g_validZooms[i - 1];
                        }
                    }
                    case ZAS_HIGHER:
                        return g_validZooms[i];
                    case ZAS_LOWER:
                        return g_validZooms[i - 1];
                    }

                    break;
                }
            }
        }
    }

    return g_validZooms[0];
}
