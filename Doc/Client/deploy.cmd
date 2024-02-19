cd en
rem mkdocs build
rem del /S /Q ..\..\..\..\sites\vg_site\doc\client
rem xcopy /E /Y site ..\..\..\..\sites\vg_site\doc\client
rem del /S /Q site
rem rmdir /S /Q site

cd ..\ru
mkdocs build
del /S /Q ..\..\..\..\sites\vg_site_ru\doc\client\
xcopy /E /Y site ..\..\..\..\sites\vg_site_ru\doc\client\
del /S /Q site
rmdir /S /Q site

pause
