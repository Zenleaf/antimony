#include <Python.h>

#include <QMenu>
#include <QMouseEvent>

#include "app/colors.h"
#include "canvas/view.h"
#include "canvas/scene.h"
#include "graph/constructor/populate.h"

CanvasView::CanvasView(CanvasScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent), selecting(false)
{
    setStyleSheet("QGraphicsView { border-style: none; }");
    setRenderHints(QPainter::Antialiasing);
    setSceneRect(-width()/2, -height()/2, width(), height());

    QAbstractScrollArea::setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QAbstractScrollArea::setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

////////////////////////////////////////////////////////////////////////////////

void CanvasView::mousePressEvent(QMouseEvent* event)
{
    QGraphicsView::mousePressEvent(event);
    if (!event->isAccepted()) {
            if (event->button() == Qt::LeftButton) {
                click_pos = mapToScene(event->pos());
            }
            else if (event->button() == Qt::RightButton) {
                QMenu* m = new QMenu(this);
                populateNodeMenu(
                        m, static_cast<CanvasScene*>(scene())->getGraph());

                m->exec(QCursor::pos());
                m->deleteLater();
            }
    }
}

void CanvasView::mouseReleaseEvent(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);

    if (selecting)
    {
        QPainterPath p;
        p.addRect(QRectF(click_pos, drag_pos));
        scene()->setSelectionArea(p);
        selecting = false;
        scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
    }
}

void CanvasView::mouseMoveEvent(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);
    if (scene()->mouseGrabberItem() == NULL && event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() & Qt::ShiftModifier)
        {
            drag_pos = mapToScene(event->pos());
            selecting = true;
            scene()->invalidate(QRectF(), QGraphicsScene::ForegroundLayer);
        }
        else
        {
            auto d = click_pos - mapToScene(event->pos());
            setSceneRect(sceneRect().translated(d.x(), d.y()));
        }
    }
}

void CanvasView::wheelEvent(QWheelEvent* event)
{
    QPointF a = mapToScene(event->pos());
    auto s = pow(1.001, -event->delta());
    scale(s, s);
    auto d = a - mapToScene(event->pos());
    setSceneRect(sceneRect().translated(d.x(), d.y()));
}

////////////////////////////////////////////////////////////////////////////////

void CanvasView::drawBackground(QPainter* painter, const QRectF& rect)
{
    painter->setBrush(Colors::base00);
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);

    if (fabs(rect.left() - rect.right()) < 5e3 &&
        fabs(rect.top() - rect.bottom()) < 5e3)
    {
        const int d = 20;
        painter->setPen(Colors::base03);
        for (int i = int(rect.left() / d) * d; i < rect.right(); i += d)
            for (int j = int(rect.top() / d) * d; j < rect.bottom(); j += d)
                painter->drawPoint(i, j);
    }
}

void CanvasView::drawForeground(QPainter* painter, const QRectF& rect)
{
    Q_UNUSED(rect);

    if (selecting)
    {
        painter->setPen(QPen(Colors::base05, 2));
        painter->drawRect(QRectF(click_pos, drag_pos));
    }
}
