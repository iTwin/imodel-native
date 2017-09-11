# Use the current date as the filename
$dateFormated = Get-Date -UFormat "%Y-%m-%d"

# Output folder
$Outputfolder = 'E:\CoverageReport\RealityModFramework\SDK\'
$OutputFolderRawFiles = Join-Path $Outputfolder 'Raw Files'
$OutputfolderHtmlReport = Join-Path $Outputfolder "HtmlReports\$dateFormated"
$OutputfolderHistory = Join-Path $Outputfolder 'History'

# Executables path
$coverageExe = 'C:\DevTools\VisualStudio2015\Team Tools\Dynamic Code Coverage Tools\CodeCoverage.exe'
$vsTestExe = 'C:\DevTools\VisualStudio2015\Common7\IDE\CommonExtensions\Microsoft\TestWindow\vstest.console.exe'
$ReportGeneratorExe = 'C:\DevTools\ReportGenerator\ReportGenerator.exe'

# Coverage files path
$newCoverageName = Join-Path $OutputFolderRawFiles ($dateFormated + '.coverage')
$coverageXmlFilename = Join-Path $OutputFolderRawFiles ($dateFormated + '.coveragexml')

# Run the test
&$vsTestExe $Env:OutRoot\Winx64\Product\RealityModFramework-Gtest\RealityModFrameworkTest.exe /Settings:$Env:SrcRoot\RealityModFramework\Tests\CodeCoverage.runsettings /platform:x64 /UseVsixExtensions:true

# Find the latest .coverage
$latestReport = Get-ChildItem $Env:SrcRoot\TestResults -Filter *.coverage -recurse | Sort-Object -Property LastWriteTime -Descending | Select-Object -First 1

# Copy it to the raw files folder
Copy-Item $latestReport.FullName $newCoverageName

# Convert the .coverage to .xmlcoverage
&$coverageExe Analyze /output:$coverageXmlFilename $newCoverageName

# Take the .xmlcoverage and generate the html report
&$ReportGeneratorExe -reports:$coverageXmlFilename -targetdir:$OutputfolderHtmlReport -historydir:$OutputfolderHistory