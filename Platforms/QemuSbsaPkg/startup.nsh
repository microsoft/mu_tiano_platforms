
#!/bin/nsh
echo -off
for %a run (0 10)
    if exist fs%a:\startup.nsh then
        fs%a:
        goto FOUND_IT
    endif
endfor

:FOUND_IT
# 1. We conditionally run a test based off of the presence of a <TestName>_JUNIT.XML. When this file is
#    present, it means that the test has completely finished and the results have been recorded. Once the
#    tests have finished, we rename them to _JUNIT_RESULT.XML so that the presence of these files will not
#    stop tests from running again.
# 2. After all tests have finished, we delete two types of files. The first is a previous run's
#    _JUNIT_RESULT.XML. This is necessary as the mv cannot overwrite a file. The second is that we delete
#    a test's _Cache.dat file. This is important because if we do not delete this file, the efi will execute,
#    but the test will not run. This is because a test uses the _Cache.dat file to see the current status
#    of the test (important for tests that require restarts)
# 3. We finally rename the current test results to _JUNIT_RESULT.XML to reset test progress and also allow
#    for test results to be read after Qemu has shut down.
if not exist MpManagementTestApp_JUNIT.XML then
    MpManagementTestApp.efi
endif
rm *_JUNIT_RESULT.XML
rm *_Cache.dat
if exist MpManagementTestApp_JUNIT.XML then
    mv MpManagementTestApp_JUNIT.XML MpManagementTestApp_JUNIT_RESULT.XML
endif
