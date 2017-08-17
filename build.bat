@echo off

@pushd .\output
@cl /EHsc /Zi /FC /nologo ..\src\myMain.cpp user32.lib /link 
@popd 
