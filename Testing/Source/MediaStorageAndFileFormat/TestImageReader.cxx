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
#include "gdcmImageReader.h"
#include "gdcmFileMetaInformation.h"
#include "gdcmSystem.h"
#include "gdcmFilename.h"
#include "gdcmByteSwap.h"
#include "gdcmTrace.h"

#include "gdcmDataImages.h"
#include "gdcmMD5DataImages.h"

const char *GetMD5Ref(const char *filepath)
{
  int i = 0;
  const char *p = gdcmMD5DataImages[i][1];
  gdcm::Filename comp(filepath);
  const char *filename = comp.GetName();
  while( p != 0 )
    {
    if( strcmp( filename, p ) == 0 )
      {
      break;
      }
    ++i;
    p = gdcmMD5DataImages[i][1];
    }
  return gdcmMD5DataImages[i][0];
}

int TestImageRead(const char* filename)
{
  gdcm::ImageReader reader;

  reader.SetFileName( filename );
  if ( reader.Read() )
    {
    int res = 0;
    const gdcm::Image &img = reader.GetImage();
    //std::cerr << "Success to read image from file: " << filename << std::endl;
    unsigned long len = img.GetBufferLength();
    if ( img.GetPhotometricInterpretation() ==
      gdcm::PhotometricInterpretation::PALETTE_COLOR )
      {
      len *= 3;
      }
    char* buffer = new char[len];
    img.GetBuffer(buffer);
    // On big Endian system we have byteswapped the buffer (duh!)
    // Since the md5sum is byte based there is now way it would detect
    // that the file is written in big endian word, so comparing against
    // a md5sum computed on LittleEndian would fail. Thus we need to
    // byteswap (again!) here:
#ifdef GDCM_WORDS_BIGENDIAN
    if( img.GetPixelType().GetBitsAllocated() == 16 )
      {
      assert( !(len % 2) );
      assert( img.GetPhotometricInterpretation() == gdcm::PhotometricInterpretation::MONOCHROME1
        || img.GetPhotometricInterpretation() == gdcm::PhotometricInterpretation::MONOCHROME2 );
      gdcm::ByteSwap<unsigned short>::SwapRangeFromSwapCodeIntoSystem(
        (unsigned short*)buffer, gdcm::SwapCode::LittleEndian, len/2);
      }
#endif
    const char *ref = GetMD5Ref(filename);
    char digest[33];
    gdcm::System::ComputeMD5(buffer, len, digest);
    if( !ref )
      {
      // new regression image needs a md5 sum
      std::cerr << "Missing md5 " << digest << " for: " << filename <<  std::endl;
      abort();
      }
    if( strcmp(digest, ref) )
      {
      std::cerr << "Problem reading image from: " << filename << std::endl;
      std::cerr << "Found " << digest << " instead of " << ref << std::endl;
      res = 1;
#if 0
      std::ofstream debug("/tmp/dump.gray");
      debug.write(buffer, len);
      debug.close();
      //abort();
#endif
      }
    delete[] buffer;
    return res;
    }

  const gdcm::FileMetaInformation &header = reader.GetFile().GetHeader();
  gdcm::MediaStorage::MSType ms = header.GetMediaStorageType();
  bool isImage = gdcm::MediaStorage::IsImage( ms );
  if( isImage )
    {
    std::cerr << "Failed to read image from file: " << filename << std::endl;
    return 1;
    }
  // else
  // well this is not an image, so thankfully we fail to read it
  assert( ms != gdcm::MediaStorage::MS_END );
  return 0;
}

int TestImageReader(int argc, char *argv[])
{
  if( argc == 2 )
    {
    const char *filename = argv[1];
    return TestImageRead(filename);
    }

  // else
  // First of get rid of warning/debug message
  gdcm::Trace::DebugOff();
  gdcm::Trace::WarningOff();
  int r = 0, i = 0;
  const char *filename;
  while( (filename = gdcmDataImages[i]) )
    {
    r += TestImageRead( filename );
    ++i;
    }

  return r;
}
