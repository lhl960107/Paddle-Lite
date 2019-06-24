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

#ifdef SOFTMAX_OP

#include "operators/kernel/softmax_kernel.h"
#include "operators/kernel/central-arm-func/softmax_arm_func.h"

namespace paddle_mobile {
namespace operators {

template <>
bool SoftmaxKernel<FPGA, float>::Init(SoftmaxParam<FPGA> *param) {
  auto input = const_cast<LoDTensor *>(param->InputX());
  auto dims = framework::vectorize(input->dims());

  auto out = param->Out();
  out->Resize(framework::make_ddim(dims));

  int input_c = 1, input_h = 1, input_w = 1;
  if (dims.size() == 4) {
    input_h = dims[1];
    input_w = dims[2];
    input_c = dims[3];
    if (input_c == 1) {  // This input is generated by FC op, dims = [N C 1 1]
      PADDLE_MOBILE_ENFORCE(input_w == 1, "Softmax input must come from FC op");
      input_c = dims[1];
      input_h = 1;
    }
  } else if (dims.size() == 2) {
    input_c = dims[1];
  }

  input->Resize(framework::make_ddim(dims));
  if ((input_c == 2) && (input->type() == type_id<int8_t>())) {
    auto input_ptr = input->data<int8_t>();
    float Si = input->scale[0];
    int16_t slope = fpga::fp32_2_fp16(Si / 127);
    out->mutable_data<int8_t>(framework::make_ddim(dims));
    fpga::format_ofm(out);
    fpga::BypassArgs args = {fpga::DATA_TYPE_FP16};
    args.input_layout_type = fpga::LAYOUT_HWC;
    args.output_layout_type = fpga::LAYOUT_CHW;
    args.input_data_type = fpga::DATA_TYPE_FP16;
    args.output_data_type = fpga::DATA_TYPE_FP16;
    args.image.address = input_ptr;
    args.image.height = input_h;
    args.image.width = input_w;
    args.image.channels = input_c;
    args.output.address = out->data<int8_t>();
    args.output.scale_address = out->scale;
    args.output.activation.activation_type = fpga::SOFTMAX;
    args.output.activation.leaky_relu_negative_slope = slope;
    param->SetFpgaArgs(args);
  } else {
    out->mutable_data<float>(framework::make_ddim(dims));
    fpga::format_ofm(out);
  }

  return true;
}

template <>
void SoftmaxKernel<FPGA, float>::Compute(const SoftmaxParam<FPGA> &param) {
  auto *in_x = (param.InputX());
  auto dims = in_x->dims();

  auto n = 1;
  auto h = 1;
  auto w = 1;
  auto c = 1;
  if (dims.size() == 4) {
    h = dims[1];
    w = dims[2];
    c = dims[3];
    if (c == 1) {  // This input is generated by FC op, dims = [N C 1 1]
      PADDLE_MOBILE_ENFORCE(w == 1, "Softmax input must come from FC op");
      c = dims[1];
      h = 1;
    }
  } else if (dims.size() == 2) {
    c = dims[1];
  }
  if ((c == 2) && (in_x->type() == type_id<int8_t>())) {
    fpga::PerformBypass(param.FpgaArgs());
  } else if (in_x->type() == type_id<int8_t>()) {
    auto in_data = in_x->data<int8_t>();
    float Si = in_x->scale[0];
    Tensor *out = param.Out();
    out->Resize({n, h, w, c});
    auto float_input_x = param.float_input_x_;
    float_input_x = std::make_shared<Tensor>();
    float_input_x->Resize(in_x->dims());
    float_input_x->init(type_id<float>().hash_code());
    fpga::format_fp32_ofm(float_input_x.get());
    auto float_input_x_data = float_input_x->data<float>();
    int dataNum = n * h * fpga::align_to_x(w * c, IMAGE_ALIGNMENT);
    for (int i = 0; i < dataNum; i++) {
      float_input_x_data[i] = in_data[i] * Si / 127;
    }
    math::SoftmaxFuntor<CPU, float>()(float_input_x.get(), out);
  } else {
    Tensor *out = param.Out();
    out->Resize({n, h, w, c});
    math::SoftmaxFuntor<CPU, float>()(in_x, out);
  }
}

}  // namespace operators
}  // namespace paddle_mobile

#endif
