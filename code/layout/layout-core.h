// ======================================================================
//                                 WINDOW                                
// ======================================================================

#define WINDOW_RADIUS 14
#define WINDOW_MARGIN 8
#define WINDOW_HEIGHT 40
#define WINDOW_WIDTH (2560 - 2 * WINDOW_MARGIN)

// -------------------------------- rect --------------------------------

#define WINDOW_CENTER(h) ((WINDOW_HEIGHT - h) / 2.0)

// ======================================================================
//                                 BUTTON                                
// ======================================================================

#define BUTTON_WIDTH 52
#define BUTTON_HEIGHT 26
#define BUTTON_FONT 26
#define BUTTON_RADIUS 10
#define BUTTON_MARGIN 12

// -------------------------------- rect --------------------------------

#define BUTTON_OUTER_H WINDOW_HEIGHT
#define BUTTON_OUTER_W (2 * BUTTON_MARGIN + BUTTON_WIDTH)
#define BUTTON_OUTER_X 0
#define BUTTON_OUTER_Y 0

#define BUTTON_INNER_H BUTTON_HEIGHT
#define BUTTON_INNER_W BUTTON_WIDTH
#define BUTTON_INNER_X BUTTON_MARGIN
#define BUTTON_INNER_Y WINDOW_CENTER(BUTTON_INNER_H)

// ======================================================================
//                                   APP                                  
// ======================================================================

#define APP_SIZE 26
#define APP_MARGIN_INNER 17
#define APP_MARGIN_OUTER 20

// -------------------------------- rect --------------------------------

#define APP_OUTER_H WINDOW_HEIGHT
#define APP_OUTER_W (2 * APP_MARGIN_INNER + APP_SIZE)
#define APP_OUTER_X 0
#define APP_OUTER_Y 0

#define APP_INNER_H APP_SIZE
#define APP_INNER_W APP_SIZE
#define APP_INNER_X APP_MARGIN_INNER
#define APP_INNER_Y WINDOW_CENTER(APP_INNER_H)

#define APPS_H APP_INNER_H
#define APPS_W (APP_OUTER_W * 8)
#define APPS_X APP_MARGIN_INNER
#define APPS_Y WINDOW_CENTER(APP_INNER_H)

// ======================================================================
//                                 PLAYER                                
// ======================================================================

#define PLAYER_MARGIN_OUTER 10
#define PLAYER_MARGIN_INNER 5
#define PLAYER_WIDTH 380

#define PLAYER_TEXT_SIZE 16
#define PLAYER_TEXT_OFFSET 1

#define PLAYER_BUTTON_OFFSET 2
#define PLAYER_BUTTON_RADIUS 7
#define PLAYER_BUTTON_MARGIN 4
#define PLAYER_BUTTON_HEIGHT 18
#define PLAYER_BUTTON_WIDTH 35

#define PLAYER_IMAGE_SIZE 38
#define PLAYER_IMAGE_RADIUS 13
#define PLAYER_TAG_SIZE 13

#define PLAYER_BAR_HEIGHT 6

// -------------------------------- rect --------------------------------

#define PLAYER_OUTER_H WINDOW_HEIGHT
#define PLAYER_OUTER_W PLAYER_WIDTH
#define PLAYER_OUTER_X 0
#define PLAYER_OUTER_Y 0

#define PLAYER_IMAGE_H PLAYER_IMAGE_SIZE
#define PLAYER_IMAGE_W PLAYER_IMAGE_SIZE
#define PLAYER_IMAGE_X PLAYER_MARGIN_OUTER
#define PLAYER_IMAGE_Y WINDOW_CENTER(PLAYER_IMAGE_H)

#define PLAYER_TEXT_H PLAYER_TEXT_SIZE
#define PLAYER_TEXT_X (PLAYER_IMAGE_X + PLAYER_IMAGE_W + PLAYER_MARGIN_INNER)
#define PLAYER_TEXT_Y PLAYER_TEXT_OFFSET
#define PLAYER_TEXT_W (PLAYER_WIDTH - PLAYER_TEXT_X - PLAYER_MARGIN_OUTER)

