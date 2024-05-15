# The user can override the default arch and default search paths
param(
    [Parameter(Mandatory = $false)]
    [string]$arch = "x86",
    [Parameter(Mandatory = $false)]
    [string]$msvcInstallPathx86 = "C:\Program Files (x86)\Microsoft Visual Studio",
    [Parameter(Mandatory = $false)]
    [string]$msvcInstallPathx64 = "C:\Program Files\Microsoft Visual Studio"
)

# Map the host arch opt to directory name
$arch2hostDir = @{
    "x86"         = "Hostx86"
    "amd64"       = "Hostx64"
    "x86_amd64"   = "Hostx86"
    "x86_arm"     = "Hostx86"
    "x86_arm64"   = "Hostx86"
    "amd64_x86"   = "Hostx64"
    "amd64_arm"   = "Hostx64"
    "amd64_arm64" = "Hostx64"
}

# Map the target arch opt to directory name
$arch2targetDir = @{
    "x86"         = "x86"
    "amd64"       = "x64"
    "x86_amd64"   = "x64"
    "x86_arm"     = "arm"
    "x86_arm64"   = "arm64"
    "amd64_x86"   = "x86"
    "amd64_arm"   = "arm"
    "amd64_arm64" = "arm64"
}

# Initialize any submodules if necessary
if (-Not(Test-Path getopt-for-windows\.git)) {
    Write-Host "Setting up the project submodules"
    Write-Host "git submodule init"
    git submodule init
    Write-Host "git submodule update"
    git submodule update
} else {
    # Write-Host "Submodules already loaded"
}

if (-Not(Test-Path ".\build_cache_$($arch)")) {
    Write-Host "Did not find build_cache, generating build_cache_$($arch).."

    Write-Host "Looking for latest vcvarsall.bat.."
    # Find vcvarsall.bat file with highest year in its path
    $vcvarsallPath =
        Get-ChildItem -Path $msvcInstallPathx86, $msvcInstallPathx64 -Recurse -Directory |
        Where-Object { $_.GetFiles("vcvarsall.bat").Count -gt 0 } |
        ForEach-Object { $_.GetFiles("vcvarsall.bat").FullName } |
        ForEach-Object {
            $path = $_
            $match = $path.Split('\\') -match "\d\d\d\d"
            $yearIndex = $path.Split('\\').IndexOf($match[0])
            if ($yearIndex -ne -1) {
                [PSCustomObject]@{
                    Path = $path
                    YearIndex = $yearIndex
                }
            }
        } |
        Sort-Object -Descending { [int]$_.Path.Split('\\')[$_.YearIndex] } |
        Select-Object -First 1 |
        ForEach-Object { $_.Path }

    if (-Not(Test-Path $vcvarsallPath)) {
        Write-Host "Could not find vcvarsall.bat, exiting"
        Exit
    }
    Write-Host "$vcvarsallPath"

    Write-Host "Looking for latest cl.exe for $($arch).."
    # Get the search path for cl.exe based on the highest year directory found searching for vcvarsall.bat files
    $match = $vcvarsallPath.Split('\\') -match "\d\d\d\d"
    $yearIndex = $vcvarsallPath.Split('\\').IndexOf($match[0])
    $clSearchPath = $vcvarsallPath.Split('\\')[0..$yearIndex] -join '\'
    # Write-Host "clSearchPath = $clSearchPath"

    # Get the directory containing cl.exe matching $arch
    $clPathWildcard = "*$($arch2hostDir[$arch])*$($arch2targetDir[$arch])*"
    # Write-Host "clPathWildcard = $clPathWildcard"
    
    # TODO: simplify? why not make cl.exe part of the search wildcard?
    $clPath =
        Get-ChildItem -Path $clSearchPath -Recurse -Directory |
        Where-Object { $_.GetFiles("cl.exe").Count -gt 0 } |
        Where-Object { ($_.FullName -like "$clPathWildcard") } |
        ForEach-Object { $_.GetFiles("cl.exe").FullName } |
        ForEach-Object {
            $path = $_
            $match = $path.Split('\\') -match "\d+\.\d+\.\d+"
            $versionIndex = $path.Split('\\').IndexOf($match[0])
            if ($versionIndex -ne -1) {
                [PSCustomObject]@{
                    Path = $path
                    VersionIndex = $versionIndex
                }
            }
        } |
        Sort-Object -Descending { [int]($_.Path.Split('\\')[$_.VersionIndex] -replace '\.') } |
        Select-Object -First 1 |
        ForEach-Object { $_.Path }
    
    if (-Not(Test-Path $clPath)) {
        Write-Host "Could not find a compiler, exiting"
        Exit
    }
    Write-Host "$clPath"

    # Indicate the build setup to user
    # Write-Host "Using configuration: $vcvarsallPath $target"
    # Write-Host "Using compiler: $clPath"

    # Write the the build setup to build_cache_$($arch)
    Write-Host "Writing build_cache_$($arch).."
    # Write-Host "`$vcvarsallPath=$vcvarsallPath"
    # Write-Host "`$clPath=$clPath"

    New-Item -ItemType File -Path ".\build_cache_$($arch)" | Out-Null
    Add-Content -Path ".\build_cache_$($arch)" -Value "vcvarsallPath=$vcvarsallPath"
    Add-Content -Path ".\build_cache_$($arch)" -Value "clPath=$clPath"
} else {
    Write-Host "Loading from build_cache_$($arch).."

    $vars = Get-Content -Path ".\build_cache_$($arch)"
    # Write-Host $vars

    # This will set vcvarsallPath and clPath
    $vars | ForEach-Object {
        $key, $value = $_.Split('=')
        Set-Variable -Name $key -Value $value
    }

    $vcvarsallPathWildcard = "*vcvarsall.bat"
    $clPathWildcard = "*$($arch2hostDir[$arch])*$($arch2targetDir[$arch])*cl.exe"
    if ((Test-Path "$vcvarsallPath") -and ($vcvarsallPath -like $vcvarsallPathWildcard) -and (Test-Path "$clPath") -and ($clPath -like $clPathWildcard)) {
        Write-Host "$vcvarsallPath"
        Write-Host "$clPath"
    } else {
        Write-Host "build_cache_$($arch) is invalid, needs to be regenerated"
        Write-Host "Deleting the build_cache_$($arch) file and restarting this script"
        Remove-Item "build_cache_$($arch)" -Force
        if (-Not(Test-Path ".\build_cache_$($arch)")) {
            Invoke-Expression -Command "powershell '&' $PSScriptRoot\build.ps1 $arch"
        } else {
            Write-Host "Can't remove build_cache, remove it manually and restart script. Exiting."
            Exit
        }
    }
 }

