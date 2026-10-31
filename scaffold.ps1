$dirs=@('app','src/core','src/network','src/storage','src/system','src/ui','include/novafetch','resources/assets','tests','scripts','.github/workflows','portable')
foreach($d in $dirs){New-Item -ItemType Directory -Force $d | Out-Null}
Write-Host 'NovaFetch directories ready.'
