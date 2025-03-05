#pragma once
#include <QRectF>
#include <QPointF>
#include <QSize>
#include <memory>
#include <map>
#include <optional>

QT_BEGIN_NAMESPACE
class QGraphicsView;
QT_END_NAMESPACE

namespace MusicTrainer::presentation {

class NoteGrid;

/**
 * @brief Manages the viewport state and dynamic grid expansion
 */
class ViewportManager {
public:
    enum class Direction {
        Up,
        Down,
        Right
    };

    struct ViewportState {
        QRectF visibleArea;     // Currently visible area in musical space
        float zoomLevel;        // Current zoom level (default 1.0)
        QPointF scrollPosition; // Current scroll position
        bool preserveOctaveExpansion{false}; // Flag to maintain expanded octave range
    };
    
    struct LoadingBoundaries {
        int verticalBuffer{6};   // Additional rows to load (half octave)
        int horizontalBuffer{4}; // Additional measures to load
    };

    explicit ViewportManager(NoteGrid* grid);
    ~ViewportManager();
    
    bool updateViewportState(const ViewportState& newState);
    QRectF getViewportBounds() const;
    void setViewportBounds(const QRectF& bounds);
    void updateViewSize(const QSize& size);
    void updateScrollPosition(const QPointF& pos);
    void updateZoomLevel(float zoom);
    QPointF mapToMusicalSpace(const QPointF& screenPoint, const QGraphicsView* view) const;
    QPointF mapFromMusicalSpace(const QPointF& musicalPoint, const QGraphicsView* view) const;
    void expandGrid(Direction direction, int amount);
    void collapseGrid(Direction direction); // New method to collapse grid in a direction
    bool canCollapse(Direction direction) const; // New method to check if collapse is possible
    std::optional<Direction> shouldExpand() const;
    const ViewportState& getViewportState() const;
    const LoadingBoundaries& getLoadingBoundaries() const;
    void setLoadingBoundaries(const LoadingBoundaries& boundaries);
    void compactUnusedRegions();

private:
    /**
     * @brief Get the octave range (always 12 semitones)
     */
    static constexpr int getOctaveRange() { return 12; }

    /**
     * @brief Enforce one octave constraint on viewport state
     * @param state State to validate
     * @return Validated state with one octave constraint
     */
    ViewportState validateState(const ViewportState& state) const;

    NoteGrid* m_grid;
    ViewportState m_currentState;
    LoadingBoundaries m_loadingBoundaries;
    float m_verticalTriggerRatio{0.9f};
    float m_horizontalTriggerRatio{0.8f};
    QSize m_viewSize;
};

} // namespace MusicTrainer::presentation