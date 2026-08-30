#pragma once

#include <pebble.h>

// Per-display layout constants.
//
// ForeCal carried two hand-placed layouts inline via PBL_IF_RECT_ELSE(): one for the
// 144x168 rectangular watches and one for 180x180 chalk. Every layer_create() call
// spelled out both frames, which made the layout hard to read and impossible to
// extend without editing dozens of call sites.
//
//   144x168 rect   aplite, basalt, diorite
//   180x180 round  chalk
//
// Both layouts keep their exact original numbers here, so this is a straight move
// with no visual change. Keeping every frame as a compile-time literal means only
// one branch is ever compiled in.
//
// Frames are expressed against PBL_DISPLAY_WIDTH/HEIGHT rather than a runtime
// layer_get_bounds() so they stay usable as constant initialisers.

#if defined(PBL_ROUND)

// ---------------------------------------------------------------- chalk, 180x180

#define LAYOUT_CURRENT_FRAME        GRect(0, 0, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT)
#define LAYOUT_CLOCK_FRAME          GRect((PBL_DISPLAY_WIDTH - 126) / 2, 13, 126, 50)
#define LAYOUT_PM_FRAME             GRect(PBL_DISPLAY_WIDTH - ((PBL_DISPLAY_WIDTH - 126) / 2), 48, 20, 15)
#define LAYOUT_WEEK_FRAME           GRect(PBL_DISPLAY_WIDTH - ((PBL_DISPLAY_WIDTH - 120) / 2), 48, 24, 15)
#define LAYOUT_DATE_FRAME           GRect((PBL_DISPLAY_WIDTH - 89) / 2, 2, 89, 26)
#define LAYOUT_DATE_ALIGN           GTextAlignmentCenter
#define LAYOUT_BT_FRAME             GRect(156, 112, 11, 18)
#define LAYOUT_BATT_FRAME           GRect(8, 112, 16, 8)
#define LAYOUT_CURR_TEMP_FRAME      GRect((PBL_DISPLAY_WIDTH - 45) / 2, 150, 45, 26)
#define LAYOUT_CURR_TEMP_ALIGN      GTextAlignmentCenter
#define LAYOUT_WIND_FRAME           GRect((PBL_DISPLAY_WIDTH - 60) / 2, 5, 45, 26)
#define LAYOUT_WIND_ALIGN           GTextAlignmentLeft

#define LAYOUT_FORECAST_FRAME       GRect(0, 63, PBL_DISPLAY_WIDTH, PBL_DISPLAY_HEIGHT - 63)
#define LAYOUT_STATUS_BG_FRAME      GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_STEPS_FRAME          GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_FORECAST_DAY_FRAME   GRect(0, -4, (PBL_DISPLAY_WIDTH / 2) - 20, 17)
#define LAYOUT_STATUS_FRAME         GRect((PBL_DISPLAY_WIDTH / 2) + 20, -4, (PBL_DISPLAY_WIDTH / 2) - 20, 17)
#define LAYOUT_HIGH_LABEL_FRAME     GRect(2, 6, 10, 24)
#define LAYOUT_HIGH_TEMP_FRAME      GRect(10, 6, 45, 24)
#define LAYOUT_LOW_LABEL_FRAME      GRect(122, 6, 10, 24)
#define LAYOUT_LOW_TEMP_FRAME       GRect(130, 6, 45, 24)
#define LAYOUT_SUN_TEXT_FRAME       GRect(100, 86, 47, 18)
#define LAYOUT_ICON_FRAME           GRect((PBL_DISPLAY_WIDTH - 34) / 2, 0, 34, 32)
#define LAYOUT_SUN_ICON_FRAME       GRect(50, 92, 20, 14)
#define LAYOUT_CONDITION_FRAME      GRect(0, 26, PBL_DISPLAY_WIDTH, 24)

#define LAYOUT_CAL_FRAME            GRect((PBL_DISPLAY_WIDTH - 124) / 2, 111, 124, 44)

// Calendar cell geometry. Cells are laid out at (d * LAYOUT_CAL_CELL_PITCH).
#define LAYOUT_CAL_CELL_PITCH       18
#define LAYOUT_CAL_CELL_W           16
#define LAYOUT_CAL_HDR_Y            (-4)
#define LAYOUT_CAL_TEXT_H           14
#define LAYOUT_CAL_HL_DY            4
#define LAYOUT_CAL_HL_H             11
#define LAYOUT_CAL_BAND1_Y          11
#define LAYOUT_CAL_BAND1_H          11
#define LAYOUT_CAL_BAND2_Y          33
#define LAYOUT_CAL_BAND2_H          13
#define LAYOUT_CAL_ROW1_Y           7
#define LAYOUT_CAL_ROW2_Y           18
#define LAYOUT_CAL_ROW3_Y           29

#define LAYOUT_CLOCK_FONT           FONT_KEY_ROBOTO_BOLD_SUBSET_49
#define LAYOUT_PM_FONT              FONT_KEY_GOTHIC_14
#define LAYOUT_WEEK_FONT            FONT_KEY_GOTHIC_14
#define LAYOUT_DATE_FONT            FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_CURR_TEMP_FONT       FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_WIND_FONT            FONT_KEY_GOTHIC_18
#define LAYOUT_FORECAST_DAY_FONT    FONT_KEY_GOTHIC_14_BOLD
#define LAYOUT_STATUS_FONT          FONT_KEY_GOTHIC_14
#define LAYOUT_TEMP_FONT            FONT_KEY_GOTHIC_24
#define LAYOUT_SUN_TEXT_FONT        FONT_KEY_GOTHIC_18
#define LAYOUT_CONDITION_FONT       FONT_KEY_GOTHIC_18
#define LAYOUT_CAL_FONT             FONT_KEY_GOTHIC_14
#define LAYOUT_CAL_FONT_BOLD        FONT_KEY_GOTHIC_14_BOLD

