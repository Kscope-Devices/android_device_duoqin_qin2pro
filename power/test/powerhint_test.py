#!/usr/bin/python
# -*- coding: utf-8 -*-

import os
import re
import time
import sys
from android_device import AndroidDevice

def debug_print(print_str):
    debug_flag = 0
    if debug_flag:
        print print_str

def detect_device():
    detect_cnt = 0
    detect_flag = 1
    while(detect_flag):
        if detect_cnt > 60:
            print "No available devices are detected to exit!!!"
            sys.exit()
        with os.popen('adb devices -l') as p:
            device_str = p.read()
        device_list = device_str.strip('\n').split('\n')
        for device_line in device_list:
            searchObj = re.search(r'(.*) device usb:(.*) product:(.*) model:(.*) device:(.*)', device_line)
            if searchObj:
                detect_flag = 0
                break
        detect_cnt += 1
        time.sleep(1)

def set_prop(device_id, prop, val):
    tmp_cmd = 'adb -s ' + device_id + ' root'
    debug_print(tmp_cmd)
    os.system(tmp_cmd)
    time.sleep(5)
    tmp_cmd = 'adb -s ' + device_id + ' shell setprop ' + prop + ' ' + str(val)
    debug_print(tmp_cmd)
    os.system(tmp_cmd)

def reboot_device(device_id):
    tmp_cmd = 'adb -s ' + device_id + ' reboot'
    debug_print(tmp_cmd)
    os.system(tmp_cmd)
    tmp_cmd = 'adb -s ' + device_id + ' wait-for-device'
    debug_print(tmp_cmd)
    os.system(tmp_cmd)
    time.sleep(20)

def resource_cfg_check(device, test_fd):
    for key in device.resource_def_cfg_dict:
        if "subsys" not in key:
            tmp_cmd = 'adb -s ' + device.id + ' shell cat ' + key
            debug_print(tmp_cmd)
            with os.popen(tmp_cmd) as p:
                tmp_val = p.read()
            # check resource default value config
            tmp_val = tmp_val.strip('\x00').strip()
            if device.resource_def_cfg_dict[key][0] != "FF":
                if device.resource_def_cfg_dict[key][0].lower() != tmp_val.lower():
                    os.write(test_fd, "Resource default value " + key + " setting fail!!!\n")
                    os.write(test_fd, "Path: " + key + "\n")
                    os.write(test_fd, "Setting value: " + device.resource_def_cfg_dict[key][0] + "\n")
                    os.write(test_fd, "Current value: " + tmp_val + "\n")

            # save resource default value
            device.resource_def_cfg_dict[key][0] = tmp_val
            debug_print("resource_def_cfg_dict " + key + " modify default value: " + tmp_val)

