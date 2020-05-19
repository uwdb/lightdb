#ifndef LIGHTDB_NALTYPE_H
#define LIGHTDB_NALTYPE_H

namespace lightdb::hevc {

    constexpr unsigned int NalUnitCodedSliceTrailN = 0;
    constexpr unsigned int NalUnitCodedSliceTrailR = 1;

    constexpr unsigned int NalUnitCodedSliceTSAN = 2;
    constexpr unsigned int NalUnitCodedSliceTSAR = 3;

    constexpr unsigned int NalUnitCodedSliceSTSAN = 4;
    constexpr unsigned int NalUnitCodedSliceSTSAR = 5;

    constexpr unsigned int NalUnitCodedSliceRADLN = 6;
    constexpr unsigned int NalUnitCodedSliceRADLR = 7;

    constexpr unsigned int NalUnitCodedSliceRASLN = 8;
    constexpr unsigned int NalUnitCodedSliceRASLR = 9;

    constexpr unsigned int NalUnitReservedVCLN10 = 10;
    constexpr unsigned int NalUnitReservedVCLN11 = 11;
    constexpr unsigned int NalUnitReservedVCLN12 = 12;
    constexpr unsigned int NalUnitReservedVCLN13 = 13;
    constexpr unsigned int NalUnitReservedVCLN14 = 14;
    constexpr unsigned int NalUnitReservedVCLN15 = 15;

    constexpr unsigned int NalUnitCodedSliceBLAWLP = 16;
    constexpr unsigned int NalUnitCodedSliceBLAWRADL = 17;
    constexpr unsigned int NalUnitCodedSliceBLANLP = 18;
    constexpr unsigned int NalUnitCodedSliceIDRWRADL = 19;
    constexpr unsigned int NalUnitCodedSliceIDRNLP = 20;
    constexpr unsigned int NalUnitCodedSliceCRA = 21;
    constexpr unsigned int NalUnitReservedIRAPVCL22 = 22;
    constexpr unsigned int NalUnitReservedIRAPVCL23 = 23;

    constexpr unsigned int NalUnitReservedVCL24 = 24;
    constexpr unsigned int NalUnitReservedVCL25 = 25;
    constexpr unsigned int NalUnitReservedVCL26 = 26;
    constexpr unsigned int NalUnitReservedVCL27 = 27;
    constexpr unsigned int NalUnitReservedVCL28 = 28;
    constexpr unsigned int NalUnitReservedVCL29 = 29;
    constexpr unsigned int NalUnitReservedVCL30 = 30;
    constexpr unsigned int NalUnitReservedVCL31 = 31;

    constexpr unsigned int NalUnitVPS = 32;
    constexpr unsigned int NalUnitSPS = 33;
    constexpr unsigned int NalUnitPPS = 34;
    constexpr unsigned int NalUnitAccessUnitDelimiter = 35;
    constexpr unsigned int NalUnitEOS = 36;
    constexpr unsigned int NalUnitEOB = 37;
    constexpr unsigned int NalUnitFillerData = 38;
    constexpr unsigned int NalUnitPrefixSEI = 39;
    constexpr unsigned int NalUnitSufficSEI = 40;

    constexpr unsigned int NalUnitReservedNVCL41 = 41;
    constexpr unsigned int NalUnitReservedNVCL42 = 42;
    constexpr unsigned int NalUnitReservedNVCL43 = 43;
    constexpr unsigned int NalUnitReservedNVCL44 = 44;
    constexpr unsigned int NalUnitReservedNVCL45 = 45;
    constexpr unsigned int NalUnitReservedNVCL46 = 46;
    constexpr unsigned int NalUnitReservedNVCL47 = 47;

    constexpr unsigned int NalUnitUnspecified48 = 48;
    constexpr unsigned int NalUnitUnspecified49 = 49;
    constexpr unsigned int NalUnitUnspecified50 = 50;
    constexpr unsigned int NalUnitUnspecified51 = 51;
    constexpr unsigned int NalUnitUnspecified52 = 52;
    constexpr unsigned int NalUnitUnspecified53 = 53;
    constexpr unsigned int NalUnitUnspecified54 = 54;
    constexpr unsigned int NalUnitUnspecified55 = 55;
    constexpr unsigned int NalUnitUnspecified56 = 56;
    constexpr unsigned int NalUnitUnspecified57 = 57;
    constexpr unsigned int NalUnitUnspecified58 = 58;
    constexpr unsigned int NalUnitUnspecified59 = 59;
    constexpr unsigned int NalUnitUnspecified60 = 60;
    constexpr unsigned int NalUnitUnspecified61 = 61;
    constexpr unsigned int NalUnitUnspecified62 = 62;
    constexpr unsigned int NalUnitUnspecified63 = 63;
    constexpr unsigned int NalUnitInvalid = 64;
    
} //namespace lightdb::hevc

#endif //LIGHTDB_NALTYPE_H
