#include <memory>
#include "progress.h"
#include "tqdm.h"

namespace lightdb {

Progress::Progress(const int total, const std::string &label)
    : bar_(std::make_unique<tqdm>()), current_(0), total_(total) {
    this->label(label);
    this->total(total);
}

Progress::~Progress() {
    finish();
}


Progress& Progress::operator++() {
    display(++current_, "", total_);
    return *this;
}

Progress& Progress::operator++(int) { return operator++(); }

void Progress::label(const std::string &label) { bar_->set_label(label); }
void Progress::total(const int total) { bar_->progress(current_, total_ = total); }
void Progress::reset() { bar_->reset(); current_ = total_ = 0; }
void Progress::finish() { bar_->finish(); current_ = total_; }

void Progress::display(const int current, const std::string &label, const int total) {
    if(label.length() > 0)
        this->label(label);
    bar_->progress(current_ = current, total >= 0 ? total_ = total : total_);
}

} // namespace lightdb
