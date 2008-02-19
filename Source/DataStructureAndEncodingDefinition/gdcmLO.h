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
#ifndef __gdcmLO_h
#define __gdcmLO_h

#include "gdcmString.h"

namespace gdcm
{

class /*GDCM_EXPORT*/ LO : public String<'\\'> /* PLEASE do not export me */
{
public:
  // typedef are not inherited:
  typedef String<'\\'> Superclass;
  typedef Superclass::value_type             value_type;
  typedef Superclass::pointer                pointer;
  typedef Superclass::reference              reference;
  typedef Superclass::const_reference        const_reference;
  typedef Superclass::size_type              size_type;
  typedef Superclass::difference_type        difference_type;
  typedef Superclass::iterator               iterator;
  typedef Superclass::const_iterator         const_iterator;
  typedef Superclass::reverse_iterator       reverse_iterator;
  typedef Superclass::const_reverse_iterator const_reverse_iterator;

  // LO constructors.
  LO(): Superclass() {}
  LO(const value_type* s): Superclass(s) {}
  LO(const value_type* s, size_type n): Superclass(s, n) {}
  LO(const Superclass& s, size_type pos=0, size_type n=npos):
    Superclass(s, pos, n) {}

  bool IsValid() const {
    // Check Length:
    size_type length = size();
    if( length > 64 ) return false;
    return true;
  }
};

} // end namespace gdcm

#endif //__gdcmLO_h

