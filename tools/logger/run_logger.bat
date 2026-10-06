@echo off
REM Double-click to start the logger. Installs pyserial the first time.
cd /d "%~dp0"
python -m pip install --quiet -r requirements.txt
python vfd_logger.py %*
pause
