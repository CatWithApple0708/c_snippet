#pragma once
/*
   _                   ___
  | |   ___  ___ _ __ / _ \ _  _ ___ _  _ ___
  | |__/ _ \/ _ \ '_ \ (_) | || / -_) || / -_)
  |____\___/\___/ .__/\__\_\\_,_\___|\_,_\___|
                |_|
*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct loop_queue_s loop_queue_t;
struct loop_queue_s {};
uint8_t loop_queue_read_one(loop_queue_t *queue);
uint32_t loop_queue_get_size(loop_queue_t *queue);
