#include "Emulation.h"
#include "BitStream.h"
#include "PictureParameterSet.h"
#include "SequenceParameterSet.h"
#include "SliceSegmentLayer.h"

namespace lightdb::hevc {

    void SliceSegmentLayer::SetAddress(const size_t address) {
        address_ = address;
        // Make sure it's byte aligned
        assert (metadata_.GetValue("end") % 8 == 0);

        // The header is the end of the metadata, so move the portion before that into
        // the header array
        auto header_end = metadata_.GetValue("end");
        BitArray header_bits(header_end);
        move(data_.begin(), data_.begin() + header_end, header_bits.begin());

        // Convert the header size from bytes to bits, add the offset
        header_bits[GetHeaderSize() * 8 + kFirstSliceFlagOffset] = address == 0;

        auto address_length = headers_.GetSequence()->GetAddressLength();
        // If the address is 0, we have nothing to insert (it's the first slice)
        if (address) {
            // Note that BitArray has to convert the address to binary and pad it
            header_bits.Insert(metadata_.GetValue("address_offset"), address, address_length);
        }

        if (!headers_.GetPicture()->HasEntryPointOffsets()) {
            auto location = metadata_.GetValue("entry_point_offset");
            if (address) {
                location += address_length;
            }
            // Insert a 0 as an exponential golomb, meaning the one bit '1'
            header_bits.Insert(location, 1, 1);
        }

        header_bits.ByteAlign();
        // The new size of the data is the size of the new header plus the size of the data
        // minus the old header size
        BitArray new_data(header_bits.size() + data_.size() - header_end);
        move(header_bits.begin(), header_bits.end(), new_data.begin());
        move(data_.begin() + header_end, data_.end(), new_data.begin() + header_bits.size());
        data_ = new_data;
    }


    IDRSliceSegmentLayer::IDRSliceSegmentLayer(const Context &context, const bytestring &data, const Headers &headers)
            : SliceSegmentLayer(context, data, headers) {

        GetBitStream().CollectValue("first_slice_segment_in_pic_flag", 1, true);
        GetBitStream().SkipBits(1);
        GetBitStream().SkipExponentialGolomb();
        GetBitStream().MarkPosition("address_offset");
        GetBitStream().SkipExponentialGolomb();
        GetBitStream().SkipBits(2);
        GetBitStream().SkipExponentialGolomb();
        GetBitStream().SkipBits(1);
        GetBitStream().MarkPosition("entry_point_offset");
        GetBitStream().SkipEntryPointOffsets(headers.GetPicture()->HasEntryPointOffsets());
        GetBitStream().CollectValue("trailing_one", 1, true);
        GetBitStream().ByteAlign(0);
        GetBitStream().MarkPosition("end");
    }

    TrailRSliceSegmentLayer::TrailRSliceSegmentLayer(const Context &context, const bytestring &data, const Headers &headers)
            : SliceSegmentLayer(context, data, headers) {

        GetBitStream().CollectValue("first_slice_segment_in_pic_flag", 1, true);
        GetBitStream().SkipExponentialGolomb();
        GetBitStream().MarkPosition("address_offset");
        GetBitStream().SkipExponentialGolomb();
        GetBitStream().SkipBits(headers.GetSequence()->GetMaxPicOrder());
        GetBitStream().SkipTrue();
        GetBitStream().SkipBits(2);
        GetBitStream().SkipFalse();
        GetBitStream().SkipBits(1, headers.GetPicture()->CabacInitPresentFlag());
        GetBitStream().SkipExponentialGolomb();
        GetBitStream().SkipExponentialGolomb();
        GetBitStream().SkipBits(1);
        GetBitStream().MarkPosition("entry_point_offset");
        GetBitStream().SkipEntryPointOffsets(headers.GetPicture()->HasEntryPointOffsets());
        GetBitStream().CollectValue("trailing_one", 1, true);
        GetBitStream().ByteAlign(0);
        GetBitStream().MarkPosition("end");
    }
}; //namespace lightdb::hevc
