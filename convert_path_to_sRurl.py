import os
import re

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
user_path = "C:\\Users\\chandrakanth\\Documents\\shows\\shots\\MHZ\\QSC\\QSC_1000\\fx\\work\\Fire\\bgeo\\QSC_1000_efx_test_fire_v001_r001.0001.geo"
# user_path = r"C:\Users\chandrakanth\Documents\shows\MHZ\assets\char\mike1\fx\work\csantharam\bgeo\test_v001.geo"
resolved_path = 'sRurl://'
shot_sub_domain = ''

def encode_entities(entities):
    for prj_config in config:
        for key, value in prj_config.items():
            global seqs
            if key == 'seq' or key == 'asset_type':
                seqs = value
            if key == 'shot' or key == 'asset':
                for seq in seqs:
                    try:
                        for shot in value[seq]:
                            if entities == shot:
                                return key
                    except: pass
            elif key == 'seq' or key == 'asset_type':
                continue
            elif entities in value: 
                return key


def solve_sub_domain(prj_path, get_show):
    
    path = os.path.join(prj_path ,get_show)
    seqs =  os.listdir(path)
    get_entities = [seq for seq in seqs if seq in user_path][0]
    # print(get_entities)
    entity_types = encode_entities(get_entities)
    if entity_types:
        global shot_sub_domain
        shot_sub_domain += f'{entity_types}={get_entities}:'
    if not os.path.isfile(os.path.join(path, get_entities)):
        solve_sub_domain(path, get_entities)


def solve_file_name(user_path):
    global shot_sub_domain
    file_name = os.path.basename(user_path)
    if 'work' in user_path:
        work_domain = user_path.split('\\work\\')[-1]
        work_domain = work_domain.split(file_name)[0]
        shot_sub_domain += f'work={work_domain[:-1]}:' 
    elif 'publish' in user_path:
        pub_domain = user_path.split('\\publish\\')[-1]
        pub_domain = pub_domain.split(file_name)[0]
        shot_sub_domain += f'pub={pub_domain}:'
    if file_name:
        version =  re.findall('_v\d{3}', file_name)
        revision = re.findall('_r\d{3}', file_name)
        extract_version_no = version[0].split('_v')[-1]
        base_file_name = file_name.split(version[0])[0] 
        
        if version and not revision:
            ext = file_name.split(version[0])[-1].replace('.','') 
            file_name_url_schema = f"file={base_file_name}?version={extract_version_no}#ext={ext}"
            shot_sub_domain+=file_name_url_schema
        if version and revision:
            extract_revision_no = revision[0].split('_r')[-1]
            ext = file_name.split(revision[0])[-1][1:]
            frame_no = re.findall('\d+', ext)
            if frame_no:
                count = len(frame_no[0])
                exts = ext.split(frame_no[0])[-1]
                ext = f"$F{count}{exts}"
            file_name_url_schema = f"file={base_file_name}?version={extract_version_no}@revision={extract_revision_no}#ext={ext}"
            shot_sub_domain+=file_name_url_schema
        pass
    pass


prj_path = "C:\\Users\\chandrakanth\\Documents\\shows"
shows = os.listdir(prj_path)
get_shows = [show for show in shows if show in user_path]
for get_show in get_shows:
    solve_sub_domain(prj_path, get_show)
    solve_file_name(user_path)
    sURl_path = resolved_path + 'show='+get_show + ':' + shot_sub_domain

    print(sURl_path)

# a = re.findall('_v\d{3}', 'test_v001_r001.geo')
