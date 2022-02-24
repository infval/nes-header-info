#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#ifdef _WIN32
#define WINDOWS_ENCODING
#endif

#ifdef WINDOWS_ENCODING
#include <windows.h>
#include <shellapi.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "hash/md5.h"
#include "hash/sha1.h"


#define NES_HEADER_INFO_VER "1.0"


// NES 2.0 XML Database

uint8_t* g_nes20db = NULL;
size_t g_nes20db_size = 0;

void OpenNES20DB(void);
void CloseNES20DB(void);
void PrintNES20DB(const char* hash);
uint8_t* bytes_find(
    const uint8_t* data,
    size_t data_len,
    const uint8_t* sub,
    size_t sub_len);
uint8_t* bytes_rfind(
    const uint8_t* data,
    size_t data_len,
    const uint8_t* sub,
    size_t sub_len);

// Misc.

long GetFILESize(FILE* file);
void CRC32Init(void);
uint32_t CRC32(const uint8_t* message, size_t count);
void SHA1_to_hex(const uint8_t hash[20], char str[41]);

#ifdef __EMSCRIPTEN__
EM_JS(void, Print, (const char* str), {
    romInfoElement.textContent += UTF8ToString(str);
})
#else
#define Print(str) printf("%s", str)
#endif

// Const

#define HEADER_SIZE     16
#define TRAINER_SIZE    512
#define MIN_FILE_SIZE   HEADER_SIZE
#define EXPANSION_COUNT 54
#define MAPPER_COUNT    256

const char* FrameTiming[] = {
    "NTSC", "PAL", "Multiple-region", "Dendy"
};

const char* ConsoleType1[] = {
    "NES/Famicom/Dendy", "Vs. System", "Playchoice 10", "Extended"
};

const char* ConsoleType2[] = {
    "NES/Famicom/Dendy", "Vs. System", "Playchoice 10", "Famiclone with Decimal Mode",
    "VT01 Monochrome", "VT01 Red/Cyan", "VT02", "VT03", "VT09", "VT32", "VT369",
    "UM6578", "PocketNES",
    "N/A", "N/A", "N/A"
};

const char* VSFlags[] = {
    "Normal", "RBI Baseball", "TKO Boxing", "Super Xevious", "Vs. Ice Climber Japan",
    "Vs. Dual", "Raid on Bungeling Bay",
    "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A", "N/A"
};

const char* VSPPUs[] = {
    "RP2C03B", "RP2C03G", "RP2C04-0001", "RP2C04-0002", "RP2C04-0003", "RP2C04-0004",
    "RC2C03B", "RC2C03C", "RC2C05-01", "RC2C05-02", "RC2C05-03", "RC2C05-04", "RC2C05-05",
    "N/A", "N/A", "N/A"
};

const char* ExpansionDevices[EXPANSION_COUNT + 1] = {
    "Unspecified",
    "Standard Controllers",
    "NES Four Score/Satellite",
    "Famicom Four Players Adapter",
    "Vs. System",
    "Vs. System with reversed inputs",
    "Vs. Pinball (Japan)",
    "Vs. Zapper",
    "Zapper",
    "Two Zappers",
    "Bandai Hyper Shot",
    "Power Pad Side A",
    "Power Pad Side B",
    "Family Trainer Side A",
    "Family Trainer Side B",
    "Arkanoid Paddle (NES)",
    "Arkanoid Paddle (Famicom)",
    "Two Arkanoid Paddles plus Data Recorder",
    "Konami Hyper Shot",
    "Coconuts Pachinko Controller",
    "Exciting Boxing Punching Bag",
    "Jissen Mahjong Controller",
    "Party Tap",
    "Oeka Kids Tablet",
    "Sunsoft Barcode Battler",
    "Miracle Piano Keyboard",
    "Pokkun Moguraa",
    "Top Rider",
    "Double-Fisted",
    "Famicom 3D System",
    "Doremikko Keyboard",
    "R.O.B. Gyro Set",
    "Famicom Data Recorder",
    "ASCII Turbo File",
    "IGS Storage Battle Box",
    "Family BASIC Keyboard",
    "Dongda PEC-586 Keyboard",
    "Bit Corp. Bit-79 Keyboard",
    "Subor Keyboard",
    "Subor Keyboard plus 3x8-bit mouse",
    "Subor Keyboard plus 24-bit mouse",
    "SNES Mouse",
    "Multicart",
    "Two SNES Controllers",
    "RacerMate Bicycle",
    "U-Force",
    "R.O.B. Stack-Up",
    "City Patrolman Lightgun",
    "Sharp C1 Cassette Interface",
    "Standard Controllers (inverted directions)",
    "Sudoku (Excalibur)",
    "ABL Pinball",
    "Golden Nugget Casino",
    "Golden Keyboard",

    "Unknown"
};

