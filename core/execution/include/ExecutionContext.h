#ifndef LIGHTDB_EXECUTIONCONTEXT_H
#define LIGHTDB_EXECUTIONCONTEXT_H

#include "Plan.h"
#include "Transaction.h"
#include "reference.h"

namespace lightdb::optimization {
    class Plan;
}

namespace lightdb::execution {

class ExecutionContext: public std::enable_shared_from_this<ExecutionContext> {
    public:
        ExecutionContext(optimization::Plan, const transactions::TransactionReference&);
        ~ExecutionContext();

        inline const auto &environment() const { return plan_.environment(); }
        inline const auto &plan() const { return plan_; }
        inline auto &allocator() const { return plan_.allocator(); }
        inline auto &transaction() const { return *transaction_; }

    private:
        optimization::Plan plan_;
        const transactions::TransactionReference transaction_;
    };

    using ExecutionContextReference = shared_reference<ExecutionContext>;

    template<typename Transaction>
    ExecutionContextReference make(optimization::Plan plan) {
        return ExecutionContextReference::make<ExecutionContext>(
                plan, transactions::TransactionReference::make<Transaction>());
    }

    class NullExecutionContext: public ExecutionContext {
    public:
        static ExecutionContextReference instance() { return instance_; }

        NullExecutionContext()
            : ExecutionContext(optimization::Plan{NullEnvironment{}, {}},
                               transactions::TransactionReference::make<NullTransaction>())
        { }

    private:
        class NullEnvironment: public Environment {
        public:
            explicit NullEnvironment(): Environment({}) { }
        };

        class NullTransaction: public transactions::Transaction {
        public:
            NullTransaction() : Transaction(0u) { }

            transactions::OutputStream& write(const logical::StoredLightField &store) override {
                LOG(WARNING) << "Writing to null transaction; this is probably not intended";
                return transactions::Transaction::write(store);
            }
            transactions::OutputStream& write(const logical::SavedLightField &save) override {
                LOG(WARNING) << "Writing to null transaction; this is probably not intended";
                return transactions::Transaction::write(save);
            }

            transactions::OutputStream& write(const catalog::Catalog &catalog, const std::string &name, const Codec& codec) override {
                LOG(WARNING) << "Writing to null transaction; this is probably not intended";
                return transactions::Transaction::write(catalog, name, codec);
            }
            transactions::OutputStream& write(const std::filesystem::path &path, const Codec &codec) override {
                LOG(WARNING) << "Writing to null transaction; this is probably not intended";
                return transactions::Transaction::write(path, codec);
            }

            void commit() override {
                LOG(WARNING) << "Committing null transaction";
            }
            void abort() override {
                LOG(WARNING) << "Aborting null transaction";
            }
        };

        static ExecutionContextReference instance_;
    };
} //namespace lightdb::execution

#endif //LIGHTDB_EXECUTIONCONTEXT_H
