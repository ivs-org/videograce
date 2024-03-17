cd en
rem mkdocs build
rem del /S /Q ..\..\..\..\sites\vg_site\doc\sdk\
rem xcopy /E /Y site ..\..\..\..\sites\vg_site\doc\sdk\
rem del /S /Q site
rem rmdir /S /Q site

cd ..\ru
mkdocs build
del /S /Q ..\..\..\..\sites\vg_site_ru\doc\sdk\
xcopy /E /Y site ..\..\..\..\sites\vg_site_ru\doc\sdk\
del /S /Q site
rmdir /S /Q site

pause
