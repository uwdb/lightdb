#ifndef LIGHTDB_VIDEOPARAMETERSET_H
#define LIGHTDB_VIDEOPARAMETERSET_H

#include "Nal.h"
#include "BitArray.h"
#include "BitStream.h"
#include "Profile.h"
#include <vector>
#include <cassert>


namespace lightdb::hevc {
    class VideoParameterSet : public Nal {
    public:

        /**
         * Interprets data as a byte stream representing a VideoParameterSet
         * @param context The context surrounding the Nal
         * @param data The byte stream
         */
        VideoParameterSet(const StitchContext &context, const bytestring &data)
                : Nal(context, data),
                  data_(data),
                  profile_size_(GetSizeInBits(VPSMaxSubLayersMinus1())) {

                // Note: the reason that we do not remove emulation prevention is that the "SetGeneralLevelIDC" method uses
                // offsets that are calculated with the expectation of the presence of emulation prevention bytes
                CHECK_EQ(profile_size_ % 8, 0);
        }

        /**
         * Sets the general level IDC value in the byte stream to be value, converting value to a byte
         * @param value The new general level IDC value
         */
        inline void SetGeneralLevelIDC(const unsigned char value) {
                // Profile size is in bits, so must be converted to bytes
                data_[kSizeBeforeProfile + profile_size_ / 8 - kGeneralLevelIDCSize] = value;
        }

        /**
         * Returns the general level IDC value
         * @return
         */
        inline unsigned int GetGeneralLevelIDC() const {
                return static_cast<unsigned char>(data_[kSizeBeforeProfile + profile_size_ / 8 - kGeneralLevelIDCSize]);
        }

        /**
         *
         * @return A string with the bytes of this Nal
         */
        inline bytestring GetBytes() const override {
                bytestring bytes(kNalMarker.size() + data_.size());
                copy(kNalMarker.begin(), kNalMarker.end(), bytes.begin());
                copy(data_.begin(), data_.end(), bytes.begin() + kNalMarker.size());
                return bytes;
        }

    private:

        /**
         *
         * @return The value necessary to pass to the GetSizeInBits method to
         * calculate the profile size
         */
        inline unsigned int VPSMaxSubLayersMinus1() const {
                return data_[GetHeaderSize() + kVPSMaxSubLayersMinus1Offset] & kVPSMaxSubLayersMinus1Mask;
        }

        bytestring data_;
        unsigned long profile_size_;

        static constexpr unsigned int kSizeBeforeProfile = 13u - kNalMarkerSize;
        static constexpr unsigned int kVPSMaxSubLayersMinus1Offset = 1u;
        static constexpr unsigned int kVPSMaxSubLayersMinus1Mask = 0x1Cu;
        static constexpr unsigned int kGeneralLevelIDCSize = 1u;

    };
}; //namespace lightdb::hevc

#endif //LIGHTDB_VIDEOPARAMETERSET_H
