/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmRescaler.h"
#include <limits>
#include <stdlib.h> // abort
#include <string.h> // memcpy

namespace gdcm
{

// parameter 'size' is in bytes
template <typename TOut, typename TIn>
void RescaleFunction(TOut *out, const TIn *in, double intercept, double slope, size_t size)
{
  size /= sizeof(TIn);
  for(size_t i = 0; i != size; ++i)
    {
    out[i] = (TOut)(slope * in[i] + intercept);
    //assert( out[i] == (TOut)(slope * in[i] + intercept) ); // will really slow down stuff...
    //assert( in[i] == (TIn)(((double)out[i] - intercept) / slope + 0.5) );
    }
}

// no such thing as partial specialization of function in c++
// so instead use this trick:
template<typename TOut, typename TIn> 
struct FImpl;

template<typename TOut, typename TIn> 
void InverseRescaleFunction(TOut *out, const TIn *in, double intercept, double slope, size_t size)
{ FImpl<TOut,TIn>::InverseRescaleFunction(out,in,intercept,slope,size); } // users, don't touch this!

template<typename TOut, typename TIn> 
struct FImpl 
{ 
  // parameter 'size' is in bytes
  static void InverseRescaleFunction( TOut *out, const TIn *in, 
    double intercept, double slope, size_t size) // users, go ahead and specialize this 
    {
    size /= sizeof(TIn);
    for(size_t i = 0; i != size; ++i)
      {
      // '+ 0.5' trick is NOT needed for image such as: gdcmData/D_CLUNIE_CT1_J2KI.dcm 
      out[i] = (TOut)(((double)in[i] - intercept) / slope );
      }
    }
};

template<typename TOut> 
struct FImpl<TOut, float>
{
  static void InverseRescaleFunction(TOut *out, const float *in,
    double intercept, double slope, size_t size)
    {
    size /= sizeof(float);
    for(size_t i = 0; i != size; ++i)
      {
      // '+ 0.5' trick is needed for instance for : gdcmData/MR-MONO2-12-shoulder.dcm
      // well known trick of adding 0.5 after a floating point type operation to properly find the
      // closest integer that will represent the transformation
      out[i] = (TOut)(((double)in[i] - intercept) / slope + 0.5);
      }
    }
};

PixelFormat::ScalarType ComputeBestFit(const PixelFormat &pf, double intercept, double slope)
{
  PixelFormat::ScalarType st = PixelFormat::UNKNOWN;
  assert( slope == (int)slope && intercept == (int)intercept);
  
  double min = slope * pf.GetMin() + intercept;
  double max = slope * pf.GetMax() + intercept;
  assert( min <= max );
  if( min >= 0 ) // unsigned
  {
	  if( max <= std::numeric_limits<uint8_t>::max() )
	  {
		  st = PixelFormat::UINT8;
	  }
	  else if( max <= std::numeric_limits<uint16_t>::max() )
	  {
		  st = PixelFormat::UINT16;
	  }
	  else if( max <= std::numeric_limits<uint32_t>::max() )
	  {
		  st = PixelFormat::UINT32;
	  }
	  else
	  {
		  abort();
	  }
  }
  else
  {
	  if( max <= std::numeric_limits<int8_t>::max() )
	  {
		  st = PixelFormat::INT8;
	  }
	  else if( max <= std::numeric_limits<int16_t>::max() )
	  {
		  st = PixelFormat::INT16;
	  }
	  else if( max <= std::numeric_limits<int32_t>::max() )
	  {
		  st = PixelFormat::INT32;
	  }
	  else
	  {
		  abort();
	  }
   }
	assert( st != PixelFormat::UNKNOWN );
  return st;
}

PixelFormat::ScalarType Rescaler::ComputeInterceptSlopePixelType()
{
  assert( PF != PixelFormat::UNKNOWN );
  PixelFormat::ScalarType output = PixelFormat::UNKNOWN;
  if( Slope != (int)Slope || Intercept != (int)Intercept)
  {
    //assert( PF != PixelFormat::INT8 && PF != PixelFormat::UINT8 ); // Is there any Object that have Rescale on char ?
	  return PixelFormat::FLOAT32;
  }
  double intercept = Intercept;
  double slope = Slope;
  output = ComputeBestFit (PF,intercept,slope);
  assert( output != PixelFormat::UNKNOWN );
  return output;
}


template <typename TIn>
void Rescaler::RescaleFunctionIntoBestFit(char *out, const TIn *in, size_t n)
{
	double intercept = Intercept;
	double slope = Slope;
  PixelFormat::ScalarType output = ComputeInterceptSlopePixelType();
  switch(output)
    {
  case PixelFormat::UINT8:
    RescaleFunction<uint8_t,TIn>((uint8_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::INT8:
    RescaleFunction<int8_t,TIn>((int8_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::UINT16:
    RescaleFunction<uint16_t,TIn>((uint16_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::INT16:
    RescaleFunction<int16_t,TIn>((int16_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::UINT32:
    RescaleFunction<uint32_t,TIn>((uint32_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::INT32:
    RescaleFunction<int32_t,TIn>((int32_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::FLOAT32:
    RescaleFunction<float,TIn>((float*)out,in,intercept,slope,n);
    break;
  default:
    abort();
    break;
    }
 }

template <typename TIn>
void Rescaler::InverseRescaleFunctionIntoBestFit(char *out, const TIn *in, size_t n)
{
	double intercept = Intercept;
	double slope = Slope;
  //PixelFormat::ScalarType output = ComputeInterceptSlopePixelType();
  PixelFormat output = ComputePixelTypeFromMinMax();
  switch(output)
    {
  case PixelFormat::UINT8:
    InverseRescaleFunction<uint8_t,TIn>((uint8_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::INT8:
    InverseRescaleFunction<int8_t,TIn>((int8_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::UINT16:
    InverseRescaleFunction<uint16_t,TIn>((uint16_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::INT16:
    InverseRescaleFunction<int16_t,TIn>((int16_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::UINT32:
    InverseRescaleFunction<uint32_t,TIn>((uint32_t*)out,in,intercept,slope,n);
    break;
  case PixelFormat::INT32:
    InverseRescaleFunction<int32_t,TIn>((int32_t*)out,in,intercept,slope,n);
    break;
  //case PixelFormat::FLOAT32:
  //  InverseRescaleFunction<float,TIn>((float*)out,in,intercept,slope,n);
  //  break;
  default:
    abort();
    break;
    }
 }


bool Rescaler::InverseRescale(char *out, const char *in, size_t n)
{
  // fast path:
  if( Slope == 1 && Intercept == 0 ) 
    {
    memcpy(out,in,n);
    return true;
    }
  // check if we are dealing with floating point type
  if( Slope != (int)Slope || Intercept != (int)Intercept)
  {
  // need to rescale as float (32bits) as slope/intercept are 32bits
  //abort();
  }
  // else integral type
  switch(PF)
    {
  //case PixelFormat::UINT8:
  //  RescaleFunctionIntoBestFit<uint8_t>(out,(uint8_t*)in,n);
  //  break;
  //case PixelFormat::INT8:
  //  RescaleFunctionIntoBestFit<int8_t>(out,(int8_t*)in,n);
  //  break;
  case PixelFormat::UINT16:
    InverseRescaleFunctionIntoBestFit<uint16_t>(out,(uint16_t*)in,n);
    break;
  case PixelFormat::INT16:
    InverseRescaleFunctionIntoBestFit<int16_t>(out,(int16_t*)in,n);
    break;
  //case PixelFormat::UINT32:
  //  RescaleFunctionIntoBestFit<uint32_t>(out,(uint32_t*)in,n);
  //  break;
  //case PixelFormat::INT32:
  //  RescaleFunctionIntoBestFit<int32_t>(out,(int32_t*)in,n);
  //  break;
  case PixelFormat::FLOAT32:
    assert( sizeof(float) == 32 / 8 );
    InverseRescaleFunctionIntoBestFit<float>(out,(float*)in,n);
    break;
  default:
    //InverseRescaleFunction<unsigned short, float>((unsigned short*)out,(float*)in,Intercept,Slope,n);
    abort();
    break;
    }
 
  return true;}

bool Rescaler::Rescale(char *out, const char *in, size_t n)
{
  // fast path:
  if( Slope == 1 && Intercept == 0 ) 
    {
    memcpy(out,in,n);
    return true;
    }
  // check if we are dealing with floating point type
  if( Slope != (int)Slope || Intercept != (int)Intercept)
  {
  // need to rescale as float (32bits) as slope/intercept are 32bits
  //abort();
  }
  // else integral type
  switch(PF)
    {
  case PixelFormat::UINT8:
    RescaleFunctionIntoBestFit<uint8_t>(out,(uint8_t*)in,n);
    break;
  case PixelFormat::INT8:
    RescaleFunctionIntoBestFit<int8_t>(out,(int8_t*)in,n);
    break;
  case PixelFormat::UINT12:
    //RescaleFunctionIntoBestFit<uint12_t>(out,in,n);
    abort();
    break;
  case PixelFormat::INT12:
    //RescaleFunctionIntoBestFit<int12_t>(out,in,n);
    abort();
    break;
  case PixelFormat::UINT16:
    RescaleFunctionIntoBestFit<uint16_t>(out,(uint16_t*)in,n);
    break;
  case PixelFormat::INT16:
    RescaleFunctionIntoBestFit<int16_t>(out,(int16_t*)in,n);
    break;
  case PixelFormat::UINT32:
    RescaleFunctionIntoBestFit<uint32_t>(out,(uint32_t*)in,n);
    break;
  case PixelFormat::INT32:
    RescaleFunctionIntoBestFit<int32_t>(out,(int32_t*)in,n);
    break;
  default:
    abort();
    break;
    }
 
  return true;
}

PixelFormat ComputeInverseBestFitFromMinMax(/*const PixelFormat &pf,*/ double intercept, double slope, double _min, double _max)
{
  PixelFormat st = PixelFormat::UNKNOWN;
  //assert( slope == (int)slope && intercept == (int)intercept);
  
  double min = (_min - intercept ) / slope;
  double max = (_max - intercept ) / slope;
  assert( min <= max );
  if( min >= 0 ) // unsigned
  {
	  if( max <= std::numeric_limits<uint8_t>::max() )
	  {
		  st = PixelFormat::UINT8;
	  }
	  else if( max <= std::numeric_limits<uint16_t>::max() )
	  {
		  st = PixelFormat::UINT16;
      assert( st.GetBitsAllocated() == 16 );
      // FIXME
      if( max <= 4096 )
        {
        st.SetBitsStored( 12 );
        st.SetHighBit( 11 );
        }
	  }
	  else if( max <= std::numeric_limits<uint32_t>::max() )
	  {
		  st = PixelFormat::UINT32;
	  }
	  else
	  {
		  abort();
	  }
  }
  else
  {
	  if( max <= std::numeric_limits<int8_t>::max() )
	  {
		  st = PixelFormat::INT8;
	  }
	  else if( max <= std::numeric_limits<int16_t>::max() )
	  {
		  st = PixelFormat::INT16;
	  }
	  else if( max <= std::numeric_limits<int32_t>::max() )
	  {
		  st = PixelFormat::INT32;
	  }
	  else
	  {
		  abort();
	  }
   }
	assert( st != PixelFormat::UNKNOWN );
	assert( st != PixelFormat::FLOAT32 && st != PixelFormat::FLOAT16 );
  return st;
}

PixelFormat Rescaler::ComputePixelTypeFromMinMax()
{
  assert( PF != PixelFormat::UNKNOWN );
  PixelFormat output = PixelFormat::UNKNOWN;
  double intercept = Intercept;
  double slope = Slope;
#if 0
  if( Slope != (int)Slope || Intercept != (int)Intercept)
    {
    //assert( PF != PixelFormat::INT8 && PF != PixelFormat::UINT8 ); // Is there any Object that have Rescale on char ?
    assert( PF == PixelFormat::FLOAT32 || PF == PixelFormat::FLOAT16 );
    PixelFormat::ScalarType dummy = PF.GetScalarType();
    switch(PF)
      {
    case PixelFormat::FLOAT16:
      output = ComputeInverseBestFitFromMinMax (/*PF,*/intercept,slope,ScalarRangeMin,ScalarRangeMax);
      break;
    case PixelFormat::FLOAT32:
      output = ComputeInverseBestFitFromMinMax (/*PF,*/intercept,slope,ScalarRangeMin,ScalarRangeMax);
      //abort();
      break;
    default:
      abort();
      }
    }
#endif
  output = ComputeInverseBestFitFromMinMax (/*PF,*/intercept,slope,ScalarRangeMin,ScalarRangeMax);
  assert( output != PixelFormat::UNKNOWN && output >= PixelFormat::UINT8 && output <= PixelFormat::INT32 );
  return output;
}

} // end namespace gdcm

