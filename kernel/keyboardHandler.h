#ifndef KEYBOARD_HANDLER_H
#define KEYBOARD_HANDLER_H

#include "types.h"

#include "printUtils.h"

#include "memUtils.h"

#include "utils.h"

#include "drivers/ata.h"

void keyboardHandler_c(uint8_t scancode);
void processBuffer();
void clearBuffer();

#endif