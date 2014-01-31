////////////////////////////////////////////////////////////////////////////////
//! @file	: CustomKernel.h
//! @date   : Jul 2013
//!
//! @brief  : Custom kernel creation for image processing
//! 
//! Copyright (C) 2013 - CRVI
//!
//! This file is part of OpenCLIPP.
//! 
//! OpenCLIPP is free software: you can redistribute it and/or modify
//! it under the terms of the GNU Lesser General Public License version 3
//! as published by the Free Software Foundation.
//! 
//! OpenCLIPP is distributed in the hope that it will be useful,
//! but WITHOUT ANY WARRANTY; without even the implied warranty of
//! MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//! GNU Lesser General Public License for more details.
//! 
//! You should have received a copy of the GNU Lesser General Public License
//! along with OpenCLIPP.  If not, see <http://www.gnu.org/licenses/>.
//! 
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "C++/Programs/Program.h"

#include "kernel_helpers.h"

/*

This file proposes a way to create custom kernels that operate on single pixels of images

The macro IMAGE_KERNEL(CL, code, in, out, ...) works like this :
   CL       is the COpenCL object to use
   code     is the mathematical operations to do on the pixels, any OpenCL C code can be used
   in       is a list of names for input images, at least one image is needed
   out      is a list of names for output images
   ...      is a list of non-image arguments for the kernel

   The macro creates an object of type ImageProgram, with automatically generated OpenCL C source.
   Upon creation, the object will build the source.
   The kernel can then be invoked with the macro LAUNCH()

The macro LAUNCH(program, in, out, ...) works like this :
   program  is the object generated by the IMAGE_KERNEL() macro
   in       is a list of image objects (descendents of IImage)
   out      is a list of image objects (descendents of IImage)
   ...      is a list of arguments
   The name and type of inputs, outputs and arguments must match what was used when calling IMAGE_KERNEL()
   All images must be have same sizes and be of compatible types :
      different image depth is allowed
      different number of channels is allowed
      mixing float/signed/unsiged is not allowed and will result in undefined behaviour
   Input images will automatically be sent to the device if they have not been sent already.
   Kernel execution is asynchronous.
   Output images are not read to the host after kernel execution.
   Use Read(true) on the output images to wait for kernel execution and read the output images if needed.


   Example :
      COpenCL CL;

      ImageProgram K = IMAGE_KERNEL(CL, R = A + B * C, In(A, B), Out(R), float C);

      LAUNCH(K, In(Img1, Img2), Out(Img3), 0.5f);

      Img3.Read(true);


   The version IMAGE_KERNEL_T() operates in the same way but takes text for the code, allowing for more complex code :
      IMAGE_KERNEL(CL, "float4 Temp1, Temp2; R = A + B * C; // Comment \n", In(A, B), Out(R), float C);

   Please note that building the source takes a significant amount of time.
   To prevent having that overhead, generate the ImageProgram object once and keep it as long as it is needed.

   Agruments passed to LAUNCH() must have exactly the same type as declared with IMAGE_KERNEL().
   LAUCH() does not verify the types it is given and will not do any type conversion :
      if the type is of a different size, CL_INVALID_ARG_SIZE will be thrown
      if the type is different but has the same size, the value will be reinterpreted, not converted
   In the example above, putting 0.5 (which is a double) or 1 (which is an int) instead of 0.5f would lead to incorrect behaviour.

*/

#define _INPUT_ARG(name) read_only image2d_t name ## _in
#define _OUTPUT_ARG(name) write_only image2d_t name ## _out
#define _INPUT_VAR(name) float4 name = READ_IMAGE( name ## _in , pos);
#define _OUTPUT_VAR(name) float4 name;
#define _OUTPUT_WRITE(name) WRITE_IMAGE( name ## _out , pos, name);

#define IMAGE_KERNEL_SOURCE(name, code, in, out, ...) \
"constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;\n"\
"#ifdef UI\n"\
"#define READ_IMAGE(img, pos) convert_float4(read_imageui(img, sampler, pos))\n"\
"#define WRITE_IMAGE(img, pos, px) write_imageui(img, pos, convert_uint4_sat(px))\n"\
"#else\n"\
"#define READ_IMAGE(img, pos) read_imagef(img, sampler, pos)\n"\
"#define WRITE_IMAGE(img, pos, px) write_imagef(img, pos, px)\n"\
"#endif\n"\
"kernel void " #name "(" STR(FOR_EACH_COMMA(_INPUT_ARG, in) ADD_COMMA(out) FOR_EACH_COMMA(_OUTPUT_ARG, out))\
   STR(ADD_COMMA(__VA_ARGS__)) #__VA_ARGS__ ")"\
"{"\
   "const int gx = get_global_id(0);"\
   "const int gy = get_global_id(1);"\
   "const int2 pos = { gx, gy };"\
   STR(FOR_EACH(_INPUT_VAR, in))\
   STR(FOR_EACH(_OUTPUT_VAR, out))\
   code ";"\
   STR(FOR_EACH(_OUTPUT_WRITE, out))\
"}"

#define IMAGE_KERNEL(CL, code, in, out, ...) ImageProgram(CL, true, IMAGE_KERNEL_SOURCE(custom_kernel, #code, in, out, __VA_ARGS__))
#define IMAGE_KERNEL_T(CL, code, in, out, ...) ImageProgram(CL, true, IMAGE_KERNEL_SOURCE(custom_kernel, code, in, out, __VA_ARGS__))

#define LAUNCH(program, in, out, ...) Kernel_(program.GetCL(), program.SelectProgram(_FIRST_IN(in)),\
   custom_kernel, DEFAULT_LOCAL_RANGE, in, out, __VA_ARGS__);

/*inline void Test()
{
   COpenCL CL;

   ImageProgram test = IMAGE_KERNEL(CL, Out1 = In1 + Arg;, In(In1), Out(Out1), int Arg, int Arg2);

   SImage SImg;
   Image Img1(CL, SImg), Img2(CL, SImg);

   LAUNCH(test, In(Img1), Out(Img2), 5, 3);


   ImageProgram test2 = IMAGE_KERNEL(CL, Out1 = In1 + Arg * In2;, In(In1, In2, In3), Out(Out1, Out2), int Arg, float Arg2);
     
   LAUNCH(test2, In(Img1, Img2, Img1), Out(Img2, Img1), 5, 0);
}*/