def scene_test(device, test_fd):
    for key in device.scene_cfg_dict:
        enable_scene_cmd = 'adb -s ' + device.id + ' shell service call power 5 i32 ' + str(int(device.scene_cfg_dict[key][0], 16)) + ' i32 1'
        disable_scene_cmd = 'adb -s ' + device.id + ' shell service call power 5 i32 ' + str(int(device.scene_cfg_dict[key][0], 16)) + ' i32 0'
        print enable_scene_cmd
        os.system(enable_scene_cmd)
        time.sleep(2)
        os.write(test_fd, "Scene " + key + " test start!!!\n")
        cfg_q = device.scene_cfg_dict[key][2]
        os.write(test_fd, "Enter scene " + key + "!!!\n")
        debug_print("config quantity: " + cfg_q)
        # check scene settings
        i = 0
        while (i < int(cfg_q)):
            file_path = device.scene_cfg_dict[key][4 + i * 2]
            set_val = device.scene_cfg_dict[key][5 + i * 2]
            tmp_cmd = 'adb -s ' + device.id + ' shell cat ' + file_path
            debug_print(tmp_cmd)
            with os.popen(tmp_cmd) as p:
                tmp_val = p.read()
            tmp_val = tmp_val.strip('\x00').strip()
            if set_val.lower() != tmp_val.lower():
                os.write(test_fd, "Path: " + file_path + " fail!!!\n")
                os.write(test_fd, "Setting value: " + set_val + "\n")
                os.write(test_fd, "Current value: " + tmp_val + "\n")
                # modify test results to failure
                device.scene_cfg_dict[key][3] = "Failure"
            i += 1
        print disable_scene_cmd
        os.system(disable_scene_cmd)
        time.sleep(2)
        os.write(test_fd, "Exit scene " + key + "!!!\n")
        # check scene restore
        i = 0
        while (i < int(cfg_q)):
            resource_key = device.scene_cfg_dict[key][4 + i * 2]
            if resource_key in device.resource_def_cfg_dict.keys():
                def_val = device.resource_def_cfg_dict[resource_key][0]
                tmp_cmd = 'adb -s ' + device.id + ' shell cat ' + resource_key
                debug_print(tmp_cmd)
                with os.popen(tmp_cmd) as p:
                    tmp_val = p.read()
                tmp_val = tmp_val.strip('\x00').strip()
                if def_val.lower() != tmp_val.lower():
                    os.write(test_fd, "Path: " + resource_key + " fail!!!\n")
                    os.write(test_fd, "Default value: " + def_val + "\n")
                    os.write(test_fd, "Current value: " + tmp_val + "\n")
                    # modify test results to failure
                    device.scene_cfg_dict[key][3] = "Failure"
            i += 1
        os.write(test_fd, "Scene " + key + " test end!!!\n\n")

def main():
    # Detecting available devices
    detect_device()
    # get device information
    with os.popen('adb devices -l') as p:
        device_str = p.read()
    device_list = device_str.strip('\n').split('\n')
    debug_print(device_list)
    for device_line in device_list:
        debug_print(device_line)
        searchObj = re.search(r'(.*) device usb:(.*) product:(.*) model:(.*) device:(.*)', device_line)
        if searchObj:
            device_id = searchObj.group(1)
            device_id = device_id.strip('\x00').strip()
            device_usb = searchObj.group(2)
            device_product = searchObj.group(3)
            device_modle = searchObj.group(4)
            device_name = searchObj.group(5)
            # Create test report file
            test_report_path = "PowerHint-test-" + device_id + "_" + device_product + "_" + time.strftime("%Y-%m-%d-%H-%M-%S", time.localtime())
            os.mkdir(test_report_path)
            test_fd_name = test_report_path + "/report.txt"
            test_fd = os.open(test_fd_name, os.O_RDWR|os.O_CREAT)

            os.write(test_fd, "Test device information device_id: " + device_id + ", device_product: " + device_product + ", device_modle: " + device_modle + ", device_name: " + device_name + "\n\n")
            # Create Android Device Object
            device = AndroidDevice(device_id, device_name, "power_scene_id_define.txt", "power_scene_config.xml", "power_resource_file_info.xml", "/vendor/etc")
            # Enable ylog
            set_prop(device.id, "persist.ylog.enabled", 1)
            tmp_cmd = 'adb -s ' + device_id + ' shell rm -rf /storage/emulated/0/ylog/*'
            debug_print(tmp_cmd)
            os.system(tmp_cmd)
            reboot_device(device.id)
            # get PowerHint config file from device
            device.get_config_file()
            # Parsing the configuration file to create a data dictionary
            device.parse_config_file()

            # PowerHint resource file config check
            resource_cfg_check(device, test_fd)

            # PowerHint scene test
            scene_test(device, test_fd)

            # get log
            tmp_cmd = 'adb -s ' + device_id + ' pull /storage/emulated/0/ylog/ ' + test_report_path
            debug_print(tmp_cmd)
            os.system(tmp_cmd)

            # print test results
            os.write(test_fd, "Test results:\n")
            for key in device.scene_cfg_dict:
                os.write(test_fd, key + ": " + device.scene_cfg_dict[key][3] + "\n")

            os.fsync(test_fd)
            os.close(test_fd)

            # delete config file
            device.clear_config_file()

            # Destroy Android Device Object
            del device

if __name__ == '__main__':
    main()
