#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <unordered_set>
#include <stack>

Cell::Cell(SheetInterface &sheet) : impl_(std::make_unique<EmptyImpl>()), sheet_(sheet) {}

Cell::~Cell() {}

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == '=' && text.size() > 1) {
        impl_ = std::make_unique<FormulaImpl>(text, this);
    } else {
        impl_ = std::make_unique<TextImpl>(text);
    }
}

void Cell::Clear() {
    impl_ = std::make_unique<EmptyImpl>();
}

Cell::Value Cell::GetValue() const {
    return impl_->GetValue();
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return impl_->GetReferencedCells();
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}
Cell::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::vector<Position> Cell::EmptyImpl::GetReferencedCells() const {
    return {};
}


std::string Cell::TextImpl::GetText() const {
    return text_;
}
Cell::Value Cell::TextImpl::GetValue() const {
    if(text_[0] == '\'') {
        return text_.substr(1);
    }
    return text_;
}

std::vector<Position> Cell::TextImpl::GetReferencedCells() const {
    return {};
}

std::string Cell::FormulaImpl::GetText() const {
    return "=" + formula_->GetExpression();
}

Cell::Value Cell::FormulaImpl::GetValue() const {
    if (!cache_) {
        auto result = formula_->Evaluate(self_->sheet_);
        cache_ = std::visit([](auto&& val) -> Value { return val; }, result);
    }
    return *cache_;
}

std::vector<Cell*> Cell::FormulaImpl::GetDependencies() const {
    return dependencies_;
}

std::vector<Position> Cell::FormulaImpl::GetReferencedCells() const {
    return formula_->GetReferencedCells();
}

void Cell::FormulaImpl::UpdateDependencies() {
    dependencies_.clear();
    for (Position& pos : formula_->GetReferencedCells()) {
        if (auto* cell = dynamic_cast<Cell*>(const_cast<SheetInterface&>(self_->sheet_).GetCell(pos))) {
            dependencies_.push_back(cell);
        }
    }
}

void Cell::FormulaImpl::CheckCircularDependency(Cell* self) {
    std::unordered_set<Cell*> visited;  
    std::unordered_set<Cell*> in_stack;  
    std::stack<std::pair<Cell*, std::vector<Cell*>::iterator>> stack;

    for (Cell* dep : dependencies_) {
        if (!dep || !dep->impl_) continue; 
        auto* dep_impl = dynamic_cast<FormulaImpl*>(dep->impl_.get());

        if (dep == self) {
            throw CircularDependencyException("Циклическая зависимость!");
        }

        stack.push({dep, dep_impl->GetDependencies().begin()});
        in_stack.insert(dep);
    }

    while (!stack.empty()) {
        auto& [current, iter] = stack.top();

        auto* current_impl = dynamic_cast<FormulaImpl*>(current->impl_.get());

        if (iter == current_impl->GetDependencies().end()) {
            stack.pop();
            in_stack.erase(current);
        } else {
            Cell* next = *iter++;

            if (in_stack.count(next)) {
                throw CircularDependencyException("Циклическая зависимость!");
            }

            if (visited.insert(next).second) {
                auto* next_impl = dynamic_cast<FormulaImpl*>(next->impl_.get());
                if (next_impl) { 
                    stack.push({next, next_impl->GetDependencies().begin()});
                    in_stack.insert(next);
                }
            }
        }
    }
}

void Cell::FormulaImpl::InvalidateCache() {
    if (cache_) {
        cache_.reset();
        for (Cell* dep : dependencies_) {
            if (dep) {
                dynamic_cast<FormulaImpl*>(dep->impl_.get())->InvalidateCache();
            }
        }
    }
}
