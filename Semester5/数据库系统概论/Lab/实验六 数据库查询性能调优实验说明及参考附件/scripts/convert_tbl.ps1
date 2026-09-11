# Check and remove the existing tbl folder
if (Test-Path -Path "tbl") {
    Remove-Item -Path "tbl" -Recurse -Force
}

# Create a new tbl folder
New-Item -Path "tbl" -ItemType Directory

# Process all .tbl files
Get-ChildItem *.tbl | ForEach-Object {
    $name = "tbl\$($_.Name)"
    Write-Host "Processing file: $name"

    # Clear the target file
    Set-Content -Path $name -Value ""

    # Remove the trailing '|' character from each line
    Get-Content $_.FullName | ForEach-Object {
        $_ -replace '\|$', ''
    } | Set-Content -Path $name
}