// Source: NintendulatorNRS, FCEUX, wiki.nesdev
const char* MapperNames[MAPPER_COUNT] = {
/*   0 */  "Nintendo NROM"
/*   1 */, "Nintendo SxROM (MMC1B+)"
/*   2 */, "Nintendo UxROM"
/*   3 */, "Nintendo CNROM"
/*   4 */, "Nintendo TxROM/HKROM (MMC3)"
/*   5 */, "Nintendo ExROM (MMC5)"
/*   6 */, "Front Fareast Magic Card 1M/2M"
/*   7 */, "Nintendo AxROM"
/*   8 */, "! Mapper 6"
/*   9 */, "Nintendo PNROM (MMC2)"
/*  10 */, "Nintendo FJROM/FKROM (MMC4)"
/*  11 */, "Color Dreams"
/*  12 */, "Gouder SL-5020B/Front Fareast Magic Card 4M"
/*  13 */, "Nintendo CPROM"
/*  14 */, "Gouder SL-1632"
/*  15 */, "K-1029"
/*  16 */, "Bandai FGC-1/2 or LZ93D50 with 24C02 EEPROM"
/*  17 */, "Front Fareast Super Magic Card"
/*  18 */, "Jaleco SS8806 (SS 880006)" // JF-NNX (EB89018-30007) boards
/*  19 */, "Namco N129/N163. Incorrectly - Namco 106"
/*  20 */, "! FDS"
/*  21 */, "Konami VRC4a/VRC4c"
/*  22 */, "Konami 351618 (VRC2a)"
/*  23 */, "Konami VRC2b/VRC4e/VRC4f"
/*  24 */, "Konami 351951 (VRC6a)"
/*  25 */, "Konami VRC2c/VRC4b/VRC4d"
/*  26 */, "Konami 351949A (VRC6b)"
/*  27 */, "CC-21 (Mi Hun Che)" // Former dupe for VRC2/VRC4 mapper, redefined with crc to mihunche boards
/*  28 */, "Action 53"
/*  29 */, "RET-CUFROM"
/*  30 */, "UNROM-512"
/*  31 */, "NSF (InfiniteNESLives), 2A03 Puritans Album"
/*  32 */, "Irem G-101"
/*  33 */, "Taito TC0190/TC0390"
/*  34 */, "AVE NINA-001/Nintendo BNROM"
/*  35 */, "J.Y. Company EL870914C, ! Mapper 209" // Warioland II
/*  36 */, "TXC 01-22000-200/400"
/*  37 */, "Nintendo ZZ" // PAL-ZZ SMB/TETRIS/NWC
/*  38 */, "Bit Corp. PCI556"
/*  39 */, "! Mapper 241, Study & Game 32-in-1"
/*  40 */, "NTDEC 2722" // SMB2j [FDS Conv]
/*  41 */, "NTDEC 2399" // CALTRON 6-in-1
/*  42 */, "AC08/LH09, Kaiser KS-7050/Kaiser KS-018/Mario Baby"
/*  43 */, "TONY-I, YS-612" // SMB2j [FDS Conv]
/*  44 */, "Super HiK 7-in-1 (MMC3)"
/*  45 */, "GA23C/SFC-011B (MMC3)"
/*  46 */, "GameStation/RumbleStation"
/*  47 */, "Nintendo NES-QJ"
/*  48 */, "Taito TC0190+PAL16R4/TC0690"
/*  49 */, "1993 Super HiK 4-in-1 (MMC3)"
/*  50 */, "N-32 (761214)" // SMB2j (FDS Conv Rev. A)
/*  51 */, "820718C (11-in-1 Ball Games)" // 1993 year version
/*  52 */, "Realtec 8213 (MMC3)"
/*  53 */, "Supervision 16-in-1"
/*  54 */, "! Mapper 201"
/*  55 */, "NCN-35A, UNIF: BTL-MARIO1-MALEE2"
/*  56 */, "Kaiser SMB3" // Super Mario Bros. 3 (unlicensed reproduction)
/*  57 */, "GK 6-in-1" // SIMBPLE BMC Pirate A
/*  58 */, "GK-192" // SIMBPLE BMC Pirate B
/*  59 */, "BS-01/VT1512A, UNIF: BMC-T3H53, BMC-D1038" // Mapper 60?
/*  60 */, "Reset-based NROM-128" // SIMBPLE BMC Pirate C"
/*  61 */, "GS-2017 (20-in-1 KAISER Rev. A)"
/*  62 */, "N-190B (700-in-1)"
/*  63 */, "NTDEC 2291 (Powerful 250-in-1)"
/*  64 */, "Tengen 800032 (RAMBO-1)"
/*  65 */, "Irem H-3001"
/*  66 */, "Nintendo GNROM/MHROM (GxROM)"
/*  67 */, "Sunsoft-3"
/*  68 */, "Sunsoft-4"
/*  69 */, "Sunsoft-5 (Sunsoft 5A/5B, Sunsoft FME-7)"
/*  70 */, "Bandai UOROM" // BA KAMEN DISCRETE
/*  71 */, "BIC BF9093/BF9097 (Camerica)"
/*  72 */, "Jaleco JF-17"
/*  73 */, "Konami VRC3"
/*  74 */, "43-393/860908C (MMC3)" // TW MMC3+VRAM Rev. A
/*  75 */, "Konami VRC1"
/*  76 */, "Namco 3446 (Namcot 108 Rev. A)"
/*  77 */, "Irem LROG017"
/*  78 */, "Jaleco JF-16/Irem IF-12 (74HC161/32)"
/*  79 */, "AVE NINA-003-006, C&E/TXC Board"
/*  80 */, "Taito P3-33/34/36 (X1-005 Rev. A)"
/*  81 */, "NTDEC N715021"
/*  82 */, "Taito P3-044 (wrong PRG order) (X1-017)"
/*  83 */, "Cony (Yoko, VRC Rev. B)"
/*  84 */, "! Unknown"
/*  85 */, "Konami VRC7"
/*  86 */, "Jaleco JF-13"
/*  87 */, "Jaleco/Konami CNROM (74*139/74 Discrete)"
/*  88 */, "Namco 3433"
/*  89 */, "Sunsoft-2"
/*  90 */, "J.Y. Company EL861226C"
/*  91 */, "EJ-006-1 / J.Y. Company YY830624C/JY830848C"
/*  92 */, "Jaleco JF-19"
/*  93 */, "Sunsoft-3R" // SUNSOFT-2 mapper with VRAM, different wiring
/*  94 */, "Nintendo UN1ROM"
/*  95 */, "Namco 3425 (Namcot 108 Rev. B)"
/*  96 */, "Oeka Kids (Bandai)"
/*  97 */, "Irem TAM-S1"
/*  98 */, "! Unknown"
/*  99 */, "Nintendo Vs. System"
/* 100 */, "Nesticle MMC3, ! Internal"
/* 101 */, "Jaleco/Konami CNROM with wrong bit order, !"
/* 102 */, "! Mapper 284"
/* 103 */, "Whirlwind Manu LH30" // Doki Doki Panic [FDS Conv]
/* 104 */, "Pegasus 5-in-1"
/* 105 */, "NES-EVENT" // NWC1990 (Nintendo)
/* 106 */, "890418 (SMB3 Pirate A)"
/* 107 */, "Magic Dragon (Magicseries Corp.)"
/* 108 */, "DH-08, UNIF: UNL-BB" // [FDS Conv]
/* 109 */, "! Mapper 137"
/* 110 */, "! Mapper 243"
/* 111 */, "GTROM (Cheapocabra)/Chinese MMC1"
/* 112 */, "NTDEC MMC3" // ASDER Board
/* 113 */, "HES 4-in-1" // HACKER/Sachen Board
/* 114 */, "6122 (MMC3)" // (SuperGame, Hosenkan)
/* 115 */, "Ka Sheng SFC-02B/-03/-004 (MMC3)"
/* 116 */, "Huang-1/2, Gouder SOMARI-P" // MMC1/MMC3/VRC2 Pirate
/* 117 */, "Future Media"
/* 118 */, "Nintendo TKSROM/TLSROM (TxSROM)"
/* 119 */, "Nintendo TQROM"
/* 120 */, "FDS Tobidase Daisakusen" // [FDS Conv]
/* 121 */, "Ka Sheng A9711/A9713 (MMC3)"
/* 122 */, "! Mapper 184"
/* 123 */, "Ka Sheng H2288 (MMC3)"
/* 124 */, "Super Game Mega Type III"
/* 125 */, "Whirlwind Manu LH32, UNIF: UNL-LH32" // [FDS Conv]
/* 126 */, "SuperJoy" // MMC3/NROM/CNROM Multicart
/* 127 */, "Double Dragon II - The Revenge (Pirate)"
/* 128 */, "1994 Super HiK 4-in-1 (Pirate)"
/* 129 */, "! Mapper 58"
/* 130 */, "! Mapper 331"
/* 131 */, "! Mapper 205"
/* 132 */, "TXC 01-22003-400/01-22111-100/01-22270-000"
/* 133 */, "Sachen 3009/72008"
/* 134 */, "WX-KB4K/T4A54A/BS-5652 (MMC3)"
/* 135 */, "! Mapper 141, TC-021A"
/* 136 */, "Sachen 3011/SA-002"
/* 137 */, "Sachen SA8259D"
/* 138 */, "Sachen SA8259B"
/* 139 */, "Sachen SA8259C"
/* 140 */, "Jaleco GNROM (JF-11/14)"
/* 141 */, "Sachen 2M-RAM-COB (S8259A)"
/* 142 */, "Kaiser KS-7032" // [FDS Conv]
/* 143 */, "Sachen TC-A001-72P/SA-014"
/* 144 */, "AGCI-50282"
/* 145 */, "Sachen SA-72007"
/* 146 */, "! Mapper 79, Sachen 3015/SA-016"
/* 147 */, "Sachen 3018, TC-011 Chinese Kungfu, TCU01"
/* 148 */, "Sachen SA-008-A, Tengen 800008, SA0037"
/* 149 */, "Sachen SA-0036"
/* 150 */, "Sachen SA-015/SA-630, S74LS374N"
/* 151 */, "! Mapper 75"
/* 152 */, "Bandai UOROM 1SM" // Arkanoid 2 (J), Gegege no Kitarou 2
/* 153 */, "Bandai LZ93D50 with 8 KiB WRAM"
/* 154 */, "Namco 3453"
/* 155 */, "Nintendo SxROM (MMC1A)"
/* 156 */, "Open Corp., Daou Infosys DIS23C01 DAOU ROM CONTROLLER"
/* 157 */, "Bandai Datach Joint ROM System"
/* 158 */, "Tengen 800037"
/* 159 */, "Bandai LZ93D50 with 24C01 EEPROM"
/* 160 */, "! Mapper 90"
/* 161 */, "! Mapper 1"
/* 162 */, "Waixing FS304"
/* 163 */, "Nanjing FC-001"
/* 164 */, "Yancheng cy2000-3, Dongda PEC-9588"
/* 165 */, "Fire Emblem (Pirate)" // MMC2*MMC3
/* 166 */, "Subor Type A"
/* 167 */, "Subor Type B"
/* 168 */, "Racermate Challenge 2"
/* 169 */, "??? (Yuxing)"
/* 170 */, "Fujiya NROM" // Shiko Game Syu
/* 171 */, "Kaiser KS-7058"
/* 172 */, "Super Mega SMCYII-900, Super Mega P-4070 (TXC)"
/* 173 */, "Idea-Tek ET.xx"
/* 174 */, "NTDEC 5-in-1"
/* 175 */, "Kaiser KS-122" // 15-in-1
/* 176 */, "Waixing FS005/FS006/FK23C(A), UNIF: BMC-Super24in1SC03"
/* 177 */, "Hengge Dianzi"
/* 178 */, "Waixing FS305/Nanjing NJ0430/PB030703-1x1"
/* 179 */, "! Mapper 176"
/* 180 */, "Reverse UNROM (Nichibutsu)" // Crazy Climber
/* 181 */, "! Mapper 185"
/* 182 */, "! Mapper 114, YH-001"
/* 183 */, "Shui Guan Pipe/Suikan Pipe (Pirate)"
/* 184 */, "Sunsoft-1"
/* 185 */, "Nintendo CNROM+Security"
/* 186 */, "Family Study Box by Fukutake Shoten" // BIOS
/* 187 */, "Ka Sheng A98402"
/* 188 */, "Karaoke Studio (Bandai)"
/* 189 */, "TXC 01-22017-000/01-22018-400"
/* 190 */, "Karaoke Studio (Zemina)"
/* 191 */, "Pirate TQROM variant (Waixing)"
/* 192 */, "Waixing FS308 (MMC3)" // TW MMC3+VRAM Rev. B
/* 193 */, "NTDEC 2394, TC-112" // War in the Gulf
/* 194 */, "Waixing FS30x (MMC3)" // TW MMC3+VRAM Rev. C
/* 195 */, "Waixing FS303 (MMC3)" // TW MMC3+VRAM Rev. D
/* 196 */, "MRCM UT1374 (MMC3)"
/* 197 */, "TLROM-512 (MMC3)"
/* 198 */, "TNROM-640 (MMC3)" // TW MMC3+VRAM Rev. E (Waixing)
/* 199 */, "TNROM with secondary WRAM (MMC3)" // Waixing
/* 200 */, "36-in-1, 1200-in-1"
/* 201 */, "8-in-1, 21-in-1 (2006-CA)"
/* 202 */, "SP60 150-in-1"
/* 203 */, "35-in-1"
/* 204 */, "4-in-1, 64-in-1, 80-in-1, 150-in-1"
/* 205 */, "JC-016-2. 3-in-1, 15-in-1"
/* 206 */, "Namco N118, Tengen MIMIC-1, DxROM"
/* 207 */, "Taito Ashura (X1-005 Rev. B)"
/* 208 */, "Street Fighter IV (Gouder)"
/* 209 */, "J.Y. Company YY850629C"
/* 210 */, "Namco N175/N340"
/* 211 */, "J.Y. Company EL860339C"
/* 212 */, "CS669, BMC Super HiK 300-in-1"
/* 213 */, "! Mapper 58"
/* 214 */, "Super Gun 20-in-1"
/* 215 */, "Realtec 823x(A), UNIF: UNL-8237, UNL-8237A"
/* 216 */, "Bonza (RCM Group)"
/* 217 */, "GI 9549. 500-in-1, 2000-in-1"
/* 218 */, "Magic Floor (Homebrew)"
/* 219 */, "Ka Sheng A9746, A9461"
/* 220 */, "! Debug Mapper"
/* 221 */, "NTDEC N625092"
/* 222 */, "810343-C, CTC-31"
/* 223 */, "! Mapper 199"
/* 224 */, "Jncota KT-008"
/* 225 */, "ET-4310/K-1010"
/* 226 */, "Tsang Hai 16 Mib, BMC 22+20-in-1, 76-in-1, Super 42-in-1"
/* 227 */, "Waixing FW01, 810449-C-A1"
/* 228 */, "Action 52, Cheetahmen II (Active Enterprises)"
/* 229 */, "31-in-1"
/* 230 */, "CTC-43A, BMC Contra+22-in-1"
/* 231 */, "20-in-1"
/* 232 */, "BIC BF9096, BMC Quattro (Camerica)"
/* 233 */, "Reset-based Tsang Hai 4+4 Mib, BMC 22+20-in-1 RST, 42-in-1"
/* 234 */, "D-1012, BMC Maxi"
/* 235 */, "Golden Game modular multicart"
/* 236 */, "Realtec 8031/8099/8106/8155"
/* 237 */, "Teletubbies 420-in-1"
/* 238 */, "Sakano MMC3, UNL6035052, Contra Fighter (Pirate)"
/* 239 */, "! Unknown"
/* 240 */, "C&E Shenghuo Liezhuan" // Jing Ke Xin Zhuan, Sheng Huo Lie Zhuan (C.&E., Sup.Ele./Hummer)
/* 241 */, "BNROM with WRAM (TXC)"
/* 242 */, "Waixing 43272"
/* 243 */, "Sachen SA-020A, S74LS374NA"
/* 244 */, "C&E Decathlon"
/* 245 */, "Waixing FS003"
/* 246 */, "C&E Fengshengban" // Feng Shen Bang (C.&E.)
/* 247 */, "! Unknown"
/* 248 */, "! Mapper 115"
/* 249 */, "Waixing MMC3 (TKROM clone)"
/* 250 */, "Nitra L4015 (MMC3)"
/* 251 */, "! Mapper 45"
/* 252 */, "Waixing Sangokushi (VRC4)"
/* 253 */, "Waixing Dragon Ball Z (VRC4)"
/* 254 */, "Pikachu Y2K"
/* 255 */, "! Mapper 225"
};

