#!/bin/sh

cd en
#mkdocs build
#rm -rf ../../../vg_site/doc/admin
#mv site ../../../vg_site/doc/admin
#mkdir -p ../../../vg_site/doc/admin/img
#cp img/* ../../../vg_site/doc/admin/img

cd ../ru
mkdocs build
rm -rf ../../../../vg_site_ru/doc/admin
mv site ../../../../vg_site_ru/doc/admin
mkdir -p ../../../../vg_site_ru/doc/admin/img
cp img/* ../../../../vg_site_ru/doc/admin/img
