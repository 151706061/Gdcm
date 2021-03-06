
FIND_PACKAGE(DCMTK)
FIND_PACKAGE(DICOM3TOOLS)

SUBDIRS(
  Source
  )

#-----------------------------------------------------------------------------
# Here is one cool test: you can pretty much test all configuration using
# ctest...well except one, the case where a user set BUILD_TESTING=OFF
# since this would deactivate the dashboard and would not submit...doh!
# So instead let's create a test that would build gdcm with this option
IF(UNIX AND GDCM_TEST_BOOTSTRAP)
  ADD_TEST(BuildGDCM ${CMAKE_CTEST_COMMAND}
    --build-and-test ${GDCM_SOURCE_DIR} ${GDCM_BINARY_DIR}/GDCMLocal
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-project GDCMLOCAL
    --build-options -DGDCM_BUILD_TESTING:BOOL=OFF -DGDCM_BUILD_SHARED_LIBS:BOOL=ON -DGDCM_WRAP_PYTHON:BOOL=ON -DGDCM_WRAP_CSHARP:BOOL=ON -DGDCM_SUPPORT_BROKEN_IMPLEMENTATION:BOOL=OFF
    )
  ADD_TEST(InstallGDCM ${CMAKE_CTEST_COMMAND}
    --build-and-test ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}
    --build-generator ${CMAKE_GENERATOR}
    --build-project GDCM
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-noclean
    --build-target install
  )
  ADD_TEST(BuildStaticGDCM ${CMAKE_CTEST_COMMAND}
    --build-and-test ${GDCM_SOURCE_DIR} ${GDCM_BINARY_DIR}/GDCMLocal2
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${CMAKE_MAKE_PROGRAM}
    --build-project GDCMLOCAL2
    --build-options -DGDCM_BUILD_TESTING:BOOL=ON -DGDCM_BUILD_SHARED_LIBS:BOOL=OFF -DGDCM_SUPPORT_BROKEN_IMPLEMENTATION:BOOL=OFF -DGDCM_LEGACY_REMOVE:BOOL=ON
    --test-command ${CMAKE_CTEST_COMMAND}
    )
ENDIF(UNIX AND GDCM_TEST_BOOTSTRAP)



