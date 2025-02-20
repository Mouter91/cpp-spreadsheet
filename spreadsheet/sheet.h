#pragma once

#include "cell.h"
#include "common.h"

#include <functional>
#include <unordered_map>

class Sheet : public SheetInterface {
public:

    struct HashPosition {
        size_t operator()(const Position& pos) const noexcept {
            return std::hash<int>()(pos.row) ^ (std::hash<int>()(pos.col) << 1);
        }
    };

    using Table = std::unordered_map<Position, std::unique_ptr<Cell>, HashPosition>;
    
    ~Sheet();

    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;


private:
	Table table_;
    Size size_table_{0, 0};

    void PosValid(Position& pos) const;
    void UpdateSize();
};
