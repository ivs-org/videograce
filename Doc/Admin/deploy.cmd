cd en
rem mkdocs build
rem del /S /Q ..\..\..\..\sites\vg_site\doc\admin\
rem xcopy /E /Y site ..\..\..\..\sites\vg_site\doc\admin\
rem del /S /Q site
rem rmdir /S /Q site

cd ..\ru
mkdocs build
del /S /Q ..\..\..\..\sites\vg_site_ru\doc\admin\
xcopy /E /Y site ..\..\..\..\sites\vg_site_ru\doc\admin\
del /S /Q site
rmdir /S /Q site

pause
