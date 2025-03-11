#pragma once

#include "GridManager.h"
#include <QGraphicsView>

namespace MusicTrainer::presentation::grid {

class TestGridView : public QGraphicsView {
    Q_OBJECT

public:
    explicit TestGridView(QWidget* parent = nullptr);
    ~TestGridView() override;

public slots:
    void handleMemoryWarning(size_t currentUsage, size_t limit);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    std::unique_ptr<QGraphicsScene> m_scene;
    std::unique_ptr<GridManager> m_gridManager;

    void setupGrid();
    void initializeMemoryConfig();
    QPointF mapToGridCoordinate(const QPoint& pos) const;
};

} // namespace MusicTrainer::presentation::grid