# Argument Default Values
$CurrentDir = Get-Location
$TESTS_ENABLED = $false

# Arguments Checker
foreach ($arg in $args) {
    switch ($arg) {
        '-t' {
            $TESTS_ENABLED = "true"
        }
        '--tests' {
            $TESTS_ENABLED = "true"
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
   -it -e TESTS_ENABLED=$TESTS_ENABLED paradise-os
