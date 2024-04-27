//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
// rTexpacker v2.0 Atlas Descriptor Code exporter v2.0                          //
//                                                                              //
// more info and bugs-report:  github.com/raylibtech/rtools                     //
// feedback and support:       ray[at]raylibtech.com                            //
//                                                                              //
// Copyright (c) 2020-2022 raylib technologies (@raylibtech)                    //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////

#define ATLAS_ROBOTO_IMAGE_PATH      "roboto.png"
#define ATLAS_ROBOTO_SPRITE_COUNT    95
#define ATLAS_ROBOTO_FONT_SIZE       30

// Atlas sprite properties
typedef struct rtpAtlasSprite {
    const char *nameId;
    int originX, originY;
    int positionX, positionY;
    int sourceWidth, sourceHeight;
    int padding;
    bool trimmed;
    int trimRecX, trimRecY, trimRecWidth, trimRecHeight;

    // Atlas contains font data
    int charValue;
    int charOffsetX, charOffsetY;
    int charAdvanceX
} rtpAtlasSprite;

// Atlas sprites array
static rtpAtlasSprite rtpDescRoboto[95] = {
    { "RobotoMono-Regular-U0020", 0, 0, 2, 2, 13, 30, 2, false, 0, 0, 0, 0, 0x0020, 0, 0, 13 },
    { "RobotoMono-Regular-U0021", 0, 0, 21, 2, 3, 30, 2, false, 0, 6, 3, 18, 0x0021, 5, 0, 13 },
    { "RobotoMono-Regular-U0022", 0, 0, 30, 2, 7, 30, 2, false, 0, 5, 7, 7, 0x0022, 3, 0, 13 },
    { "RobotoMono-Regular-U0023", 0, 0, 43, 2, 14, 30, 2, false, 0, 6, 14, 17, 0x0023, 0, 0, 13 },
    { "RobotoMono-Regular-U0024", 0, 0, 63, 2, 12, 30, 2, false, 0, 4, 12, 22, 0x0024, 1, 0, 13 },
    { "RobotoMono-Regular-U0025", 0, 0, 81, 2, 14, 30, 2, false, 0, 6, 14, 18, 0x0025, 0, 0, 13 },
    { "RobotoMono-Regular-U0026", 0, 0, 101, 2, 13, 30, 2, false, 0, 6, 13, 18, 0x0026, 1, 0, 13 },
    { "RobotoMono-Regular-U0027", 0, 0, 120, 2, 3, 30, 2, false, 0, 5, 3, 7, 0x0027, 5, 0, 13 },
    { "RobotoMono-Regular-U0028", 0, 0, 129, 2, 7, 30, 2, false, 0, 4, 7, 25, 0x0028, 3, 0, 13 },
    { "RobotoMono-Regular-U0029", 0, 0, 142, 2, 7, 30, 2, false, 0, 4, 7, 25, 0x0029, 3, 0, 13 },
    { "RobotoMono-Regular-U002A", 0, 0, 155, 2, 12, 30, 2, false, 0, 9, 12, 12, 0x002A, 1, 0, 13 },
    { "RobotoMono-Regular-U002B", 0, 0, 173, 2, 12, 30, 2, false, 0, 9, 12, 13, 0x002B, 1, 0, 13 },
    { "RobotoMono-Regular-U002C", 0, 0, 191, 2, 5, 30, 2, false, 0, 20, 5, 7, 0x002C, 3, 0, 13 },
    { "RobotoMono-Regular-U002D", 0, 0, 202, 2, 9, 30, 2, false, 0, 15, 9, 2, 0x002D, 2, 0, 13 },
    { "RobotoMono-Regular-U002E", 0, 0, 217, 2, 4, 30, 2, false, 0, 20, 4, 4, 0x002E, 5, 0, 13 },
    { "RobotoMono-Regular-U002F", 0, 0, 227, 2, 10, 30, 2, false, 0, 6, 10, 19, 0x002F, 2, 0, 13 },
    { "RobotoMono-Regular-U0030", 0, 0, 2, 38, 12, 30, 2, false, 0, 6, 12, 18, 0x0030, 1, 0, 13 },
    { "RobotoMono-Regular-U0031", 0, 0, 20, 38, 7, 30, 2, false, 0, 6, 7, 17, 0x0031, 2, 0, 13 },
    { "RobotoMono-Regular-U0032", 0, 0, 33, 38, 12, 30, 2, false, 0, 6, 12, 17, 0x0032, 0, 0, 13 },
    { "RobotoMono-Regular-U0033", 0, 0, 51, 38, 11, 30, 2, false, 0, 6, 11, 18, 0x0033, 1, 0, 13 },
    { "RobotoMono-Regular-U0034", 0, 0, 68, 38, 13, 30, 2, false, 0, 6, 13, 17, 0x0034, 0, 0, 13 },
    { "RobotoMono-Regular-U0035", 0, 0, 87, 38, 11, 30, 2, false, 0, 6, 11, 18, 0x0035, 2, 0, 13 },
    { "RobotoMono-Regular-U0036", 0, 0, 104, 38, 11, 30, 2, false, 0, 6, 11, 18, 0x0036, 1, 0, 13 },
    { "RobotoMono-Regular-U0037", 0, 0, 121, 38, 12, 30, 2, false, 0, 6, 12, 17, 0x0037, 1, 0, 13 },
    { "RobotoMono-Regular-U0038", 0, 0, 139, 38, 12, 30, 2, false, 0, 6, 12, 18, 0x0038, 1, 0, 13 },
    { "RobotoMono-Regular-U0039", 0, 0, 157, 38, 11, 30, 2, false, 0, 6, 11, 18, 0x0039, 1, 0, 13 },
    { "RobotoMono-Regular-U003A", 0, 0, 174, 38, 4, 30, 2, false, 0, 10, 4, 14, 0x003A, 6, 0, 13 },
    { "RobotoMono-Regular-U003B", 0, 0, 184, 38, 5, 30, 2, false, 0, 10, 5, 17, 0x003B, 5, 0, 13 },
    { "RobotoMono-Regular-U003C", 0, 0, 195, 38, 11, 30, 2, false, 0, 10, 11, 11, 0x003C, 1, 0, 13 },
    { "RobotoMono-Regular-U003D", 0, 0, 212, 38, 11, 30, 2, false, 0, 12, 11, 7, 0x003D, 1, 0, 13 },
    { "RobotoMono-Regular-U003E", 0, 0, 229, 38, 11, 30, 2, false, 0, 10, 11, 11, 0x003E, 1, 0, 13 },
    { "RobotoMono-Regular-U003F", 0, 0, 2, 74, 10, 30, 2, false, 0, 6, 10, 18, 0x003F, 2, 0, 13 },
    { "RobotoMono-Regular-U0040", 0, 0, 18, 74, 13, 30, 2, false, 0, 6, 13, 18, 0x0040, 0, 0, 13 },
    { "RobotoMono-Regular-U0041", 0, 0, 37, 74, 13, 30, 2, false, 0, 6, 13, 17, 0x0041, 0, 0, 13 },
    { "RobotoMono-Regular-U0042", 0, 0, 56, 74, 12, 30, 2, false, 0, 6, 12, 17, 0x0042, 1, 0, 13 },
    { "RobotoMono-Regular-U0043", 0, 0, 74, 74, 12, 30, 2, false, 0, 6, 12, 18, 0x0043, 1, 0, 13 },
    { "RobotoMono-Regular-U0044", 0, 0, 92, 74, 12, 30, 2, false, 0, 6, 12, 17, 0x0044, 1, 0, 13 },
    { "RobotoMono-Regular-U0045", 0, 0, 110, 74, 10, 30, 2, false, 0, 6, 10, 17, 0x0045, 2, 0, 13 },
    { "RobotoMono-Regular-U0046", 0, 0, 126, 74, 11, 30, 2, false, 0, 6, 11, 17, 0x0046, 2, 0, 13 },
    { "RobotoMono-Regular-U0047", 0, 0, 143, 74, 12, 30, 2, false, 0, 6, 12, 18, 0x0047, 1, 0, 13 },
    { "RobotoMono-Regular-U0048", 0, 0, 161, 74, 12, 30, 2, false, 0, 6, 12, 17, 0x0048, 1, 0, 13 },
    { "RobotoMono-Regular-U0049", 0, 0, 179, 74, 11, 30, 2, false, 0, 6, 11, 17, 0x0049, 1, 0, 13 },
    { "RobotoMono-Regular-U004A", 0, 0, 196, 74, 11, 30, 2, false, 0, 6, 11, 18, 0x004A, 1, 0, 13 },
    { "RobotoMono-Regular-U004B", 0, 0, 213, 74, 13, 30, 2, false, 0, 6, 13, 17, 0x004B, 1, 0, 13 },
    { "RobotoMono-Regular-U004C", 0, 0, 232, 74, 11, 30, 2, false, 0, 6, 11, 17, 0x004C, 2, 0, 13 },
    { "RobotoMono-Regular-U004D", 0, 0, 2, 110, 12, 30, 2, false, 0, 6, 12, 17, 0x004D, 1, 0, 13 },
    { "RobotoMono-Regular-U004E", 0, 0, 20, 110, 12, 30, 2, false, 0, 6, 12, 17, 0x004E, 1, 0, 13 },
    { "RobotoMono-Regular-U004F", 0, 0, 38, 110, 12, 30, 2, false, 0, 6, 12, 18, 0x004F, 1, 0, 13 },
    { "RobotoMono-Regular-U0050", 0, 0, 56, 110, 11, 30, 2, false, 0, 6, 11, 17, 0x0050, 2, 0, 13 },
    { "RobotoMono-Regular-U0051", 0, 0, 73, 110, 12, 30, 2, false, 0, 6, 12, 20, 0x0051, 1, 0, 13 },
    { "RobotoMono-Regular-U0052", 0, 0, 91, 110, 11, 30, 2, false, 0, 6, 11, 17, 0x0052, 2, 0, 13 },
    { "RobotoMono-Regular-U0053", 0, 0, 108, 110, 12, 30, 2, false, 0, 6, 12, 18, 0x0053, 1, 0, 13 },
    { "RobotoMono-Regular-U0054", 0, 0, 126, 110, 13, 30, 2, false, 0, 6, 13, 17, 0x0054, 0, 0, 13 },
    { "RobotoMono-Regular-U0055", 0, 0, 145, 110, 12, 30, 2, false, 0, 6, 12, 18, 0x0055, 1, 0, 13 },
    { "RobotoMono-Regular-U0056", 0, 0, 163, 110, 13, 30, 2, false, 0, 6, 13, 17, 0x0056, 0, 0, 13 },
    { "RobotoMono-Regular-U0057", 0, 0, 182, 110, 14, 30, 2, false, 0, 6, 14, 17, 0x0057, 0, 0, 13 },
    { "RobotoMono-Regular-U0058", 0, 0, 202, 110, 13, 30, 2, false, 1, 6, 12, 17, 0x0058, 0, 0, 13 },
    { "RobotoMono-Regular-U0059", 0, 0, 221, 110, 13, 30, 2, false, 0, 6, 13, 17, 0x0059, 0, 0, 13 },
    { "RobotoMono-Regular-U005A", 0, 0, 2, 146, 11, 30, 2, false, 0, 6, 11, 17, 0x005A, 1, 0, 13 },
    { "RobotoMono-Regular-U005B", 0, 0, 19, 146, 6, 30, 2, false, 0, 4, 6, 23, 0x005B, 4, 0, 13 },
    { "RobotoMono-Regular-U005C", 0, 0, 31, 146, 10, 30, 2, false, 0, 6, 10, 19, 0x005C, 2, 0, 13 },
    { "RobotoMono-Regular-U005D", 0, 0, 47, 146, 5, 30, 2, false, 0, 4, 5, 23, 0x005D, 4, 0, 13 },
    { "RobotoMono-Regular-U005E", 0, 0, 58, 146, 10, 30, 2, false, 0, 6, 10, 10, 0x005E, 2, 0, 13 },
    { "RobotoMono-Regular-U005F", 0, 0, 74, 146, 11, 30, 2, false, 0, 23, 11, 2, 0x005F, 1, 0, 13 },
    { "RobotoMono-Regular-U0060", 0, 0, 91, 146, 6, 30, 2, false, 0, 6, 5, 4, 0x0060, 4, 0, 13 },
    { "RobotoMono-Regular-U0061", 0, 0, 103, 146, 11, 30, 2, false, 0, 10, 11, 14, 0x0061, 1, 0, 13 },
    { "RobotoMono-Regular-U0062", 0, 0, 120, 146, 12, 30, 2, false, 0, 5, 12, 19, 0x0062, 1, 0, 13 },
    { "RobotoMono-Regular-U0063", 0, 0, 138, 146, 11, 30, 2, false, 0, 10, 11, 14, 0x0063, 1, 0, 13 },
    { "RobotoMono-Regular-U0064", 0, 0, 155, 146, 11, 30, 2, false, 0, 5, 11, 19, 0x0064, 1, 0, 13 },
    { "RobotoMono-Regular-U0065", 0, 0, 172, 146, 12, 30, 2, false, 0, 10, 12, 14, 0x0065, 1, 0, 13 },
    { "RobotoMono-Regular-U0066", 0, 0, 190, 146, 12, 30, 2, false, 0, 5, 12, 18, 0x0066, 1, 0, 13 },
    { "RobotoMono-Regular-U0067", 0, 0, 208, 146, 11, 30, 2, false, 0, 10, 11, 18, 0x0067, 1, 0, 13 },
    { "RobotoMono-Regular-U0068", 0, 0, 225, 146, 11, 30, 2, false, 0, 5, 11, 18, 0x0068, 1, 0, 13 },
    { "RobotoMono-Regular-U0069", 0, 0, 2, 182, 11, 30, 2, false, 0, 6, 11, 17, 0x0069, 2, 0, 13 },
    { "RobotoMono-Regular-U006A", 0, 0, 19, 182, 8, 30, 2, false, 0, 6, 8, 22, 0x006A, 2, 0, 13 },
    { "RobotoMono-Regular-U006B", 0, 0, 33, 182, 12, 30, 2, false, 0, 5, 12, 18, 0x006B, 1, 0, 13 },
    { "RobotoMono-Regular-U006C", 0, 0, 51, 182, 11, 30, 2, false, 0, 5, 11, 18, 0x006C, 2, 0, 13 },
    { "RobotoMono-Regular-U006D", 0, 0, 68, 182, 12, 30, 2, false, 0, 10, 12, 13, 0x006D, 1, 0, 13 },
    { "RobotoMono-Regular-U006E", 0, 0, 86, 182, 11, 30, 2, false, 0, 10, 11, 13, 0x006E, 1, 0, 13 },
    { "RobotoMono-Regular-U006F", 0, 0, 103, 182, 12, 30, 2, false, 0, 10, 12, 14, 0x006F, 1, 0, 13 },
    { "RobotoMono-Regular-U0070", 0, 0, 121, 182, 12, 30, 2, false, 0, 10, 12, 18, 0x0070, 1, 0, 13 },
    { "RobotoMono-Regular-U0071", 0, 0, 139, 182, 11, 30, 2, false, 0, 10, 11, 18, 0x0071, 1, 0, 13 },
    { "RobotoMono-Regular-U0072", 0, 0, 156, 182, 9, 30, 2, false, 0, 10, 9, 13, 0x0072, 3, 0, 13 },
    { "RobotoMono-Regular-U0073", 0, 0, 171, 182, 11, 30, 2, false, 0, 10, 11, 14, 0x0073, 1, 0, 13 },
    { "RobotoMono-Regular-U0074", 0, 0, 188, 182, 11, 30, 2, false, 0, 8, 11, 16, 0x0074, 1, 0, 13 },
    { "RobotoMono-Regular-U0075", 0, 0, 205, 182, 11, 30, 2, false, 1, 10, 10, 14, 0x0075, 1, 0, 13 },
    { "RobotoMono-Regular-U0076", 0, 0, 222, 182, 12, 30, 2, false, 0, 10, 12, 13, 0x0076, 1, 0, 13 },
    { "RobotoMono-Regular-U0077", 0, 0, 2, 218, 14, 30, 2, false, 0, 10, 14, 13, 0x0077, 0, 0, 13 },
    { "RobotoMono-Regular-U0078", 0, 0, 22, 218, 12, 30, 2, false, 0, 10, 12, 13, 0x0078, 1, 0, 13 },
    { "RobotoMono-Regular-U0079", 0, 0, 40, 218, 13, 30, 2, false, 0, 10, 13, 18, 0x0079, 0, 0, 13 },
    { "RobotoMono-Regular-U007A", 0, 0, 59, 218, 12, 30, 2, false, 0, 10, 12, 13, 0x007A, 1, 0, 13 },
    { "RobotoMono-Regular-U007B", 0, 0, 77, 218, 9, 30, 2, false, 0, 5, 9, 23, 0x007B, 3, 0, 13 },
    { "RobotoMono-Regular-U007C", 0, 0, 92, 218, 3, 30, 2, false, 0, 6, 3, 22, 0x007C, 5, 0, 13 },
    { "RobotoMono-Regular-U007D", 0, 0, 101, 218, 9, 30, 2, false, 0, 5, 9, 23, 0x007D, 3, 0, 13 },
    { "RobotoMono-Regular-U007E", 0, 0, 116, 218, 14, 30, 2, false, 0, 14, 14, 5, 0x007E, 0, 0, 13 },
};
