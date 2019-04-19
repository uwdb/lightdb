#ifndef LIGHTDB_TRANSACTION_H
#define LIGHTDB_TRANSACTION_H

#include "Model.h"
#include "Catalog.h"
#include "Files.h"
#include "Plan.h"
#include "reference.h"

namespace lightdb::transactions {

class OutputStream {
    friend class SingleNodeVolatileTransaction;

public:
    OutputStream(const Transaction &transaction, const catalog::Entry &entry, const Codec &codec)
            : OutputStream(transaction, entry, entry.volume(), entry.geometry(), codec)
    { }

    OutputStream(const Transaction &transaction, const catalog::Entry &entry,
                 CompositeVolume volume, const GeometryReference &geometry, Codec codec)
            : transaction_(transaction),
              entry_(entry),
              filename_(catalog::Files::staging_filename(transaction, entry)),
              codec_(std::move(codec)),
              volume_(std::move(volume)),
              geometry_(geometry),
              stream_(filename_)
    { }

    OutputStream(const Transaction &transaction, const std::filesystem::path &filename,
                 CompositeVolume volume, const GeometryReference &geometry, Codec codec)
            : transaction_(transaction),
              entry_(),
              filename_(catalog::Files::staging_filename(transaction, filename)),
              codec_(std::move(codec)),
              volume_(std::move(volume)),
              geometry_(geometry),
              stream_(filename_)
    { }

    OutputStream(const OutputStream &base, const std::filesystem::path& path,
                 const unsigned int version, const unsigned int index)
            : transaction_(base.transaction_),
              entry_(base.entry_),
              filename_(catalog::Files::stream_filename(path, version, index)),
              codec_(base.codec_),
              volume_(base.volume_),
              geometry_(base.geometry_),
              stream_()
    { }

    OutputStream(const OutputStream&) = delete;
    OutputStream(OutputStream&&) = default;

    std::ofstream& stream() { return stream_; }
    const std::filesystem::path &filename() const { return filename_; }
    const auto &entry() const { return entry_; }
    const auto &volume() const { return volume_; }
    const auto &geometry() const { return geometry_; }
    const auto &codec() const { return codec_; }

private:
    std::filesystem::path destination_filename(unsigned int version) const;

    const Transaction &transaction_;
    const std::optional<catalog::Entry> entry_;
    const std::filesystem::path filename_;
    const Codec codec_;
    const CompositeVolume volume_;
    const GeometryReference geometry_;
    std::ofstream stream_;
};

class Transaction {
public:
    Transaction(const Transaction&) = delete;
    Transaction(Transaction&&) = default;

    virtual ~Transaction() = default;

    unsigned long id() const { return id_; }
    std::vector<OutputStream> &outputs() { return outputs_; }
    const std::vector<OutputStream> &outputs() const { return outputs_; }

    virtual void commit() = 0;
    virtual void abort() = 0;

    virtual OutputStream& write(const logical::StoredLightField &store, const GeometryReference &geometry) {
        return write(store.catalog(), store.name(), store.volume(), geometry, store.codec());
    }
    virtual OutputStream& write(const logical::SavedLightField &save, const GeometryReference &geometry) {
        return write(save.filename(), save.volume(), geometry, save.codec());
    }
    virtual OutputStream& write(const catalog::Catalog &catalog, const std::string &name,
                                const CompositeVolume &volume, const GeometryReference &geometry, const Codec& codec) {
        return outputs_.emplace_back(*this, catalog.get(name, true).downcast<logical::ScannedLightField>().entry(),
                                     volume, geometry, codec);
    }
    virtual OutputStream& write(const catalog::Entry &entry, const std::string &name,
                                const CompositeVolume &volume, const GeometryReference &geometry,const Codec& codec) {
        return outputs_.emplace_back(*this, entry, volume, geometry, codec);
    }
    virtual OutputStream& write(const std::filesystem::path &path, const CompositeVolume &volume,
                                const GeometryReference &geometry,const Codec &codec) {
        return outputs_.emplace_back(*this, path, volume, geometry, codec);
    }

protected:
    explicit Transaction(const unsigned long id)
            : id_(id)
    { }

    static std::recursive_mutex global;

private:
    unsigned long id_;
    std::vector<OutputStream> outputs_;
};

class SingleNodeVolatileTransaction: public Transaction {
public:
    SingleNodeVolatileTransaction()
        : Transaction(0u),
          complete_(false)
    { }

    ~SingleNodeVolatileTransaction() override {
        if(!complete_)
            commit();
    }

    void commit() override;

    void abort() override;

private:
    void write_metadata(const std::filesystem::path&, unsigned int version);

    bool complete_;
};

using TransactionReference = shared_reference<Transaction>;

} // namespace lightdb::transactions

#endif //LIGHTDB_TRANSACTION_H
