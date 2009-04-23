/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2009 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
/*
 * PS 3.15 / E.1 / Basic Application Level Confidentiality Profile
 * Implementation of E.1.1 De-identify & E.1.2 Re-identify
 */

#include "gdcmReader.h"
#include "gdcmWriter.h"
#include "gdcmVersion.h"
#include "gdcmRSA.h"
#include "gdcmSystem.h"
#include "gdcmUIDGenerator.h"
#include "gdcmAnonymizer.h"
#include "gdcmGlobal.h"
#include "gdcmHAVEGE.h"
#include "gdcmX509.h"
#include "gdcmDefs.h"
#include "gdcmDirectory.h"

#include <getopt.h>

void PrintVersion()
{
  std::cout << "gdcmanon: gdcm " << gdcm::Version::GetVersion() << " ";
  const char date[] = "$Date$";
  std::cout << date << std::endl;
}

// FIXME
  int deidentify = 0;
  int reidentify = 0;


bool AnonymizeOneFile(gdcm::Anonymizer &anon, const char *filename, const char *outfilename)
{
  gdcm::Reader reader;
  reader.SetFileName( filename );
  if( !reader.Read() )
    {
      std::cerr << "Could not read : " << filename << std::endl;
    return false;
    }
  gdcm::File &file = reader.GetFile();
  gdcm::MediaStorage ms;
  ms.SetFromFile(file);
  if( !gdcm::Defs::GetIODNameFromMediaStorage(ms) )
    {
    std::cerr << "The Media Storage Type of your file is not supported: " << ms << std::endl;
    std::cerr << "Please report" << std::endl;
    return false;
    }

  anon.SetFile( file );

  if( deidentify )
    {
    //anon.RemovePrivateTags();
    //anon.RemoveRetired();
    if( !anon.BasicApplicationLevelConfidentialityProfile( true ) )
      {
      std::cerr << "Could not De-indentify : " << filename << std::endl;
      return false;
      }

    // FIXME:
    gdcm::FileMetaInformation &fmi = file.GetHeader();
    //fmi.Remove( gdcm::Tag(0x0002,0x0003) ); // will be regenerated
    fmi.Remove( gdcm::Tag(0x0002,0x0012) ); // will be regenerated
    fmi.Remove( gdcm::Tag(0x0002,0x0013) ); //  '   '    '
    fmi.Remove( gdcm::Tag(0x0002,0x0016) ); //  '   '    '

    }
  else if ( reidentify )
    {
    if( !anon.BasicApplicationLevelConfidentialityProfile( false ) )
      {
      std::cerr << "Could not Re-indentify : " << filename << std::endl;
      return false;
      }
    }

  gdcm::Writer writer;
  writer.SetFileName( outfilename );
  writer.SetFile( file );
  if( !writer.Write() )
    {
      std::cerr << "Could not Write : " << outfilename << std::endl;
    return false;
    }
  return true;
}

bool GetRSAKey(gdcm::RSA &rsa, const char *path = 0)
{
  std::string id_rsa_path;
  if( !path || !*path )
    {
    // By default on *nix system there should be a id_rsa file in $HOME/.ssh. Let's try parsing it:
    char *home = getenv("HOME");
    if(!home) return false;

    id_rsa_path = home;
    id_rsa_path += "/.ssh/id_rsa";
    }
  else
    {
    id_rsa_path = path;
    }

  if( !gdcm::System::FileExists( id_rsa_path.c_str() ) )
    {
    std::cerr << "Could not find file: " << id_rsa_path << std::endl;
    return false;
    }

  int err_x509 = rsa.X509ParseKeyfile( id_rsa_path.c_str() );
  if( err_x509 == gdcm::X509::ERR_X509_KEY_PASSWORD_REQUIRED  )
    {
    std::cout << "Enter passphrase:" << std::endl;
    std::string passphrase;
    std::cin >> passphrase;
    err_x509 = rsa.X509ParseKeyfile( id_rsa_path.c_str(), passphrase.c_str() );
    passphrase.clear(); // paranoid security
    }
  else if( err_x509 == gdcm::X509::ERR_X509_KEY_PASSWORD_MISMATCH )
    {
    std::cerr << "Passphrase mismatch" << std::endl;
    return false;
    }
  else if( err_x509 != 0 )
    {
    std::cerr << std::hex << err_x509 << std::endl;
    return false;
    }
  assert( err_x509 == 0 ); // success == 0

  if( rsa.CheckPubkey() != 0 || rsa.CheckPrivkey() != 0 )
    {
    std::cerr << "Invalid Pub/Priv key" << std::endl;
    return false;
    }

  return true;
}

