////////////////////////////////////////////////////////////////////////////////
//! @file	: Program.h
//! @date   : Jul 2013
//!
//! @brief  : Objects representing OpenCL programs
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

/// Programs operate either on Image objects or on ImageBuffer objects
/// Some programs are available in both standard (Image) and Vector (ImageBuffer) versions.
/// For those programs, the Vector version is usually substantially faster as it will use vector processing

#pragma once

#include "../OpenCL.h"
#include "../Image.h"

namespace OpenCLIPP
{

/// Creates a cl::Program from a .cl file (or from source code)
class CL_API Program
{
public:

   /// Constructor - from .cl file.
   /// Program is not built automatically, Build() must be called before using the program.
   /// \param CL : A COpenCL instance
   /// \param Path : Path of the .cl file - must be relative to the path given by COpenCL::SetClFilesPath()
   /// \param options : A string to give as options when invoking the OpenCL C compiler (useful to specify defines)
   Program(COpenCL& CL, const char * Path, const char * options = "");

   /// Constructor - from source code.
   /// Program is not built automatically, Build() must be called before using the program.
   /// \param CL : A COpenCL instance
   /// \param FromSource : Indicates that source code is directly specified (instead of using a .cl file)
   /// \param Source : OpenCL C source code of the program
   /// \param options : A string to give as options when invoking the OpenCL C compiler (useful to specify defines)
   Program(COpenCL& CL, bool FromSource, const char * Source, const char * options = "");

   /// Destructor.
   virtual ~Program() { }

   /// Builds the OpenCL program.
   /// Uses either the source or the content of the .cl file given to one of the constructors
   /// as content of the program.
   /// Compiler errors are dealt in the following way : 
   ///  - A cl::Error exception is thrown
   ///  - Build messages are sent to std::cerr
   /// Take note that building an OpenCL C program can be long (sometimes >100ms).
   /// So build the program in advance if a result is desired quickly.
   /// \return true if build operation is succesful
   bool Build();

   operator cl::Program& ()
   {
      return m_Program;
   }

protected:
   COpenCL * m_CL;            ///< Pointer to the COpenCL object this program is assotiated to
   std::string m_Path;        ///< Path of the .cl file
   std::string m_Source;      ///< Given source of the program
   std::string m_Options;     ///< Options to give to the OpenCL C compiler
   cl::Program m_Program;     ///< The ecapsulated program object
   bool m_Built;              ///< true when the program has been successfully built

   static std::string LoadClFile(const std::string& Path);   ///< Reads the content of the given file
};


/// Holder of multiple Program (not useable directly)
class CL_API MultiProgram
{
public:
   virtual ~MultiProgram() { }   ///< Destructor

   COpenCL& GetCL() { return *m_CL; }  ///< Returns a reference to the COpenCL object this program is assotiated to

protected:
   MultiProgram(COpenCL& CL);    ///< Constructor

   /// Sets the information about a .cl program file
   /// \param Path : Path of the .cl file - must be relative to the path given by COpenCL::SetClFilesPath()
   /// \param NbPrograms : Number of different versions of the same program to create
   /// \param Defines : An array of NbPrograms strings that contain the defines (pre-defined preprocessor macros)
   ///   to use for each versions of the program
   void SetProgramInfo(const char * Path, uint NbPrograms, const char ** Defines);

   /// Sets the information about a program
   /// \param fromSource : Indicates wether a .cl file is used or if source code is supplied
   /// \param Source : Source code (used if fromSource is true)
   /// \param NbPrograms : Number of different versions of the same program to create
   /// \param Defines : An array of NbPrograms strings that contain the defines (pre-defined preprocessor macros)
   ///   to use for each versions of the program
   /// \param Path : Path of the .cl file - must be relative to the path given by COpenCL::SetClFilesPath().
   ///   used if fromSource is false
   void SetProgramInfo(bool fromSource, const char * Source, uint NbPrograms,
      const char ** Defines, const char * Path = "");

   Program& GetProgram(uint Id);    ///< Builds the program specified by Id and returns a reference to it

   void PrepareProgram(uint Id);    ///< Builds the program specified by Id

   COpenCL * m_CL;   ///< Pointer to the COpenCL object this program is assotiated to

private:
   typedef std::shared_ptr<Program> ProgramPtr;

