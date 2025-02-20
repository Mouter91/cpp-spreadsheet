#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() {}

void Sheet::SetCell(Position pos, std::string text) {
    PosValid(pos);

    if (!table_[pos]) {
        table_[pos] = std::make_unique<Cell>(*this);
    }

    table_[pos]->Set(text);

    size_table_.rows = std::max(size_table_.rows, pos.row + 1);
    size_table_.cols = std::max(size_table_.cols, pos.col + 1);         
}

const CellInterface* Sheet::GetCell(Position pos) const {
    PosValid(pos);
    auto it = table_.find(pos);
    return (it != table_.end()) ? it->second.get() : nullptr;
}
CellInterface* Sheet::GetCell(Position pos) {
    PosValid(pos);
    auto it = table_.find(pos);
    return (it != table_.end()) ? it->second.get() : nullptr;
}

void Sheet::ClearCell(Position pos) {
    PosValid(pos);
    table_.erase(pos);
    UpdateSize(); 
}

Size Sheet::GetPrintableSize() const {
    return size_table_;
}

void Sheet::PrintValues(std::ostream& output) const {
    for (int i = 0; i < size_table_.rows; ++i) {
        for (int j = 0; j < size_table_.cols; ++j) {
            auto cell = GetCell({i, j});
            Cell::Value val = cell ? cell->GetValue() : Cell::Value{};

            std::visit([&output, j, cols = size_table_.cols](const auto& value) {  
                output << value << (j + 1 == cols ? "" : "\t");
            }, val);
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    for (int i = 0; i < size_table_.rows; ++i) {
        for (int j = 0; j < size_table_.cols; ++j) {
            auto cell = GetCell({i, j});
            std::string text;
            if (cell) {
                text = cell->GetText();
            }
            output << text << (j + 1 == size_table_.cols ? "" : "\t");            
        }
        output << '\n';
    }    
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

void Sheet::PosValid(Position& pos) const {
    if(!pos.IsValid()) {
        throw InvalidPositionException("This is invalid position"s + std::to_string(pos.row) + " " + std::to_string(pos.col));
    }
}

void Sheet::UpdateSize() {
    size_table_ = {0, 0};

    for (const auto& [pos, cell] : table_) {
        size_table_.rows = std::max(size_table_.rows, pos.row + 1);
        size_table_.cols = std::max(size_table_.cols, pos.col + 1);
    }
}
