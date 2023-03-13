#define MKSEGID(device,focal,detector,module) ((((device<<6 | focal) << 6) | detector)<<8 | module)

// Device or ExpGroup   0--63
#define BIGRIPS    0
#define ZeroDegree 1
#define SAMURAI    2
#define SHARAQ     11
#define DALI       20
#define GAMMAGR    21
#define BETAGR     22
#define RADIIGR    23
#define SAMURAIGR  24
#define USERGR     57
#define DAQ        58  // for special case

// Focal plane  0--63
#define F1   1
#define F2   2
#define F3   3
#define F4   4
#define F5   5
#define F6   6
#define F7   7
#define F8   8
#define F9   9
#define F10  10
#define F11  11
#define F12  12
#define F13  13
/* 20--21, 62 for SHARAQ */
#define FDAQ 58       // for special case 
#define B3F  63

// For SHARAQ
#define H7  7
#define H8  8
#define H9  9
#define H10 10
#define S0  20
#define S1  21
#define S2  22
#define SHB3F 62

// Detector  0--63
#define SCALER   63
#define ISCALER  62
#define B2SCALER 61
#define TS       60
#define DTMON    59
#define DDAQ     58       // for special case 
#define ICGAS    57
#define RF       50
#define RF2      51
#define PPACQ    0
#define PPACT    1
#define PLAQ     2
#define PLAT     3
#define ICE      4
#define ICT      5
#define SSDE     6
#define SSDT     7
#define ISGEA    8
#define ISGET    9
#define COIN    10
#define NAIE    11
#define TOTE    11
#define PILEUP  12
#define STOPPLA 13
#define DUPPLAT 14
#define PLAQ2   15
#define NAIT    16
#define TOTQ    17
#define LABRA   18
#define LABRT   19
// 20
#define DALIA   21
#define DALIT   22
#define BETAA   23
#define BETAT   24
#define HIA     25
#define CLOVERA 26
#define CLOVERT 27
#define VETOA   28
#define VETOT   29
#define TECHNO  30
//
// For SHARAQ
#define SHBLD     31
#define SHCRDC    32
#define SHCRDCA   33
#define SHCRDCC   34
#define SHCRDCT1  35
#define SHCRDCT2  36
#define SHCRDCA2  37
#define SHCRDCA3  38

// For Must2
//#define MUST2DET 50
#define PLAYOBI  50

//// for gamma11
//#define GAMMAPLAT 52

// For BigRIPS development
#define PLATC     60

// Module  0--255
#define C16        0
#define C24        1
#define AD413AC    2    // AD413A + w/o Zero supp.   (CAMAC read)
#define AD413ACZ   3    // AD413A + Zero suppression (CAMAC read)
#define AD413AM    4    // AD413A + w/o Zero supp.   + Memory
#define AD413A     5    // AD413A + Zero suppression + Memory
#define AD413AMZ   5    // AD413A + Zero suppression + Memory
#define L3377      6    // LeCroy 3377 Double word (CAMAC read)
#define L3377CW    6    // LeCroy 3377 Double word (CAMAC read)
#define L3377CS    7    // LeCroy 3377 Single word (CAMAC read)
#define P7166      9    // Philips 7166 + w/o Zero supp.
#define P716X      9    // Philips 7166 + w/o Zero supp.
#define P716XZ    10    // Philips 7166 + Zero suppression
#define CTM405    11
#define L3377MW   16    // LeCroy 3377 Double word   + Memory
#define L3377MS   17    // LeCroy 3377 Single word   + Memory
#define L4300BC   11    // LeCroy 4300B + w/o Zero supp.   (CAMAC read)
#define L4300BCZ  12    // LeCroy 4300B + Zero suppression (CAMAC read)
#define L4300BM   13    // LeCroy 4300B + w/o Zero supp.   + Memory
#define L4300BMZ  14    // LeCroy 4300B + Zero suppression + Memory
#define V775      21
#define V785      21
#define V792      21
#define V550      22
#define V767      23
#define V1190     24
#define V1190A    24
#define V1190B    24
#define V1290     25
#define V1190C    26
#define DSP       29
#define A3100     31
#define MADC32    32    // Mesytech ADC
#define LUPOIO      41
#define LUPOTS      42
#define LUPOMTS     43  // for dead time monitor
#define MUST2VXI    50
#define MDAQ        58  // for special case
#define UNDEFINE8   8
#define UNDEFINE15  15
#define UNDEFINE18  18
#define UNDEFINE19  19
#define UNDEFINE20  20
#define UNDEFINE27  27
#define UNDEFINE28  28
#define UNDEFINE30  30
#define UNDEFINE33  33
#define UNDEFINE34  34
#define UNDEFINE35  35
#define UNDEFINE36  36

#define V2740 51 //ogio added

// ScalerID 0--65535
#define DTMONSCALER    1
#define CCNET01SCALER 21
#define CCNET02SCALER 22
#define CCNET03SCALER 23
#define CCNET04SCALER 24
#define CCNET05SCALER 25
#define CCNET06SCALER 26
#define CCNET07SCALER 27
#define F7DAQSCALER   31
#define SSMSCALER     33
#define F11DAQSCALER  34
#define F11GESCALER   35
#define F3DAQSCALER   36
#define GGDAQ01SCALER 41
//
#define SHSSMSCALER   100
