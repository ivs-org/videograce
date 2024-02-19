#!/bin/sh

cd en
#mkdocs build
#rm -rf ../../../vg_site/doc/client
#mv site ../../../vg_site/doc/client

cd ../ru
mkdocs build
rm -rf ../../../../vg_site_ru/doc/client
mv site ../../../../vg_site_ru/doc/client
