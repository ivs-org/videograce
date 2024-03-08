
import os
import gdown
import json
import tarfile

boost = "1Gmtj8Q0Wwz66aNxieyrN0ESoFK_eX5i4"

db = "1rnQHwFwclj37z-dbnhXbwtFL4VhciDzr"

ipp = "1pERT7jJj19xm7n2EkHXVHD3ocRvQXP3a"

lame = "1uPlI10YGXg_OwEVFp6s2Yh9DL8upWDKB"

opus = "1UePbTVzcbh3nBU4qJfJVsZBMgxwXn5QU"

vpx = "1itxXUHPcsP0pYJ353k1NuvmXqglO61P"

web_m = "1e1FAoztxQ2r7Y5bl8AYSsEY59R_8XhGH"

open_ssl = "1-BB5E9sW7MYf-kV5XIwr7rES6YEeVWGh"

wui = "1wkuiUt2ur91tHmTutmKOShQ7IhSwtpEE"

libs = { "boost":    boost,
         "db":       db,
         "ipp":      ipp,
         "lame":     lame,
         "opus":     opus,
         "vpx":      vpx,
         "web_m":    web_m,
         "open_ssl": open_ssl,
         "wui":      wui }

def install_item(name, id):
    print("Downloading: " + name)

    file_name = name + ".tar.bz2"
    gdown.download(id=id, output=file_name, quiet=False)

    tar = tarfile.open(file_name, "r:bz2")  
    tar.extractall()
    tar.close()

    os.remove(file_name)

    print("Installation " + name + " completed")

history_file_name = 'history.json'

prev_libs = {}
if os.path.exists(history_file_name):
    with open(history_file_name) as json_file:
        prev_libs = json.load(json_file)

for name, id in libs.items():
    if (name not in prev_libs or prev_libs[name] != libs[name]):
        install_item(name, id)

with open(history_file_name, 'w') as history_file: 
    history_file.write(json.dumps(libs))
