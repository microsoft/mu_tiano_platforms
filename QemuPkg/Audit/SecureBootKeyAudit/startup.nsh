@echo -off
for %b run (0 9)
  if exist FS%b:\SecureBootKeyAuditApp.efi then
    FS%b:
    cd \
    echo Starting the Secure Boot ECIT audit...
    SecureBootKeyAuditApp.efi
    echo Secure Boot ECIT audit returned.
    goto Done
  endif
endfor

echo ERROR: SecureBootKeyAuditApp.efi was not found on FS0 through FS9.

:Done
