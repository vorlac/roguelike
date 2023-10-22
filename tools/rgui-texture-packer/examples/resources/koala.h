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

#define ATLAS_KOALA_IMAGE_PATH      "koala.png"
#define ATLAS_KOALA_SPRITE_COUNT    50

// Atlas sprite properties
typedef struct rtpAtlasSprite {
    const char *nameId;
    int originX, originY;
    int positionX, positionY;
    int sourceWidth, sourceHeight;
    int padding;
    bool trimmed;
    int trimRecX, trimRecY, trimRecWidth, trimRecHeight;
} rtpAtlasSprite;

// Atlas sprites array
static rtpAtlasSprite rtpDescKoala[50] = {
    { "ending_button_trophy", 0, 0, 888, 735, 123, 123, 0, false, 0, 0, 123, 123 },
    { "ending_goals_check_v", 0, 0, 865, 929, 50, 42, 0, false, 2, 2, 48, 38 },
    { "ending_goals_check_x", 0, 0, 60, 903, 42, 51, 0, false, 0, 0, 42, 51 },
    { "ending_goals_icon_death", 0, 0, 696, 892, 60, 60, 0, false, 3, 3, 54, 56 },
    { "ending_goals_icon_leaves", 0, 0, 636, 892, 60, 60, 0, false, 3, 3, 54, 56 },
    { "ending_goals_icon_special", 0, 0, 0, 903, 60, 60, 0, false, 3, 3, 54, 56 },
    { "ending_goals_icon_time", 0, 0, 576, 892, 60, 60, 0, false, 3, 3, 54, 56 },
    { "ending_paint_koalabee", 0, 0, 219, 348, 219, 216, 0, false, 9, 6, 202, 207 },
    { "ending_paint_koaladingo", 0, 0, 765, 519, 219, 216, 0, false, 0, 6, 219, 210 },
    { "ending_paint_koalaeagle", 0, 0, 765, 303, 219, 216, 0, false, 0, 54, 219, 153 },
    { "ending_paint_koalafire", 0, 0, 512, 553, 219, 216, 0, false, 21, 12, 182, 187 },
    { "ending_paint_koalageneric", 0, 0, 512, 303, 253, 250, 0, false, 0, 0, 253, 250 },
    { "ending_paint_koalaowl", 0, 0, 765, 858, 100, 81, 0, false, 0, 0, 100, 81 },
    { "ending_paint_koalasnake", 0, 0, 0, 348, 219, 216, 0, false, 29, 21, 132, 184 },
    { "ending_plate_headbee", 0, 0, 440, 888, 62, 60, 0, false, 0, 0, 62, 60 },
    { "ending_plate_headdingo", 0, 0, 926, 858, 56, 70, 0, false, 2, 0, 54, 70 },
    { "ending_plate_headeagle", 0, 0, 982, 924, 39, 48, 0, false, 0, 0, 39, 48 },
    { "ending_plate_headowl", 0, 0, 327, 908, 72, 48, 0, false, 2, 0, 68, 48 },
    { "ending_plate_headsnake", 0, 0, 119, 880, 46, 67, 0, false, 3, 0, 39, 65 },
    { "ending_score_enemyicon", 0, 0, 327, 817, 113, 91, 0, false, 0, 0, 113, 91 },
    { "ending_score_frame", 0, 0, 0, 780, 119, 123, 0, false, 0, 0, 119, 123 },
    { "ending_score_frameback", 0, 0, 635, 769, 119, 123, 0, false, 0, 2, 119, 119 },
    { "ending_score_leavesicon", 0, 0, 330, 564, 135, 130, 0, false, 0, 0, 135, 130 },
    { "gameplay_countdown_1", 0, 0, 0, 564, 110, 216, 0, false, 18, 11, 79, 193 },
    { "gameplay_countdown_2", 0, 0, 110, 564, 110, 216, 0, false, 12, 13, 91, 192 },
    { "gameplay_countdown_3", 0, 0, 220, 564, 110, 216, 0, false, 11, 12, 92, 192 },
    { "gameplay_gui_seasonsclock_disc", 0, 0, 512, 0, 300, 303, 0, false, 0, 6, 300, 297 },
    { "gameplay_koala_dash", 0, 0, 219, 780, 100, 100, 0, false, 18, 3, 64, 94 },
    { "gameplay_koala_die", 0, 0, 119, 780, 100, 100, 0, false, 10, 11, 81, 78 },
    { "gameplay_props_leaf_lil", 0, 0, 230, 880, 64, 64, 0, false, 13, 4, 50, 54 },
    { "gameplay_props_leaf_mid", 0, 0, 512, 892, 64, 64, 0, false, 0, 6, 56, 46 },
    { "particle_dandelion", 0, 0, 576, 952, 32, 32, 0, false, 2, 1, 24, 30 },
    { "particle_dandelion_bw", 0, 0, 640, 952, 32, 32, 0, false, 2, 1, 24, 30 },
    { "particle_ecualyptusflower", 0, 0, 440, 948, 32, 32, 0, false, 4, 1, 25, 29 },
    { "particle_ecualyptusflower_bw", 0, 0, 119, 947, 32, 32, 0, false, 4, 1, 25, 29 },
    { "particle_ecualyptusleaf", 0, 0, 165, 946, 32, 32, 0, false, 2, 0, 29, 32 },
    { "particle_icecrystal", 0, 0, 262, 944, 32, 32, 0, false, 1, 0, 30, 32 },
    { "particle_icecrystal_bw", 0, 0, 608, 952, 32, 32, 0, false, 1, 0, 30, 32 },
    { "particle_planetreeleaf", 0, 0, 230, 944, 32, 32, 0, false, 3, 1, 28, 31 },
    { "particle_planetreeleaf_bw", 0, 0, 926, 928, 32, 32, 0, false, 3, 1, 28, 31 },
    { "particle_waterdrop", 0, 0, 765, 939, 32, 32, 0, false, 13, 2, 5, 24 },
    { "particle_waterdrop_bw", 0, 0, 797, 939, 32, 32, 0, false, 13, 2, 5, 24 },
    { "title_music_off", 0, 0, 165, 880, 65, 66, 0, false, 0, 0, 65, 66 },
    { "title_music_on", 0, 0, 982, 858, 39, 66, 0, false, 0, 0, 39, 66 },
    { "title_speaker_off", 0, 0, 865, 858, 61, 71, 0, false, 0, 0, 61, 71 },
    { "title_speaker_on", 0, 0, 440, 817, 61, 71, 0, false, 0, 0, 61, 71 },
    { "title_titletext", 0, 0, 0, 0, 512, 348, 0, false, 0, 6, 510, 335 },
    { "ending_button_replay", 0, 0, 765, 735, 123, 123, 0, false, 0, 0, 123, 123 },
    { "ending_button_share", 0, 0, 330, 694, 123, 123, 0, false, 0, 0, 123, 123 },
    { "ending_button_shop", 0, 0, 512, 769, 123, 123, 0, false, 0, 0, 123, 123 },
};