const char* MakerNames[256] = {
/* 00 */  "None"
/* 01 */, "Nintendo" // Nintendo R&D1
/* 02 */, "Rocket Games/Ajinomoto" // Unknown
/* 03 */, "Imagineer-Zoom" // Unknown
/* 04 */, "Gray Matter" // Unknown
/* 05 */, "Zamuse" // Unknown
/* 06 */, "Falcom" // Unknown
/* 07 */, "Enix"
/* 08 */, "Capcom"
/* 09 */, "Hot-B" // Hot B Co., Ltd.
/* 0A */, "Jaleco"
/* 0B */, "Coconuts Japan"
/* 0C */, "Coconuts Japan/G.X.Media" // Unknown
/* 0D */, "Micronet" // Unknown
/* 0E */, "Technos" // Unknown
/* 0F */, "Mebio Software" // Unknown
/* 10 */, ""
/* 11 */, "Starfish" // Unknown
/* 12 */, "Infocom" // Unknown
/* 13 */, "Electronic Arts Japan" // Unknown
/* 14 */, ""
/* 15 */, "Cobra Team" // Unknown
/* 16 */, "Human/Field" // Unknown
/* 17 */, "KOEI" // Unknown
/* 18 */, "Hudson Soft" // Hudson Soft Japan
/* 19 */, "S.C.P./Game Village/B-AI" // Unknown
/* 1A */, "Yanoman" // Yonoman??? // Unknown
/* 1B */, ""
/* 1C */, "Tecmo Products" // Unknown
/* 1D */, "Japan Glary Business" // Unknown
/* 1E */, "Forum/OpenSystem" // Unknown
/* 1F */, "Virgin Games (Japan)" // Unknown
/* 20 */, "Destination Software/KSS" // Unknown
/* 21 */, "Sunsoft/Tokai Engineering" // Unknown
/* 22 */, "POW (Planning Office Wada)/VR 1 Japan" // Unknown
/* 23 */, "Micro World" // Unknown
/* 24 */, "PCM Complete" // Unknown
/* 25 */, "San-X" // Unknown
/* 26 */, "Enix" // Unknown
/* 27 */, "Loriciel/Electro Brain" // Unknown
/* 28 */, "Kemco Japan" // Unknown
/* 29 */, "SETA" // Seta Co., Ltd.
/* 2A */, "Culture Brain" // Unknown
/* 2B */, ""
/* 2C */, "Palsoft" // Unknown
/* 2D */, "Visit Co., Ltd." // Unknown
/* 2E */, "Intec" // Unknown
/* 2F */, "System Sacom" // Unknown
/* 30 */, "Viacom" // Unknown
/* 31 */, "Carrozzeria" // Nintendo??? // Unknown
/* 32 */, "Dynamic" // Bandai??? // Unknown
/* 33 */, "Ocean/Acclaim" // Unknown
/* 34 */, "Magifact" // Konami??? // Unknown
/* 35 */, "Hect (Hector)"
/* 36 */, "Codemasters" // Unknown
/* 37 */, "Taito/GAGA Communications" // Unknown
/* 38 */, "Laguna" // Hudson??? // Unknown
/* 39 */, "Telstar Fun & Games/Event/Taito" // Banpresto??? // Unknown
/* 3A */, ""
/* 3B */, "Arcade Zone Ltd." // Unknown
/* 3C */, "Entertainment International/Empire Software" // Unknown
/* 3D */, "Loriciel" // Unknown
/* 3E */, "Gremlin Graphics" // Unknown
/* 3F */, "K.Amusement Leasing Co."
/* 40 */, "Seika Corp." // Unknown
/* 41 */, "UBI SOFT Entertainment Software" // Unknown
/* 42 */, "Sunsoft US" // Atlus??? // Unknown
/* 43 */, ""
/* 44 */, "Life Fitness" // Malibu??? // Unknown
/* 45 */, ""
/* 46 */, "System 3" // angel??? // Unknown
/* 47 */, "Spectrum Holobyte" // Bullet-Proof??? // Unknown
/* 48 */, ""
/* 49 */, "TOSE/IREM"
/* 4A */, "Gakken/Interlink"
/* 4B */, "Raya Systems" // Unknown
/* 4C */, "Renovation Products" // Unknown
/* 4D */, "Malibu Games" // Unknown
/* 4E */, ""
/* 4F */, "Eidos/U.S. Gold" // Unknown
/* 50 */, "Absolute Entertainment" // Unknown
/* 51 */, "Acclaim" // Unknown
/* 52 */, "Activision" // Unknown
/* 53 */, "American Sammy" // Unknown
/* 54 */, "Take 2/GameTek" // Konami??? // Take-Two Interactive | 2K Games // Unknown
/* 55 */, "Hi Tech Entertainment" // Unknown
/* 56 */, "LJN Ltd." // Unknown
/* 57 */, "Matchbox" // Unknown
/* 58 */, "Mattel" // Unknown
/* 59 */, "Milton Bradley" // Unknown
/* 5A */, "Mindscape/Red Orb Entertainment" // Unknown
/* 5B */, "Romstar" // Unknown
/* 5C */, "Taxan" // Unknown
/* 5D */, "Midway/Tradewest" // Unknown
/* 5E */, ""
/* 5F */, "Kyugo Boueki" // American Softworks Corp.???
/* 60 */, "Titus" // Unknown
/* 61 */, "Virgin Interactive" // Unknown
/* 62 */, "Maxis" // Unknown
/* 63 */, ""
/* 64 */, "LucasArts Entertainment" // Unknown
/* 65 */, ""
/* 66 */, ""
/* 67 */, "Ocean" // Unknown
/* 68 */, ""
/* 69 */, "Electronic Arts" // Unknown
/* 6A */, ""
/* 6B */, "Laser Beam" // Unknown
/* 6C */, ""
/* 6D */, ""
/* 6E */, "Elite Systems" // Unknown
/* 6F */, "Electro Brain" // Unknown
/* 70 */, "Infogrames" // Unknown
/* 71 */, "Interplay" // Unknown
/* 72 */, "JVC (US)" // Broderbund??? // Unknown
/* 73 */, "Parker Brothers" // sculptured??? // Unknown
/* 74 */, ""
/* 75 */, "SCI (Sales Curve Interactive)/Storm" // Unknown
/* 76 */, ""
/* 77 */, ""
/* 78 */, "THQ Software" // Unknown
/* 79 */, "Accolade Inc." // Unknown
/* 7A */, "Triffix Entertainment" // Unknown
/* 7B */, ""
/* 7C */, "Microprose Software" // Unknown
/* 7D */, "Universal Interactive/Sierra/Simon & Schuster" // Universal Interactive Studios | Vivendi Games // Unknown
/* 7E */, ""
/* 7F */, "Kemco" // Unknown
/* 80 */, "Misawa" // Unknown
/* 81 */, "Teichiku" // Unknown
/* 82 */, "Namco Ltd." // Unknown
/* 83 */, "Coconuts Japan" // LOZC???
/* 84 */, "KOEI" // Unknown
/* 85 */, "G.O.1"
/* 86 */, "Tokuma Shoten Intermedia"
/* 87 */, "Tsukuda Original" // Unknown
/* 88 */, "DATAM-Polystar" // Unknown
/* 89 */, ""
/* 8A */, ""
/* 8B */, "Bulletproof Software" // Unknown
/* 8C */, "Vic Tokai Inc."
/* 8E */, ""
/* 8E */, "Character Soft"
/* 8F */, "I'Max"
/* 90 */, "Takara" // Takara Amusement // Unknown
/* 91 */, "Chun Soft" // Unknown
/* 92 */, "Video System Co., Ltd./McO'River" // Unknown
/* 93 */, "BEC" // Ocean/Acclaim??? // Unknown
/* 94 */, ""
/* 95 */, "Varie" // Unknown
/* 96 */, "Yonezawa/S’pal" // Yonezawa PR21
/* 97 */, "Kaneko" // Unknown
/* 98 */, ""
/* 99 */, "Pack-In-Video/Victor Interactive Software" // Marvelous Interactive | Rising Star Games
/* 9A */, "Nihon Bussan/Nichibutsu"
/* 9B */, "Tecmo" // Unknown
/* 9C */, "Imagineer"
/* 9D */, ""
/* 9E */, "Face"
/* 9F */, "Nova" // Unknown
/* A0 */, "Telenet" // Unknown
/* A1 */, "Hori" // Unknown
/* A2 */, ""
/* A3 */, ""
/* A4 */, "Konami"
/* A5 */, "K.Amusement Leasing Co." // Unknown
/* A6 */, "Kawada" // Unknown
/* A7 */, "Takara"
/* A8 */, ""
/* A9 */, "Technos Japan Corp." // Unknown
/* AA */, "Victor Musical Industries/JVC (Europe/Japan)" // Mini Putt - Accolade, A Wave.
/* AB */, "Hi-Score Media Work"
/* AC */, "Toei Animation" // Unknown
/* AD */, "Toho" // Unknown
/* AE */, "TSS"
/* AF */, "Namco" // Unknown
/* B0 */, "Acclaim Japan" // Unknown
/* B1 */, "ASCII Co./Nexoft"
/* B2 */, "Bandai" // Unknown
/* B3 */, ""
/* B4 */, "Enix" // Unknown
/* B5 */, ""
/* B6 */, "HAL Laboratory/Halken"
/* B7 */, "SNK" // Unknown
/* B8 */, ""
/* B9 */, "Pony Canyon Hanbai"
/* BA */, "Culture Brain"
/* BB */, "Sunsoft"
/* BC */, "Toshiba EMI" // Unknown
/* BD */, "Sony Imagesoft" // Unknown
/* BE */, ""
/* BF */, "Sammy"
/* C0 */, "Taito" // Unknown
/* C1 */, ""
/* C2 */, "Kemco" // Unknown
/* C3 */, "Square"
/* C4 */, "Tokuma Shoten"
/* C5 */, "Data East"
/* C6 */, "Tokyo Shoseki/Tonkin House"
/* C7 */, ""
/* C8 */, "Koei"
/* C9 */, ""
/* CA */, "Konami/Ultra/Palcom"
/* CB */, "Vap/NTVIC"
/* CC */, "Use Co., Ltd."
/* CD */, "Meldac" // Unknown
/* CE */, "Pony Canyon (Japan)/FCI (US)"
/* CF */, "Angel/Sotsu Agency/Sunrise"
/* D0 */, "Taito/Disco" // Unknown
/* D1 */, "SOFEL"
/* D2 */, "Quest Corp." // Visco, Bothtec
/* D3 */, "Sigma Entertainment"
/* D4 */, "Ask Kodansha" // Unknown
/* D5 */, ""
/* D6 */, "Naxat Soft" // Kaga Tech | Kaga Create
/* D7 */, "Copya System" // Unknown
/* D8 */, "Capcom Co., Ltd." // Unknown
/* D9 */, "Banpresto"
/* DA */, "Tomy"
/* DB */, "Acclaim/LJN Japan" // Unknown
/* DC */, ""
/* DD */, "NCS"
/* DE */, "Human Entertainment" // Unknown
/* DF */, "Altron"
/* E0 */, "Jaleco" // Unknown
/* E1 */, "Towachiki"
/* E2 */, "Yutaka"
/* E3 */, "Varie" // Kaken
/* E4 */, "T&ESoft" // Unknown
/* E5 */, "Epoch Co., Ltd."
/* E6 */, ""
/* E7 */, "Athena Co., Ltd."
/* E8 */, "Asmik"
/* E9 */, "Natsume"
/* EA */, "King Records" // Akagawa Jirou no Yuurei Ressha (赤川次郎の幽霊列車)
/* EB */, "Atlus"
/* EC */, "Epic/Sony Records (Japan)" // Unknown
/* ED */, "Pixel Multimedia"
/* EE */, "IGS (Information Global Service)"
/* EF */, "Fuji Television"
/* F0 */, "A Wave" // Unknown
/* F1 */, "Motown Software" // Unknown
/* F2 */, "Left Field Entertainment" // Unknown
/* F3 */, "Extreme Ent. Grp." // Unknown
/* F4 */, "TecMagik" // Unknown
/* F5 */, ""
/* F6 */, ""
/* F7 */, ""
/* F8 */, ""
/* F9 */, "Cybersoft" // Unknown
/* FA */, ""
/* FB */, "Psygnosis" // Unknown
/* FC */, ""
/* FD */, ""
/* FE */, "Taito" // Davidson/Western Tech.
/* FF */, "Reserved"
};


