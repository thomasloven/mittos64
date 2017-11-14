#pragma once

#define GDT_CODE      (3<<11)
#define GDT_DPL(lvl)  ((lvl)<<13)
#define GDT_PRESENT   (1<<15)
#define GDT_LONG      (1<<21)
