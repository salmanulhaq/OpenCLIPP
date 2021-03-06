////////////////////////////////////////////////////////////////////////////////
//! @file	: benchStatistics.hpp
//! @date   : Jul 2013
//!
//! @brief  : Creates classes for all supported statistical reductions
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

#include "benchReduceBase.hpp"

#define REDUCE_DST_TYPE DataType
#define IPP_REDUCE_HINT
#define REDUCE_CMP_TOLERANCE SUCCESS_EPSILON
#define CUDA_REDUCE_SAME_TYPE

#define BENCH_NAME Min
#include "benchReduce.hpp"
#undef BENCH_NAME

#define BENCH_NAME Max
#include "benchReduce.hpp"
#undef BENCH_NAME

/*#define BENCH_NAME MinAbs
#include "benchReduce.hpp"
#undef BENCH_NAME

#define BENCH_NAME MaxAbs
#include "benchReduce.hpp"
#undef BENCH_NAME*/

#undef IPP_REDUCE_HINT
#undef REDUCE_DST_TYPE
#undef CUDA_REDUCE_SAME_TYPE


#define REDUCE_DST_TYPE double
#define IPP_REDUCE_HINT , ippAlgHintNone

#undef REDUCE_CMP_TOLERANCE
// Sum and Mean do calculations with very high values
// But in the GPU the calculations are done in floats
// So the results will have less precision
// So we allow higher tolerance here
#define REDUCE_CMP_TOLERANCE 0.001f

#define BENCH_NAME Sum
#include "benchReduce.hpp"
#undef BENCH_NAME

#define BENCH_NAME Mean
#include "benchReduce.hpp"
#undef BENCH_NAME

/*#define BENCH_NAME MeanSqr
#include "benchReduce.hpp"
#undef BENCH_NAME*/

#undef IPP_REDUCE_HINT
#undef REDUCE_DST_TYPE
#undef REDUCE_CMP_TOLERANCE
