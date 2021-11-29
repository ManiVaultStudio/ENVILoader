# CI-CD

Linux & Macos (Travis) | Windows (Appveyor)
--- | ---
[![Build Status](https://travis-ci.com/bldrvnlw/conan-ENVILoaderPlugin.svg?branch=master)](https://travis-ci.com/bldrvnlw/conan-ENVILoaderPlugin) | [![Build status](https://ci.appveyor.com/api/projects/status/l5d1vamvwo0aa3jq?svg=true)](https://ci.appveyor.com/project/bldrvnlw/conan-ENVIloaderplugin)


Pushing to the hdps/ENVILoaderPlugin develop wil trigger the conan CI_CD to start a build via the configured Webhook.

Currently the following build matrix is performed

OS | Architecture | Compiler
--- | --- | ---
Windows | x64 | MSVC 2017
Linux | x86_64 | gcc 9
Macos | x86_64 | clang 10

# ENVILoaderPlugin
Loader plugin for high dimensional images in ENVI format the HDPS framework
