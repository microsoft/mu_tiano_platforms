#
#  Script for running QEMU with the appriopriate options for the given SKU/ARCH.
#  This script is inteded to be run as part of the release package and will assume
#  file layout and firmware dependent options that may not be true if used standalone.
#
#  Copyright (c) Microsoft Corporation
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#

param (
  [switch]$UpdateFW = $False,
  [switch]$UpdateQemu = $False,
  [switch]$Headless = $False,
  [switch]$Verbose = $False,
  [string]$FWPath = ".",
  [string]$Arch = "x64",
  [string]$Disk = "",
  [string]$DbgPort = "",
  [string]$SerialPort = "",
  [string]$Cores = "2",
  [string]$FWVersion = "1.1.4",
  [string]$BuildType = "RELEASE",
  [int]$Memory = 4096
)

$ArgumentList = @()

if ($UpdateFW) {
  $fwInfos = @(
    @( "QemuQ35", "x64/" ),
    @( "QemuSbsa", "aarch64/" )
  )

  foreach ($fwInfo in $fwInfos) {
    $fwUrl = "https://github.com/microsoft/mu_tiano_platforms/releases/download/v" + $FWVersion + "/Mu." + $fwInfo[0] + ".FW." + $BuildType + "-" + $FWVersion + ".zip"
    $fwZip = $fwInfo[0] + ".zip"
    $fwDest = $FWPath + "/" + $fwInfo[1]

    Write-Host "Downloading " $fwInfo[0] " to $fwDest."
    Invoke-WebRequest -Uri $fwUrl -OutFile $fwZip
    if (test-path  $fwDest) {
      Remove-Item  $fwDest -Recurse -Force -Confirm:$false
    }
    Expand-Archive -LiteralPath $fwZip -DestinationPath $fwDest
    Remove-Item $fwZip -Confirm:$false
  }

  return
}

#
# Process Arch specific parameters.
#

$fwPathCode = $FWPath
$fwPathData = $FWPath

$QemuCommand = ""
if ($Arch -eq "x64") {
  $QemuCommand = "qemu-system-x86_64"

  $fwPathCode += "/x64/VisualStudio-x64/QEMUQ35_CODE.fd"
  $fwPathData += "/x64/VisualStudio-x64/QEMUQ35_VARS.fd"

  $ArgumentList += "-machine q35,smm=on"
  $ArgumentList += "-cpu qemu64,+rdrand,umip,+smep"
  $ArgumentList += "-global ICH9-LPC.disable_s3=1"

  $ArgumentList += "-debugcon stdio"
  $ArgumentList += "-global isa-debugcon.iobase=0x402"
}
elseif ($Arch -eq "arm64" -or $Arch -eq "aarch64") {
  $QemuCommand = "qemu-system-aarch64"

  # These values cannot be changed for SBSA due to limitation.
  Write-Host "Setting to one core for AARCH64."
  $Cores = 1

  $fwPathCode += "/aarch64/GCC-AARCH64/SECURE_FLASH0.fd"
  $fwPathData += "/aarch64/GCC-AARCH64/QEMU_EFI.fd"

  $ArgumentList += "-machine sbsa-ref"
  $ArgumentList += "-cpu max"

  $ArgumentList += "-serial stdio"
}
else {
  Throw "$Arch is not a supported architecture!"
}

#
# Check that files exist before launching QEMU.
#

if ((-not(Test-Path -path $fwPathCode))) {
  throw "Cannot find firmware code file! " + $fwPathCode
}

if ((-not(Test-Path -path $fwPathData))) {
  throw "Cannot find firmware data file! " + $fwPathData
}

#
# Build argument list.
#

# Memory
$ArgumentList += "-m " + $Memory

# Processors
$ArgumentList += "-smp $Cores"

# Flash storage
$ArgumentList += "-global driver=cfi.pflash01,property=secure,value=on"
$ArgumentList += "-drive if=pflash,format=raw,unit=0,file=$fwPathCode,readonly=on"
$ArgumentList += "-drive if=pflash,format=raw,unit=1,file=$fwPathData"

# SMBIOS
$ArgumentList += "-smbios type=0,vendor=Palindrome,uefi=on"
$ArgumentList += "-smbios type=1,manufacturer=Palindrome,product=MuQemu,serial=42-42-42-42"

# Disk
if ($Disk -ne "") {
  $ArgumentList += "-hda $Disk"
}

# Standard devices
# $ArgumentList += "-device qemu-xhci,id=usb"
# $ArgumentList += "-device usb-mouse,id=input0,bus=usb.0,port=1"
# $ArgumentList += "-device usb-kbd,id=input1,bus=usb.0,port=2"
$ArgumentList += "-nic model=e1000"

# Display
if ($Headless) {
  $ArgumentList += "-display none"
}
else {
  $ArgumentList += "-vga cirrus"
}

# Debug & Serial ports
if ($DbgPort -ne "") {
  $ArgumentList += "-gdb tcp::$DbgPort"
}

if ($SerialPort -ne "") {
  $ArgumentList += "-serial tcp:127.0.0.1:$SerialPort,server,nowait"
  Write-Host "Opening serial port. Connect using the following"
  Write-Host "    windbgx.exe -k com:ipport=$SerialPort,port=127.0.0.1 -v"
}

#
# Start the QEMU process
#

Write-Output $ArgumentList
Start-Process -FilePath $QemuCommand -ArgumentList $ArgumentList -NoNewWindow -Wait
