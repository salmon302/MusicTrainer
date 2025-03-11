# Notegrid/Viewport Implementation Redesign

## Current Implementation Analysis

After examining the code, I've identified several key aspects of the current implementation:

1. **Data Structure**:
   - NoteGrid uses a nested std::map for sparse storage: `std::map<int, std::map<int, std::unique_ptr<GridCell>>>`
   - Each GridCell manages its own QGraphicsRectItem for rendering

2. **Viewport Management**:
   - ViewportManager handles viewport state, expansion triggers, and coordinate transformations
   - Tightly coupled with NoteGrid (direct pointer access)

3. **Rendering Approach**:
   - Individual QGraphicsItems for each note and grid line
   - Updates happen on viewport changes and note additions

4. **Memory Management**:
   - Some memory optimization with compactUnusedRegions, but could be improved
   - No clear strategy for handling very large scores

## Issues with Current Implementation

1. **Scalability Limitations**:
   - Nested map structure has O(log n) lookup complexity
   - Individual QGraphicsItems for each note and grid line create overhead
   - No batching of rendering operations

2. **Tight Coupling**:
   - ViewportManager directly accesses NoteGrid
   - ScoreView owns both and manages their interaction

3. **Memory Efficiency**:
   - Current approach may not efficiently handle very large scores
   - Grid cells are created even when just displaying grid lines

4. **Extensibility Challenges**:
   - Difficult to add new visualization strategies
   - Hard to implement different grid styles or note representations

## Proposed Redesign

I propose a redesign that focuses on:

1. **Improved Data Structures**
2. **Decoupled Components**
3. **Optimized Rendering**
4. **Enhanced Memory Management**
5. **Better Extensibility**

### 1. Improved Data Structures

Replace the nested std::map with a more efficient spatial data structure:

```cpp
// Current approach
std::map<int, std::map<int, std::unique_ptr<GridCell>>> m_gridCells;

// Proposed approach
class SpatialIndex {
public:
    void insert(int x, int y, std::unique_ptr<GridCell> cell);
    GridCell* find(int x, int y);
    std::vector<GridCell*> queryRange(int minX, int maxX, int minY, int maxY);
    void remove(int x, int y);
    void clear();
private:
    // Implementation could use a quadtree, spatial hash, or custom structure
    // optimized for 2D musical grid access patterns
};
```

The SpatialIndex class could be implemented using:
- A quadtree for efficient spatial queries
- A spatial hash for fast lookups
- A custom structure optimized for musical grid patterns

Benefits:
- Faster range queries for visible notes
- More efficient memory usage
- Better scalability for large scores

### 2. Decoupled Components with Clear Interfaces

Introduce interfaces to decouple the components:

```cpp
// Grid data interface
class IGridModel {
public:
    virtual ~IGridModel() = default;
    virtual void setDimensions(const GridDimensions& dimensions) = 0;
    virtual const GridDimensions& getDimensions() const = 0;
    virtual void addNote(const Note& note, int voiceIndex, int position) = 0;
    virtual bool hasNoteAt(int position, int pitch) const = 0;
    virtual std::vector<const Note*> getNotesInRange(int minPos, int maxPos, int minPitch, int maxPitch) = 0;
    // Other data access methods
};

// Grid rendering interface
class IGridRenderer {
public:
    virtual ~IGridRenderer() = default;
    virtual void updateVisibleArea(const QRectF& visibleArea, float zoomLevel) = 0;
    virtual void renderGridLines(bool majorLines) = 0;
    virtual void renderNotes(const std::vector<const Note*>& notes) = 0;
    virtual void clear() = 0;
    // Other rendering methods
};

// Viewport management interface
class IViewportController {
public:
    virtual ~IViewportController() = default;
    virtual void setViewportBounds(const QRectF& bounds) = 0;
    virtual QRectF getViewportBounds() const = 0;
    virtual QPointF mapToMusicalSpace(const QPointF& screenPoint) const = 0;
    virtual QPointF mapFromMusicalSpace(const QPointF& musicalPoint) const = 0;
    virtual void expandGrid(GridDirection direction, int amount) = 0;
    // Other viewport methods
};
```

Benefits:
- Clear separation of concerns
- Easier to test individual components
- Flexibility to swap implementations
- Better maintainability

### 3. Optimized Rendering with Batching

Implement a more efficient rendering strategy:

