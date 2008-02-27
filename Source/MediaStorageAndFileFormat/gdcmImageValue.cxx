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
#include "gdcmImageValue.h"
#include "gdcmExplicitDataElement.h"
#include "gdcmByteValue.h"
#include "gdcmDataSet.h"
#include "gdcmSequenceOfFragments.h"
#include "gdcmFragment.h"
#include "gdcmRAWCodec.h"
#include "gdcmJPEGCodec.h"
#include "gdcmJPEG2000Codec.h"
#include "gdcmRLECodec.h"

namespace gdcm
{

bool ImageValue::TryRAWCodec(char *buffer) const
{
  unsigned long len = GetBufferLength();
  const TransferSyntax &ts = GetTransferSyntax();

  const ByteValue *bv = PixelData.GetByteValue();
  if( bv )
    {
    if( len != bv->GetLength() )
      {
      // SIEMENS_GBS_III-16-ACR_NEMA_1.acr
      gdcmDebugMacro( "Pixel Length " << bv->GetLength() <<
        " is different from computed value " << len );
      }
    RAWCodec codec;
    codec.SetPlanarConfiguration( GetPlanarConfiguration() );
    codec.SetPhotometricInterpretation( GetPhotometricInterpretation() );
    codec.SetLUT( GetLUT() );
    codec.SetPixelFormat( GetPixelFormat() );
    codec.SetNeedByteSwap( GetNeedByteSwap() );
    DataElement out;
    bool r = codec.Decode(PixelData, out);

    const ByteValue *outbv = out.GetByteValue();
    assert( outbv );
    unsigned long check = outbv->GetLength();  // FIXME
    // FIXME
    if ( GetPhotometricInterpretation() == 
      PhotometricInterpretation::PALETTE_COLOR )
      {
      assert( check == 3*len );
      }
    else
      {
      assert( check == len );
      }
    memcpy(buffer, outbv->GetPointer(), outbv->GetLength() );  // FIXME
    return r;
    }
  return false;
}

bool ImageValue::TryJPEGCodec(char *buffer) const
{
  unsigned long len = GetBufferLength();
  const TransferSyntax &ts = GetTransferSyntax();


    JPEGCodec codec;
    if( codec.CanDecode( ts ) )
      {
      codec.SetPlanarConfiguration( GetPlanarConfiguration() );
      codec.SetPhotometricInterpretation( GetPhotometricInterpretation() );
      codec.SetPixelFormat( GetPixelFormat() );
      DataElement out;
      bool r = codec.Decode(PixelData, out);
      assert( r );
    const ByteValue *outbv = out.GetByteValue();
    assert( outbv );
    unsigned long check = outbv->GetLength();  // FIXME
    memcpy(buffer, outbv->GetPointer(), outbv->GetLength() );  // FIXME

      return true;
      }
  return false;
}
   
bool ImageValue::TryJPEG2000Codec(char *buffer) const
{
  unsigned long len = GetBufferLength();
  const TransferSyntax &ts = GetTransferSyntax();

      JPEG2000Codec codec;
    if( codec.CanDecode( ts ) )
      {
      codec.SetPlanarConfiguration( GetPlanarConfiguration() );
      codec.SetPhotometricInterpretation( GetPhotometricInterpretation() );
      DataElement out;
      bool r = codec.Decode(PixelData, out);
      assert( r );
    const ByteValue *outbv = out.GetByteValue();
    assert( outbv );
    unsigned long check = outbv->GetLength();  // FIXME
    memcpy(buffer, outbv->GetPointer(), outbv->GetLength() );  // FIXME
      return r;
      }
  return false;
}

bool ImageValue::TryRLECodec(char *buffer) const
{
  const TransferSyntax &ts = GetTransferSyntax();

      RLECodec codec;
    if( codec.CanDecode( ts ) )
      {
      //assert( sf->GetNumberOfFragments() == 1 );
      //assert( sf->GetNumberOfFragments() == GetDimensions(2) );
      codec.SetPlanarConfiguration( GetPlanarConfiguration() );
      codec.SetPhotometricInterpretation( GetPhotometricInterpretation() );
      codec.SetPixelFormat( GetPixelFormat() );
      codec.SetLUT( GetLUT() );
      DataElement out;
      bool r = codec.Decode(PixelData, out);

      return true;
 
      }
  return false;
}

bool ImageValue::GetBuffer(char *buffer) const
{
  bool success = false;
  if( !success ) success = TryRAWCodec(buffer);
  if( !success ) success = TryJPEGCodec(buffer);
  if( !success ) success = TryJPEG2000Codec(buffer);
  if( !success ) success = TryRLECodec(buffer);
  if( !success )
    {
    buffer = 0;
    abort();
    }

  return success;
}

} // end namespace gdcm