typedef struct {
    uint32_t mapper;
    uint32_t submapper;
    uint64_t PRGSize;
    uint64_t CHRSize;
    bool isBattery;
    bool isTrainer;
    bool is4Screen;
    bool isVertMirroring;
    bool isExtended;
    bool isPAL_iNES;
    uint32_t PRGRAMSize8K_iNES;
    uint32_t PRGRAMSize;
    uint32_t CHRRAMSize;
    uint32_t PRGSaveRAMSize;
    uint32_t CHRSaveRAMSize;
    uint32_t frameTiming;
    uint32_t consoleType1;
    uint32_t consoleType2;
    uint32_t consoleType3;
    uint32_t miscROMs;
    uint32_t expansion;
} NESInfo;

NESInfo GetNESInfo(const uint8_t* header)
{
    NESInfo info;
    info.PRGSize = header[4] * 0x4000;
    info.CHRSize = header[5] * 0x2000;
    info.mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);
    info.isVertMirroring = (header[6] & 0x01) == 0x01;
    info.isBattery = (header[6] & 0x02) == 0x02;
    info.isTrainer = (header[6] & 0x04) == 0x04;
    info.is4Screen = (header[6] & 0x08) == 0x08;
    info.isExtended = (header[7] & 0x0C) == 0x08;
    info.consoleType1 = header[7] & 3;

    // iNES 1.0
    info.PRGRAMSize8K_iNES = header[8];
    info.isPAL_iNES = (header[9] & 0x01) == 0x01;

    // NES 2.0
    info.submapper = 0;
    info.PRGRAMSize = 0;
    info.CHRRAMSize = 0;
    info.PRGSaveRAMSize = 0;
    info.CHRSaveRAMSize = 0;
    info.frameTiming = 0;
    info.consoleType2 = 0;
    info.consoleType3 = 0;
    info.miscROMs = 0;
    info.expansion = 0;
    if (info.isExtended) {
        info.mapper   |= (header[8] & 0x0F) << 8;
        info.submapper = (header[8] & 0xF0) >> 4;

        uint64_t PRGSizeHigh = header[9] & 0x0F;
        if (PRGSizeHigh != 0x0F) {
            info.PRGSize += (PRGSizeHigh << 8) * 0x4000;
        }
        else {
            uint8_t E = header[4] >> 2;
            if (E > 0x3D) { // >= 2 EiB (exbibyte)
                info.PRGSize = 0;
            }
            else {
                info.PRGSize = ((uint64_t)1 << E) * ((header[4] & 3) * 2 + 1);
            }
        }

        uint64_t CHRSizeHigh = (header[9] & 0xF0) >> 4;
        if (CHRSizeHigh != 0x0F) {
            info.CHRSize += (CHRSizeHigh << 8) * 0x2000;
        }
        else {
            uint8_t E = header[5] >> 2;
            if (E > 0x3D) { // >= 2 EiB (exbibyte)
                info.CHRSize = 0;
            }
            else {
                info.CHRSize = ((uint64_t)1 << E) * ((header[5] & 3) * 2 + 1);
            }
        }

        uint32_t tmp = 0;
        tmp = header[10] & 0x0F;
        if (tmp != 0) info.PRGRAMSize = 64 << tmp;
        tmp = header[11] & 0x0F;
        if (tmp != 0) info.CHRRAMSize = 64 << tmp;

        if (info.isBattery) {
            tmp = (header[10] & 0xF0) >> 4;
            if (tmp != 0) info.PRGSaveRAMSize = 64 << tmp;
            tmp = (header[11] & 0xF0) >> 4;
            if (tmp != 0) info.CHRSaveRAMSize = 64 << tmp;
        }

        info.frameTiming = header[12] & 0x03;
        info.consoleType2 = header[13] & 0x0F;
        info.consoleType3 = (header[13] & 0xF0) >> 4;
        info.miscROMs  = header[14] & 3;
        info.expansion = header[15];
        if (info.expansion > EXPANSION_COUNT) {
            info.expansion = EXPANSION_COUNT + 1; // Unknown
        }
    }
    return info;
}