```cpp
class BatchedGridRenderer : public IGridRenderer {
public:
    void updateVisibleArea(const QRectF& visibleArea, float zoomLevel) override {
        m_visibleArea = visibleArea;
        m_zoomLevel = zoomLevel;
        updateRenderingBatches();
    }
    
    void renderGridLines(bool majorLines) override {
        // Use a single QGraphicsItem for all grid lines of the same type
        // or use QGraphicsScene::drawLine directly for better performance
    }
    
    void renderNotes(const std::vector<const Note*>& notes) override {
        // Group notes by voice/color and render in batches
        // Consider using QGraphicsItemGroup or custom QGraphicsItem
    }
    
private:
    void updateRenderingBatches() {
        // Determine what needs to be rendered based on visible area
        // Create/update batched rendering items
    }
    
    QRectF m_visibleArea;
    float m_zoomLevel;
    std::vector<QGraphicsItem*> m_batchedItems;
};
```

Benefits:
- Reduced number of QGraphicsItems
- Better rendering performance
- Lower memory overhead
- Smoother scrolling and zooming

### 4. Enhanced Memory Management

Implement a more sophisticated memory management strategy:

```cpp
class GridMemoryManager {
public:
    void setActiveRegion(const QRectF& visibleRegion, const QRectF& bufferRegion) {
        m_visibleRegion = visibleRegion;
        m_bufferRegion = bufferRegion;
        updateMemoryUsage();
    }
    
    bool shouldLoad(int x, int y) const {
        return isInRegion(x, y, m_bufferRegion);
    }
    
    bool shouldUnload(int x, int y) const {
        return !isInRegion(x, y, m_bufferRegion);
    }
    
    void registerCellCreation(int x, int y) {
        m_cellLocations.insert({x, y});
    }
    
    void registerCellDeletion(int x, int y) {
        m_cellLocations.erase({x, y});
    }
    
    std::vector<std::pair<int, int>> getCellsToUnload() const {
        std::vector<std::pair<int, int>> result;
        for (const auto& loc : m_cellLocations) {
            if (shouldUnload(loc.first, loc.second)) {
                result.push_back(loc);
            }
        }
        return result;
    }
    
private:
    bool isInRegion(int x, int y, const QRectF& region) const {
        return x >= region.left() && x <= region.right() &&
               y >= region.top() && y <= region.bottom();
    }
    
    void updateMemoryUsage() {
        // Implement memory usage tracking and optimization
    }
    
    QRectF m_visibleRegion;
    QRectF m_bufferRegion;
    std::set<std::pair<int, int>> m_cellLocations;
};
```

Benefits:
- Proactive memory management
- Predictable memory usage
- Better handling of large scores
- Reduced memory fragmentation

### 5. Better Extensibility with Strategy Pattern

Use the strategy pattern to allow different grid visualization approaches:

```cpp
// Grid visualization strategy interface
class IGridVisualizationStrategy {
public:
    virtual ~IGridVisualizationStrategy() = default;
    virtual void renderNote(const Note& note, int voiceIndex, QGraphicsScene* scene) = 0;
    virtual void renderGridLine(bool isMajor, bool isHorizontal, float position, QGraphicsScene* scene) = 0;
    virtual QColor getNoteColor(int voiceIndex) const = 0;
};

// Standard piano roll visualization
class PianoRollVisualization : public IGridVisualizationStrategy {
    // Implementation
};

// Traditional notation visualization
class TraditionalNotationVisualization : public IGridVisualizationStrategy {
    // Implementation
};

// Then in the renderer:
class ConfigurableGridRenderer : public IGridRenderer {
public:
    void setVisualizationStrategy(std::unique_ptr<IGridVisualizationStrategy> strategy) {
        m_visualizationStrategy = std::move(strategy);
        updateAllVisuals();
    }
    
    // Other methods using m_visualizationStrategy
    
private:
    std::unique_ptr<IGridVisualizationStrategy> m_visualizationStrategy;
};
```

Benefits:
- Support for multiple visualization styles
- Easy to add new visualization strategies
- Consistent rendering interface
- Separation of rendering logic from data

### 6. Improved Coordinate Transformation

Centralize coordinate transformation in a dedicated class:

