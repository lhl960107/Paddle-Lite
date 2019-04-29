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

#ifdef CONV_OP

#include "operators/kernel/conv_kernel.h"
#include "operators/kernel/cl/cl-kernel-func/conv_func.h"

namespace paddle_mobile {
namespace operators {

template <>
bool ConvKernelGpu<float>::Init(ConvParam *param) {
  PADDLE_MOBILE_ENFORCE(param->Filter()->ClImage()->dims()[2] ==
                                param->Filter()->ClImage()->dims()[3] &&
                            param->Paddings()[0] == param->Paddings()[1],
                        "need equal");

  int offset = static_cast<int>(param->Filter()->ClImage()->dims()[2]) / 2 -
               static_cast<int>(param->Paddings()[1]);
  param->SetOffset(offset);

  DLOG << " init helper: " << &cl_helper_;
  DLOG << " conv kernel add kernel ~ ";
  DLOG << " width of one block: " << param->Filter()->ClImage()->dims()[3];
  DLOG << " height of one block: " << param->Filter()->ClImage()->dims()[2];
  DLOG << " filter dims: " << param->Filter()->ClImage()->dims();

  if (param->Filter()->ClImage()->dims()[2] == 1 &&
      param->Filter()->ClImage()->dims()[3] == 1) {
    param->Filter()->ClImage()->InitNImage(cl_helper_.CLContext(),
                                           cl_helper_.CLCommandQueue());
    this->cl_helper_.AddKernel("conv_1x1_spl", "conv_kernel.cl");
    DLOG << "conv 1x1";

  } else if (param->Filter()->ClImage()->dims()[1] == 1 &&
             param->Input()->ClImage()->dims()[1] ==
                 param->Output()->ClImage()->dims()[1] &&
             param->Filter()->ClImage()->dims()[2] == 3) {
    param->Filter()->ClImage()->InitDWImage(cl_helper_.CLContext(),
                                            cl_helper_.CLCommandQueue());
    this->cl_helper_.AddKernel("depth_conv_3x3", "depthwise_conv_kernel.cl");

  } else if (param->Filter()->ClImage()->dims()[2] == 3 &&
             param->Filter()->ClImage()->dims()[3] == 3) {
    param->Filter()->ClImage()->InitCLImage(cl_helper_.CLContext(),
                                            cl_helper_.CLCommandQueue());
    this->cl_helper_.AddKernel("conv_3x3", "conv_kernel.cl");

  } else {
    PADDLE_MOBILE_THROW_EXCEPTION(" not support ");
  }

  return true;
}

template <>
void ConvKernelGpu<float>::Compute(const ConvParam &param) {
  ConvAddBnRelu(this->cl_helper_, param);
}

template class ConvKernelGpu<float>;

}  // namespace operators
}  // namespace paddle_mobile

#endif