// https://wiki.nesdev.org/w/index.php?title=Nintendo_header
typedef struct {
    char title[16 + 1];    // $FFE0-$FFEF. ASCII (0x20 - 0x5A).
    uint16_t PRGChecksum;  // $FFF0-$FFF1.
    uint16_t CHRChecksum;  // $FFF2-$FFF3.
    uint32_t PRGSize;      // $FFF4. D7-D4: PRG size.
    uint32_t CHRSize;      //        D2-D0: CHR size.
    bool isCHRRAM;         //        D3: 0 = CHR ROM, 1 = CHR RAM.
    bool isVertMirroring;  // $FFF5. D7: 0 = Vertical, 1 = Horizontal.
    uint8_t mapper;        //        D6-D0: 0 = NROM, 1 = CNROM, 2 = UNROM, 3 = GNROM, 4 = MMC (any).
    uint8_t titleEncoding; // $FFF6. 0 = No title entered, 1 = ASCII, 2 = Another encoding.
    uint8_t titleLen;      // $FFF7. Valid Title Length - 1. 0 if no title entered.
    uint8_t makerCode;     // $FFF8. The same used for the FDS, GB, GBC and SNES headers:
                           //        1 = Nintendo, 2-254 = everyone else. 255 must be reserved.
    uint8_t validation;    // $FFF9. Header Validation Byte. 8-bit checksum of $FFF2-$FFF9 should = 0.
} NintendoHeader;

