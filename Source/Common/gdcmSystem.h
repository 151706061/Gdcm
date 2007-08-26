/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006 Mathieu Malaterre
  Copyright (c) 1993-2005 CREATIS
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __gdcmSystem_h
#define __gdcmSystem_h

#include "gdcmTypes.h"

namespace gdcm
{

/**
 * \brief Class to do system operation
 * \note bla
 */
class System
{
public:
  // system operation 
  static bool MakeDirectory(const char *path); 
  static bool FileExists(const char* filename);
  static bool FileIsDirectory(const char* name);
  static bool RemoveFile(const char* source);

  static const char *GetLastSystemError();
  static unsigned long FileSize(const char* filename);

  // MD5 stuff
  // digest_str needs to be at least : strlen = [2*16+1];
  static bool ComputeMD5(const char *buffer, const unsigned long buf_len,
    char *digest_str);
  static bool ComputeFileMD5(const char *filename, char *digest_str);

  // TODO some system calls
  // Chdir
  // copy a file

protected:
  static bool GetPermissions(const char* file, unsigned short& mode);
  static bool SetPermissions(const char* file, unsigned short mode);
};

} // end namespace gdcm

#endif //__gdcmSystem_h
