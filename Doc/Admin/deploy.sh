#!/bin/sh

cd en
#mkdocs build
#rm -rf ../../../vg_site/doc/admin
#mv site ../../../vg_site/doc/admin

cd ../ru
mkdocs build
rm -rf ../../../../vg_site_ru/doc/admin
mv site ../../../../vg_site_ru/doc/admin

