import os
import sys
entity_config = "C:\\Users\\chandrakanth\\Documents\\houdini19.5\\dso\\entity.txt"
sRurl_file = "C:\\Users\\chandrakanth\\Documents\\houdini19.5\\dso\\sRurl.txt"
config = [
    {
        'asset_type': ['char', 'prop', 'env'],
        'asset':{'char': ['mike1']},
        'seq': ['QSC', 'SSS'],
        'shot': {'QSC': ['QSC_1000', 'QSC_1100']},
        'step': [
                'anim', 'cc', 'cfx', 'comp', \
                    'fx', 'lay', 'lgt', 'mmv', \
                    'plate', 'roto', 'pnt' 
                ]
    }
]

def solve_sRurl(sRurl):

    entity_seq_asset = ''
    if 'shot' in sRurl:
        shot = sRurl.split('shot=')[-1].split(':')[0]
    elif 'asset' in sRurl:
         shot = sRurl.split('asset=')[-1].split(':')[0]
    for prj_config in config:
        if 'shot' in sRurl:
            shot_entity = prj_config['shot']
        elif 'asset' in sRurl:
            shot_entity = prj_config['asset']
        for seqs, shots in shot_entity.items():
            if shot in shots:
                entity_seq_asset = seqs
    with open(entity_config, "w") as config_file:
        config_file.write(entity_seq_asset)

with open(sRurl_file, "r") as surl_file:
    url_file = surl_file.readline()

# solve_sRurl("sRurl://show=MHZ:shot=QSC_1000:step=efx:work=Fire\bgeo:file=QSC_1000_efx_test_fire?version=001@revision=001#ext=$F4.geo")
solve_sRurl(url_file)