bool GetNintendoHeader(const uint8_t* header, NintendoHeader* nh)
{
    uint8_t checksum = 0;
    for (size_t i = 0x12; i < 0x1A; i++) {
        checksum += header[i];
    }
    bool isZero = true;
    for (size_t i = 0x10; i < 0x1A; i++) {
        if (header[i] != 0) {
            isZero = false;
            break;
        }
    }
    if (checksum != 0 || isZero) {
        return 0;
    }

    nh->PRGChecksum = (header[0x10] << 8) | header[0x11];
    nh->CHRChecksum = (header[0x12] << 8) | header[0x13];
    nh->isCHRRAM = (header[0x14] & 0x08) == 0x08;
    nh->isVertMirroring = (header[0x15] & 0x80) != 0x80;
    nh->mapper = header[0x15] & 0x7F;
    nh->titleEncoding = header[0x16];
    nh->titleLen = header[0x17];
    if (nh->titleLen != 0) {
        nh->titleLen++;
    }
    if (nh->titleLen > 16) {
        return 0;
    }
    nh->makerCode = header[0x18];
    nh->validation = header[0x19];

    const uint32_t sizes[] = {
           8 * 1024, // or 64 * 1024
          16 * 1024,
          32 * 1024,
         128 * 1024,
         256 * 1024,
         512 * 1024,
        1024 * 1024, // ?
        2048 * 1024  // ?
    };

    nh->PRGSize = sizes[(header[0x14] >> 4) & 0x07];
    if (nh->PRGSize == sizes[0] 
        && nh->mapper >= 2 // != NROM && != CNROM
    ) {
        nh->PRGSize = 64 * 1024;
    }
    nh->CHRSize = sizes[header[0x14] & 0x07];
    if (nh->CHRSize == sizes[0]
        && nh->mapper == 4 // == MMC
        && !nh->isCHRRAM   // ?
    ) {
        nh->CHRSize = 64 * 1024;
    }

    nh->title[0] = '\0';
    if (nh->titleEncoding == 1 || nh->titleEncoding == 2) {
        for (size_t i = 0; i < 16; i++) {
            if (header[i] < 0x20 || header[i] > 0x5A) {
                nh->title[i] = ' ';
            }
            else {
                nh->title[i] = (char)header[i];
            }
        }
        nh->title[16] = '\0';
    }

    return 1;
}


void PrintHash(const uint8_t* src, size_t size, const char* name)
{
    char buf[128 + 1] = {0};

    if (src == NULL || size == 0) {
        snprintf(buf, sizeof(buf), "\n%s CRC32: N/A", name);
        Print(buf);
        Print("\n        MD5  : N/A");
        Print("\n        SHA-1: N/A");
        return;
    }

    uint32_t crc = 0;
    uint8_t sha1_val[21];
    char hash_str[41] = {0};

    crc = CRC32(src, size);
    snprintf(buf, sizeof(buf), "\n%s CRC32: %08X | Size: %" PRIuPTR, name, crc, size);
    Print(buf);

    md5BytesAsStr(src, size, hash_str);
    snprintf(buf, sizeof(buf), "\n        MD5  : %s", hash_str);
    Print(buf);

    SHA1(src, size, sha1_val);
    SHA1_to_hex(sha1_val, hash_str);
    snprintf(buf, sizeof(buf), "\n        SHA-1: %s", hash_str);
    Print(buf);

    if (g_nes20db != NULL) {
        PrintNES20DB(hash_str);
    }
}