#define PLAYER_BUTTON_H PLAYER_BUTTON_HEIGHT
#define PLAYER_BUTTON_W PLAYER_BUTTON_WIDTH
#define PLAYER_BUTTON_X (PLAYER_IMAGE_X + PLAYER_IMAGE_W + PLAYER_MARGIN_INNER)
#define PLAYER_BUTTON_Y (WINDOW_HEIGHT - PLAYER_BUTTON_HEIGHT - PLAYER_BUTTON_OFFSET)

#define PLAYER_BUTTONS_H PLAYER_BUTTON_HEIGHT
#define PLAYER_BUTTONS_W (3 * PLAYER_BUTTON_WIDTH + 2 * PLAYER_MARGIN_INNER)
#define PLAYER_BUTTONS_X PLAYER_BUTTON_X
#define PLAYER_BUTTONS_Y PLAYER_BUTTON_Y

#define PLAYER_BAR_H PLAYER_BAR_HEIGHT
#define PLAYER_BAR_X (PLAYER_BUTTON_X + 3 * (PLAYER_BUTTON_W + PLAYER_MARGIN_INNER))
#define PLAYER_BAR_Y (WINDOW_HEIGHT - (PLAYER_BUTTON_HEIGHT + PLAYER_BAR_HEIGHT) / 2.0 - PLAYER_BUTTON_OFFSET)
#define PLAYER_BAR_W (PLAYER_WIDTH - PLAYER_BAR_X - PLAYER_MARGIN_OUTER)

// ======================================================================
//                                WORKSPACE                               
// ======================================================================

#define WORKSPACE_STROKE 2
#define WORKSPACE_RADIUS 4
#define WORKSPACE_MARGIN 6
#define WORKSPACE_WIDTH 36
#define WORKSPACE_HEIGHT 18

// -------------------------------- rect --------------------------------

#define WORKSPACE_OUTER_H WINDOW_HEIGHT
#define WORKSPACE_OUTER_W (2 * WORKSPACE_MARGIN + WORKSPACE_WIDTH)
#define WORKSPACE_OUTER_X 0
#define WORKSPACE_OUTER_Y 0

#define WORKSPACE_INNER_H WORKSPACE_HEIGHT
#define WORKSPACE_INNER_W WORKSPACE_WIDTH
#define WORKSPACE_INNER_X WORKSPACE_MARGIN
#define WORKSPACE_INNER_Y WINDOW_CENTER(WORKSPACE_HEIGHT)

#define WORKSPACE_ALL_H WORKSPACE_INNER_H
#define WORKSPACE_ALL_W (WORKSPACE_OUTER_W * 10)
#define WORKSPACE_ALL_X 0
#define WORKSPACE_ALL_Y WORKSPACE_INNER_Y

// ======================================================================
//                                  CLOCK                                 
// ======================================================================

#define CLOCK_MARGIN_OUTER 20

#define CLOCK_TIME_OFFSET 3
#define CLOCK_TIME_SIZE 19
#define CLOCK_TIME_LENGTH 60

#define CLOCK_DATE_OFFSET 3
#define CLOCK_DATE_SIZE 14
#define CLOCK_DATE_LENGTH 125

// -------------------------------- rect --------------------------------

#define CLOCK_OUTER_H WINDOW_WIDTH
#define CLOCK_OUTER_W (CLOCK_DATE_LENGTH + 2 * CLOCK_MARGIN_OUTER)
#define CLOCK_OUTER_X 0
#define CLOCK_OUTER_Y 0

#define CLOCK_TIME_H CLOCK_TIME_SIZE
#define CLOCK_TIME_W CLOCK_TIME_LENGTH
#define CLOCK_TIME_X (CLOCK_MARGIN_OUTER + CLOCK_DATE_LENGTH - CLOCK_TIME_LENGTH)
#define CLOCK_TIME_Y CLOCK_TIME_OFFSET

