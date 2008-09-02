/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __gdcmByteSwap_h
#define __gdcmByteSwap_h

#include "gdcmTypes.h"
#include "gdcmSwapCode.h"

namespace gdcm
{

/**
 * \brief ByteSwap
 * Perform machine dependent byte swaping (Little Endian,
 * Big Endian, Bad Little Endian, Bad Big Endian).
 * TODO: bswap_32 / bswap_64 ...
 */
template<class T>
class ByteSwap
{
public:
  /** Query the machine Endian-ness. */
  static bool SystemIsBigEndian ();
  static bool SystemIsLittleEndian ();

  static void Swap(T &p);
  static void SwapFromSwapCodeIntoSystem(T &p, SwapCode const &sc);
  static void SwapRange(T *p, unsigned int num);
  static void SwapRangeFromSwapCodeIntoSystem(T *p, SwapCode const &sc,
    unsigned int num);

protected:
//  ByteSwap() {}
//  ~ByteSwap() {}

private:
  
};

/**
 * \example TestByteSwap.cxx
 * This is a C++ example on how to use gdcm::ByteSwap
 */

} // end namespace gdcm

#include "gdcmByteSwap.txx"

#endif //__gdcmByteSwap_h

