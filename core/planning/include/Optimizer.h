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
        explicit Optimizer(execution::Environment environment)
            : environment_(std::move(environment))
        { }

        static const Optimizer &instance()
        {
            if(instance_ != nullptr)
                return *instance_;
            else
                throw InvalidArgumentError("No ambient optimizer specified", "instance");
        }
        template<typename TOptimizer, typename... Args>
        static const Optimizer &instance(Args &&... args) {
            return *(instance_ = std::make_unique<TOptimizer>(args...));
        }


        const Plan optimize(const LightFieldReference &source,
                            size_t iteration_limit=std::numeric_limits<size_t>::max()) const
            { return optimize(std::vector<LightFieldReference>{source}, iteration_limit); }
        const Plan optimize(const std::vector<LightFieldReference> &sources,
                            size_t iteration_limit=std::numeric_limits<size_t>::max()) const
            { return optimize(sources.begin(), sources.end(), iteration_limit); }

        template<typename InputIterator>
        const Plan optimize(InputIterator first, const InputIterator last,
                            size_t iteration_limit=std::numeric_limits<size_t>::max()) const {
            Plan plan(environment(), first, last);
            bool modified;
            auto _rules = rules();
            auto iterations = 0u;

            do {
                modified = std::any_of(_rules.begin(), _rules.end(), [this, &plan](auto &rule) {
                    return rule->apply(environment(), plan); });

                LOG_IF(WARNING, iterations++ == 1000) << "Optimizer modified plan >1000 times; may not converge";
            } while (modified && iteration_limit--);

            return plan;
        }

        inline const execution::Environment &environment() const noexcept { return environment_; }

    protected:
        virtual const rule_vector rules() const = 0;

        template<typename T, typename... Args>
        auto make_rule(Args &&... args) const { return std::make_shared<T>(args...); }

    private:
        static std::unique_ptr<Optimizer> instance_;
        const execution::Environment environment_;
    };

    class OptimizerRule : public StatefulLightFieldVisitor<bool> {
    public:
        virtual bool apply(const execution::Environment &environment, Plan &plan) {
            current_ = plan;
            return std::any_of(plan.sinks().begin(), plan.sinks().end(), [this](auto &source) {
                return this->accept(source); });
        }

        inline Plan &plan() { return current_.value(); }
    private:
        std::optional<std::reference_wrapper<Plan>> current_;
    };

    using OptimizerReference = lightdb::shared_reference<Optimizer>;
}

#endif //LIGHTDB_OPTIMIZER_H
