#pragma once

#include "common.h"
#include "formula.h"

#include <optional>

class Cell : public CellInterface {
public:
    Cell(SheetInterface &sheet);
    ~Cell();

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    class Impl {
    public:
        virtual ~Impl() = default;
        virtual std::string GetText() const = 0;
        virtual Value GetValue() const = 0;
        virtual std::vector<Position> GetReferencedCells() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        std::string GetText() const override;
        Value GetValue() const override;
        std::vector<Position> GetReferencedCells() const override;
    };

    class TextImpl : public Impl {
    public:
        explicit TextImpl(std::string text) : text_(std::move(text)) {}

        std::string GetText() const override;
        Value GetValue() const override;
        std::vector<Position> GetReferencedCells() const override;

    private:
        std::string text_;
    };

    class FormulaImpl : public Impl {
    public:
        explicit FormulaImpl(std::string text, Cell* self)
            : formula_(ParseFormula(text.substr(1))), self_(self) {
            UpdateDependencies();
            CheckCircularDependency(self);
            InvalidateCache();
        }

        std::string GetText() const override;
        Value GetValue() const override;
        std::vector<Cell*> GetDependencies() const;
        std::vector<Position> GetReferencedCells() const override;

    private:
        void UpdateDependencies();
        void CheckCircularDependency(Cell* self);
        void InvalidateCache();

        std::unique_ptr<FormulaInterface> formula_;
        std::vector<Cell*> dependencies_;
        mutable std::optional<Value> cache_;
        Cell* self_;
    };

    std::unique_ptr<Impl> impl_;
    const SheetInterface& sheet_;
};
