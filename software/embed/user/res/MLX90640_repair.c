#include "mlx90640_repair.h"
//
#include <stdint.h>

uint16_t mlx90640_eeprom_repair_data[] = {
    0x00C5, 0xB99F, 0x0000, 0x2061, 0x0005, 0x0320, 0x03E0, 0x0E28, 0xD928,
    0x0189, 0x0499, 0x0000, 0x1901, 0x0000, 0x0000, 0xBE33, 0x4220, 0xFFBE,
    0x1212, 0x0212, 0x0202, 0xE1F2, 0xD1E1, 0x9FC0, 0x0113, 0x0113, 0xF103,
    0xF102, 0xE1F3, 0xD0E2, 0xD0E2, 0xAFD0, 0x8895, 0x3950, 0xEECB, 0x2100,
    0x3332, 0x2233, 0x0011, 0xCCEE, 0xDCBB, 0x0FFE, 0x2111, 0x3232, 0x2233,
    0x1122, 0xFF11, 0xCCEE, 0x1698, 0x2FAF, 0x2553, 0xA584, 0x7655, 0xF9CB,
    0x3131, 0x2D2D, 0x2452, 0x08B6, 0x17B6, 0x0623, 0xF600, 0xC5CC, 0x97B1,
    0x2669, 0x03EE, 0x007E, 0x242E, 0xFFEC, 0x041E, 0x03A0, 0x243E, 0xFBAE,
    0x07BE, 0x0440, 0x1C8E, 0xFFCC, 0x049E, 0xF810, 0x189E, 0xF3EC, 0x07B0,
    0x0050, 0x10CE, 0xF0DE, 0x0090, 0x04A0, 0x1850, 0xF85E, 0xFBB0, 0xF770,
    0x1040, 0xEFF0, 0x0020, 0xF7E2, 0x1810, 0xF050, 0xFBC0, 0xF45E, 0xF80C,
    0x0BCE, 0xFFE0, 0xF38E, 0xF81C, 0x0780, 0xFFA2, 0xF81E, 0xF46C, 0x0BA0,
    0x0062, 0xEFEC, 0xEC6C, 0xFFBE, 0xFF92, 0xF03E, 0xE49C, 0x04B0, 0xFC72,
    0xF880, 0xF42C, 0x0C40, 0xF7A2, 0xEF6E, 0xE81E, 0xFFD0, 0x0010, 0xEFB0,
    0xF7EE, 0x0430, 0x037E, 0x0010, 0x27CE, 0xFF7C, 0x0400, 0xFFC0, 0x207E,
    0xF3DE, 0x0B90, 0x0460, 0x1C7E, 0xFF8E, 0x0850, 0xFBD0, 0x1870, 0xF39E,
    0xFBF0, 0x03C0, 0x105E, 0xF44E, 0x0BC0, 0x0820, 0x1C10, 0xFFDE, 0x03A2,
    0xFB80, 0x0C60, 0xEFE0, 0x040E, 0xF7D2, 0x1FB0, 0xFF90, 0xFBE2, 0xF46E,
    0xF82C, 0x0FCE, 0xFC62, 0xF01E, 0xF4DC, 0x0430, 0x03F2, 0xF8AE, 0xF0BC,
    0x0FC0, 0x04A2, 0xEC1E, 0xECAC, 0x03D0, 0xF432, 0xF40E, 0xE89E, 0x0880,
    0x0404, 0xFC60, 0xF45C, 0x1022, 0xFFF4, 0xEFC0, 0xE8BE, 0xFC50, 0x0452,
    0xEC30, 0xF7FE, 0x0FE2, 0x1310, 0x005E, 0x203E, 0xFF8C, 0x13F0, 0x03E0,
    0x245E, 0xFB9E, 0x0FDE, 0x0810, 0x1C6E, 0xFC0E, 0x1470, 0x0330, 0x1820,
    0xF79E, 0x0BA0, 0x0040, 0x1050, 0xF87E, 0x13B0, 0x13C2, 0x1860, 0xFC5E,
    0xFFD2, 0xFF32, 0x1020, 0xF3E0, 0x0800, 0xFF82, 0x1FD0, 0xFFA0, 0x0B52,
    0xF09E, 0xF07C, 0x0BC0, 0x0422, 0xF03E, 0xF49C, 0x07D0, 0x0012, 0xF45E,
    0xF08C, 0x044E, 0x08A2, 0xF36E, 0xF05C, 0xFFE0, 0xFFE4, 0xF08E, 0xE89E,
    0x04B0, 0x0BD4, 0xFFFE, 0xF07E, 0x0C80, 0xF814, 0xF370, 0xE85E, 0xFC32,
    0x0052, 0xF3D0, 0xF43E, 0x0BF0, 0x07A0, 0x0490, 0x207E, 0xFC0E, 0x0460,
    0x0400, 0x24B0, 0xF49C, 0x0400, 0x04A0, 0x1D00, 0xFFCE, 0x0C90, 0xFC00,
    0x1880, 0xF7CE, 0x03C2, 0x0402, 0x1480, 0xFC00, 0x0812, 0x0C30, 0x1C10,
    0xF8AE, 0x03D2, 0xFFB2, 0x1050, 0xF020, 0x0870, 0xF852, 0x1C52, 0xF840,
    0x0B62, 0xFC5E, 0xFC4C, 0x17D0, 0x0802, 0xFFCE, 0x005C, 0x0840, 0x07B2,
    0xFC6E, 0xF8CC, 0x1770, 0x0C52, 0xFBCE, 0xF83C, 0x0F60, 0x0784, 0x03D0,
    0xF44C, 0x17B2, 0x0FC4, 0x0BE0, 0xFFDE, 0x1452, 0x0772, 0xFF50, 0xF40C,
    0x0FD0, 0x1022, 0xF800, 0x03FE, 0x1002, 0x03C2, 0x04A2, 0x206E, 0xFC1E,
    0x04A0, 0x0030, 0x20CE, 0xF48E, 0xFC8E, 0x0800, 0x1C7E, 0xFFEE, 0x012E,
    0xFC10, 0x148E, 0xF3EE, 0xF830, 0x07D2, 0x1080, 0xF86E, 0x0412, 0x0C42,
    0x1C20, 0xF8B0, 0x03B2, 0x0352, 0x1062, 0xF010, 0x0470, 0xFFF2, 0x1C52,
    0xF470, 0x0372, 0xF46E, 0xF82C, 0x0FD0, 0x0044, 0xF7EE, 0xF85E, 0x0410,
    0xFC32, 0x03BE, 0xF42E, 0x1380, 0x00B0, 0xF7BE, 0xF01E, 0x0780, 0xFBE2,
    0xFF90, 0xEC3E, 0x0C10, 0x0BB4, 0x07E0, 0xFBCE, 0x1050, 0x0744, 0xFAF0,
    0xF3FE, 0x0BA2, 0x0C02, 0xFB90, 0xFFEE, 0x0C22, 0x0BA0, 0x0BE0, 0x1C6E,
    0x0390, 0x1012, 0x03E0, 0x2030, 0xFBBE, 0x0410, 0x0420, 0x1810, 0xFF4E,
    0x1060, 0x0342, 0x17D0, 0xF70E, 0xFF90, 0x03E0, 0x13B2, 0xF430, 0x0FC2,
    0x0C22, 0x1410, 0xF850, 0x0332, 0xFF42, 0x0080, 0xEFB0, 0x07F0, 0xFBE2,
    0x1422, 0xF810, 0x03B2, 0xF40E, 0xF07C, 0x0BB0, 0x0834, 0xF01E, 0xF44C,
    0x07D0, 0x0014, 0xF43E, 0xF01E, 0x0F60, 0x0C64, 0xF360, 0xF3FC, 0x0710,
    0xFFA2, 0xF400, 0xEFDE, 0x0840, 0x0BC4, 0xFC2E, 0xF03E, 0x0C72, 0x0344,
    0xEF60, 0xE49E, 0x03C0, 0x0802, 0xF000, 0xF44E, 0x0832, 0xF790, 0x0070,
    0x1830, 0xFBAE, 0xF860, 0xFC22, 0x14B0, 0xEFFE, 0xF7F0, 0x0790, 0x1060,
    0xFB6E, 0x0450, 0xFF32, 0x0CA0, 0xEBCE, 0xF7A2, 0x0352, 0x0812, 0xF7CE,
    0xF450, 0x0412, 0x1000, 0xFB90, 0xFB32, 0xF772, 0x0820, 0xE800, 0xFC40,
    0xFF82, 0x1052, 0xF7F0, 0xFF94, 0xF47E, 0xF83E, 0x0FB0, 0x0072, 0xF420,
    0xF4CC, 0x0410, 0xFFF2, 0x039E, 0xF46C, 0x1770, 0x0C62, 0xFB50, 0xECBE,
    0x07D0, 0x03A4, 0x0360, 0xEC2C, 0x17E0, 0x0052, 0x0420, 0xFBFE, 0x1BA2,
    0x0B44, 0xFB80, 0xF04E, 0x0822, 0x0C54, 0x0390, 0xFC60, 0x1412, 0xF810,
    0x0CD0, 0x1CF0, 0x004E, 0xFD50, 0x0472, 0x2090, 0xF4BE, 0x0040, 0x1000,
    0x1C60, 0x001E, 0x0C50, 0x0B72, 0x1860, 0xFBAE, 0x03A2, 0x0FF2, 0x1070,
    0xF8AE, 0x0440, 0x1402, 0x1C00, 0x0060, 0x03B0, 0xFC32, 0x1010, 0xF060,
    0x0870, 0x0BB2, 0x1C42, 0x0020, 0xFC02, 0xF8D0, 0xF4EE, 0x1050, 0xFD52,
    0xF88E, 0xFC8E, 0x08A0, 0x0432, 0x0400, 0xFC6C, 0x1400, 0x1052, 0x0370,
    0xF87C, 0x1390, 0x0BA4, 0x0400, 0xF07C, 0x14A0, 0x0C74, 0x0C50, 0x000E,
    0x1C72, 0x0FB4, 0xF850, 0xF83E, 0x1072, 0x1862, 0x0BC0, 0x0460, 0x2032,
    0x03A0, 0x0502, 0x1890, 0xFC6E, 0x00A0, 0x0032, 0x14A0, 0xF7CE, 0x03F0,
    0x0BF2, 0x1020, 0xF43E, 0x0C40, 0x0342, 0x0890, 0xF3C0, 0xFFF2, 0x0422,
    0x0860, 0xF870, 0x0412, 0x0C22, 0x1BF2, 0x07B0, 0xFFA2, 0x03C2, 0x0082,
    0xF010, 0x0430, 0x07F2, 0x0CC2, 0xFC80, 0xFFA4, 0xECF0, 0xF07E, 0x0840,
    0xFC84, 0xF020, 0xEC9E, 0x03E0, 0x03F2, 0xFFE0, 0xF02E, 0x0420, 0x0C32,
    0xF740, 0xE88E, 0x03C0, 0xFFF4, 0xF810, 0xE85E, 0x0C80, 0x0804, 0x0810,
    0xFBEE, 0x1BA0, 0x0784, 0xF7C0, 0xE89E, 0x0412, 0x0C32, 0xFFF0, 0xF8B0,
    0x1082, 0xEBC2, 0x0062, 0x08B0, 0xF7F0, 0xE8D0, 0xF862, 0x08C0, 0xEFF0,
    0xEFD0, 0xFC60, 0x08A0, 0xF76E, 0xFC20, 0xF7C2, 0x0BF0, 0xEB70, 0xF3B2,
    0xFFC2, 0x03E2, 0xF000, 0xEC50, 0x0402, 0x0800, 0xF420, 0xF3A2, 0xFB72,
    0x07E2, 0xE800, 0xFC12, 0xFC14, 0x0C62, 0x03D2, 0xF3A4, 0xF440, 0xE89E,
    0x0FD2, 0xF0B2, 0xEC50, 0xECBE, 0x07F0, 0xFBC2, 0xF840, 0xECAE, 0x1350,
    0x0BF2, 0xF3C0, 0xEFEE, 0x0770, 0xFFA2, 0xFFB0, 0xEBCE, 0x1402, 0xFC42,
    0x0BF0, 0xF7FE, 0x1402, 0x0382, 0xFF70, 0xF7DE, 0x13F2, 0x1412, 0x0012,
    0xFC50, 0x27D2, 0xE7F0, 0x04B2, 0x08C0, 0xF480, 0xF040, 0x0002, 0x0900,
    0xF040, 0xE880, 0x0462, 0x0C70, 0xF020, 0x0050, 0x0382, 0x0C20, 0xEF90,
    0xF770, 0x07C2, 0x0FB0, 0xF830, 0xF820, 0x0C62, 0x13F0, 0xFC20, 0xF3D2,
    0x03B2, 0x0832, 0xF040, 0x0040, 0x0412, 0x1022, 0x0410, 0xEBC2, 0xE890,
    0xE48E, 0x0060, 0xF424, 0xEFE0, 0xE8CE, 0x0020, 0xEC52, 0xF450, 0xEC3E,
    0x0410, 0x0422, 0xF770, 0xEC0E, 0x0790, 0xFF62, 0xFFC0, 0xEFAE, 0x1030,
    0x0424, 0x0450, 0xF7EE, 0x1422, 0x03B2, 0xFF90, 0xF02E, 0x1022, 0x1422,
    0x07F0, 0x03FE, 0x27E2, 0xE840, 0x0C72, 0x0CC0, 0xFC20, 0xFFF0, 0x07E0,
    0x1020, 0xFB7E, 0xF7F0, 0x0FE2, 0x086E, 0xFBE0, 0x0070, 0x0B32, 0x00BE,
    0xF740, 0xFF90, 0x0FB2, 0x0850, 0xFC40, 0x07E0, 0x1FE2, 0x1410, 0x0BE0,
    0xFB30, 0x0B82, 0xFC60, 0xFFC0, 0x0010, 0x13C4, 0x0800, 0x13D0, 0xE422,
    0xF050, 0xE08E, 0x07F0, 0xFBC2, 0xF3C0, 0xE81E, 0x0762, 0xFBD2, 0xFBC0,
    0xE85E, 0x03D0, 0x0472, 0xF730, 0xE4BE, 0x0750, 0x0392, 0xFFC0, 0xE84E,
    0x1440, 0x0FD2, 0x0FD0, 0xF41E, 0x23E2, 0x0B44, 0xFF70, 0xE86E, 0x13A2,
    0x17F2, 0x0BA2, 0x03D0, 0x27C2};