#ifndef LIGHTDB_OPTIMIZER_H
#define LIGHTDB_OPTIMIZER_H

#include "LightField.h"
#include "Visitor.h"
#include "Configuration.h"
#include "Environment.h"
#include "Plan.h"
#include <limits>
#include <any>

namespace lightdb::optimization {
    class OptimizerRule;
    using rule_vector = std::vector<std::shared_ptr<OptimizerRule>>;

    class Optimizer {
    public:
        explicit Optimizer(const execution::Environment &environment)
            : environment_(environment)
        { }

        const Plan optimize(const LightFieldReference &source,
                            size_t iteration_limit=std::numeric_limits<size_t>::max())
            { return optimize(std::vector<LightFieldReference>{source}, iteration_limit); }
        const Plan optimize(const std::vector<LightFieldReference> &sources,
                            size_t iteration_limit=std::numeric_limits<size_t>::max())
            { return optimize(sources.begin(), sources.end(), iteration_limit); }

        template<typename InputIterator>
        const Plan optimize(InputIterator first, const InputIterator last,
                            size_t iteration_limit=std::numeric_limits<size_t>::max()) {
            Plan plan(environment(), first, last);
            bool modified;
            auto _rules = rules();

            do {
                modified = std::any_of(_rules.begin(), _rules.end(), [this, &plan](auto &rule) {
                    return rule->apply(environment_, plan); });
            } while (modified && iteration_limit--);

            return plan;
        }

        const execution::Environment environment() const noexcept { return environment_; }

    protected:
        virtual const rule_vector rules() = 0;

        template<typename T, typename... _Args>
        auto make_rule(_Args &&... args) { return std::make_shared<T>(args...); }

    private:
        const execution::Environment &environment_;
    };

    class OptimizerRule : public StatefulLightFieldVisitor<bool> {
    public:
        virtual bool apply(const execution::Environment &environment, Plan &plan) {
            current_ = plan;
            return std::any_of(plan.sinks().begin(), plan.sinks().end(), [this](auto &source) {
                return this->accept(source); });
        }

        Plan &plan() { return current_.value(); }
    private:
        std::optional<std::reference_wrapper<Plan>> current_;
    };

}

#endif //LIGHTDB_OPTIMIZER_H