# If the env isn't already configured,
# Run the correct vcvarsall.bat and save the resulting environment in a temp file
# TODO: this can probably be handled better by compiling in a subshell
# if the user changes arch, it's incorrect to run vcvarsall.bat again in the same shell, because that might overfill PATH
# see: https://developercommunity.visualstudio.com/t/vcvarsallbat-reports-the-input-line-is-too-long-if/257260
# Configuring a subshell just for the compilation allows the script to build the project for any arch without restarting the user's shell
if (-Not(Test-Path Env:DevEnvDir)) {
    Write-Host "Configuring environment for compilation.."
    cmd.exe "/C" "call ""${vcvarsallPath}"" $arch && set > %temp%\vcvars.txt"

    # Copy the set up environment from the temp file to this script's env
    Get-Content "$env:temp\vcvars.txt" |
        Foreach-Object {
        if ($_ -match "^(.*?)=(.*)$") {
            Set-Content "env:\$($matches[1])" $matches[2]  
        }
    }
} else {
    Write-Host "Environment already configured for compilation by vcvarsall.bat, and reconfiguration is not supported by Microsoft"
    Write-Host "(If the present environment is misconfigured, possibly because the host or target arch changed, reinvoke this script from a new shell)"
}

# Maybe add path to cl.exe to PATH
$clDir = Split-Path $clPath -Parent
if (-Not($env:PATH.Contains($clDir))) {
    $env:PATH = "$clDir;$env:PATH"
    Write-Host "Added dir for cl.exe to PATH: '$clDir'"
} else {
    Write-Host "Found dir for cl.exe in PATH: '$clDir'"
}

Write-Host "Building the project.."
Write-Host "cl /std:c++latest /arch:IA32 /EHs /Immu /Ipcb /Ivm /DDEBUG_OPTS /DVERBOSE_OPTS /Fevm.exe main.cpp vm\vm.cpp mmu\mmu.cpp pcb\pcb.cpp"
cl /std:c++latest /arch:IA32 /EHs /Immu /Ipcb /Ivm /DDEBUG_OPTS /DVERBOSE_OPTS /Fevm.exe main.cpp vm\vm.cpp mmu\mmu.cpp pcb\pcb.cpp