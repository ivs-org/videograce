
import os
import gdown
import json
import zipfile

boost = "1YBgZBEHw9kpGD4oU1Kf4mYxplnpMsGBP"

db = "1cHhNEk0_BfDmp_MAUO5ubStPBYkzJnPE"

filters = "1fvni13AN9E_pxqEuuRT4tc2PIIgztw7P"

ipp = "1tMCgWtaF8iu_wqjXhyOd0wn4CZ_pFsmQ"

lame = "1_69Z-394jv2M1J--TRyD7Botuyxvpv8q"

opus = "1WON36LqnKiQqNHAZCTeL3qOmGaWHIQMh"

vpx = "1uWWiu6_7cBAtdXog06AIT-cggzsF69UQ"

web_m = "1Roe3DWRoR5jg1GqTsvpFSU_t3KsUabH9"

open_ssl = "1U5Pm4iiJumWcDC03bLd6NY_ifdhCYB2m"

wui = "1M0XXtbu_pZYppD9FhyBV3hQWoizjQvBE"

libs = { "boost":    boost,
         "db":       db,
         "filters":  filters,
         "ipp":      ipp,
         "lame":     lame,
         "opus":     opus,
         "vpx":      vpx,
         "web_m":    web_m,
         "open_ssl": open_ssl,
         "wui":      wui }

def install_item(name, id):
    print("Downloading: " + name)

    file_name = name + ".zip"
    gdown.download(id=id, output=file_name, quiet=False)

    with zipfile.ZipFile(file_name, 'r') as zip_ref:
        zip_ref.extractall(".")

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
