=======================
 PowerHint test script
=======================

This script is used to test the PowerHint function in UNISOC Android devices.

Requirements
============

Python 3.5 is the minimum Python version required.

Test Conditions
===============

1. UNISOC Android devices
2. Android version is userdebug or eng

Testing
=======

The test is mainly implemented through the adb shell command,
and will test all android devices connected to the computer.

You can run tests like this::

    $ ./powerhint_test.py

The test report will be generated in the current folder.
The name format of the test report folder like this::
    PowerHint-test-"device_id"_"device_product"_"date"
The test report folder contains the test report file report.txt
and related logs during the test.
