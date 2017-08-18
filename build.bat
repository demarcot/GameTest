@echo off

@pushd .\output
@cl /EHsc /Zi /FC /nologo ..\src\myMain.cpp gdi32.lib user32.lib /link 
@popd 
