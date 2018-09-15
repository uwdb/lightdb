//
// Created by sophi on 4/26/2018.
//

#ifndef LIGHTDB_ACCESSDELIMITER_H
#define LIGHTDB_ACCESSDELIMITER_H

#include <string>
#include "Nal.h"


namespace lightdb {

    class AccessDelimiter : public Nal {
    public:
        AccessDelimiter(const Context &context, bytestring const &data) : Nal(context, data) {}
        bytestring GetBytes() const override;
    };

}

#endif //LIGHTDB_ACCESSDELIMITER_H
