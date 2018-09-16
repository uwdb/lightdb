#ifndef LIGHTDB_OPAQUE_H
#define LIGHTDB_OPAQUE_H

#include "Nal.h"

namespace lightdb::hevc {

  class Opaque : public Nal {
  public:

      Opaque(const Context &context, const bytestring &data)
              : Nal(context, data)
      { }

      /**
       *
       * @return A string wtih the bytes of this Nal
       */
      inline bytestring GetBytes() const override {
          auto data = Nal::GetBytes();
          data.insert(data.begin(), kNalMarker.begin(), kNalMarker.end());
          return data;
      }
  };

}; //namespace lightdb::hevc

#endif //LIGHTDB_OPAQUE_H
