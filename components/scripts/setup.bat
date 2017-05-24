:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: This script should automatically call any child scripts to setup all assets needed for the tools  ::
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

@echo off

set GAME_DIR=..\..
set ASSET_UTIL=%GAME_DIR%\bin\asset_util.exe

"%ASSET_UTIL%" extract-iwd --all
"%ASSET_UTIL%" extract-ff --all

ents.bat