void PrintHelp()
{
  PrintVersion();
  std::cout << "Usage: gdcmanon [OPTION]... FILE..." << std::endl;
  std::cout << "PS 3.15 / E.1 / Basic Application Level Confidentiality Profile" << std::endl;
  std::cout << "Implementation of E.1.1 De-identify & E.1.2 Re-identify" << std::endl;
  std::cout << "Parameter (required):" << std::endl;
  std::cout << "  -i --input               DICOM filename / directory" << std::endl;
  std::cout << "  -o --output              DICOM filename / directory" << std::endl;
  std::cout << "  -d --de-identify         De-identify DICOM (default)" << std::endl;
  std::cout << "  -R --re-identify         Re-identify DICOM" << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "     --root-uid            Root UID." << std::endl;
  std::cout << "     --resources-path      Resources path." << std::endl;
  std::cout << "General Options:" << std::endl;
  std::cout << "  -V --verbose   more verbose (warning+error)." << std::endl;
  std::cout << "  -W --warning   print warning info." << std::endl;
  std::cout << "  -D --debug     print debug info." << std::endl;
  std::cout << "  -E --error     print error info." << std::endl;
  std::cout << "  -h --help      print help." << std::endl;
  std::cout << "  -v --version   print version." << std::endl;
  std::cout << "Env var:" << std::endl;
  std::cout << "  GDCM_ROOT_UID Root UID" << std::endl;
  std::cout << "  GDCM_RESOURCES_PATH path pointing to resources files (Part3.xml, ...)" << std::endl;
}