   std::vector<ProgramPtr> m_Programs;
};

/// A program that operates on Images.
/// Contains 3 program versions : int, uint and float
class CL_API ImageProgram : public MultiProgram
{
public:

   /// Initialize the program with a .cl file.
   /// Program is not built by the constructor, it will be built when needed.
   /// Call PrepareFor() to have the program ready for later use.
   /// \param CL : A COpenCL instance
   /// \param Path : Path of the .cl file - must be relative to the path given by COpenCL::SetClFilesPath()
   ImageProgram(COpenCL& CL, const char * Path);

   /// Initialize the program with source code.
   /// Program is not built by the constructor, it will be built when needed.
   /// Call PrepareFor() to have the program ready for later use.
   /// \param CL : A COpenCL instance
   /// \param fromSource : Indicates that source code is directly specified (instead of using a .cl file)
   /// \param Source : OpenCL C source code of the program
   ImageProgram(COpenCL& CL, bool fromSource, const char * Source);

   /// Enumerates allowed pixel types
   enum EPixelTypes      // Keep in synch with list in constructor
   {
      Signed,        ///< Signed integer
      Unsigned,      ///< Unsigned integer
      Float,         ///< float (32-bit only)
      NbPixelTypes,
   };

   /// Build the version of the program appropriate for this image.
   /// Building can take a lot of time (100+ms) so it is better to build
   /// the program during when starting so it will be ready when needed.
   void PrepareFor(ImageBase& Source);

   /// Selects the appropriate program version for this image.
   /// Also builds the program version if it was not already built.
   Program& SelectProgram(ImageBase& Source);
};


/// A program that operates on ImageBuffers.
/// Contains a program version for each data type : S8, U8, S16, U16, S32, U32, F32
class CL_API ImageBufferProgram : public MultiProgram
{
public:

   /// Initialize the program with a .cl file.
   /// Program is not built by the constructor, it will be built when needed.
   /// Call PrepareFor() to have the program ready for later use.
   /// \param CL : A COpenCL instance
   /// \param Path : Path of the .cl file - must be relative to the path given by COpenCL::SetClFilesPath()
   ImageBufferProgram(COpenCL& CL, const char * Path);

   /// Initialize the program with source code.
   /// Program is not built by the constructor, it will be built when needed.
   /// Call PrepareFor() to have the program ready for later use.
   /// \param CL : A COpenCL instance
   /// \param fromSource : Indicates that source code is directly specified (instead of using a .cl file)
   /// \param Source : OpenCL C source code of the program
   ImageBufferProgram(COpenCL& CL, bool fromSource, const char * Source);

   /// Build the version of the program appropriate for this image.
   /// Building can take a lot of time (100+ms) so it is better to build
   /// the program during when starting so it will be ready when needed.
   void PrepareFor(ImageBase& Source);

   /// Selects the appropriate program version for this image.
   /// Also builds the program version if it was not already built.
   Program& SelectProgram(ImageBase& Source);

   const static int NbPixelTypes = SImage::NbDataTypes;  ///< Number of possible pixel types
};

// Helper functions for programs
bool SameType(const ImageBase& Img1, const ImageBase& Img2);         ///< Returns true if both images are of the same type
void CheckFloat(const ImageBase& Img);                               ///< Checks that the image contains float, throws a cl::Error if not
void CheckNotFloat(const ImageBase& Img);                            ///< Checks that the image does not contains float, throws a cl::Error if not
void CheckSameSize(const ImageBase& Img1, const ImageBase& Img2);    ///< Checks that both images have the same size, throws a cl::Error if not

/// Checks sizes + float/signed/unsigned, throws a cl::Error if not.
/// Checks that both images are of the same size and same type (float/signed/unsigned)
/// Useful for kernels that work with cl::Image2D
void CheckCompatibility(const ImageBase& Img1, const ImageBase& Img2);

/// Checks sizes + float/signed/unsigned + depth, throws a cl::Error if not.
/// Checks that both images are of the same size and same type (float/signed/unsigned & depth)
/// Useful for channel conversion kernels
void CheckSizeAndType(const ImageBase& Img1, const ImageBase& Img2);

/// Checks sizes + float/signed/unsigned + depth + nb channels, throws a cl::Error if not.
/// Checks that both images are of the same size and same type (float/signed/unsigned & depth) and have the same number of channels
/// Useful for cl::Buffer based kernels
void CheckSimilarity(const ImageBase& Img1, const ImageBase& Img2);

}
