#ifndef PANTONE_COLORS_H
#define PANTONE_COLORS_H

#include <Arduino.h>

struct PantoneColor {
    String name;
    uint8_t r, g, b;
};

// 精选 100 个 Pantone 经典色号数据库
const PantoneColor pantoneDB[] = {
    {"Pantone 5493 C", 137, 172, 172}, // TechIN 514
    {"Pantone 18-1652", 210, 38, 48},  // Fiery Red
    {"Pantone 19-4052", 15, 76, 129},  // Classic Blue
    {"Pantone 13-0647", 245, 223, 77}, // Illuminating
    {"Pantone 15-3919", 145, 168, 208},// Serenity
    {"Pantone 13-1520", 247, 202, 201},// Rose Quartz
    {"Pantone 18-3224", 173, 94, 153}, // Radiant Orchid
    {"Pantone 17-1463", 221, 65, 36},  // Tangerine Tango
    {"Pantone 15-5519", 69, 181, 170}, // Turquoise
    {"Pantone 14-0848", 240, 192, 90}, // Mimosa
    {"Pantone 19-1664", 153, 27, 30},  // True Red
    {"Pantone 17-5104", 147, 149, 151},// Ultimate Gray
    {"Pantone 16-1546", 255, 111, 97}, // Living Coral
    {"Pantone 18-3838", 95, 75, 139},  // Ultra Violet
    {"Pantone 15-0343", 136, 176, 75}, // Greenery
    {"Pantone 18-1438", 150, 79, 76},  // Marsala
    {"Pantone 17-5641", 0, 148, 115},  // Emerald
    {"Pantone 17-2031", 222, 76, 138}, // Honeysuckle
    {"Pantone 15-4020", 123, 163, 212},// Cerulean
    {"Pantone 19-1557", 138, 40, 47},  // Chili Pepper
    {"Pantone Black 6 C", 16, 24, 32}, // Pure Black
    {"Pantone White", 255, 255, 255},  // Pure White
    // 红/粉色系
    {"Warm Red C", 249, 66, 58}, {"Rubine Red C", 206, 0, 88}, 
    {"Rhodamine Red C", 225, 0, 152}, {"Pink C", 214, 37, 152},
    {"Pantone 1788 C", 238, 39, 55}, {"Pantone 185 C", 228, 0, 43},
    {"Pantone 199 C", 213, 0, 50}, {"Pantone 200 C", 186, 12, 47},
    {"Pantone 201 C", 157, 34, 53}, {"Pantone 202 C", 134, 38, 51},
    // 橙/黄色系
    {"Orange 021 C", 254, 80, 0}, {"Yellow C", 254, 223, 0},
    {"Yellow 012 C", 255, 215, 0}, {"Pantone 109 C", 255, 209, 0},
    {"Pantone 116 C", 255, 205, 0}, {"Pantone 123 C", 255, 199, 44},
    {"Pantone 130 C", 242, 169, 0}, {"Pantone 137 C", 255, 163, 0},
    {"Pantone 151 C", 255, 130, 0}, {"Pantone 158 C", 232, 119, 34},
    // 蓝/紫色系
    {"Reflex Blue C", 0, 20, 137}, {"Blue 072 C", 16, 6, 159},
    {"Violet C", 68, 0, 153}, {"Purple C", 187, 41, 187},
    {"Pantone 285 C", 0, 114, 206}, {"Pantone 286 C", 0, 51, 160},
    {"Pantone 293 C", 0, 85, 164}, {"Pantone 300 C", 0, 94, 184},
    {"Pantone 306 C", 0, 181, 226}, {"Pantone 312 C", 0, 174, 199},
    {"Pantone 259 C", 109, 32, 119}, {"Pantone 266 C", 117, 59, 189},
    {"Pantone 273 C", 46, 26, 71}, {"Pantone 2728 C", 0, 71, 187},
    // 绿色系
    {"Green C", 0, 171, 132}, {"Pantone 347 C", 0, 154, 68},
    {"Pantone 348 C", 0, 132, 61}, {"Pantone 354 C", 0, 177, 64},
    {"Pantone 361 C", 67, 176, 42}, {"Pantone 368 C", 120, 190, 32},
    {"Pantone 375 C", 151, 215, 0}, {"Pantone 382 C", 193, 216, 47},
    {"Pantone 3278 C", 0, 155, 119}, {"Pantone 334 C", 0, 151, 117},
    // 棕/中性色系
    {"Brown 469 C", 96, 58, 40}, {"Pantone 464 C", 137, 92, 39},
    {"Pantone 7511 C", 194, 136, 80}, {"Pantone 7530 C", 163, 153, 142},
    {"Pantone 422 C", 158, 162, 162}, {"Pantone 424 C", 112, 115, 114},
    {"Pantone 430 C", 124, 135, 142}, {"Pantone 432 C", 51, 63, 72},
    {"Pantone 7545 C", 66, 85, 99}, {"Pantone 7546 C", 43, 60, 76},
    // 扩充更多深浅变化
    {"Pantone 102 C", 252, 233, 0}, {"Pantone 150 C", 255, 138, 61},
    {"Pantone 186 C", 200, 16, 46}, {"Pantone 212 C", 240, 78, 152},
    {"Pantone 240 C", 209, 44, 201}, {"Pantone 281 C", 0, 32, 91},
    {"Pantone 315 C", 0, 103, 127}, {"Pantone 341 C", 0, 119, 73},
    {"Pantone 376 C", 132, 189, 0}, {"Pantone 469 C", 96, 58, 40},
    {"Pantone 518 C", 69, 38, 66}, {"Pantone 540 C", 0, 48, 87},
    {"Pantone 553 C", 23, 62, 53}, {"Pantone 661 C", 0, 53, 148},
    {"Pantone 7406 C", 241, 196, 0}, {"Pantone 7462 C", 0, 85, 135},
    {"Pantone 7499 C", 241, 230, 178}, {"Pantone 7529 C", 183, 169, 154},
    {"Pantone 7749 C", 65, 116, 61}, {"Pantone 7753 C", 133, 113, 77}
};

const int dbSize = sizeof(pantoneDB) / sizeof(pantoneDB[0]);

#endif