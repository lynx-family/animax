# Building AnimaXExample for Android

This document provides instructions for building the AnimaX Example Android app from source. If you just want to try out AnimaX, you can download the pre-built APK from the releases page instead.

## System Requirements

- 100GB or more of disk space
- Git/Python3(>=3.9) installed

## Install Dependencies

The following dependencies are needed:

- JDK 11
- Android development environment
- Python library

### JDK

#### Install JDK11

- MacOS

    We recommend using Homebrew to install the OpenJDK distribution called Zulu, which is provided by Azul.

    ```bash
    brew install --cask zulu@11
    ```

    You can use the following command to confirm whether the installation is successful.

    ```bash
    javac --version
    ```

    If the installation is successful, the terminal will output javac version number 11.

- Linux

    On Ubuntu or Debian:

    ```bash
    sudo apt install openjdk-11-jdk 
    ```

    On RHEL or CentOS:

    ```bash
    sudo yum install openjdk-11-jdk 
    ```

- Windows

    We recommend using winget to install the OpenJDK distribution.

    ```powershell
    winget install -e --id ojdkbuild.openjdk.11.jdk
    ```

    You can use the following command to confirm whether the installation is successful.

    ```powershell
    javac --version
    ```

    If the installation is successful, the terminal will output javac version number 11.x.xx (minor version might vary).

#### Update JAVA_HOME

Confirm your JDK installation directory. If you follow the above steps, the JDK path is likely to be 

- `/Library/Java/JavaVirtualMachines/zulu-11.jdk/Contents/Home` on MacOS
- `/usr/lib/jvm/java-11-openjdk-amd64` on Linux
- `C:\Program Files\ojdkbuild\java-11-openjdk-11.0.15-1` on Windows.

Add the following statement to your environment configuration file on Linux or MacOS (it may be ~/.zshrc or ~/.bash_profile or ~/.bashrc, depending on your terminal environment):

- MacOS

    ```bash
    export JAVA_HOME=/Library/Java/JavaVirtualMachines/zulu-11.jdk/Contents/Home
    export PATH=$JAVA_HOME/bin:$PATH
    ```

- Linux 

    ```bash
    export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64
    export PATH=$JAVA_HOME/bin:$PATH
    ```

- Windows

    ```powershell
    # the minor version of openjdk might vary
    $JDK_PATH="$env:ProgramFiles\ojdkbuild\java-11-openjdk-11.x.xx"
    [Environment]::SetEnvironmentVariable('JAVA_HOME', $JDK_PATH, 'User')
    $EXISTING_PATH = [Environment]::GetEnvironmentVariable('PATH', 'User')
    [Environment]::SetEnvironmentVariable('PATH', "$JDK_PATH;$EXISTING_PATH", 'User')
    ```

    Instead of using PowerShell to execute the commands above, you can also add `%ProgramFiles%\ojdkbuild\java-11-openjdk-11.x.xx` to user scope's environment variables Path and JAVA_HOME through "Editing System Environment Variables".

### Android Development Environment

Configuring the Android development environment required by AnimaXExample includes the following step:

#### Configure ANDROID_HOME

Add the ANDROID_HOME variable to your environment configuration file on Linux or MacOS (maybe ~/.zshrc or ~/.bash_profile or ~/.bashrc, depending on your terminal environment).

If you have installed the Android SDK before, please set ANDROID_HOME to the installation directory of the Android SDK.(If you have previously installed Android SDK by Android Studio, the installation path of the Android SDK is usually located at $HOME/Library/Android/sdk on MacOS and Linux or %USERPROFILE%\AppData\Local\Android\Sdk on Windows)

- MacOS and Linux

    ```bash
    export ANDROID_HOME=<path-to-android-sdk>
    ```

- Windows

    ```powershell
    [Environment]::SetEnvironmentVariable('ANDROID_HOME', $path-to-android-sdk, 'User')
    ```

    Instead of using PowerShell to execute the commands above, you can also add Android SDK's path to user scope's environment variables ANDROID_HOME through "Editing System Environment Variables".

### Python Library

We recommend using pyenv to manage python environment.
To install pyenv: 

- [`https://github.com/pyenv/pyenv`](https://github.com/pyenv/pyenv) on MacOS and Linux.
- [`https://github.com/pyenv-win/pyenv-win`](https://github.com/pyenv-win/pyenv-win) on Windows.

Install python with version higher or equal to 3.9 using pyenv:  

```
pyenv install 3.9 # or higher
pyenv global 3.9 # or higher
```

## Get the Code

### Pull the Repository

Pull the code from the Github repository.

```
git clone https://github.com/lynx-family/animax.git
```

### Get the Dependent Files

After getting the project repository, execute the following commands in the root directory of the project to get the project dependent files.

- MacOS and Linux

    ```bash
    cd animax
    source tools/envsetup.sh
    tools/hab sync .
    ```

- Windows

    ```powershell
    cd animax
    tools\envsetup.ps1
    tools\hab.ps1 sync .
    ```

    > notice: tools/envsetup.ps1 will add ninja to user scope's Path environment variable for building.

### Install the Android Components

## Build and Run

You can compile AnimaXExample through the command line terminal or Android Studio. The following two methods are introduced respectively.

### Method 1: Build and Run using Android Studio

#### Open the Project

1. Use Android Studio to open the `/example/android` directory of the project.

2. Make sure that the JDK used by your Android Studio points to the JDK 11 installed in the above steps: 

    1. Open Settings > Build,Execution,Deployment > Build Tools > Gradle, modify the Default Gradle JDK.
    2. Fill in the path of your JAVA_HOME. If you follow the JDK configuration steps above, it is likely to be
        - `/Library/Java/JavaVirtualMachines/zulu-11.jdk/Contents/Home` on MacOS.
        - `/usr/lib/jvm/java-11-openjdk-amd64` on Linux.
        - `C:\Program Files\ojdkbuild\java-11-openjdk-11.0.15-1` on Windows.

3. Trigger Gradle sync.

#### Build and Run

Select the `AnimaXExample` module and click the `Run` button to experience AnimaXExample on your device. If using avd, please wait until the build is done.

### Method 2: Build and Run using the Command Line

#### Build

Enter the `example/android` directory from the project root directory and execute the following command.

```
cd example/android
./gradlew :AnimaXExample:assembleDebug --no-daemon
```

This command will generate AnimaXExample-debug.apk in the `example/android/AnimaXExample/build/outputs/apk/debug/` folder.

> notice: If you have just setup the development environment in the same terminal session, some environment variables might not take effect. Please restart a terminal session (or VS Code, Android Studio) then retry.

#### Install

You can install the above .apk file on your device using the adb command.

```
adb install example/android/AnimaXExample/build/outputs/apk/debug/AnimaXExample-debug.apk
```

If the adb command is not found, you can add the path to the adb command in the environment configuration file on Linux or MacOS (~/.zshrc or ~/.bash_profile or ~/.bashrc):

- MacOS and Linux

    ```
    export PATH=${PATH}:${ANDROID_HOME}/platform-tools
    ```

- Windows

    After executing the following command, restart PowerShell to use the adb command.

    ```
    $EXISTING_PATH = [Environment]::GetEnvironmentVariable('PATH', 'User')
    $ANDROID_HOME= [Environment]::GetEnvironmentVariable('ANDROID_HOME', 'User')
    [Environment]::SetEnvironmentVariable('PATH', "$ANDROID_HOME\platform-tools;$EXISTING_PATH ", 'User')
    ```

    Instead of using PowerShell to execute the commands above, you can also add $ANDROID_HOME/platform-tools to user scope's environment variables Path through "Editing System Environment Variables".
