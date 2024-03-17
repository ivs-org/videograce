#!/bin/sh

cd en
#mkdocs build
#rm -rf ../../../vg_site/doc/sdk
#mv site ../../../vg_site/doc/sdk

cd ../ru
mkdocs build
rm -rf ../../../../vg_site_ru/doc/sdk
mv site ../../../../vg_site_ru/doc/sdk

