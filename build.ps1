# Argument Default Values
$CurrentDir = Get-Location
$TESTS_ENABLED = $false
$BUILD_PROGRAMS = $false

# Arguments Checker
foreach ($arg in $args) {
    switch ($arg) {
        '-t' {
            $TESTS_ENABLED = "true"
        }
        '--tests' {
            $TESTS_ENABLED = "true"
        }
        '-b' {
            $BUILD_PROGRAMS = "true"
        }
        '--build_programs' {
            $BUILD_PROGRAMS = "true"
        }
        default {
            Write-Host "Invalid option: $arg"
            exit 1
        }
    }
}

# Builds Operating System

docker run `
    --mount type=bind,source=$CurrentDir/build,target=/usr/app/build `
    --mount type=bind,source=$CurrentDir/src,target=/usr/app/src `
    --mount type=bind,source=$CurrentDir/docker-include,target=/usr/app/scripts `
    --mount type=bind,source=$CurrentDir/elf2sun,target=/usr/app/elf2sun `
    -it -e TESTS_ENABLED=$TESTS_ENABLED -e BUILD_PROGRAMS=$BUILD_PROGRAMS paradise-os
