#include <QtGui>
#include "imgview.hpp"

ImageView::ImageView(QWidget *parent, QImage image) : QWidget(parent), view(image) {
	this->setAttribute(Qt::WA_AcceptTouchEvents, true);
	qRegisterMetaType<ZKEEP>("ZKEEP");
	mouseHider->setSingleShot(true);
	QObject::connect(mouseHider, SIGNAL(timeout()), this, SLOT(hideMouse()));
	this->setMouseTracking(true);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

ImageView::~ImageView() {
	
}

QSize ImageView::sizeHint() const {
	return view.size();
}

void ImageView::paintEvent(QPaintEvent *QPE) {
	QPainter paint(this);
	paint.fillRect(QPE->rect(), QBrush(QColor(0, 0, 0)));
	
	paint.setRenderHint(QPainter::Antialiasing, true);
	paint.setRenderHint(QPainter::SmoothPixmapTransform, smoothPaint);
	
	if (this->width() > 0 && this->height() > 0 && view.width() > 0 && view.height() > 0) {
		this->calculateView();
		QSize drawSize;
		if (paintCompletePartial) {
			drawSize = this->size();
		} else if ((partRect.width() < this->width() && partRect.height() < this->height())) {
			drawSize = partRect.size() / zoom;
		} else {
			drawSize = partRect.size().scaled(this->size(), Qt::KeepAspectRatio);
		}
		drawRect.setX(((this->width() - drawSize.width()) / 2.0f));
		drawRect.setY(((this->height() - drawSize.height()) / 2.0f));
		drawRect.setSize(drawSize);
		paint.drawImage(drawRect, view, partRect);
	}
	QWidget::paintEvent(QPE);
}

void ImageView::resizeEvent(QResizeEvent *QRE) {
	this->calculateZoomLevels();
	QWidget::resizeEvent(QRE);
	emit resized(QRE->size());
}

void ImageView::wheelEvent(QWheelEvent *QWE) {
	if (QWE->angleDelta().y()) {
		QWE->accept();
		this->setZoom((1.0f - QWE->angleDelta().y() / 360.0f / 3.0f) * zoom, QPointF(QWE->position().x() / (float)this->width() , QWE->position().y() / (float)this->height()));
		this->update();
	}
	QWidget::wheelEvent(QWE);
}

void ImageView::mousePressEvent(QMouseEvent *QME) {
	this->showMouse();
	if (QME->button() == Qt::LeftButton) {
		QME->accept();
		this->setFocus();
		this->mouseMoving = true;
		this->prevMPos = QME->pos();
	}
	if (QME->button() == Qt::MiddleButton) {
		this->setZoom(1.0f, QPointF(QME->pos().x() / (float)this->width() , QME->pos().y() / (float)this->height()));
		this->update();
	}
	QWidget::mousePressEvent(QME);
}

void ImageView::mouseReleaseEvent(QMouseEvent *QME) {
	if (QME->button() == Qt::LeftButton) {
		this->mouseMoving = false;
	}
	QWidget::mouseReleaseEvent(QME);
}

void ImageView::mouseMoveEvent(QMouseEvent *QME) {
	this->showMouse();
	if (mouseMoving && !touchOverride) {
		if (keep == KEEP_EXPANDED || keep == KEEP_EQUAL) keep = KEEP_NONE;
		QPointF nPosAdj = ((QPointF)prevMPos - (QPointF)QME->pos()) * zoom;
		QPointF prevView = viewOffset;
		this->viewOffset.rx() += nPosAdj.x();
		this->viewOffset.ry() += nPosAdj.y();
		this->calculateView();
		if (viewOffset.toPoint() != prevView.toPoint()) {
			this->update();
		}
		this->prevMPos = QME->pos();
	}
	QWidget::mouseMoveEvent(QME);
}

bool ImageView::event(QEvent * ev) {
	
	if (ev->type() == QInputEvent::TouchBegin) {
		ev->accept();
		touchOverride = true;
	} else if (ev->type() == QInputEvent::TouchEnd) {
		ev->accept();
		touchOverride = false;
	} else if (ev->type() == QInputEvent::TouchUpdate) {
		QTouchEvent * tev = dynamic_cast<QTouchEvent *>(ev);
		tev->accept();
		
		
		QPointF centerNow, centerLast;
		
		for (auto & tp : tev->points()) {
			centerNow += tp.position();
			centerLast += tp.lastPosition();
		}
		centerNow /= tev->points().length();
		centerLast /= tev->points().length();
		
		qreal pinch = 0;
		for (auto & tp : tev->points()) {
			pinch += QVector2D::dotProduct(QVector2D(tp.position() - tp.lastPosition()).normalized(), QVector2D(centerNow - tp.position()).normalized());
		}
		
		this->setZoom((1.0f - pinch / 50000) * zoom, QPointF(centerNow.x() / (float)this->width() , centerNow.y() / (float)this->height()));
		
		qDebug() << "Touch Center:" << centerNow << "Last:" << centerLast; 
		QPointF centerDelta = (centerNow - centerLast) * zoom;
		
		this->viewOffset.rx() -= centerDelta.x();
		this->viewOffset.ry() -= centerDelta.y();
		this->calculateView();
		this->update();
		
	}
	
	return QWidget::event(ev);
}

void ImageView::setImage(QImage newView, ZKEEP keepStart) {
	this->keep = keepStart;
	if (this->view != newView) {
		this->view = newView;
		this->update();
	}
}

QImage ImageView::getImageOfView() {
	QImage part = view.copy(partRect);
	return part;
}

void ImageView::setZoom(qreal nZoom, QPointF focus) {
	if (nZoom < zoomMin) nZoom = zoomMin;
	if (nZoom > zoomMax) nZoom = zoomMax;
	if (zoom != nZoom) {
		QSize sizeZ = this->size() * zoom;
		QSize sizeNZ = this->size() * nZoom;
		QSize sizeZV = this->view.size();
		if (sizeZV.width() > sizeZ.width()) {
			sizeZV.setWidth(sizeZ.width());
		}
		if (sizeZV.height() > sizeZ.height()) {
			sizeZV.setHeight(sizeZ.height());
		}
		float xmod = (sizeZV.width() - sizeNZ.width()) * focus.x();
		float ymod = (sizeZV.height() - sizeNZ.height()) * focus.y();
		this->viewOffset.rx() += xmod;
		this->viewOffset.ry() += ymod;
		zoom = nZoom;
		keep = KEEP_NONE;
	} else if (keep == KEEP_NONE && zoom == nZoom && zoom != zoomMin) {
		keep = KEEP_FIT;
	}
}

void ImageView::centerView() {
	QSize sizeZ = this->size() * zoomMax;
	QSize sizeNZ = this->size();
	QSize sizeZV = this->view.size();
	if (sizeZV.width() > sizeZ.width()) {
		sizeZV.setWidth(sizeZ.width());
	}
	if (sizeZV.height() > sizeZ.height()) {
		sizeZV.setHeight(sizeZ.height());
	}
	float xmod = (sizeZV.width() - sizeNZ.width()) * 0.5f;
	float ymod = (sizeZV.height() - sizeNZ.height()) * 0.5f;
	this->viewOffset.setX(xmod);
	this->viewOffset.setY(ymod);
}

void ImageView::calculateZoomLevels() {
	float xmax = view.width() / (float) this->width();
	float ymax = view.height() / (float) this->height();
	if (xmax > ymax) {
		zoomMax = xmax;
		zoomExp = ymax;
	} else {
		zoomExp = xmax;
		zoomMax = ymax;
	}
	zoomFit = zoomMax;
	if (zoomMax < 1.0f) zoomMax = 1.0f;
}

void ImageView::calculateView() {
	this->calculateZoomLevels();
	if (zoom == zoomMax && keep == KEEP_NONE) keep = KEEP_FIT;
	switch (keep) {
	case KEEP_NONE:
		break;
	case KEEP_FIT:
		zoom = zoomMax;
		this->viewOffset = QPointF(0, 0);
		break;
	case KEEP_FIT_FORCE:
		zoom = zoomFit;
		this->viewOffset = QPointF(0, 0);
		break;
	case KEEP_EXPANDED:
	{
		QSize mScr = this->size().scaled(this->view.size(), Qt::KeepAspectRatio);
		this->viewOffset.setX((this->view.size().width() - mScr.width())/ 2.0f);
		this->viewOffset.setY((this->view.size().height() - mScr.height())/ 2.0f);
		zoom = zoomExp;
	} break;
	case KEEP_EQUAL:
		this->zoom = 1.0f;
		this->centerView();
		break;
	}
	QSize partSize = this->size() * zoom;
	this->paintCompletePartial = true;
	if (partSize.width() > view.width()) {
		partSize.setWidth(view.width());
		this->paintCompletePartial = false;
	}
	if (partSize.height() > view.height()) {
		partSize.setHeight(view.height());
		this->paintCompletePartial = false;
	}
	if (viewOffset.x() > this->view.width() - partSize.width()) {
		viewOffset.setX(this->view.width() - partSize.width());
	}
	if (viewOffset.y() > this->view.height() - partSize.height()) {
		viewOffset.setY(this->view.height() - partSize.height());
	}
	if (viewOffset.x() < 0) {
		viewOffset.setX(0);
	}
	if (viewOffset.y() < 0) {
		viewOffset.setY(0);
	}
	
	partRect = QRect(viewOffset.toPoint(), partSize);
}