void PrintNESInfo(const uint8_t* source, size_t file_size)
{
    char buf[256] = {0};
    const char* NoYesStr[] = {"No", "Yes"};
    const char* MirroringStr[] = {"Horizontal", "Vertical"};

    NESInfo info = GetNESInfo(source);
    Print("-------------*-----------------------------------------");
    if (info.isExtended) {
        Print("\n              NES 2.0");
    }
    else {
        Print("\n              iNES");
    }
    Print("\n ");
    for (size_t i = 0; i < HEADER_SIZE; i++) {
        snprintf(buf + i * 3, sizeof(buf) - i * 3, "%02X ", source[i]);
    }
    Print(buf);
    Print("\n-------------*-----------------------------------------");
    snprintf(buf, sizeof(buf), "\nMapper Number: %u", info.mapper);
    Print(buf);
    if (info.mapper < MAPPER_COUNT) {
        Print(" = ");
        Print(MapperNames[info.mapper]);
    }
    snprintf(buf, sizeof(buf), "\nPRG ROM  Size: %5" PRIu64 " KiB = %8" PRIu64 " B",
        info.PRGSize / 1024, info.PRGSize);
    Print(buf);
    snprintf(buf, sizeof(buf), "\nCHR ROM  Size: %5" PRIu64 " KiB = %8" PRIu64 " B",
        info.CHRSize / 1024, info.CHRSize);
    Print(buf);
    Print("\nMirroring    : "); Print(MirroringStr[(int)info.isVertMirroring]);
    Print("\n4-screen VRAM: "); Print(NoYesStr[(int)info.is4Screen]);
    Print("\nBattery      : "); Print(NoYesStr[(int)info.isBattery]);
    Print("\nTrainer      : "); Print(NoYesStr[(int)info.isTrainer]);
    Print("\nConsole Type : ");
    snprintf(buf, sizeof(buf), "%s (#%u)", ConsoleType1[info.consoleType1], info.consoleType1);
    Print(buf);
    if (!info.isExtended) {
        Print("\nTV system    : ");
        snprintf(buf, sizeof(buf), "%s (#%d)", FrameTiming[(int)info.isPAL_iNES], (int)info.isPAL_iNES);
        Print(buf);
        if (info.PRGRAMSize8K_iNES == 0) {
            Print("\nPRG RAM  Size: 0 or 8192 B (#0)");
        }
        else {
            uint32_t prgRamSize = info.PRGRAMSize8K_iNES * 0x2000;
            Print("\nPRG RAM  Size: ");
            snprintf(buf, sizeof(buf), "%5u KiB = %8u B (#%u)",
                prgRamSize / 1024, prgRamSize, info.PRGRAMSize8K_iNES);
            Print(buf);
        }
    }
    else {
        Print("\nTV system    : N/A");
        Print("\nPRG RAM  Size: N/A");
    }

    if (info.isExtended) {
        Print("\n-------------*-----------------------------------------");
        snprintf(buf, sizeof(buf), "\nSubmapper    : %u", info.submapper);
        Print(buf);
        snprintf(buf, sizeof(buf), "\nPRG RAM  Size: %5u KiB = %8u B",
            info.PRGRAMSize / 1024, info.PRGRAMSize);
        Print(buf);
        snprintf(buf, sizeof(buf), "\nCHR RAM  Size: %5u KiB = %8u B",
            info.CHRRAMSize / 1024, info.CHRRAMSize);
        Print(buf);
        snprintf(buf, sizeof(buf), "\nPRG Save Size: %5u KiB = %8u B",
            info.PRGSaveRAMSize / 1024, info.PRGSaveRAMSize);
        Print(buf);
        snprintf(buf, sizeof(buf), "\nCHR Save Size: %5u KiB = %8u B",
            info.CHRSaveRAMSize / 1024, info.CHRSaveRAMSize);
        Print(buf);
        snprintf(buf, sizeof(buf), "\nFrame Timing : %s (#%u)",
            FrameTiming[info.frameTiming], info.frameTiming);
        Print(buf);
        if (info.consoleType1 == 0x03) {
            Print("\nConsole Ext. : ");
            snprintf(buf, sizeof(buf), "%s (#%u)", ConsoleType2[info.consoleType2], info.consoleType2);
            Print(buf);
        }
        else {
            Print("\nConsole Ext. : N/A");
        }
        if (info.consoleType1 == 0x01) {
            Print("\nVS PPU       : ");
            snprintf(buf, sizeof(buf), "%s (#%u)", VSPPUs[info.consoleType2], info.consoleType2);
            Print(buf);
            Print("\nVS Type      : ");
            snprintf(buf, sizeof(buf), "%s (#%u)", VSFlags[info.consoleType3], info.consoleType3);
            Print(buf);
        }
        else {
            Print("\nVS PPU       : N/A");
            Print("\nVS Type      : N/A");
        }
        Print("\nMisc ROMs    : ");
        snprintf(buf, sizeof(buf), "%u", info.miscROMs);
        Print(buf);
        Print("\nExpansion    : ");
        snprintf(buf, sizeof(buf), "%s (#%u)", ExpansionDevices[info.expansion], info.expansion);
        Print(buf);
    }


    CRC32Init();

    Print("\n-------------*-----------------------------------------");
    PrintHash(source, file_size, "File   ");

    const uint8_t* src = NULL;
    size_t src_pos = 0;

    Print("\n-------------*-----------------------------------------");
    if (file_size > HEADER_SIZE) {
        src = source + HEADER_SIZE;
        size_t rom_size = file_size - HEADER_SIZE;
        PrintHash(src, rom_size, "ROM    ");
    }
    else {
        PrintHash(NULL, 0, "ROM    ");
    }

    bool prev_exists = true;
    bool prg_exists = false;

    src_pos = HEADER_SIZE;

    if (info.isTrainer) {
        Print("\n-------------*-----------------------------------------");
        if (file_size - src_pos >= TRAINER_SIZE) {
            src = source + src_pos;
            PrintHash(src, TRAINER_SIZE, "Trainer");
            src_pos += TRAINER_SIZE;
        }
        else {
            PrintHash(NULL, 0, "Trainer");
            prev_exists = false;
        }
    }

    if (info.PRGSize != 0) {
        Print("\n-------------*-----------------------------------------");
        if (prev_exists
            && file_size - src_pos >= info.PRGSize
        ) {
            src = source + src_pos;
            PrintHash(src, info.PRGSize, "PRG ROM");
            src_pos += info.PRGSize;
            prg_exists = true;
        }
        else {
            PrintHash(NULL, 0, "PRG ROM");
            prev_exists = false;
        }
    }

    if (info.CHRSize != 0) {
        Print("\n-------------*-----------------------------------------");
        if (prev_exists
            && file_size - src_pos >= info.CHRSize
        ) {
            src = source + src_pos;
            PrintHash(src, info.CHRSize, "CHR ROM");
            src_pos += info.CHRSize;
        }
        else {
            PrintHash(NULL, 0, "CHR ROM");
            prev_exists = false;
        }
    }

    if (prev_exists && file_size > src_pos) {
        Print("\n-------------*-----------------------------------------");
        src = source + src_pos;
        size_t misc_size = file_size - src_pos;
        PrintHash(src, misc_size, "Misc   ");
    }
    else if (info.isExtended && info.miscROMs != 0) {
        Print("\n-------------*-----------------------------------------");
        PrintHash(NULL, 0, "Misc   ");
    }

    if (prg_exists && info.PRGSize >= 0x20) {
        src_pos = HEADER_SIZE + info.PRGSize - 0x20;
        if (info.isTrainer) {
            src_pos += TRAINER_SIZE;
        }
        src = source + src_pos;

        NintendoHeader nh;
        if (GetNintendoHeader(src, &nh)) {
            Print("\n-------------*-----------------------------------------");
            Print("\n              Nintendo Header");
            Print("\n ");
            for (size_t i = 0x00; i < 0x10; i++) {
                snprintf(buf + i * 3, sizeof(buf) - i * 3, "%02X ", src[i]);
            }
            Print(buf);
            Print("\n ");
            for (size_t i = 0x10, j = 0; i < 0x1A; i++, j++) {
                snprintf(buf + j * 3, sizeof(buf) - j * 3, "%02X ", src[i]);
            }
            Print(buf);
            Print("\n-------------*-----------------------------------------");
            snprintf(buf, sizeof(buf), "\nTitle        : {%s}", nh.title);
            Print(buf);
            const char* MapperNintendo[] = {
                "NROM", "CNROM", "UNROM", "GNROM", "MMC"
            };
            Print("\nMapper       : ");
            if (nh.mapper < sizeof(MapperNintendo)/sizeof(MapperNintendo[0])) {
                snprintf(buf, sizeof(buf), "%s (#%u)", MapperNintendo[nh.mapper], nh.mapper);
            }
            else {
                snprintf(buf, sizeof(buf), "Unknown (#%u)", nh.mapper);
            }
            Print(buf);
            snprintf(buf, sizeof(buf), "\nPRG ROM  Size: %3u KiB = %6u B",
                nh.PRGSize / 1024, nh.PRGSize);
            Print(buf);
            if (nh.PRGSize == 8 * 1024) {
                Print(" (or 64 KiB)");
            }
            else if (nh.PRGSize == 64 * 1024) {
                Print(" (or  8 KiB)");
            }
            snprintf(buf, sizeof(buf), "\nCHR      Size: %3u KiB = %6u B",
                nh.CHRSize / 1024, nh.CHRSize);
            Print(buf);
            if (nh.CHRSize == 8 * 1024) {
                Print(" (or 64 KiB)");
            }
            else if (nh.CHRSize == 64 * 1024) {
                Print(" (or  8 KiB)");
            }
            Print("\nCHR RAM      : "); Print(NoYesStr[(int)nh.isCHRRAM]);
            Print("\nMirroring    : "); Print(MirroringStr[(int)nh.isVertMirroring]);
            snprintf(buf, sizeof(buf), "\nMaker's Code : 0x%02X = ", nh.makerCode);
            Print(buf);
            Print(MakerNames[nh.makerCode]);
            snprintf(buf, sizeof(buf), "\nChecksum     : PRG = %04X, CHR = %04X, Validation = %02X",
                nh.PRGChecksum, nh.CHRChecksum, nh.validation);
            Print(buf);
        }
    }
}


int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("NES Header Info v" NES_HEADER_INFO_VER "\n");
        printf("usage: %s rom.nes\n", argv[0]);
        printf("optional: nes20db.xml in the current directory");
        return 1;
    }

#ifdef WINDOWS_ENCODING
    LPWSTR* w_argv;
    int w_argc;

    w_argv = CommandLineToArgvW(GetCommandLineW(), &w_argc);
    if (w_argv == NULL) {
        fprintf(stderr, "Error: CommandLineToArgvW()\n");
        return 1;
    }
    // Debug
    //for (int i = 0; i < w_argc; i++) {
    //    // Bad: wprintf(L"%d: %ls\n", i, w_argv[i]);
    //    printf("%d: ", i);
    //    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), w_argv[i], lstrlenW(w_argv[i]), NULL, NULL);
    //    printf("\n");
    //}

    FILE* fp = _wfopen(w_argv[1], L"rb");
    LocalFree(w_argv);
#else
    FILE* fp = fopen(argv[1], "rb");
#endif
    if (fp == NULL) {
        fprintf(stderr, "Can't open: %s", argv[1]);
        return 1;
    }
    size_t file_size = GetFILESize(fp);
    if (file_size == (size_t)-1) {
        fprintf(stderr, "Error: GetFileSize()");
        fclose(fp);
        return 1;
    }
    if (file_size < MIN_FILE_SIZE) {
        fprintf(stderr, "Error: file size is too small");
        fclose(fp);
        return 1;
    }
    uint8_t* source = (uint8_t*)malloc(file_size);
    if (source == NULL) {
        fprintf(stderr, "Error: malloc()");
        fclose(fp);
        return 1;
    }
    size_t read_bytes = fread(source, sizeof(uint8_t), file_size, fp);
    if (read_bytes != file_size) {
        fprintf(stderr, "Can't read: %s", argv[1]);
        fclose(fp);
        free(source);
        return 1;
    }
    fclose(fp);

    if (memcmp(source, "NES\x1A", 4)) {
        fprintf(stderr, "Error: file is not an iNES ROM image");
        free(source);
        return 1;
    }

    OpenNES20DB();

    PrintNESInfo(source, file_size);

    CloseNES20DB();
    free(source);
    return 0;
}


