#ifndef LIGHTDB_EMULATION_H
#define LIGHTDB_EMULATION_H

#include "BitArray.h"
#include "Nal.h"

namespace lightdb {

        /**
         * Removes the emulation_prevention_three byte from "data" starting at the byte at
         * "start" and ending at "end". Then converts data to a BitArray and returns it. All of data is converted
         * to a bit stream, but only the portion between [start, end) is modified by the removal of
         * the three byte
         * @param data The byte stream to be modified
         * @param start The start of the emulation prevention removal, in bytes
         * @param end The end of the removal, in bytes. Removal does not affect the byte at end
         * @return A bit stream representing the entirety of data, with the emulation_prevention_three
         * bytes removed
         */
        utility::BitArray RemoveEmulationPrevention(const bytestring &data, const unsigned long start, const unsigned long end);

        /**
         * Adds the emulation_prevention_three byte to "data" starting at the byte at
         * "start" and ending at "end". Then converts data to a string of bytes and returns it, adding the
         * kNalMarker at the front. All of data is converted to a byte stream, but only
         * the portion starting at [start, end) is modified by the addition of the three byte
         * @param data The bit stream to be modified
         * @param start The start of the emulation prevention addition, in bytes
         * @param end The end of the removal, in bytes. Addition does not affect the byte at end
         * @return A string of bytes representing the entirety of data with the nal marker
         * at the front, with the emulation_prevention_three bytes added
         */
        bytestring AddEmulationPreventionAndMarker(const utility::BitArray data, const unsigned long start, const unsigned long end);

}
#endif //LIGHTDB_EMULATION_H