#else

// -------------------------------------------- 144x168 rect: aplite, basalt, diorite

#define LAYOUT_CURRENT_FRAME        GRect(0, 0, PBL_DISPLAY_WIDTH, 58)
#define LAYOUT_CLOCK_FRAME          GRect(-1, -13, 126, 50)
#define LAYOUT_PM_FRAME             GRect(123, 23, 20, 15)
#define LAYOUT_WEEK_FRAME           GRect(122, 23, 24, 15)
#define LAYOUT_DATE_FRAME           GRect(54, 30, 89, 26)
#define LAYOUT_DATE_ALIGN           GTextAlignmentRight
#define LAYOUT_BT_FRAME             GRect(128, 0, 11, 18)
#define LAYOUT_BATT_FRAME           GRect(126, 18, 16, 8)
#define LAYOUT_CURR_TEMP_FRAME      GRect(0, 30, 45, 26)
#define LAYOUT_CURR_TEMP_ALIGN      GTextAlignmentLeft
#define LAYOUT_WIND_FRAME           GRect(45, 34, 45, 26)
#define LAYOUT_WIND_ALIGN           GTextAlignmentCenter

#define LAYOUT_FORECAST_FRAME       GRect(0, 57, PBL_DISPLAY_WIDTH, 64)
#define LAYOUT_STATUS_BG_FRAME      GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_STEPS_FRAME          GRect(0, -4, PBL_DISPLAY_WIDTH, 17)
#define LAYOUT_FORECAST_DAY_FRAME   GRect(0, -4, 64, 17)
#define LAYOUT_STATUS_FRAME         GRect(60, -4, 84, 17)
#define LAYOUT_HIGH_LABEL_FRAME     GRect(1, 6, 10, 24)
#define LAYOUT_HIGH_TEMP_FRAME      GRect(9, 6, 45, 24)
#define LAYOUT_LOW_LABEL_FRAME      GRect(1, 23, 10, 24)
#define LAYOUT_LOW_TEMP_FRAME       GRect(9, 23, 45, 24)
#define LAYOUT_SUN_TEXT_FRAME       GRect(101, 26, 47, 18)
#define LAYOUT_ICON_FRAME           GRect(66, 16, 32, 32)
#define LAYOUT_SUN_ICON_FRAME       GRect(115, 17, 20, 14)
#define LAYOUT_CONDITION_FRAME      GRect(0, 43, PBL_DISPLAY_WIDTH, 24)

#define LAYOUT_CAL_FRAME            GRect(0, 122, PBL_DISPLAY_WIDTH, 47)

#define LAYOUT_CAL_CELL_PITCH       21
#define LAYOUT_CAL_CELL_W           19
#define LAYOUT_CAL_HDR_Y            (-4)
#define LAYOUT_CAL_TEXT_H           14
#define LAYOUT_CAL_HL_DY            4
#define LAYOUT_CAL_HL_H             11
#define LAYOUT_CAL_BAND1_Y          11
#define LAYOUT_CAL_BAND1_H          11
#ifdef PBL_COLOR
#define LAYOUT_CAL_BAND2_Y          33
#define LAYOUT_CAL_BAND2_H          13
#define LAYOUT_CAL_ROW1_Y           7
#define LAYOUT_CAL_ROW2_Y           18
#define LAYOUT_CAL_ROW3_Y           29
#else
#define LAYOUT_CAL_BAND2_Y          35
#define LAYOUT_CAL_BAND2_H          11
#define LAYOUT_CAL_ROW1_Y           7
#define LAYOUT_CAL_ROW2_Y           19
#define LAYOUT_CAL_ROW3_Y           31
#endif

#define LAYOUT_CLOCK_FONT           FONT_KEY_ROBOTO_BOLD_SUBSET_49
#define LAYOUT_PM_FONT              FONT_KEY_GOTHIC_14
#define LAYOUT_WEEK_FONT            FONT_KEY_GOTHIC_14
#define LAYOUT_DATE_FONT            FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_CURR_TEMP_FONT       FONT_KEY_GOTHIC_24_BOLD
#define LAYOUT_WIND_FONT            FONT_KEY_GOTHIC_18
#define LAYOUT_FORECAST_DAY_FONT    FONT_KEY_GOTHIC_14_BOLD
#define LAYOUT_STATUS_FONT          FONT_KEY_GOTHIC_14
#define LAYOUT_TEMP_FONT            FONT_KEY_GOTHIC_24
#define LAYOUT_SUN_TEXT_FONT        FONT_KEY_GOTHIC_18
#define LAYOUT_CONDITION_FONT       FONT_KEY_GOTHIC_18
#define LAYOUT_CAL_FONT             FONT_KEY_GOTHIC_14
#define LAYOUT_CAL_FONT_BOLD        FONT_KEY_GOTHIC_14_BOLD

#endif

// The round layout puts the date across the top where the rect layout puts wind
// speed, so wind stays a rectangular-only element.
#ifdef PBL_ROUND
#define LAYOUT_SHOW_WIND_LAYER      0
#else
#define LAYOUT_SHOW_WIND_LAYER      1
#endif
