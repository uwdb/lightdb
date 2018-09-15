

#ifndef LIGHTDB_OPAQUE_H
#define LIGHTDB_OPAQUE_H

#include <string>
#include "Nal.h"

namespace lightdb {

  class Opaque : public Nal {
  public:

      Opaque(const Context &context, const bytestring &data) : Nal(context, data) {}

      /**
       *
       * @return A string wtih the bytes of this Nal
       */
      bytestring GetBytes() const override;

  };

}

#endif //LIGHTDB_OPAQUE_H
