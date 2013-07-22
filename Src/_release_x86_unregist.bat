@echo off
cd /d "%~dp0"
regsvr32 Release\DSFilters.ax /u
