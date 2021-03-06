////////////////////////////////////////////////////////////////////////////////
//! @file	: OpenCL.h 
//! @date   : Jul 2013
//!
//! @brief  : COpenCL object - takes care of initializing OpenCL
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


#include "Basic.h"

// We are working with exceptions instead of having error checking on every cl api call
#define __CL_ENABLE_EXCEPTIONS

#include <cl/cl.hpp>


#include <memory>

namespace OpenCLIPP
{

class Color;

/// Takes care of initializing OpenCL
/// Contains an OpenCL Device, Context and CommandQueue
/// An instance of this object is needed to create most other object of the library
class CL_API COpenCL
{
public:

   /// Initializes OpenCL.
   /// \param PreferredPlatform : Can be set to a specific platform (Ex: "Intel") and
   ///         that platform will be used if available. If the preferred platform is not
   ///         found or is not specified, the default OpenCL platform will be used.
   /// \param deviceType : can be used to specicy usage of a device type (Ex: CL_DEVICE_TYPE_GPU)
   ///         See cl_device_type for allowed values
   COpenCL(const char * PreferredPlatform = "", cl_device_type deviceType = CL_DEVICE_TYPE_ALL);

   /// Returns the name of the OpenCL device
   std::string GetDeviceName() const;

   /// Returns the name of the OpenCL error
   /// \param status : An OpenCL error code
   /// \return the name of the given error code
   static const char * ErrorName(cl_int status);

   /// Tells COpenCL where the .cl files are located.
   /// It must be called with the full path of "CL/CL/cl files".
   /// .cl file location must be specified before creating any program.
   /// \param Path : Full path where the .cl files are located
   static void SetClFilesPath(const char * Path);

   /// Returns the .cl files path (for internal use)
   /// \return the path where the .cl files are located
   static const std::string& GetClFilePath();

   /// Returns the OpenCL CommandQueue (for internal use)
   cl::CommandQueue& GetQueue();

   /// Returns the color image converter program (for internal use)
   Color& GetColorConverter();

   operator cl::Context& ();        ///< Converts to a cl::Context
   operator cl::CommandQueue& ();   ///< Converts to a cl::CommandQueue
   operator cl::Device& ();         ///< Converts to a cl::Device

   operator cl_context ();          ///< Converts to a cl_context
   operator cl_command_queue ();    ///< Converts to a cl_command_queue
   operator cl_device_id ();        ///< Converts to a cl_device_id

   /// True when running on Intel platform on CPU device (for internal use)
   bool IsOnIntelCPU() const;

   /// No-copy support on the OpenCL device.
   /// True when the device uses the same memory for the host and the device.
   /// When true, image transfers to the device are instantaneous.
   bool SupportsNoCopy() const;

   /* Running custom kernels :

   COpenCL CL;

   cl::Program MyProgram(CL, MyKernel_Source, true);  // Compile a program from kernel

   auto MyKernel = cl::make_kernel<arg1_type, arg2_type, ...>(MyProgram, "my_kernel_name");  // Prepare a kernel object

   MyKernel(cl::EnqueueArgs(CL.GetQueue(), MyGlobalRange), arg1, arg2, ...);     // Enqueue the kernel execution

   // See OpenCL C++ API doc for more detail : http://www.khronos.org/registry/cl/specs/opencl-cplusplus-1.2.pdf
   */

protected:
   cl::Platform m_Platform;      ///< The OpenCL platform (like nVidia)
   cl::Device m_Device;          ///< The OpenCL device (like GTX 680)
   cl::Context m_Context;        ///< The OpenCL context for the device
   cl::CommandQueue m_Queue;     ///< The OpenCL command queue for the context

   std::shared_ptr<Color> m_ColorConverter;  ///< Instance of the color converter program used to automatically convert 3 channel images to 4 channel images

   static std::string m_ClFilesPath;   ///< Path to the .cl files
};

}  // End of namespace
