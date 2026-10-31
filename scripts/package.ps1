$ErrorActionPreference='Stop'
$exe=Get-ChildItem build -Filter NovaFetch.exe -Recurse | Select-Object -First 1
if(!$exe){throw 'Build first'}
New-Item -ItemType Directory -Force dist/NovaFetch | Out-Null
Copy-Item $exe.FullName dist/NovaFetch/
Copy-Item README.md dist/NovaFetch/
Compress-Archive dist/NovaFetch/* NovaFetch-Portable-Windows-x64.zip -Force