// NES 2.0 XML Database

void OpenNES20DB(void)
{
    if (g_nes20db != NULL) {
        CloseNES20DB();
    }

    FILE* fp = fopen("nes20db.xml", "rb");
    if (fp == NULL) {
        return;
    }
    size_t file_size = GetFILESize(fp);
    if (file_size == (size_t)-1) {
        fprintf(stderr, "Error: GetFileSize() - nes20db.xml");
        fclose(fp);
        return;
    }
    uint8_t* source = (uint8_t*)malloc(file_size);
    if (source == NULL) {
        fprintf(stderr, "Error: malloc() - nes20db.xml");
        fclose(fp);
        return;
    }
    size_t read_bytes = fread(source, sizeof(uint8_t), file_size, fp);
    if (read_bytes != file_size) {
        fprintf(stderr, "Can't read: nes20db.xml");
        fclose(fp);
        free(source);
        return;
    }
    g_nes20db = source;
    g_nes20db_size = file_size;
    printf("nes20db.xml is found\n");
}

void CloseNES20DB(void)
{
    free(g_nes20db);
    g_nes20db = NULL;
    g_nes20db_size = 0;
}

void PrintNES20DB(const char* hash)
{
    const uint8_t* pos = g_nes20db;
    const size_t hash_len = strlen(hash); // SHA-1: 40
    for (;;) {
        const uint8_t* f = bytes_find(pos, g_nes20db_size - (pos - g_nes20db),
                                      (const uint8_t*)hash, hash_len);
        if (f == NULL) {
            return;
        }
        pos = f + hash_len;
        pos = bytes_find(pos, g_nes20db_size - (pos - g_nes20db),
                         (const uint8_t*)"</game>", 7);
        if (pos == NULL) {
            return;
        }
        pos += 7;

        f = bytes_rfind(g_nes20db, (f - g_nes20db),
                        (const uint8_t*)"<!-- ", 5);
        if (f == NULL) {
            return;
        }
        f += 5;
        const uint8_t* fend = bytes_find(f, g_nes20db_size - (f - g_nes20db),
                                         (const uint8_t*)" -->", 4);
        if (fend == NULL) {
            return;
        }

        size_t name_len = fend - f;
#ifdef WINDOWS_ENCODING
        wchar_t bufw[256 + 1] = {0};
        int convertResult = MultiByteToWideChar(CP_UTF8, 0, (const char*)f, name_len, bufw, 256);
        if (convertResult > 0 && name_len != 0) {
            printf("\n");
            WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), bufw, lstrlenW(bufw), NULL, NULL);
        }
        else {
            printf("\nError: MultiByteToWideChar()");
        }
#else
        char buf[256 + 1] = {0};
        if (name_len + 1 > sizeof(buf)) {
            name_len = sizeof(buf) - 1;
        }
        memcpy(buf, f, name_len);
        buf[name_len] = '\0';
        printf("\n%s", buf);
#endif
    }
}

uint8_t* bytes_find(
    const uint8_t* data,
    size_t data_len,
    const uint8_t* sub,
    size_t sub_len)
{
    if (data_len < sub_len || !sub_len || data == NULL || sub == NULL)
        return NULL;
    //if (!sub_len)
    //    return (uint8_t*)data;

    const uint8_t* last = data + (data_len - sub_len + 1);
    for (const uint8_t* h = data; h < last; h++) {
        if (h[0] == sub[0] && !memcmp(h, sub, sub_len)) {
            return (uint8_t*)h;
        }
    }
    return NULL;
}

uint8_t* bytes_rfind(
    const uint8_t* data,
    size_t data_len,
    const uint8_t* sub,
    size_t sub_len)
{
    if (data_len < sub_len || !sub_len || data == NULL || sub == NULL)
        return NULL;

    const uint8_t* h = data + (data_len - sub_len);
    for (;; h--) {
        if (h[0] == sub[0] && !memcmp(h, sub, sub_len)) {
            return (uint8_t*)h;
        }
        if (data == h) {
            break;
        }
    }
    return NULL;
}


// Misc

// Max: 2 GB
long GetFILESize(FILE* file)
{
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    return size;
}

// https://create.stephan-brumme.com/crc32/
static uint32_t Crc32Lookup[8][256];
void CRC32Init(void)
{
    Crc32Lookup[0][0] = 0;
    // compute each power of two (all numbers with exactly one bit set)
    uint32_t crc = Crc32Lookup[0][0x80] = 0xEDB88320;
    for (size_t next = 0x40; next != 0; next >>= 1) {
        crc = (crc >> 1) ^ ((crc & 1) * 0xEDB88320);
        Crc32Lookup[0][next] = crc;
    }
    // compute all values between two powers of two
    // i.e. 3, 5,6,7, 9,10,11,12,13,14,15, 17,...
    for (size_t powerOfTwo = 2; powerOfTwo <= 0x80; powerOfTwo <<= 1) {
        uint32_t crcExtraBit = Crc32Lookup[0][powerOfTwo];
        for (size_t i = 1; i < powerOfTwo; i++)
            Crc32Lookup[0][i + powerOfTwo] = Crc32Lookup[0][i] ^ crcExtraBit;
    }
    for (size_t i = 0; i <= 0xFF; i++) {
        // for Slicing-by-4 and Slicing-by-8
        Crc32Lookup[1][i] = (Crc32Lookup[0][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[0][i] & 0xFF];
        Crc32Lookup[2][i] = (Crc32Lookup[1][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[1][i] & 0xFF];
        Crc32Lookup[3][i] = (Crc32Lookup[2][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[2][i] & 0xFF];
        // only Slicing-by-8
        Crc32Lookup[4][i] = (Crc32Lookup[3][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[3][i] & 0xFF];
        Crc32Lookup[5][i] = (Crc32Lookup[4][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[4][i] & 0xFF];
        Crc32Lookup[6][i] = (Crc32Lookup[5][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[5][i] & 0xFF];
        Crc32Lookup[7][i] = (Crc32Lookup[6][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[6][i] & 0xFF];
    }
}

// Endian-independent
uint32_t CRC32(const uint8_t* data, size_t length)
{
    const uint8_t* current = data;
    uint32_t crc = 0xFFFFFFFF; // ~previousCrc32;
    // process eight bytes at once
    while (length >= 8) {
        uint32_t one = (current[0]      ) |
                       (current[1] <<  8) |
                       (current[2] << 16) |
                       (current[3] << 24);
        one ^= crc;
        crc = Crc32Lookup[7][(one      ) & 0xFF] ^
              Crc32Lookup[6][(one >>  8) & 0xFF] ^
              Crc32Lookup[5][(one >> 16) & 0xFF] ^
              Crc32Lookup[4][(one >> 24)       ] ^
              Crc32Lookup[3][current[4]] ^
              Crc32Lookup[2][current[5]] ^
              Crc32Lookup[1][current[6]] ^
              Crc32Lookup[0][current[7]];
        current += 8;
        length -= 8;
    }
    // remaining 1 to 7 bytes
    while (length--)
        crc = (crc >> 8) ^ Crc32Lookup[0][(crc & 0xFF) ^ *current++];
    return ~crc;
}

void SHA1_to_hex(const uint8_t hash[20], char str[41])
{
    for (size_t i = 0; i < 20; i++) {
        sprintf(str + i * 2, "%02X", hash[i]);
    }
    str[40] = '\0';
}
