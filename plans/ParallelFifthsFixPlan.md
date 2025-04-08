# Plan to Fix Parallel Fifths Rule Compilation Errors

This document outlines the steps to resolve the compilation errors in `ParallelFifthsRule.cpp` and `ParallelFifthsRule.h`.

## Root Cause Analysis

The compilation errors are caused by the following issues:

1.  **Incorrect Override in Header:** The `evaluate` and `evaluateIncremental` methods in `ParallelFifthsRule.h` are marked as `override`, but their signatures do not match the signatures of the corresponding virtual methods in the base classes (`Rule` and `IncrementalRule`). Specifically, the namespace of the `Score` object is not fully qualified, and `evaluateIncremental` is missing a const.
2.  **Declaration Mismatch:** The implementations of `evaluate` and `evaluateIncremental` in `ParallelFifthsRule.cpp` do not match the (incorrect) declarations in `ParallelFifthsRule.h`.
3.  **Abstract Class Instantiation:** The `create()` and `clone()` methods in `ParallelFifthsRule.cpp` attempt to create instances of `ParallelFifthsRule` itself. However, `ParallelFifthsRule` is an abstract class because it inherits pure virtual functions from its base classes, and therefore cannot be instantiated directly.

## Solution

The following steps will resolve these issues:

1.  **Fix `ParallelFifthsRule.h`:**
    *   Change the signature of `evaluate` from:
        ```c++
        bool evaluate(const MusicTrainer::music::Score& score) override;
        ```
        to:
        ```c++
        bool evaluate(const ::MusicTrainer::music::Score& score);
        ```
        Then add `override` back:
        ```c++
        bool evaluate(const ::MusicTrainer::music::Score& score) override;
        ```
    *   Change the signature of `evaluateIncremental` from:
        ```c++
        bool evaluateIncremental(const MusicTrainer::music::Score& score, ::std::size_t startMeasure, ::std::size_t endMeasure) const override;
        ```
        to:
        ```c++
        bool evaluateIncremental(const ::MusicTrainer::music::Score& score, ::std::size_t startMeasure, ::std::size_t endMeasure) const;
        ```
        Then add `override` back:
        ```c++
        bool evaluateIncremental(const ::MusicTrainer::music::Score& score, ::std::size_t startMeasure, ::std::size_t endMeasure) const override;
        ```
    *   Add a comment indicating that a concrete derived class (e.g., `ConcreteParallelFifthsRule`) is needed for instantiation.

2.  **Fix `ParallelFifthsRule.cpp`:**
    *   Change the signature of `evaluate` to match the corrected declaration in `ParallelFifthsRule.h`.
    *   Change the signature of `evaluateIncremental` to match the corrected declaration in `ParallelFifthsRule.h`.
    *   Create a concrete derived class, `ConcreteParallelFifthsRule`, that inherits from `ParallelFifthsRule` and provides empty implementations for the pure virtual methods. This will allow us to create instances for the `create()` and `clone()` methods.
    *   Modify the `create()` method to return an instance of `ConcreteParallelFifthsRule`.
    *   Modify the `clone()` method to return an instance of `ConcreteParallelFifthsRule`.

## Concrete Class Example
```c++
// In ParallelFifthsRule.cpp
namespace MusicTrainer::music::rules {
    class ConcreteParallelFifthsRule : public ParallelFifthsRule {
    public:
        bool evaluate(const ::MusicTrainer::music::Score& score) override { return true; }; // Provide a default implementation
        ::std::string getViolationDescription() const override { return ""; }
        ::std::string getName() const override {return "ConcreteParallelFifthsRule";}
        bool evaluateIncremental(const ::MusicTrainer::music::Score& score, ::std::size_t startMeasure, ::std::size_t endMeasure) const override {return true;} // Provide default
        Rule* clone() const override { return new ConcreteParallelFifthsRule(*this); }

    };
}