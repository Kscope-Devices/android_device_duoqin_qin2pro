#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import re
import time
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

class AndroidDevice(object):
    def __init__(self, device_id, device_name, scene_id_cfg_file, scene_cfg_file, resource_cfg_file, cfg_file_path):
        self.id = device_id
        self.name = device_name
        self.scene_id_cfg_file = scene_id_cfg_file
        self.scene_cfg_file = scene_cfg_file
        self.resource_cfg_file = resource_cfg_file
        self.cfg_file_path = cfg_file_path
        # scene_cfg_dict data structure:
        # dict{scene_name: [scene_id, scene_name, config_quantity, test_results
        #       config_file_path, config_value, ..]}
        # Example:
        # dict{"interaction_launch": ["0x7f000102", "interaction_launch", "5", "Success"
        #       "/dev/cluster0_freq_max", "1BC560",
        #       "/dev/cluster0_freq_min", "1BC560",
        #       "/dev/cluster1_freq_max", "1EF1E0",
        #       "/dev/cluster1_freq_min", "1EF1E0",
        #       "/sys/class/devfreq/scene-frequency/sprd_governor/scene_boost_dfs", "max"]}
        self.scene_cfg_dict = {}
        # resource_def_cfg_dict data structure:
        # dict{file_name: [default_value, ..]}
        # Example:
        # dict{"/sys/devices/system/cpu/cpuhotplug/cluster1_core_max_limit": ["4"]}
        self.resource_def_cfg_dict = {}

    def debug_print(self, print_str):
        debug_flag = 0
        if debug_flag:
            print print_str

    def get_config_file(self):
        tmp_cmd = 'adb -s ' + self.id + ' root'
        self.debug_print(tmp_cmd)
        os.system(tmp_cmd)
        time.sleep(5)
        tmp_cmd = 'adb -s ' + self.id + ' pull ' + self.cfg_file_path + '/' + self.scene_id_cfg_file + ' .'
        self.debug_print(tmp_cmd)
        os.system(tmp_cmd)
        tmp_cmd = 'adb -s ' + self.id + ' pull ' + self.cfg_file_path+'/' + self.resource_cfg_file + ' .'
        self.debug_print(tmp_cmd)
        os.system(tmp_cmd)
        tmp_cmd = 'adb -s ' + self.id + ' pull ' + self.cfg_file_path + '/' + self.scene_cfg_file + ' .'
        self.debug_print(tmp_cmd)
        os.system(tmp_cmd)

    def clear_config_file(self):
        os.system('rm -f ' + self.scene_id_cfg_file)
        os.system('rm -f ' + self.resource_cfg_file)
        os.system('rm -f ' + self.scene_cfg_file)

    def __parse_subsys_in_resource_config_file(self, root, scene_name, subsys_name, subsys_val):
        for subsys in root.iter('subsys'):
            if subsys.attrib['name'] == subsys_name:
                for conf in subsys.iter('conf'):
                    if conf.attrib['name'] == subsys_val:
                        cfg_count = 0
                        for conf_set in conf:
                            conf_set_path = conf_set.attrib['path']
                            conf_set_file = conf_set.attrib['file']
                            conf_set_val = conf_set.attrib['value']
                            tmp_str = conf_set_path + "/" + conf_set_file
                            self.scene_cfg_dict[scene_name].append(tmp_str)
                            self.debug_print("scene_cfg_dict " + scene_name + " add config file: " + tmp_str)
                            self.scene_cfg_dict[scene_name].append(conf_set_val)
                            self.debug_print("scene_cfg_dict " + scene_name + " add config value: " + conf_set_val)
                            cfg_count += 1
        return cfg_count

    def parse_config_file(self):
        tree = ET.parse(self.scene_cfg_file)
        scene_cfg_root = tree.getroot()
        tree = ET.parse(self.resource_cfg_file)
        resource_cfg_root = tree.getroot()

        # parse scene config file
        for mode in scene_cfg_root:
            if mode.tag == "mode" and mode.attrib['name'] == "normal":
                for scene in mode:
                    scene_name = scene.attrib['name']
                    self.scene_cfg_dict.setdefault(scene_name, [])
                    self.debug_print("create scene_cfg_dict key: " + scene_name)
                    self.scene_cfg_dict[scene_name].append("0")
                    self.debug_print("scene_cfg_dict " + scene_name + " add scene_id: 0")
                    self.scene_cfg_dict[scene_name].append(scene_name)
                    self.debug_print("scene_cfg_dict " + scene_name + " add scene_name: " + scene_name)
                    self.scene_cfg_dict[scene_name].append("0")
                    self.debug_print("scene_cfg_dict " + scene_name + " add config_quantity: 0")
                    self.scene_cfg_dict[scene_name].append("Success")
                    self.debug_print("scene_cfg_dict " + scene_name + " add default test results: Success")
                    cfg_quantity = 0
                    cfg_count = 0
                    for scene_set in scene:
                        scene_set_path = scene_set.attrib['path']
                        scene_set_file = scene_set.attrib['file']
                        scene_set_val = scene_set.attrib['value']
                        if scene_set_path == "subsys":
                            cfg_quantity += self.__parse_subsys_in_resource_config_file(resource_cfg_root, scene_name, scene_set_file, scene_set_val)
                        else:
                            if scene_set_file == "scene_boost_dfs":
                                scene_set_file = "ddrinfo_cur_freq"
                                tmp_cmd = "adb -s " + self.id + " shell cat " + scene_set_path + "/ddrinfo_freq_table"
                                with os.popen(tmp_cmd) as p:
                                    tmp_val = p.read()
                                tmp_val = tmp_val.replace('\n', '')
                                tmp_val = tmp_val.replace('\r', '')
                                self.debug_print(tmp_val)
                                tmp_list = tmp_val.strip(' ').split(' ')
                                self.debug_print(tmp_list)
                                if scene_set_val == "max":
                                    scene_set_val = tmp_list[-1]
                            tmp_str = scene_set_path + "/" + scene_set_file
                            self.scene_cfg_dict[scene_name].append(tmp_str)
                            self.debug_print("scene_cfg_dict " + scene_name + " add config file: " + tmp_str)
                            self.scene_cfg_dict[scene_name].append(scene_set_val)
                            self.debug_print("scene_cfg_dict " + scene_name + " add config value: " + scene_set_val)
                            cfg_quantity += 1
                    else:
                        # update the configuration quantity in the dictionary
                        if cfg_quantity != 0:
                            self.scene_cfg_dict[scene_name][2] = str(cfg_quantity)
                            self.debug_print("scene_cfg_dict " + scene_name + " add config_quantity: " + str(cfg_quantity))

        # parse scene ID config file
        scene_id_cfg_file_fd = open(self.scene_id_cfg_file, 'r')
        try:
            scene_id_lines = scene_id_cfg_file_fd.readlines()
            for line in scene_id_lines:
                # scene id match: 0x00000001 0x00000000 vsync
                searchObj = re.search(r'(0x.{8}) (0x.{8}) (.*)', line)
                if searchObj:
                    scene_id = searchObj.group(1)
                    scene_name = searchObj.group(3)
                    if scene_name in self.scene_cfg_dict.keys():
                        self.scene_cfg_dict[scene_name][0] = scene_id
                        self.debug_print("scene_cfg_dict " + scene_name + " add scene_id: " + scene_id)
        finally:
            scene_id_cfg_file_fd.close()

        # parse resource config file default value
        for file_node in resource_cfg_root.iter('file'):
            if file_node.attrib['path'] == "subsys":
                continue
            key = file_node.attrib['path'] + "/" + file_node.attrib['file']
            if 'no_has_def' in file_node.attrib.keys():
                if file_node.attrib['no_has_def'] == "1":
                    continue
            else:
                for attr in file_node:
                    if attr.attrib['name'] == "def_value":
                        file_node_def_val = attr.attrib['value']
                        break
                else:
                    file_node_def_val = "FF"
            self.resource_def_cfg_dict.setdefault(key, [])
            self.debug_print("create resource_def_cfg_dict key: " + key)
            self.resource_def_cfg_dict[key].append(file_node_def_val)
            self.debug_print("resource_def_cfg_dict " + key + " add default value: " + file_node_def_val)
        for subsys_node in resource_cfg_root.iter('subsys'):
            for inode in subsys_node.iter('inode'):
                key = inode.attrib['path'] + "/" + inode.attrib['file']
                inode_def_val = "FF"
                self.resource_def_cfg_dict.setdefault(key, [])
                self.debug_print("create resource_def_cfg_dict key: " + key)
                self.resource_def_cfg_dict[key].append(inode_def_val)
                self.debug_print("resource_def_cfg_dict " + key + " add default value: " + inode_def_val)