```cpp
class CoordinateTransformer {
public:
    CoordinateTransformer(float gridUnit, float noteHeight, float zoomBase)
        : m_gridUnit(gridUnit), m_noteHeight(noteHeight), m_zoomBase(zoomBase) {}
    
    void setZoomLevel(float zoomLevel) {
        m_currentZoom = zoomLevel;
    }
    
    QPointF toScreenSpace(int position, int pitch) const {
        return QPointF(
            position * m_gridUnit * m_currentZoom,
            (127 - pitch) * m_noteHeight * m_currentZoom
        );
    }
    
    std::pair<int, int> toMusicalSpace(const QPointF& screenPoint) const {
        return {
            static_cast<int>(screenPoint.x() / (m_gridUnit * m_currentZoom)),
            127 - static_cast<int>(screenPoint.y() / (m_noteHeight * m_currentZoom))
        };
    }
    
private:
    float m_gridUnit;
    float m_noteHeight;
    float m_zoomBase;
    float m_currentZoom = 1.0f;
};
```

Benefits:
- Centralized coordinate transformation logic
- Consistent mapping between coordinate spaces
- Easier to maintain and update
- Reduced duplication of transformation code

## Implementation Plan

1. **Create the interfaces** and base classes for the new design
   - Define IGridModel, IGridRenderer, IViewportController interfaces
   - Create abstract base classes for each interface

2. **Implement the SpatialIndex** data structure for efficient grid cell storage
   - Create a quadtree or spatial hash implementation
   - Add unit tests to verify performance and correctness

3. **Develop the BatchedGridRenderer** for optimized rendering
   - Implement batched rendering for grid lines
   - Implement batched rendering for notes
   - Add performance benchmarks

4. **Implement the GridMemoryManager** for better memory management
   - Create memory tracking and optimization logic
   - Add unit tests for memory management

5. **Create visualization strategies** for different rendering approaches
   - Implement the standard piano roll visualization
   - Add support for traditional notation (optional)

6. **Refactor the existing code** to use the new components
   - Update NoteGrid to implement IGridModel
   - Update ViewportManager to implement IViewportController
   - Create a new renderer implementing IGridRenderer
   - Update ScoreView to use the new interfaces

7. **Add unit tests** to verify the new implementation
   - Test each component individually
   - Test integration between components
   - Benchmark performance improvements

## Migration Strategy

To minimize disruption, we can implement this redesign in phases:

1. **Phase 1**: Create the new interfaces and implement them with adapter classes that wrap the existing implementation
2. **Phase 2**: Replace the data structure with the SpatialIndex while maintaining the existing API
3. **Phase 3**: Implement the batched rendering approach
4. **Phase 4**: Add the memory management improvements
5. **Phase 5**: Introduce the visualization strategies

This approach allows for incremental improvements and testing at each phase, reducing the risk of regressions.

## Expected Benefits

1. **Improved Performance**:
   - Faster rendering for large scores
   - Reduced memory usage
   - Smoother scrolling and zooming

2. **Better Scalability**:
   - Support for much larger scores
   - Efficient handling of complex musical content
   - Predictable performance as score size increases

3. **Enhanced Maintainability**:
   - Clear separation of concerns
   - Well-defined interfaces
   - Easier to test and debug

4. **Greater Extensibility**:
   - Support for different visualization styles
   - Easier to add new features
   - More flexible architecture

## Visual and Interactive Improvements

Based on additional requirements, we've enhanced the design to include visual and interactive improvements:

### 1. Improved Viewport Layout

We've added a `ViewportLayoutManager` that:
- Maximizes the viewport area by reducing surrounding elements
- Provides configurable UI element visibility (toolbars, sidebars, etc.)
- Supports auto-hiding UI elements when not in use
- Includes a minimap for navigation in large scores
- Enables fullscreen mode for maximum editing space

### 2. Adaptive Grid Expansion

The new `AdaptiveGridExpansionManager` provides:
- Intelligent vertical/horizontal stretching based on content
- Improved octave/bar expansion and collapse
- Preservation of musical alignment (octaves, measures)
- Automatic contraction of empty areas
- Configurable expansion thresholds and behavior

### 3. Note Length-Based Grid Adaptation

The system now includes:
- Automatic grid density adjustment based on note lengths
- Intelligent grid redrawing when smaller note values are used
- Configurable minimum and maximum grid densities
- Preservation of grid consistency during editing

### 4. User Experience Enhancements

Additional improvements include:
- Smoother transitions during grid expansion/contraction
- Visual indicators for expandable regions
- Improved note placement preview
- Context-aware grid line emphasis
- Optimized viewport culling for better performance

These visual and interactive improvements complement the architectural changes to provide a more intuitive, responsive, and efficient user experience while maintaining the scalability and performance benefits of the core redesign.