int main(int argc, char *argv[])
{
  int c;
  //int digit_optind = 0;

  std::string filename;
  gdcm::Directory::FilenamesType filenames;
  std::string outfilename;
  gdcm::Directory::FilenamesType outfilenames;
  std::string root;
  std::string xmlpath;
  std::string rsa_path;
  int resourcespath = 0;
  int rsapath = 0;
  int rootuid = 0;
  int verbose = 0;
  int warning = 0;
  int debug = 0;
  int error = 0;
  int help = 0;
  int version = 0;
  while (1) {
    //int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
        {"input", 1, 0, 0},                 // i
        {"output", 1, 0, 0},                // o
        {"root-uid", 1, &rootuid, 1}, // specific Root (not GDCM)
        {"resources-path", 1, &resourcespath, 1},
        {"de-identify", 0, &deidentify, 1},
        {"re-identify", 0, &reidentify, 1},
        {"rsa-path", 1, &rsapath, 1},

        {"verbose", 0, &verbose, 1},
        {"warning", 0, &warning, 1},
        {"debug", 0, &debug, 1},
        {"error", 0, &error, 1},
        {"help", 0, &help, 1},
        {"version", 0, &version, 1},

        {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "i:o:dRVWDEhv",
      long_options, &option_index);
    if (c == -1)
      {
      break;
      }

    switch (c)
      {
    case 0:
        {
        const char *s = long_options[option_index].name;
        //printf ("option %s", s);
        if (optarg)
          {
          if( option_index == 0 ) /* input */
            {
            assert( strcmp(s, "input") == 0 );
            assert( filename.empty() );
            filename = optarg;
            }
          else if( option_index == 1 ) /* input */
            {
            assert( strcmp(s, "output") == 0 );
            assert( outfilename.empty() );
            outfilename = optarg;
            }
          else if( option_index == 2 ) /* root-uid */
            {
            assert( strcmp(s, "root-uid") == 0 );
            assert( root.empty() );
            root = optarg;
            }
          else if( option_index == 3 ) /* resources-path */
            {
            assert( strcmp(s, "resources-path") == 0 );
            assert( xmlpath.empty() );
            xmlpath = optarg;
            }
          else if( option_index == 6 ) /* rsa-path */
            {
            assert( strcmp(s, "rsa-path") == 0 );
            assert( rsa_path.empty() );
            rsa_path = optarg;
            }
          //printf (" with arg %s", optarg);
          }
        //printf ("\n");
        }
      break;

    case 'i':
      assert( filename.empty() );
      filename = optarg;
      break;

    case 'o':
      assert( outfilename.empty() );
      outfilename = optarg;
      break;

    case 'D':
      deidentify = 1;
      break;

    case 'R':
      reidentify = 1;
      break;

    case 'V':
      verbose = 1;
      break;

    case 'W':
      warning = 1;
      break;

    case 'd':
      debug = 1;
      break;

    case 'E':
      error = 1;
      break;

    case 'h':
      help = 1;
      break;

    case 'v':
      version = 1;
      break;

    case '?':
      break;

    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
      }
  }

  if (optind < argc)
    {
    std::vector<std::string> files;
    while (optind < argc)
      {
      //printf ("%s\n", argv[optind++]);
      files.push_back( argv[optind++] );
      }
    //printf ("\n");
    if( files.size() == 2 
      && filename.empty()
      && outfilename.empty() 
    )
      {
      filename = files[0];
      outfilename = files[1];
      }
    else
      {
      PrintHelp();
      return 1;
      }
    }

  if( version )
    {
    //std::cout << "version" << std::endl;
    PrintVersion();
    return 0;
    }

  if( help )
    {
    //std::cout << "help" << std::endl;
    PrintHelp();
    return 0;
    }

  if( filename.empty() )
    {
    //std::cerr << "Need input file (-i)\n";
    PrintHelp();
    return 1;
    }

  // by default de-identify
  if( !deidentify && !reidentify)
    {
    deidentify = 1;
    }

  if( deidentify && reidentify )
    {
    return 1;
    }

  // Are we in single file or directory mode:
  if( !gdcm::System::FileExists(filename.c_str()) )
    {
    // doh !
    return 1;
    }

  //
  unsigned int nfiles = 1;
  gdcm::Directory dir;
  bool recursive = false; //true;
  if( gdcm::System::FileIsDirectory(filename.c_str()) )
    {
    if( !gdcm::System::FileIsDirectory(outfilename.c_str()) )
      {
      return 1;
      }
    nfiles = dir.Load(filename, recursive);
    filenames = dir.GetFilenames();
    gdcm::Directory::FilenamesType::const_iterator it = filenames.begin();
    for( ; it != filenames.end(); ++it )
      {
      std::string dup = *it;
      std::string out = dup.replace(0, filename.size(), outfilename );
      outfilenames.push_back( out );
      }
    }
  else
    {
    filenames.push_back( filename );
    outfilenames.push_back( outfilename );
    }

  if( filenames.size() != outfilenames.size() )
    {
    std::cerr << "Something went really wrong" << std::endl;
    return 1;
    }

  // Debug is a little too verbose
  gdcm::Trace::SetDebug( debug );
  gdcm::Trace::SetWarning( warning );
  gdcm::Trace::SetError( error );
  // when verbose is true, make sure warning+error are turned on:
  if( verbose )
    {
    gdcm::Trace::SetWarning( verbose );
    gdcm::Trace::SetError( verbose);
    }

  gdcm::FileMetaInformation::SetSourceApplicationEntityTitle( "gdcmanon" );
  gdcm::Global& g = gdcm::Global::GetInstance();
  if( !resourcespath )
    {
    const char *xmlpathenv = getenv("GDCM_RESOURCES_PATH");
    if( xmlpathenv )
      {
      // Make sure to look for XML dict in user explicitly specified dir first:
      xmlpath = xmlpathenv;
      resourcespath = 1;
      }
    }
  if( resourcespath )
    {
    // xmlpath is set either by the cmd line option or the env var
    if( !g.Prepend( xmlpath.c_str() ) )
      {
      std::cerr << "specified Resources Path is not valid: " << xmlpath << std::endl;
      return 1;
      }
    }
  // All set, then load the XML files:
  if( !g.LoadResourcesFiles() )
    {
    return 1;
    }
  const gdcm::Defs &defs = g.GetDefs();
  if( !rootuid )
    {
    // only read the env var is no explicit cmd line option
    // maybe there is an env var defined... let's check
    const char *rootuid_env = getenv("GDCM_ROOT_UID");
    if( rootuid_env )
      {
      rootuid = 1;
      root = rootuid_env;
      }
    }
  if( rootuid )
    {
    // root is set either by the cmd line option or the env var
    if( !gdcm::UIDGenerator::IsValid( root.c_str() ) )
      {
      std::cerr << "specified Root UID is not valid: " << root << std::endl;
      return 1;
      }
    gdcm::UIDGenerator::SetRoot( root.c_str() );
    }

  // Setup gdcm::Anonymizer

  // Get RSA key
  const unsigned int KEY_LEN = 32;
  gdcm::RSA rsa;
  if( !GetRSAKey(rsa, rsa_path.c_str()) )
    {
    return 1;
    }
  if( rsa.GetLenkey() != KEY_LEN * 8 )
    {
    std::cerr << "Wrong key len: " << rsa.GetLenkey() << std::endl;
    return 1;
    }

  gdcm::AES aes;
  char key[ KEY_LEN ] = {};
  // randomize key:
  gdcm::HAVEGE havege;
  for(unsigned int j = 0; j < KEY_LEN / 8; ++j )
    key[j] = havege.Rand();

  if( deidentify )
    {
    if( !aes.SetkeyEnc( key, KEY_LEN ) ) return 1;
    }
  else if ( reidentify )
    {
    if( !aes.SetkeyDec( key, KEY_LEN ) ) return 1;
    }

  gdcm::Anonymizer anon;
  anon.SetAESKey( aes ); // pass by COPY !

  for(unsigned int i = 0; i < nfiles; ++i)
    {
    const char *in  = filenames[i].c_str();
    const char *out = outfilenames[i].c_str();
    if( !AnonymizeOneFile(anon, in, out) )
      {
      //std::cerr << "Could not anonymize: " << in << std::endl;
      return 1;
      }
    }

  // Save the AES key in an RSA enveloppe:
  char rsa_plaintext[KEY_LEN];
  char rsa_ciphertext[KEY_LEN*8] = {};
  memcpy( rsa_plaintext, key, KEY_LEN );

  int err = rsa.Pkcs1Encrypt( gdcm::RSA::PUBLIC, KEY_LEN, rsa_plaintext, rsa_ciphertext );
  if( err != 0 )
    {
    std::cerr << "Pkcs1Encrypt failed with: " << err << std::endl;
    return 1;
    }
  //std::cout << rsa_ciphertext << std::endl;
  //int olen = 0;
  //unsigned char buf[KEY_LEN*8] = {};
  //err = rsa.Pkcs1Decrypt( gdcm::RSA::PRIVATE, olen, rsa_ciphertext, buf, sizeof(buf) );

  return 0;
}

