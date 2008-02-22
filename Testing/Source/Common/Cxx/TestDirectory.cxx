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
#include "gdcmDirectory.h"
#include "gdcmTesting.h"

int TestOneDirectory(const char *path, bool recursive = false )
{
  gdcm::Directory d;
  d.Load( path, recursive );
  d.Print( std::cout );
  return 0;
}

int TestDirectory(int argc, char *argv[])
{
  int res = 0;
  if( argc > 1 )
    {
    bool recursive = false;
    if ( argc > 2 )
      {
      recursive = atoi(argv[2]);
      }
    res += TestOneDirectory( argv[1], recursive);
    }
  else
    {
    const char *path = gdcm::Testing::GetDataRoot();
    res += TestOneDirectory( path );
    }

  res += TestOneDirectory( "" );

  return res;
}

