#!/bin/bash
#
# References
#   https://docs.microsoft.com/en-us/azure/devops/pipelines/ecosystems/android?view=azure-devops
#
echo ${ANDROID_HOME} 1>&2

# echo "y" | $ANDROID_HOME/tools/bin/sdkmanager --install 'cmake;3.18.1'
echo "y" | $ANDROID_HOME/tools/bin/sdkmanager --install 'ndk;23.1.7779620'

echo "y" | $ANDROID_HOME/tools/bin/sdkmanager --install 'system-images;android-28;google_apis;x86_64'
echo "no" | $ANDROID_HOME/tools/bin/avdmanager create avd -n xamarin_android_emulator -k 'system-images;android-28;google_apis;x86_64' --force
$ANDROID_HOME/emulator/emulator -list-avds 1>&2

nohup $ANDROID_HOME/emulator/emulator -avd xamarin_android_emulator -no-snapshot > /dev/null 2>&1 &
$ANDROID_HOME/platform-tools/adb wait-for-device shell 'while [[ -z $(getprop sys.boot_completed | tr -d '\r') ]]; do sleep 1; done; input keyevent 82' 1>&2
$ANDROID_HOME/platform-tools/adb devices 1>&2
