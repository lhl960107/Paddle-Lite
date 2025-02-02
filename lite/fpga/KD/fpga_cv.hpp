/* Copyright (c) 2019 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#pragma once

#include <stdlib.h>
#include "lite/fpga/KD/float16.hpp"
#include "lite/fpga/KD/llapi/zynqmp_api.h"
#include "lite/fpga/KD/pe.hpp"

void fpga_resize(float* input,
                 int input_width,
                 int input_height,
                 int input_channel,
                 uint8_t* output,
                 int output_width,
                 int output_height);
