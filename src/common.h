#pragma once
#include "pico/stdlib.h"


#define MILLIS	(to_ms_since_boot(get_absolute_time()))


#define PHY_DORMANT_TIME_MS   20000
#define PHY_DORMANT_THRESHOLD 0.03f
#define PHY_FRICTION  0.8f
#define PHY_ROTATE_KOEF 2.0f


#define SCR_BL_CHANGE_PERIOD_MS 15


#if 0
#define FONT font24cyr
#define CARDS prophet_cards_ua
#define CARDS_MAX prophet_cards_ua_size
#else
#define FONT font24eng
#define CARDS prophet_cards_en
#define CARDS_MAX prophet_cards_en_size
#endif