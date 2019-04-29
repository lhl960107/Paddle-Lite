/* Copyright (c) 2018 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#ifdef FUSION_FC_OP

#include "operators/kernel/fusion_fc_kernel.h"
#include "operators/kernel/central-arm-func/fusion_fc_arm_func.h"

namespace paddle_mobile {
namespace operators {

template <>
bool FusionFcKernelCpu<float>::Init(FusionFcParam *param) {
  return true;
}

template <>
void FusionFcKernelCpu<float>::Compute(const FusionFcParam &param) {
  FusionFcCompute<float, float>(param);
  param.Out()->LodTensor()->set_lod(param.InputX()->LodTensor()->lod());
}

template class FusionFcKernelCpu<float>;

#ifdef FUSION_FC_INT8_OP
template <>
bool FusionFcKernelCpu<int8_t>::Init(FusionFcParam *param) {
  return true;
}

template <>
void FusionFcKernelCpu<int8_t>::Compute(const FusionFcParam &param) {
  FusionFcCompute<int8_t, int32_t>(param);
  param.Out()->InnerLoDTensor()->set_lod(
      param.InputX()->InnerLoDTensor()->lod());
}

template class FusionFcKernelCpu<int8_t>;
#endif

}  // namespace operators
}  // namespace paddle_mobile

#endif
