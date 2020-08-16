#!/bin/bash
yes | sdkmanager ${1:-"ndk;21.3.6528147"} --channel=3;
