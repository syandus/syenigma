pushd "%~dp0"
if NOT %pw%.==. goto gotpw
:readpw
set /p pw=Password:
if %pw%.==. goto readpw
:gotpw
set pw=%pw:"=%
if "%pw%"=="" goto readpw
C:\SVN\Syandus_Company\Source\Verisign\signtool.exe ^
sign ^
/f C:\SVN\Syandus_Company\Source\Verisign\Syandus.pfx ^
/p %pw% ^
/t http://timestamp.comodoca.com/authenticode ^
merck_courses_english_v1.msi
dir merck_courses_english_v1.msi
