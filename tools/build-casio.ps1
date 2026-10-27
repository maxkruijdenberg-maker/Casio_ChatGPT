param([Parameter(Mandatory=$true)][string]$SdkPath)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$sdk = (Resolve-Path $SdkPath).Path
$cc = Join-Path $sdk 'bin\sh3eb-elf-gcc.exe'
$mk = Join-Path $sdk 'bin\mkg3a.exe'
if (!(Test-Path $cc) -or !(Test-Path $mk)) { throw 'Gebruik de uitgepakte PrizmSDK-win-0.6-map met de submap bin.' }
New-Item -ItemType Directory -Force "$root\casio\build" | Out-Null
New-Item -ItemType Directory -Force "$root\dist" | Out-Null
$flags = @('-mb','-m4a-nofpu','-mhitachi','-fno-pic','-fno-pie','-ffreestanding','-Os','-Wall','-Wno-misleading-indentation','-Wno-unused-function',"-I$sdk\include")
& $cc @flags '-c' "$root\casio\main.c" '-o' "$root\casio\build\main.o"
if ($LASTEXITCODE -ne 0) { throw 'Casio-compilatie mislukt.' }
& $cc @flags '-c' "$root\casio\native_syscalls.S" '-o' "$root\casio\build\native-syscalls.o"
if ($LASTEXITCODE -ne 0) { throw 'Casio-syscallcompilatie mislukt.' }
& $cc @flags '-nostdlib' "-T$sdk\toolchain\prizm.x" "-L$sdk\lib" "$root\casio\build\main.o" "$root\casio\build\native-syscalls.o" '-lc' '-lfxcg' '-lgcc' '-o' "$root\casio\build\ChatGPT.bin"
if ($LASTEXITCODE -ne 0) { throw 'Casio-linken mislukt.' }
& $mk '-n' 'basic:ChatGPT' '-i' "uns:$root\casio\unselected.bmp" '-i' "sel:$root\casio\selected.bmp" "$root\casio\build\ChatGPT.bin" "$root\dist\ChatGPT.g3a"
if ($LASTEXITCODE -ne 0) { throw 'Maken van G3A mislukt.' }
Write-Host "Gebouwd: $root\dist\ChatGPT.g3a"
