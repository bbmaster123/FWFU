function Get-EmailDate {
    param (
        [string]$filePath
    )

    try {
        # Read the content of the .eml file
        $content = Get-Content -Path $filePath -Raw

        # Use regex to find the "Date:" header and extract the date string
        if ($content -match "Date:\s*(.*?)(\r?\n|\r)") {
            $dateString = $matches[1].Trim()

            # Define an array of potential date formats
            $dateFormats = @(
                "ddd, d MMM yyyy HH:mm:ss zzz",
                "ddd, dd MMM yyyy HH:mm:ss zzz",
                "ddd, d MMM yyyy HH:mm:ss -zzzz",
                "ddd, dd MMM yyyy HH:mm:ss -zzzz",
                "ddd, d MMM yyyy HH:mm:ss zzzz",
                "ddd, dd MMM yyyy HH:mm:ss zzzz",
                "ddd, d MMM yyyy HH:mm:ss",
                "ddd, dd MMM yyyy HH:mm:ss"
            )

            foreach ($format in $dateFormats) {
                try {
                    # Attempt to parse the date string
                    $date = [datetime]::ParseExact($dateString, $format, $null)
                    return $date
                } catch {
                    # Continue trying other formats
                    continue
                }
            }

            throw "Failed to parse date with any known formats: $dateString"
        } else {
            throw "Date header not found"
        }
    } catch {
        Write-Error "Failed to parse date: $($_.Exception.Message)"
    }
}

# Get the list of .eml files
$emlFiles = Get-ChildItem -Path "C:\mail" -Filter *.eml

foreach ($file in $emlFiles) {
    try {
        # Get the original date from the .eml file
        $originalDate = Get-EmailDate -filePath $file.FullName

        if ($originalDate) {
            # Set the file's LastWriteTime and CreationTime to the original date
            $(Get-Item -Path $file.FullName).LastWriteTime = $originalDate
            $(Get-Item -Path $file.FullName).CreationTime = $originalDate

            Write-Output "Updated: $($file.FullName) with date $($originalDate.ToString("MM/dd/yyyy HH:mm:ss"))"
        } else {
            throw "Could not parse date for file: $($file.FullName)"
        }
    } catch {
        Write-Error "$_"
    }
}
