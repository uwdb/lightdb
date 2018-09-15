#ifndef LIGHTDB_ACCESSDELIMITER_H
#define LIGHTDB_ACCESSDELIMITER_H

#include "Nal.h"

namespace lightdb {

    class AccessDelimiter : public Nal {
    public:
        AccessDelimiter(const Context &context, bytestring const &data) : Nal(context, data) {}
        bytestring GetBytes() const override;
    };

}

#endif //LIGHTDB_ACCESSDELIMITER_H
