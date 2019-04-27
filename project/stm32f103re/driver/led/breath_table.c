
#include <stdlib.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>


uint16_t breath_table[] = {1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 9, 9, 9, 9, 10, 10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 15, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 23, 24, 25, 25, 26, 26, 27, 28, 28, 29, 30, 30, 31, 32, 33, 33, 34, 35, 36, 36, 37, 38, 39, 40, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 49, 50, 51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 65, 66, 67, 68, 69, 71, 72, 73, 74, 76, 77, 78, 80, 81, 83, 84, 85, 87, 88, 90, 91, 93, 94, 96, 97, 99, 100, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119, 121, 122, 124, 126, 128, 130, 132, 134, 135, 137, 139, 141, 143, 145, 147, 149, 151, 153, 155, 157, 159, 162, 164, 166, 168, 170, 172, 175, 177, 179, 181, 183, 186, 188, 190, 193, 195, 197, 199, 202, 204, 207, 209, 211, 214, 216, 219, 221, 223, 226, 228, 231, 233, 236, 238, 241, 243, 246, 248, 251, 253, 256, 258, 261, 264, 266, 269, 271, 274, 276, 279, 282, 284, 287, 289, 292, 295, 297, 300, 302, 305, 307, 310, 313, 315, 318, 320, 323, 326, 328, 331, 333, 336, 338, 341, 343, 346, 348, 351, 353, 356, 358, 361, 363, 366, 368, 371, 373, 376, 378, 380, 383, 385, 387, 390, 392, 394, 397, 399, 401, 403, 406, 408, 410, 412, 414, 416, 418, 420, 422, 425, 427, 429, 430, 432, 434, 436, 438, 440, 442, 444, 445, 447, 449, 450, 452, 454, 455, 457, 458, 460, 461, 463, 464, 466, 467, 468, 470, 471, 472, 473, 475, 476, 477, 478, 479, 480, 481, 482, 483, 484, 485, 485, 486, 487, 488, 488, 489, 490, 490, 491, 491, 492, 492, 492, 493, 493, 493, 494, 494, 494, 494, 494, 494, 494, 494, 494, 494, 494, 494, 494, 493, 493, 493, 492, 492, 492, 491, 491, 490, 490, 489, 488, 488, 487, 486, 485, 485, 484, 483, 482, 481, 480, 479, 478, 477, 476, 475, 473, 472, 471, 470, 468, 467, 466, 464, 463, 461, 460, 458, 457, 455, 454, 452, 450, 449, 447, 445, 444, 442, 440, 438, 436, 434, 432, 430, 429, 427, 425, 422, 420, 418, 416, 414, 412, 410, 408, 406, 403, 401, 399, 397, 394, 392, 390, 387, 385, 383, 380, 378, 376, 373, 371, 368, 366, 363, 361, 358, 356, 353, 351, 348, 346, 343, 341, 338, 336, 333, 331, 328, 326, 323, 320, 318, 315, 313, 310, 307, 305, 302, 300, 297, 295, 292, 289, 287, 284, 282, 279, 276, 274, 271, 269, 266, 264, 261, 258, 256, 253, 251, 248, 246, 243, 241, 238, 236, 233, 231, 228, 226, 223, 221, 219, 216, 214, 211, 209, 207, 204, 202, 199, 197, 195, 193, 190, 188, 186, 183, 181, 179, 177, 175, 172, 170, 168, 166, 164, 162, 159, 157, 155, 153, 151, 149, 147, 145, 143, 141, 139, 137, 135, 134, 132, 130, 128, 126, 124, 122, 121, 119, 117, 115, 114, 112, 110, 109, 107, 105, 104, 102, 100, 99, 97, 96, 94, 93, 91, 90, 88, 87, 85, 84, 83, 81, 80, 78, 77, 76, 74, 73, 72, 71, 69, 68, 67, 66, 65, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 52, 51, 50, 49, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 40, 39, 38, 37, 36, 36, 35, 34, 33, 33, 32, 31, 30, 30, 29, 28, 28, 27, 26, 26, 25, 25, 24, 23, 23, 22, 22, 21, 21, 20, 20, 19, 19, 18, 18, 17, 17, 16, 16, 15, 15, 15, 14, 14, 13, 13, 13, 12, 12, 11, 11, 11, 10, 10, 10, 9, 9, 9, 9, 8, 8, 8, 7, 7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 1};

uint16_t breath_len = sizeof(breath_table)/sizeof(breath_table[0]);