#define CLOCK_DATE_H CLOCK_DATE_SIZE
#define CLOCK_DATE_W CLOCK_DATE_LENGTH
#define CLOCK_DATE_X CLOCK_MARGIN_OUTER
#define CLOCK_DATE_Y (WINDOW_HEIGHT - CLOCK_DATE_H - CLOCK_TIME_OFFSET)

// ======================================================================
//                                  METER                                 
// ======================================================================

#define METER_MARGIN_INNER 10
#define METER_MARGIN_OUTER 10
#define METER_ICON_SIZE 22
#define METER_TEXT_SIZE 18
#define METER_FILL_SIZE 32
#define METER_FILL_FONT 14
#define METER_FILL_STROKE 4.5

#define METER_TEXT_WIDE_LENGTH 80
#define METER_TEXT_THIN_LENGTH 50

// -------------------------------- rect --------------------------------

#define METER_ICON_H METER_ICON_SIZE
#define METER_ICON_W METER_ICON_SIZE
#define METER_ICON_X METER_MARGIN_OUTER
#define METER_ICON_Y WINDOW_CENTER(METER_ICON_H)

#define METER_FILL_H METER_FILL_SIZE
#define METER_FILL_W METER_FILL_SIZE
#define METER_FILL_X (METER_ICON_X + METER_ICON_SIZE + METER_MARGIN_INNER)
#define METER_FILL_Y WINDOW_CENTER(METER_ICON_H)

#define METER_TEXT_X (METER_FILL_X + METER_FILL_W + METER_MARGIN_INNER)

#define METER_TEXT_WIDE_H METER_TEXT_SIZE
#define METER_TEXT_WIDE_W METER_TEXT_WIDE_LENGTH
#define METER_TEXT_WIDE_X METER_TEXT_X
#define METER_TEXT_WIDE_Y WINDOW_CENTER(METER_TEXT_WIDE_W)

#define METER_TEXT_THIN_H METER_TEXT_SIZE
#define METER_TEXT_THIN_W METER_TEXT_THIN_LENGTH
#define METER_TEXT_THIN_X METER_TEXT_X
#define METER_TEXT_THIN_Y WINDOW_CENTER(METER_TEXT_WIDE_W)

// ======================================================================
//                                  AUTO                                 
// ======================================================================

#define MENU_POS 0

#define APPS_POS (MENU_POS + BUTTON_OUTER_W + APP_MARGIN_OUTER)
#define APP_POS(i) (APPS_POS + i * APP_OUTER_W + APP_INNER_X)

#define PLAYER_POS (APP_POS(8) - APP_INNER_X + APP_MARGIN_OUTER)

#define PLAYER_BUTTON_POS(i) (PLAYER_POS + i * (PLAYER_BUTTON_W + PLAYER_MARGIN_INNER))

#define WORKSPACES_POS ((WINDOW_WIDTH / 2.0 - 5 * WORKSPACE_OUTER_W))
#define WORKSPACE_POS(i) (WORKSPACES_POS + i * WORKSPACE_OUTER_W + WORKSPACE_INNER_X)

#define METERS_POS (WINDOW_WIDTH - 14)
#define METER4_POS (METERS_POS - METER_TEXT_X - METER_TEXT_WIDE_LENGTH - METER_MARGIN_OUTER)
#define METER3_POS (METER4_POS - METER_TEXT_X - METER_TEXT_WIDE_LENGTH - METER_MARGIN_OUTER)
#define METER2_POS (METER3_POS - METER_TEXT_X - METER_TEXT_WIDE_LENGTH - METER_MARGIN_OUTER)
#define METER1_POS (METER2_POS - METER_TEXT_X - METER_TEXT_THIN_LENGTH - METER_MARGIN_OUTER)
#define METER0_POS (METER1_POS - METER_TEXT_X - METER_TEXT_THIN_LENGTH - METER_MARGIN_OUTER)

#define CLOCK_POS (METER0_POS - CLOCK_OUTER_W)
